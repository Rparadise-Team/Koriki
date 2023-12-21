# st - simple terminal
# See LICENSE file for copyright and license details.

include config.mk

SRC = st.c keyboard.c font.c msg_queue.c
OBJ = ${SRC:.c=.o}

all: options st 

options:
	@echo st build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo $(CC) $<
	@${CC} -c ${CFLAGS} $<

st: ${OBJ}
	@echo $(CC) -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f st ${OBJ}

.PHONY: all options clean
