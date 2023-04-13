#ifndef __MSG_QUEUE__
#define __MSG_QUEUE__

#define QUEUE_NAME "st-sdl"
#define QUEUE_ID 1

typedef struct {
	long type;
	long data;
} message_t;

typedef int queue_t;

enum { MSG_CLIENT=1, MSG_SERVER, MSG_REQUEST_SHUTDOWN, MSG_SHUTDOWN, MSG_REQUEST_INIT };

queue_t queue_create();
queue_t queue_open();
int queue_send(queue_t qid, message_t *message);
int queue_read(queue_t qid, long type, message_t *message);
int queue_peek(queue_t qid, long type);
int queue_remove(queue_t qid);

#endif
