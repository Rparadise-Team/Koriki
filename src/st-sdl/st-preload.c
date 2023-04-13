#define _GNU_SOURCE
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include "msg_queue.h"

static int sdl_init_was_called = 0;
static int (*real_SDL_Init)(int) = NULL;

int SDL_Init(int flags) {
	if (sdl_init_was_called == 0) {
		queue_t qid = queue_open();
		if(qid != -1) {
			message_t message = {.type = MSG_CLIENT, .data = MSG_REQUEST_SHUTDOWN };
			queue_send(qid, &message);
			queue_read(qid, MSG_SERVER, &message);
		}

		real_SDL_Init = dlsym(RTLD_NEXT, "SDL_Init");
		sdl_init_was_called = 1;
	}
	return real_SDL_Init(flags);
}

void __attribute__((destructor)) preload_cleanup() {
	if(sdl_init_was_called) {
		queue_t qid = queue_open();
		if(qid != -1) {
			message_t message = {.type = MSG_CLIENT, .data = MSG_REQUEST_INIT };
			queue_send(qid, &message);
		}
	}
}

