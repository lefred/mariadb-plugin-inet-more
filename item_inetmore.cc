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
#include "item_inetmore.h"

#include "common.h"
#include <mysqld_error.h>

bool Item_func_ip_class::fix_length_and_dec(THD *thd)
{
  collation.set(DTCollation_numeric());
  fix_char_length(32);
  set_maybe_null();
  return FALSE;
}

LEX_CSTRING Item_func_ip_class::func_name_cstring() const
{
  static LEX_CSTRING name= {STRING_WITH_LEN("ip_class")};
  return name;
}

String *Item_func_ip_class::val_str(String *str)
{
  Inet_more_addr addr;
  if (!inet_more_parse_item(args[0], &addr))
  {
    if (!args[0]->null_value)
      my_printf_error(ER_UNKNOWN_ERROR, "ip_class: not a valid IP address",
                      MYF(0));
    null_value= true;
    return nullptr;
  }

  std::string out;
  if (!inet_more_ip_class(addr, &out) ||
      str->copy(out.c_str(), static_cast<uint>(out.size()),
                collation.collation))
  {
    null_value= true;
    return nullptr;
  }

  null_value= false;
  return str;
}

bool Item_func_cidr_contains::fix_length_and_dec(THD *thd)
{
  decimals= 0;
  max_length= 1;
  set_maybe_null();
  return FALSE;
}

LEX_CSTRING Item_func_cidr_contains::func_name_cstring() const
{
  static LEX_CSTRING name= {STRING_WITH_LEN("cidr_contains")};
  return name;
}

bool Item_func_cidr_contains::val_bool()
{
  bool out= false;
  if (!inet_more_cidr_contains(args[0], args[1], &out))
  {
    if (!args[0]->null_value && !args[1]->null_value)
      my_printf_error(ER_UNKNOWN_ERROR,
                      "cidr_contains: not a valid CIDR or IP address", MYF(0));
    null_value= true;
    return false;
  }

  null_value= false;
  return out;
}
