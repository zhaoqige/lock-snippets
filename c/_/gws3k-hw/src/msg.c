/*
 * msg.c
 *
 *  Created on: May 18, 2016
 *  Updated on: June 07, 2016
 *      Author: qige
 */

#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "conf.h"
#include "msg.h"


int msg_init(const char *key, int *msgid)
{
#ifdef GWS_USE_KEYFILE
	*msgid = msgget(ftok(key, 1), 0666 | IPC_CREAT);
#else
	*msgid = msgget((key_t) key, 0666 | IPC_CREAT);
#endif
	if (*msgid == -1) {
		MSG_DBG("-> (init) * init failed (msgget)\n");
		return MSG_ERR_GET;
	}
	return MSG_OK;
}

int msg_send(const int msgid, const void *msg, const unsigned int msg_length)
{
	if (msgsnd(msgid, msg, msg_length, 0) < 0) {
		return MSG_ERR_SEND_FAILED;
	}
	return MSG_OK;
}
int msg_recv(const int msgid, void *data, unsigned int *msg_length, const int msg_type)
{
	if (msgrcv(msgid, data, *msg_length, msg_type, IPC_NOWAIT) < 0) {
		return MSG_ERR_RECV_FAILED;
	}
	return MSG_OK;
}

int msg_free(const int msgid)
{
	if (msgid != -1) {
		if (msgctl(msgid, IPC_RMID, NULL)) {
			MSG_DBG("-> (quit) * wanring: close (msgctl)\n");
			return MSG_ERR_RMID;
		}
	}
	return MSG_OK;
}

