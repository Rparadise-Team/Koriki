# st version
VERSION = 0.3

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I. -I./SDL_ttf/bb10_simulator_prefix/include/SDL
LIBS = -lc -L./SDL_ttf/bb10_simulator_prefix/lib

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS += -g -pedantic -Wall -Os ${INCS} ${CPPFLAGS} ${MAKEFLAGS} `./SDL/bb10_simulator_prefix/bin/sdl-config --cflags`
LDFLAGS += -g ${LIBS} `./SDL/bb10_simulator_prefix/bin/sdl-config --libs` -lSDL_ttf

# compiler and linker
CC ?= cc

st.bar: st
	mkdir -p lib/
	cp -p SDL/bb10_simulator_prefix/lib/libSDL-1.2.so.11 lib/
	cp -p SDL_ttf/bb10_simulator_prefix/lib/libSDL_ttf-2.0.so.10 lib/
	cp -p ${QNX_TARGET}/x86/lib/libTouchControlOverlay.so.1 lib/
	blackberry-nativepackager -package -devMode -target bar st.bar blackberry-tablet.xml st lib/libSDL-1.2.so.11 lib/libSDL_ttf-2.0.so.10 lib/libTouchControlOverlay.so.1 LiberationMono-Regular.ttf LiberationMono-Bold.ttf

.PHONY: clean bb10clean

clean: bb10clean

bb10clean:
	$(RM) -r lib st.bar
