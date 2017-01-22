/*
 * shm.h
 *
 *  Created on: May 18, 2016
 *  Updated on: June 07, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef SHM_H_
#define SHM_H_

#define DEBUG_SHM

enum SHM_ERR {
	SHM_OK = 0,
	SHM_ERR_GET = -50,
	SHM_ERR_AT,
};

#ifdef DEBUG_SHM
#define SHM_DBG(format, ...)		{printf("-shm-: "format, ##__VA_ARGS__);}
#else
#define SHM_DBG(fmt, ...)			{}
#endif

int shm_init(const char *keyfile, int *shmid, void **shm, const unsigned int size);
int shm_free(const int shmid, void *shm);

#endif /* SHM_H_ */
