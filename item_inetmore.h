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

#ifndef ITEM_INETMORE_INCLUDED
#define ITEM_INETMORE_INCLUDED

#include "item.h"

class Item_func_ip_class : public Item_str_func
{
public:
  Item_func_ip_class(THD *thd, Item *arg1) : Item_str_func(thd, arg1) {}
  String *val_str(String *str) override;
  bool fix_length_and_dec(THD *thd) override;
  LEX_CSTRING func_name_cstring() const override;
  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_ip_class>(thd, this);
  }
};

class Item_func_cidr_contains : public Item_bool_func
{
public:
  Item_func_cidr_contains(THD *thd, Item *arg1, Item *arg2)
      : Item_bool_func(thd, arg1, arg2)
  {
  }
  bool val_bool() override;
  bool fix_length_and_dec(THD *thd) override;
  LEX_CSTRING func_name_cstring() const override;
  Item *shallow_copy(THD *thd) const override
  {
    return get_item_copy<Item_func_cidr_contains>(thd, this);
  }
};

#endif
