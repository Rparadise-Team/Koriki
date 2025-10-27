# Software Name
PROGRAM = invoker.dge
#PLATFORM = BITTBOY
#PLATFORM = RFW
#PLATFORM = OD
#PLATFORM = NPG
PLATFORM = MMIYOO
#PLATFORM = PC

# Compiler

ifeq ($(PLATFORM), BITTBOY)
	CC = /opt/bittboy-toolchain/bin/arm-buildroot-linux-musleabi-gcc
else ifeq ($(PLATFORM), RFW)
	CC = /opt/rg300-toolchain/usr/bin/mipsel-linux-gcc
else ifeq ($(PLATFORM), OD)
	CC = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc
else ifeq ($(PLATFORM), OD-BETA)
	CC = /home/bittboy/Downloads/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc	
else ifeq ($(PLATFORM), NPG)
	CC = /opt/gcw0-toolchain/usr/bin/mipsel-gcw0-linux-uclibc-gcc	
else ifeq ($(PLATFORM), MMIYOO)
	CC = $(CROSS_COMPILE)gcc
else
	PROGRAM = /home/bittboy/git/simplemenu/simplemenu/output/invoker.dge
	CC = gcc
endif

# You can use Ofast too but it can be more prone to bugs, careful.
ifeq ($(PLATFORM), BITTBOY)
	CFLAGS = -DTARGET_BITTBOY -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -no-pie -static
else ifeq ($(PLATFORM), RFW)
	CFLAGS = -DTARGET_RFW -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -no-pie
else ifeq ($(PLATFORM), OD)
	CFLAGS = -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -std=gnu99
else ifeq ($(PLATFORM), OD-BETA)
	CFLAGS = -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -std=gnu99	
else ifeq ($(PLATFORM), NPG)
	CFLAGS = -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -std=gnu99
else ifeq ($(PLATFORM), MMIYOO)
	CFLAGS = -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -std=gnu99
else
	CFLAGS = -DTARGET_PC -O2 -fdata-sections -ffunction-sections -fno-PIC -flto -Wall -Wextra -no-pie	
endif

CFLAGS +=  -I.

LIBS +=
LDFLAGS = -Wl,--start-group $(LIBS) -Wl,--end-group -Wl,--as-needed -Wl,--gc-sections -flto

ifeq ($(DEBUG), YES)
CFLAGS +=  -DDEBUG -g3
else
LDFLAGS	+=  -s
endif

CFILES =  invoker.c
SFILES = 

OFILES = $(SFILES:.S=.o) $(CFILES:.c=.o)

$(PROGRAM):	$(OFILES)
	@$(CC) $(CFLAGS) $(OFILES) -o $@ $(LDFLAGS)
	@echo "Linking complete!"

all: $(PROGRAM)

%.o: %.c
	 @$(CC) $(CFLAGS) -c $< -o $@
	 @echo "Compiled "$<" successfully!"

clean:
	 @-rm -f $(OFILES) $(MAPFILE) $(PROGRAM)