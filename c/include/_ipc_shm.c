/*
 * _ipc_shm.c
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "_ipc_shm.h"


int shm_init(const void *key, int *sid, void **saddr, const uint ssize)
{
#ifdef IPC_SHM_USE_KEY_FILE
	const char *k = (const char *) key;
	*sid = shmget(ftok(k, 1), ssize, 0666 | IPC_CREAT);
#else
	int *k = (int *) key;
	*sid = shmget((key_t) *k, ssize, 0666 | IPC_CREAT);
#endif
	if (*sid == -1) {
		SHM_DBG("-> (ERROR) * init failed (shmget).\n");
		return SHM_ERR_GET;
	}

	*saddr = (void *) shmat(*sid, NULL, 0);
	if (*saddr && *saddr == (void *) -1) {
		SHM_DBG(" (ERROR) * init failed (shmat)\n");
		return SHM_ERR_AT;
	}

	return SHM_OK;
}

void shm_read(const void *src, void *dest, const uint length)
{
	memcpy(dest, src, length);
}

void shm_write(const void *src, void *dest, const uint length)
{
	memcpy(dest, src, length);
}

int shm_free(int *sid, void **saddr)
{
	if (shmdt(*saddr)) {
		SHM_DBG("(warning) * warning: close (shmdt)\n");
	}

	if (*sid != -1) {
		if (shmctl(*sid, IPC_RMID, NULL) < 0) {
			SHM_DBG("(warning) * warning: close (shmctl)\n");
		}
	}

	return SHM_OK;
}



