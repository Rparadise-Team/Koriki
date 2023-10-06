CROSS_COMPILE ?= /opt/miyoomini83/bin/arm-linux-gnueabihf-
ARCH = -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd

TARGET = cpuclock

CC 	= $(CROSS_COMPILE)gcc

SOURCES = . 
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
OFILES = $(CFILES:.c=.o)

CFLAGS  = -Os $(ARCH) -ffunction-sections -fdata-sections -Wall
LDFLAGS = $(CFLAGS) -Wl,--gc-sections -s

$(TARGET): $(OFILES)
	$(CC) $(OFILES) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OFILES)
