/*
 * _ipc_sem.c
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#include <stdio.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


#include "_ipc_sem.h"


int sem_get(const void  *key, int *sid, const uint ssize)
{
#ifdef IPC_SEM_USE_KEY_FILE
	const char *k = (const char *) key;
    (*sid) = semget(ftok(k, 1), ssize, IPC_CREAT | 0666);
#else
    int *k = (int *) key;
    (*sid) = semget((key_t) *k, ssize, IPC_CREAT | 0666);
#endif
    if (*sid < 0) {
        return SEM_ERR_SEMGET;
    }
    return SEM_OK;
}

int sem_init(int sid, const uint ssize, int init_value)
{
    union semun semu;
    semu.val = init_value;

    uint i;
    for(i = 0; i < ssize; i ++) {
        if (semctl(sid, i, SETVAL, semu) == -1) {
                SEM_DBG("(job) * Initialize semaphore (%d/%d/%d)\n", \
                    sid, ssize, init_value);
                return SEM_ERR_SEMCTL;
        }
    }
    return SEM_OK;
}

int sem_del(int *sid, const uint snum)
{
    union semun semun;
    if (semctl(*sid, snum, IPC_RMID, semun) == -1) {
            SEM_DBG("(job) Delete semaphore (%d/%d)\n", \
                *sid, snum);
            return SEM_ERR_SEMCTL;
    }
    return SEM_OK;
}

void sem_del_all(int *sid, const uint ssize)
{
    uint i;
    for(i = 0; i < ssize; i ++) {
        sem_del(sid, i);
    }
}

int sem_p(const int sid, const uint snum)
{
    struct sembuf sem_b;
    sem_b.sem_num = snum;
    sem_b.sem_op = IPC_SEM_V;
    sem_b.sem_flg = SEM_UNDO;

    if (semop(sid, &sem_b, 1) == -1) {
            SEM_DBG("(job) P operation (%d/%d)\n", sid, snum);
            return SEM_ERR_SEMOP_V;
    }
    return SEM_OK;
}

int sem_v(const int sid, const uint snum)
{
    struct sembuf sem_b;
    sem_b.sem_num = snum;
    sem_b.sem_op = IPC_SEM_P;
    sem_b.sem_flg = SEM_UNDO;

    if (semop(sid, &sem_b, 1) == -1) {
            SEM_DBG("(job) V operation (%d/%d)\n", sid, snum);
            return SEM_ERR_SEMOP_P;
    }
    return SEM_OK;
}
