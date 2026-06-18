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

#include <mariadb.h>
#include "item_inetmore.h"
#include <mysql/plugin_function.h>
#include <sql_class.h>

class Create_func_ip_class : public Create_func_arg1
{
public:
  Item *create_1_arg(THD *thd, Item *arg1) override
  {
    return new (thd->mem_root) Item_func_ip_class(thd, arg1);
  }
  static Create_func_ip_class s_singleton;

protected:
  Create_func_ip_class() {}
  ~Create_func_ip_class() override {}
};

Create_func_ip_class Create_func_ip_class::s_singleton;

class Create_func_cidr_contains : public Create_func_arg2
{
public:
  Item *create_2_arg(THD *thd, Item *arg1, Item *arg2) override
  {
    return new (thd->mem_root) Item_func_cidr_contains(thd, arg1, arg2);
  }
  static Create_func_cidr_contains s_singleton;

protected:
  Create_func_cidr_contains() {}
  ~Create_func_cidr_contains() override {}
};

Create_func_cidr_contains Create_func_cidr_contains::s_singleton;

#define BUILDER(F) &F::s_singleton

static Plugin_function
    plugin_descriptor_function_ip_class(BUILDER(Create_func_ip_class)),
    plugin_descriptor_function_cidr_contains(
        BUILDER(Create_func_cidr_contains));

/*************************************************************************/

maria_declare_plugin(inet_more){MariaDB_FUNCTION_PLUGIN,
                                &plugin_descriptor_function_ip_class,
                                "ip_class",
                                "lefred",
                                "Function IP_CLASS()",
                                PLUGIN_LICENSE_GPL,
                                0,
                                0,
                                0x0100,
                                NULL,
                                NULL,
                                "1.0",
                                MariaDB_PLUGIN_MATURITY_BETA},
    {MariaDB_FUNCTION_PLUGIN,
     &plugin_descriptor_function_cidr_contains,
     "cidr_contains",
     "lefred",
     "Function CIDR_CONTAINS()",
     PLUGIN_LICENSE_GPL,
     0,
     0,
     0x0100,
     NULL,
     NULL,
     "1.0",
     MariaDB_PLUGIN_MATURITY_BETA} maria_declare_plugin_end;
