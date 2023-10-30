CROSS   = /opt/miyoomini-toolchain/bin/arm-linux-gnueabihf-
TARGET  = show_hotkeys
#CFLAGS  = -I/usr/include
CFLAGS  = -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/sysroot/usr/include
LDFLAGS = -lSDL -lSDL_image -ljson-c

all:
	$(CROSS)gcc main.c $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -rf $(TARGET)
