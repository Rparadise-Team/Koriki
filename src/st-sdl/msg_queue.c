#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

#include "msg_queue.h"

queue_t queue_create() {
	key_t keyval = ftok(QUEUE_NAME, QUEUE_ID);
	return msgget( keyval, IPC_CREAT | 0660 );
}

queue_t queue_open() {
	key_t keyval = ftok(QUEUE_NAME, QUEUE_ID);
	return msgget( keyval, 0660 );
}

int queue_send(queue_t qid, message_t *message) {
	int length = sizeof(message_t) - sizeof(long);        

	return msgsnd( qid, message, length, 0);
}

int queue_read(queue_t qid, long type, message_t *message) {
	int length = sizeof(message_t) - sizeof(long);        

	return msgrcv( qid, message, length, type,  0);
}

int queue_peek(queue_t qid, long type) {
	if(msgrcv( qid, NULL, 0, type,  IPC_NOWAIT) == -1) {
		if(errno == E2BIG) return 1;
	}

	return 0;
}

int queue_remove(queue_t qid) {
	return msgctl(qid, IPC_RMID, 0);
}

