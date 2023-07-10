# st version
VERSION = 0.3

# Customize below to fit your system

# paths
PREFIX = /opt/gcw0-toolchain/usr/
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I.  -I/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LIBS = -lc -lutil -L/opt/gcw0-toolchain/usr/mipsel-gcw0-linux-uclibc/sysroot/usr/lib -lSDL -lpthread

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"  -fPIC
CFLAGS += -g -Wall ${INCS} ${CPPFLAGS} -DRS97 -fPIC -std=gnu11 
LDFLAGS += -g ${LIBS} -lSDL

# compiler and linker
CC = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
