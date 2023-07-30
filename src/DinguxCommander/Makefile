#
#	DinguxCommander Makefile for MiyooMini
#

CXX := $(CROSS_COMPILE)g++

PLATFORM ?= $(UNION_PLATFORM)
ifeq (,$(PLATFORM))
PLATFORM=linux
endif

ifeq ($(PLATFORM),miyoomini)
CXXFLAGS := -Os -marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7ve+simd
else
CXXFLAGS := -Os
endif

SDL_CONFIG := $(shell $(CXX) -print-sysroot)/usr/bin/sdl-config
CXXFLAGS += $(shell $(SDL_CONFIG) --cflags)

CXXFLAGS += -DPATH_DEFAULT=\"/mnt/SDCARD\"
CXXFLAGS += -DFILE_SYSTEM=\"/dev/mmcblk0p1\"
CXXFLAGS += -DCMDR_KEY_UP=SDLK_UP
CXXFLAGS += -DCMDR_KEY_RIGHT=SDLK_RIGHT
CXXFLAGS += -DCMDR_KEY_DOWN=SDLK_DOWN
CXXFLAGS += -DCMDR_KEY_LEFT=SDLK_LEFT
CXXFLAGS += -DCMDR_KEY_OPEN=SDLK_SPACE		# A
CXXFLAGS += -DCMDR_KEY_PARENT=SDLK_LCTRL	# B
CXXFLAGS += -DCMDR_KEY_OPERATION=SDLK_LSHIFT	# X
CXXFLAGS += -DCMDR_KEY_SYSTEM=SDLK_LALT		# Y
CXXFLAGS += -DCMDR_KEY_PAGEUP=SDLK_e		# L1 / L2 = SDLK_TAB
CXXFLAGS += -DCMDR_KEY_PAGEDOWN=SDLK_t		# R1 / R2 = SDLK_BACKSPACE
CXXFLAGS += -DCMDR_KEY_SELECT=SDLK_RCTRL	# SELECT
CXXFLAGS += -DCMDR_KEY_TRANSFER=SDLK_RETURN	# START
CXXFLAGS += -DCMDR_KEY_MENU=SDLK_ESCAPE		# MENU (added)
CXXFLAGS += -DOSK_KEY_SYSTEM_IS_BACKSPACE=ON
CXXFLAGS += -DSCREEN_WIDTH=640
CXXFLAGS += -DSCREEN_HEIGHT=480
CXXFLAGS += -DPPU_X=1.66666
CXXFLAGS += -DPPU_Y=1.66666
CXXFLAGS += -DSCREEN_BPP=32
CXXFLAGS += -DFONTS='{"SourceCodePro-Semibold.ttf",16},{"SourceCodePro-Regular.ttf",16},{"/customer/app/wqy-microhei.ttc",16}'
ifeq ($(PLATFORM),miyoomini)
CXXFLAGS += -DMIYOOMINI
endif

RESDIR := res
CXXFLAGS += -DRESDIR="\"$(RESDIR)\""

LINKFLAGS += -s
LINKFLAGS += $(shell $(SDL_CONFIG) --libs) -lSDL_image -lSDL_ttf
ifeq ($(PLATFORM),miyoomini)
LINKFLAGS += -lmi_sys -lmi_gfx -lgme -lz -lunrar
endif

CMD := @
SUM := @echo

OUTDIR := ./output

EXECUTABLE := $(OUTDIR)/DinguxCommander

OBJS :=	main.o commander.o config.o dialog.o fileLister.o fileutils.o keyboard.o panel.o resourceManager.o \
	screen.o sdl_ttf_multifont.o sdlutils.o text_edit.o utf8.o text_viewer.o image_viewer.o  window.o \
	SDL_rotozoom.o
ifeq ($(PLATFORM),miyoomini)
OBJS += gfx.o
endif

DEPFILES := $(patsubst %.o,$(OUTDIR)/%.d,$(OBJS))

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): $(addprefix $(OUTDIR)/,$(OBJS))
	$(SUM) "  LINK    $@"
	$(CMD)$(CXX) $(LINKFLAGS) -o $@ $^

$(OUTDIR)/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(SUM) "  CXX     $@"
	$(CMD)$(CXX) $(CXXFLAGS) -MP -MMD -MF $(@:%.o=%.d) -c $< -o $@
	@touch $@ # Force .o file to be newer than .d file.

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(@D)
	$(SUM) "  CXX     $@"
	$(CMD)$(CXX) $(CXXFLAGS) -MP -MMD -MF $(@:%.o=%.d) -c $< -o $@
	@touch $@ # Force .o file to be newer than .d file.

clean:
	$(SUM) "  RM      $(OUTDIR)"
	$(CMD)rm -rf $(OUTDIR)

# Load dependency files.
-include $(DEPFILES)

# Generate dependencies that do not exist yet.
# This is only in case some .d files have been deleted;
# in normal operation this rule is never triggered.
$(DEPFILES):
