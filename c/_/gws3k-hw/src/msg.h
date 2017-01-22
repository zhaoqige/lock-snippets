/*
 * msg.h
 *
 *  Created on: May 18, 2016
 *  Updated on: June 07, 2016
 *      Author: qige
 */

#ifndef MSG_H_
#define MSG_H_

enum MSG_ERR {
	MSG_OK = 0,
	MSG_ERR_GET = -50,
	MSG_ERR_SEND_FAILED,
	MSG_ERR_RECV_FAILED,
	MSG_ERR_RMID,
};

#define MSG_DBG(format, ...)		{printf("-msg-: "format, ##__VA_ARGS__);}
//#define MSG_DBG(fmt, ...)			{}

int msg_init(const char *key, int *msgid);
int msg_send(const int msgid, const void *msg, const unsigned int msg_length);
int msg_recv(const int msgid, void *data, unsigned int *msg_length, const int msg_type);
int msg_free(const int msgid);

#endif /* MSG_H_ */
