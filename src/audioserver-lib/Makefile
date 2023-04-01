CROSS_COMPILE=/opt/miyoomini-toolchain/bin/arm-linux-gnueabihf-

TARGET = as_preload.so

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))

CFLAGS = -Os -marm -mtune=cortex-a7 -mfpu=neon -mfloat-abi=hard -march=armv7-a
CFLAGS += -fPIC -Wall
LDFLAGS	= -shared -ldl -lSDL -lSDL_sound -lpthread -lmi_sys -lmi_ao -lmi_common -s

$(TARGET): $(CFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CFILES) -o $@

clean:
	rm -f $(TARGET)
