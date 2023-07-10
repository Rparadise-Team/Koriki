# st version
VERSION = 0.3

# Customize below to fit your system

# paths
PREFIX = /opt/miyoomini-toolchain/usr/
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I.  -I/opt/miyoomini-toolchain/usr/arm-miyoomini-linux-gnueabihf/sysroot/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LIBS = -lc -lutil -L/opt/miyoomini-toolchain/usr/arm-miyoomini-linux-gnueabihf/sysroot/usr/lib -lSDL -lpthread -lmi_sys -lmi_gfx -lmi_ao -lmi_common -Wl,-Bstatic,-lutil,-Bdynamic

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"  -fPIC
CFLAGS = -Os -marm -mtune=cortex-a7 -march=armv7-a -mfpu=neon -mfloat-abi=hard
CFLAGS += -g -Wall ${INCS} ${CPPFLAGS} -DMIYOOMINI -fPIC -ffunction-sections -fdata-sections -std=gnu11 
LDFLAGS += -g ${LIBS} -lSDL -Wl,--gc-sections -s

# compiler and linker
CC = /opt/miyoomini-toolchain/usr/bin/arm-miyoomini-linux-gnueabihf-gcc
