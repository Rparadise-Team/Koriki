# st version
VERSION = 0.3

# Customize below to fit your system

# paths
CROSS_COMPILE ?= /opt/miyoomini-toolchain/bin/arm-linux-gnueabihf-
CC = ${CROSS_COMPILE}gcc
SYSROOT	?= $(shell ${CC} --print-sysroot)

# includes and libs
INCS = -I. -I${SYSROOT}/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT
LIBS = -lc -L${SYSROOT}/usr/lib -lSDL -lpthread -Wl,-Bstatic,-lutil,-Bdynamic

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -Os -marm -mtune=cortex-a7 -march=armv7ve+simd -mfpu=neon-vfpv4 -mfloat-abi=hard
CFLAGS += ${INCS} ${CPPFLAGS} -DMIYOOMINI -std=gnu11
CFLAGS += -fPIC -ffunction-sections -fdata-sections -Wall
LDFLAGS = ${LIBS} -Wl,--gc-sections -s
