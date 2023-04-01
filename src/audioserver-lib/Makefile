CROSS_COMPILE=/opt/miyoomini-toolchain/bin/arm-linux-gnueabihf-

TARGET = as_preload.so

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))

CFLAGS = -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd
CFLAGS += -fPIC -Wall
LDFLAGS	= -shared -ldl -s -lSDL -lmi_ao -lmi_sys -lmi_common

$(TARGET): $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -o $@

clean:
	rm -f $(TARGET)
