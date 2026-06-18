/* Copyright (c) 2026 lefred (Frederic Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */

#define MYSQL_SERVER
#include "mariadb.h"
#include "common.h"

#include "item.h"
#include "sql_string.h"
#include <arpa/inet.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>

static bool parse_ipv4(const char *str, Inet_more_addr *addr)
{
  unsigned char parsed[4];
  if (inet_pton(AF_INET, str, parsed) != 1)
    return false;

  addr->family= INET_MORE_IPV4;
  memset(addr->bytes, 0, sizeof(addr->bytes));
  memcpy(addr->bytes, parsed, sizeof(parsed));
  return true;
}

static bool parse_ipv6(const char *str, Inet_more_addr *addr)
{
  unsigned char parsed[16];
  if (inet_pton(AF_INET6, str, parsed) != 1)
    return false;

  addr->family= INET_MORE_IPV6;
  memcpy(addr->bytes, parsed, sizeof(parsed));
  return true;
}

bool inet_more_parse_string(const char *str, size_t length, CHARSET_INFO *cs,
                            Inet_more_addr *addr)
{
  (void) cs;
  std::string text(str, length);
  if (parse_ipv4(text.c_str(), addr))
    return true;

  if (parse_ipv6(text.c_str(), addr))
    return true;

  addr->family= INET_MORE_NONE;
  return false;
}

bool inet_more_parse_item(Item *item, Inet_more_addr *addr)
{
  StringBuffer<128> tmp;
  String *str= item->val_str_ascii(&tmp);
  if (!str)
  {
    addr->family= INET_MORE_NONE;
    return false;
  }
  return inet_more_parse_string(str->ptr(), str->length(), str->charset(),
                                addr);
}

bool inet_more_ip_class(const Inet_more_addr &addr, std::string *out)
{
  if (!out || addr.family == INET_MORE_NONE)
    return false;

  if (addr.family == INET_MORE_IPV4)
  {
    const unsigned char *b= addr.bytes;
    if (b[0] == 0)
      *out= "unspecified";
    else if (b[0] == 10)
      *out= "private";
    else if (b[0] == 127)
      *out= "loopback";
    else if (b[0] == 169 && b[1] == 254)
      *out= "link-local";
    else if (b[0] == 172 && b[1] >= 16 && b[1] <= 31)
      *out= "private";
    else if (b[0] == 192 && b[1] == 168)
      *out= "private";
    else if (b[0] == 100 && b[1] >= 64 && b[1] <= 127)
      *out= "carrier-grade-nat";
    else if (b[0] == 192 && b[1] == 0 && b[2] == 2)
      *out= "documentation";
    else if (b[0] == 198 && b[1] == 51 && b[2] == 100)
      *out= "documentation";
    else if (b[0] == 203 && b[1] == 0 && b[2] == 113)
      *out= "documentation";
    else if (b[0] >= 224 && b[0] <= 239)
      *out= "multicast";
    else if (b[0] >= 240)
      *out= (b[0] == 255 && b[1] == 255 && b[2] == 255 && b[3] == 255)
                ? "broadcast"
                : "reserved";
    else
      *out= "global";
    return true;
  }

  const unsigned char *b= addr.bytes;
  bool all_zero= true;
  for (int i= 0; i < 16; i++)
    all_zero= all_zero && b[i] == 0;

  if (all_zero)
    *out= "unspecified";
  else if (memcmp(b, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1", 16) == 0)
    *out= "loopback";
  else if (b[0] == 0xff)
    *out= "multicast";
  else if (b[0] == 0xfe && (b[1] & 0xc0) == 0x80)
    *out= "link-local";
  else if ((b[0] & 0xfe) == 0xfc)
    *out= "unique-local";
  else if (b[0] == 0x20 && b[1] == 0x01 && b[2] == 0x0d && b[3] == 0xb8)
    *out= "documentation";
  else if (b[0] == 0x20 && b[1] == 0x02)
    *out= "6to4";
  else if (b[0] == 0x20 && b[1] == 0x01 && b[2] == 0x00 && b[3] == 0x00)
    *out= "teredo";
  else
    *out= "global";
  return true;
}

static bool split_cidr(const String &src, std::string *addr, int *prefix)
{
  std::string text(src.ptr(), src.length());
  size_t slash= text.find('/');
  if (slash == std::string::npos)
  {
    *addr= text;
    *prefix= -1;
    return true;
  }

  if (text.find('/', slash + 1) != std::string::npos)
    return false;

  std::string prefix_text= text.substr(slash + 1);
  if (prefix_text.empty())
    return false;
  for (size_t i= 0; i < prefix_text.size(); i++)
    if (!std::isdigit(static_cast<unsigned char>(prefix_text[i])))
      return false;

  char *end= nullptr;
  long parsed= std::strtol(prefix_text.c_str(), &end, 10);
  if (!end || *end != '\0')
    return false;

  *addr= text.substr(0, slash);
  *prefix= static_cast<int>(parsed);
  return true;
}

static bool parse_cidr_item(Item *item, Inet_more_addr *network, int *prefix)
{
  StringBuffer<128> tmp;
  String *str= item->val_str_ascii(&tmp);
  if (!str)
    return false;

  std::string addr_text;
  if (!split_cidr(*str, &addr_text, prefix))
    return false;

  if (!inet_more_parse_string(addr_text.c_str(), addr_text.size(),
                              str->charset(), network))
    return false;

  if (*prefix < 0)
    *prefix= network->family == INET_MORE_IPV4 ? 32 : 128;

  int max_prefix= network->family == INET_MORE_IPV4 ? 32 : 128;
  return *prefix >= 0 && *prefix <= max_prefix;
}

static bool prefix_match(const unsigned char *network,
                         const unsigned char *addr, int prefix)
{
  int full_bytes= prefix / 8;
  int extra_bits= prefix % 8;

  if (full_bytes && memcmp(network, addr, full_bytes) != 0)
    return false;

  if (!extra_bits)
    return true;

  unsigned char mask= static_cast<unsigned char>(0xff << (8 - extra_bits));
  return (network[full_bytes] & mask) == (addr[full_bytes] & mask);
}

bool inet_more_cidr_contains(Item *cidr_item, Item *addr_item, bool *out)
{
  Inet_more_addr network;
  int prefix= 0;
  if (!parse_cidr_item(cidr_item, &network, &prefix))
    return false;

  Inet_more_addr addr;
  if (!inet_more_parse_item(addr_item, &addr))
    return false;

  if (network.family != addr.family)
  {
    *out= false;
    return true;
  }

  *out= prefix_match(network.bytes, addr.bytes, prefix);
  return true;
}
