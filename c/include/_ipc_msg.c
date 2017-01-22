/*
 * _ipc_msg.c
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#include <string.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "_ipc_msg.h"


int msg_init(const void *key, int *mid)
{
#ifdef IPC_MSG_USE_KEY_FILE
	const char *k = (const char *) key;
	*mid = msgget(ftok(k, 1), 0666 | IPC_CREAT);
#else
	int *k = (int *) key;
	*mid = msgget((key_t) *k, 0666 | IPC_CREAT);
#endif
	if (*mid == -1) {
		MSG_DBG("(ERROR) * init failed (msgget)\n");
		return MSG_ERR_GET;
	}
	return MSG_OK;
}

int msg_send(const int mid, const void *msg, const uint msg_length)
{
	if (msgsnd(mid, msg, msg_length, 0) < 0) {
		return MSG_ERR_SEND_FAILED;
	}
	return MSG_OK;
}
int msg_recv(const int mid, void *data, uint *msg_length, const int msg_type)
{
	if (msgrcv(mid, data, *msg_length, msg_type, IPC_NOWAIT) < 0) {
		return MSG_ERR_RECV_FAILED;
	}
	return MSG_OK;
}

int msg_free(int *mid)
{
	if (*mid != -1) {
		if (msgctl(*mid, IPC_RMID, NULL)) {
			MSG_DBG("(ERROR) * free up (msgctl)\n");
			return MSG_ERR_RMID;
		}
	}
	return MSG_OK;
}

