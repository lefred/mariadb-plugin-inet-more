# Copyright (c) 2026 lefred (Frederic Descamps)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.

MYSQL_ADD_PLUGIN(inet_more
                 plugin.cc item_inetmore.cc common.cc
                 RECOMPILE_FOR_EMBEDDED
                 MODULE_ONLY COMPONENT Test)
