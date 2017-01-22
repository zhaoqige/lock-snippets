/*
 * _ipc_msg.h
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#ifndef IPC_MSG_H_
#define IPC_MSG_H_

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

// empty debug print out
#ifdef ENV_DEBUG_MSG
#define MSG_DBG(format, ...)		{printf("<msg> "format, ##__VA_ARGS__);}
#else
#define MSG_DBG(fmt, ...)			{}
#endif

// pre defines
//#define IPC_MSG_USE_KEY_FILE

enum IPC_MSG_ERR {
	MSG_OK = 0,
	MSG_ERR_GET = 1,
	MSG_ERR_SEND_FAILED,
	MSG_ERR_RECV_FAILED,
	MSG_ERR_RMID,
};

int msg_init(const void *key, int *mid);
int msg_send(const int mid, const void *msg, const uint msg_length);
int msg_recv(const int mid, void *data, uint *msg_length, const int msg_type);
int msg_free(int *msgid);

#endif /* IPC_MSG_H_ */
