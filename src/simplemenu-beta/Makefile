# Software Name
TARGET = simplemenu
#PLATFORM = PC
#PLATFORM = RFW
#PLATFORM = OD
#PLATFORM = OD-BETA
#PLATFORM = BITTBOY
#PLATFORM = RK3326
PLATFORM = MMIYOO

#Flags
MM_NOQUIT = 0  # Controls if we show Quit option in Session menu on Miyoo Mini. In that device there is no optional startup script. The closest is /mnt/SDCARD/.tmp_update/updater but we can't count on it because it is used by most distributions, so we control the quit menu manually by mean of this flag.
NOLOADING = 0  # Controls whether the 'LOADING...' message is displayed at the bottom right of the screen during some slow processes.

# Compiler
ifeq ($(PLATFORM), BITTBOY)
	CC = /opt/bittboy-toolchain/bin/arm-buildroot-linux-musleabi-gcc
	LINKER = /opt/bittboy-toolchain/bin/arm-buildroot-linux-musleabi-gcc
	CFLAGS = -DTARGET_BITTBOY -Ofast -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra
	LIBS += -lSDL -lasound -lSDL_image -lpng -ljpeg -lSDL_ttf -lfreetype -lz -lbz2
else ifeq ($(PLATFORM), RFW)
	CC = /opt/rg300-toolchain/usr/bin/mipsel-linux-gcc
	LINKER = /opt/rg300-toolchain/usr/bin/mipsel-linux-gcc
	CFLAGS = -DTARGET_RFW -DUSE_GZIP -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra
	LIBS += -lSDL -lSDL_sound -lSDL_image -lSDL_ttf -lopk -lz
else ifeq ($(PLATFORM), OD)
	CC = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
	LINKER = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
	CFLAGS = -DTARGET_OD -DUSE_GZIP -Ofast -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -std=gnu99
	LIBS += -lSDL -lSDL_sound -lSDL_image -lSDL_ttf -lshake -lpthread -lopk -lz
else ifeq ($(PLATFORM), OD-BETA)
	CC = /home/bittboy/Downloads/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
	LINKER = /home/bittboy/Downloads/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
	CFLAGS = -DTARGET_OD_BETA -DUSE_GZIP -Ofast -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra 
	LIBS += -lSDL -lSDL_sound -lSDL_image -lSDL_ttf -lshake -lpthread -lopk -lz
else ifeq ($(PLATFORM), RK3326)
	CC = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
	LINKER = /opt/rk3326-toolchain/usr/bin/arm-buildroot-linux-uclibcgnueabi-gcc
	CFLAGS = -DTARGET_RK3326 -DUSE_GZIP -Ofast -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -std=gnu99
	LIBS += -lSDL -lSDL_sound -lSDL_image -lSDL_ttf -lz -lbz2
else ifeq ($(PLATFORM), MMIYOO)
    CC = arm-linux-gnueabihf-gcc
    LINKER   = arm-linux-gnueabihf-gcc
    CFLAGS = -DMIYOOMINI -DUSE_GZIP -Ofast -fdata-sections -ffunction-sections -fPIC -flto -Wall -Wextra -std=gnu99
	ifeq ($(MM_NOQUIT), 1)
		CFLAGS += -DMM_NOQUIT
	endif
    LIBS += -lSDL -lSDL_image -lSDL_sound -lSDL_mixer -lSDL_ttf -lpthread -lopk -lini -lmi_sys -lmi_common -lmi_panel -lmi_ao -lmi_disp -lz
else
	TARGET = simplemenu-x86
	CC = gcc
	LINKER   = gcc
	CFLAGS = -DTARGET_PC -DUSE_GZIP -Ofast -fdata-sections -ffunction-sections -fPIC -flto -Wall -Wextra -std=gnu99
	LIBS += -lSDL -lasound -lSDL_image -lSDL_ttf -lopk -lz
endif
ifeq ($(NOLOADING), 1)
	CFLAGS += -DNOLOADING
endif

# You can use Ofast too but it can be more prone to bugs, careful.
CFLAGS +=  -I.
LDFLAGS = -Wl,--start-group $(LIBS) -Wl,--end-group -Wl,--as-needed -Wl,--gc-sections -flto

DEBUG=NO

ifeq ($(DEBUG), NO)
	CFLAGS +=  -DDEBUG -g3
else
	LDFLAGS	+=  -s -lm
endif

SRCDIR   = src/logic
OBJDIR   = src/obj
BINDIR   = output
SOURCES  := $(wildcard $(SRCDIR)/*.c)

ifeq ($(PLATFORM), BITTBOY)
	SOURCES := $(filter-out src/logic/control_od.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_od.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_mmiyoo.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/cJSON.c, $(SOURCES))
else ifeq ($(PLATFORM), RFW)
	SOURCES := $(filter-out src/logic/control_od.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_od.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_mmiyoo.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/cJSON.c, $(SOURCES))		
else ifeq ($(PLATFORM), OD)
	SOURCES := $(filter-out src/logic/control_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_mmiyoo.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/cJSON.c, $(SOURCES))
else ifeq ($(PLATFORM), OD-BETA)
	SOURCES := $(filter-out src/logic/control_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_mmiyoo.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/cJSON.c, $(SOURCES))
else ifeq ($(PLATFORM), NPG)
	SOURCES := $(filter-out src/logic/control_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_mmiyoo.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/cJSON.c, $(SOURCES))
else ifeq ($(PLATFORM), MMIYOO)
	SOURCES := $(filter-out src/logic/control_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_od.c, $(SOURCES))
else
	SOURCES := $(filter-out src/logic/control_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/control_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_rfw.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_bittboy.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/system_logic_mmiyoo.c, $(SOURCES))
	SOURCES := $(filter-out src/logic/cJSON.c, $(SOURCES))
endif 

OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

rm       = rm -f
	
$(BINDIR)/$(TARGET): $(OBJECTS)
	@$(LINKER) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONY: clean
clean:
	@$(rm) $(OBJECTS)
	@echo "Cleanup complete!"

.PHONY: remove
remove: clean
	@$(rm) $(BINDIR)/$(TARGET)
	@echo "Executable removed!"