# 2016 Patrick Tudor

SHELL = /bin/sh
CFLAGS = -Wall -Wextra
INSTALL = /usr/bin/install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

.PHONY: default help clean

default: help

hiredis-freebsd:
	portmaster -BD databases/hiredis

hiredis-fedora:
	dnf -y install hiredis hiredis-devel

freebsd:
	clang -march=native -fstack-protector -fPIC -O -L/usr/local/lib -I/usr/local/include/hiredis -lhiredis $(CFLAGS) -o tudor-apache-redis-logger tudor-apache-redis-logger.c

linux:
	gcc -m64 -lhiredis -mtune=generic -fPIC -fstack-protector-strong -L/usr/lib64 -I/usr/include/hiredis -std=gnu99 $(CFLAGS) -o tudor-apache-redis-logger tudor-apache-redis-logger.c

install:
	$(INSTALL_PROGRAM) tudor-apache-redis-logger $(DESTDIR)$(bindir)/tudor-apache-redis-logger

clean:
	rm -f tudor-apache-redis-logger

help:
	@echo "Oops! Try make linux or make freebsd then make install"
