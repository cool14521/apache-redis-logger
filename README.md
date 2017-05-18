
This is a program that can take apache status codes and increment redis counters.

The expected formated is hostname,virtualhost,statuscode and the result is a forever incrementing counter in redis on those pairs.

Please use hiredis >= 13.

```bash
git clone git@github.com:ptudor/apache-redis-logger
cd apache-redis-logger ; make linux && make install
cd apache-redis-logger ; make freebsd && make install
```

Add a CustomLog at the vhost level.

```apache
LogFormat "%A,%v,%s" redis
LogFormat "%{HOST}e,%v,%s" redis-with-env
<VirtualHost *:80>
  ServerName www.example.com
  CustomLog "|/usr/local/bin/tudor-apache-redis-logger -s localhost -p 6379" redis
</VirtualHost>
```

Results. Unsorted when raw, so add a 'ksort' or similar to your code before the json encode and it helps humans debug.

```bash
# redis-cli -n 2 hgetall 200 
1) "www.example.com"
2) "2558854"
3) "www.example.edu"
4) "767939"
5) "www.example.net"
6) "1082460"
# redis-cli -n 2 hgetall 503
1) "www.example.net"
2) "153"
3) "www.example.com"
4) "1180"
```

