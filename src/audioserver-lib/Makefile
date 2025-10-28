# Toolchain para Miyoo Mini (ARM Cortex-A7)
CROSS_COMPILE = /opt/miyoomini-toolchain/bin/arm-linux-gnueabihf-

# Target output
TARGET = as_preload.so

# Compiler y herramientas
CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

# Directorios de código fuente
SOURCES = .
CFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))

# Include paths (ajusta según tu estructura)
INCLUDES = -I. -I/opt/miyoomini-toolchain/arm-linux-gnueabihf/libc/usr/include

# Flags de compilación optimizados para Cortex-A7
CFLAGS = -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve
CFLAGS += -fPIC -Wall -Wextra
CFLAGS += -ffunction-sections -fdata-sections  # Permite dead code elimination
CFLAGS += $(INCLUDES)

# Flags de linkado
LDFLAGS = -shared -Wl,--gc-sections  # Elimina secciones no usadas
LDFLAGS += -Wl,-soname,$(TARGET)     # Establece soname correcto
LIBS = -ldl -pthread                 # Solo dlopen y pthread, NO lmi_ao

# Build types
DEBUG ?= 0
ifeq ($(DEBUG),1)
    CFLAGS += -g -DDEBUG
    STRIP_CMD = @echo "Debug build - skipping strip"
else
    CFLAGS += -DNDEBUG
    STRIP_CMD = $(STRIP) --strip-unneeded $(TARGET)
endif

# Targets
.PHONY: all clean install debug release

all: $(TARGET)

$(TARGET): $(CFILES)
	@echo "Building $(TARGET) for Miyoo Mini (Cortex-A7)..."
	$(CC) $(CFLAGS) $(CFILES) $(LDFLAGS) $(LIBS) -o $@
	$(STRIP_CMD)
	@echo "Build complete: $(TARGET)"
	@echo "Size: $$(du -h $(TARGET) | cut -f1)"

debug:
	$(MAKE) DEBUG=1

release:
	$(MAKE) DEBUG=0

# Instalación (ajusta la ruta según tu sistema)
install: $(TARGET)
	@echo "Installing $(TARGET)..."
	install -m 755 $(TARGET) /mnt/SDCARD/.tmp_update/lib/
	@echo "Installation complete"

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET) *.o *~
	@echo "Clean complete"

# Información del build
info:
	@echo "Compiler: $(CC)"
	@echo "Target: $(TARGET)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIBS: $(LIBS)"
	@echo "Sources: $(CFILES)"

