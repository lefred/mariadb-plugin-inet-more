# mariadb-plugin-inet-more

![mariadb-plugin-inet-more](logo/inet_more.png)

MariaDB function plugin that adds IP address classification and CIDR membership
helpers. The functions accept MariaDB `INET4` values, MariaDB `INET6` values,
or string values.

The plugin provides two SQL functions:

| Function | Description |
| --- | --- |
| `ip_class(ip)` | Returns an address class such as `private`, `global`, `loopback`, `link-local`, or `multicast` |
| `cidr_contains(cidr, ip)` | Returns whether an address is contained in a CIDR network |

Invalid IP or CIDR text raises an error. `NULL` input returns `NULL`.

## Build

This plugin is intended to be built as part of a MariaDB source tree. Place the
plugin directory under the MariaDB plugin directory and build MariaDB with the
plugin enabled.

The CMake target builds the module as `inet_more`.

## Installation

Install the plugin module in MariaDB:

```sql
INSTALL SONAME 'inet_more';
```

Verify that the two function plugins are loaded:

```sql
SELECT plugin_name, plugin_type, plugin_library, plugin_description,
       plugin_author
FROM information_schema.PLUGINS
WHERE plugin_type = 'FUNCTION'
  AND plugin_library = 'inet_more.so'
ORDER BY plugin_name;
```

Expected functions:

```text
+---------------+-------------+----------------+--------------------------+---------------+
| plugin_name   | plugin_type | plugin_library | plugin_description       | plugin_author |
+---------------+-------------+----------------+--------------------------+---------------+
| cidr_contains | FUNCTION    | inet_more.so   | Function CIDR_CONTAINS() | lefred        |
| ip_class      | FUNCTION    | inet_more.so   | Function IP_CLASS()      | lefred        |
+---------------+-------------+----------------+--------------------------+---------------+
```

Uninstall the plugin module:

```sql
UNINSTALL SONAME 'inet_more';
```

## Examples

### `ip_class()`

Classify IPv4 and IPv6 string values:

```sql
SELECT ip_class('10.1.2.3') AS ipv4_private,
       ip_class('8.8.8.8') AS ipv4_global,
       ip_class('::1') AS ipv6_loopback,
       ip_class('fe80::1') AS ipv6_link_local;
```

Result:

```text
+--------------+-------------+---------------+-----------------+
| ipv4_private | ipv4_global | ipv6_loopback | ipv6_link_local |
+--------------+-------------+---------------+-----------------+
| private      | global      | loopback      | link-local      |
+--------------+-------------+---------------+-----------------+
```

Use native `INET4` and `INET6` values:

```sql
CREATE TABLE t1 (a INET4, b INET6);
INSERT INTO t1 VALUES ('192.168.10.20', '2001:db8::abcd');

SELECT ip_class(a) AS inet4_class,
       ip_class(b) AS inet6_class
FROM t1;
```

Result:

```text
+-------------+---------------+
| inet4_class | inet6_class   |
+-------------+---------------+
| private     | documentation |
+-------------+---------------+
```

### `cidr_contains()`

Check IPv4 and IPv6 CIDR membership:

```sql
SELECT cidr_contains('192.168.0.0/16', '192.168.10.20') AS v4_inside,
       cidr_contains('192.168.0.0/16', '192.169.10.20') AS v4_outside,
       cidr_contains('2001:db8::/32', '2001:db8::abcd') AS v6_inside,
       cidr_contains('2001:db8::/32', '2001:db9::abcd') AS v6_outside;
```

Result:

```text
+-----------+------------+-----------+------------+
| v4_inside | v4_outside | v6_inside | v6_outside |
+-----------+------------+-----------+------------+
|         1 |          0 |         1 |          0 |
+-----------+------------+-----------+------------+
```

The CIDR argument can also be a single address without a prefix. In that case,
it is treated as a single-host network: `/32` for IPv4 and `/128` for IPv6.

## NULL and Error Behavior

`NULL` input returns `NULL`:

```sql
SELECT ip_class(NULL),
       cidr_contains(NULL, '192.168.1.1'),
       cidr_contains('192.168.0.0/16', NULL);
```

Invalid IP text raises an error:

```sql
SELECT ip_class('not-an-ip');
```

Result:

```text
ERROR 1105 (HY000): ip_class: not a valid IP address
```

Invalid CIDR text raises an error:

```sql
SELECT cidr_contains('192.168.0.0/33', '192.168.1.1');
```

Result:

```text
ERROR 1105 (HY000): cidr_contains: not a valid CIDR or IP address
```
