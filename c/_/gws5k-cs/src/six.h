/*
 * six.h
 *
 *  Created on: July 13, 2016
 *      Author: Qige Zhao
 */

#ifndef SIX_H_
#define SIX_H_


//#define DEBUG_SHM


/*
 * PART I: pipe/cli
 */
const char *get_ip(const char *ifname);
const char *get_mac(const char *ifname);
char *cli_read(const char *cmd);
void cli_exec(const char *cmd);
void print_file(const char *filename, const char *str);



/*
 * PART II: IPC SHM
 */
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



#endif /* SIX_H_ */
