/*
 * shm.c
 *
 *  Created on: May 18, 2016
 *  Updated on: June 07, 2016
 *      Author: qige
 */

#include <stdio.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "conf.h"
#include "shm.h"

int shm_init(const char *key, int *shmid, void **shm, const unsigned int size)
{
#ifdef GWS_USE_KEYFILE
	*shmid = shmget(ftok(key, 1), size, 0666 | IPC_CREAT);
#else
	*shmid = shmget((key_t) key, size, 0666 | IPC_CREAT);
#endif
	if (*shmid == -1) {
		SHM_DBG("-> (init) * init failed (shmget).\n");
		return SHM_ERR_GET;
	}

	*shm = (void *) shmat(*shmid, NULL, 0);
	if (*shm && *shm == (void *) -1) {
		SHM_DBG("-> (init) * init failed (shmat)\n");
		return SHM_ERR_AT;
	}

	return SHM_OK;
}

int shm_free(const int shmid, void *shm)
{
	if (shmdt(shm)) {
		SHM_DBG("-> (quit) * warning: close (shmdt)\n");
	}

	if (shmid != -1) {
		if (shmctl(shmid, IPC_RMID, NULL) < 0) {
			SHM_DBG("-> (quit) * warning: close (shmctl)\n");
		}
	}

	return SHM_OK;
}
