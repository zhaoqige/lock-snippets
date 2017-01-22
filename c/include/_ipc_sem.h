/*
 * _ipc_sem.h
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#ifndef IPC_SEM_H_
#define IPC_SEM_H_

#include <sys/sem.h>

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

// empty debug print out
#ifdef ENV_DEBUG_SEM
#define SEM_DBG(format, ...)		{printf("<sem> "format, ##__VA_ARGS__);}
#else
#define SEM_DBG(fmt, ...)			{}
#endif

// pre defines
//#define IPC_SEM_USE_KEY_FILE

enum IPC_SEM_OPS{
	IPC_SEM_V               = -1,
	IPC_SEM_DEFAULT_STOP    = 0,
	IPC_SEM_P               = 1
};

enum IPC_SEM_ERR {
	SEM_ERR_SEMGET      = -5,
	SEM_ERR_SEMCTL,
	SEM_ERR_SEMOP_V,
	SEM_ERR_SEMOP_P,

	SEM_OK              = 0
};

// semaphore
// ready, running, blocking
union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};

int sem_get(const void  *key, int *sid, const uint ssize);
int sem_init(int sid, const uint ssize, int init_value);

int sem_p(const int sid, const uint snum);
int sem_v(const int sid, const uint snum);

int sem_del(int *sid, const uint snum);
void sem_del_all(int *sid, const uint ssize);

#endif /* IPC_SEM_H_ */
