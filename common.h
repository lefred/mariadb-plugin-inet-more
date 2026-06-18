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

#ifndef INET_MORE_COMMON_INCLUDED
#define INET_MORE_COMMON_INCLUDED

#include "mariadb.h"
#include <string>

enum Inet_more_family
{
  INET_MORE_NONE= 0,
  INET_MORE_IPV4= 4,
  INET_MORE_IPV6= 6
};

struct Inet_more_addr
{
  Inet_more_family family;
  unsigned char bytes[16];
};

bool inet_more_parse_item(Item *item, Inet_more_addr *addr);
bool inet_more_parse_string(const char *str, size_t length, CHARSET_INFO *cs,
                            Inet_more_addr *addr);
bool inet_more_ip_class(const Inet_more_addr &addr, std::string *out);
bool inet_more_cidr_contains(Item *cidr_item, Item *addr_item, bool *out);

#endif
