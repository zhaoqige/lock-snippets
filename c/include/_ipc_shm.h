/*
 * _ipc_shm.h
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#ifndef IPC_SHM_H_
#define IPC_SHM_H_

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

// empty debug print out
#ifdef ENV_DEBUG_SHM
#define SHM_DBG(format, ...)		{printf("<shm> "format, ##__VA_ARGS__);}
#else
#define SHM_DBG(fmt, ...)			{}
#endif

// pre defines
//#define IPC_SHM_USE_KEY_FILE

enum IPC_SHM_ERR {
	SHM_OK = 0,
	SHM_ERR_GET = 1,
	SHM_ERR_AT,
};

int  shm_init(const void *key, int *sid, void **saddr, const uint ssize);
void shm_read(const void *src, void *dest, const uint length);
void shm_write(const void *src, void *dest, const uint length);
int  shm_free(int *sid, void **saddr);

#endif /* IPC_SHM_H_ */
