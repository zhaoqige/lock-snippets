/*
 * six.c
 *
 *  Created on: July 13, 2016
 *      Author: Qige Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "conf.h"
#include "six.h"


/*
 * PART I: pipe/cli
 */
const char *get_ip(const char *ifname)
{
	char cmd[128];
	memset(cmd, 0x00, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),
			"ifconfig %s | grep 'inet addr' | cut -d ':' -f2 | cut -d ' ' -f1",
			ifname);
	return cli_read(cmd);
}

const char *get_mac(const char *ifname)
{
	char cmd[128];
	memset(cmd, 0x00, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),
			"ifconfig %s | grep 'HWaddr' | cut -d 'W' -f2 | cut -d ' ' -f2",
			ifname);
	return cli_read(cmd);
}

char *cli_read(const char *cmd)
{
	FILE *fp;
	unsigned int buffer_length;
	static char buffer[128];

	fp = popen(cmd, "r");
	memset(buffer, 0x00, sizeof(buffer));
	fgets(buffer, sizeof(buffer)-1, fp);
	buffer_length = strlen(buffer);
	if (buffer_length > 0) {
		if (buffer[buffer_length-1] == '\n') buffer[buffer_length-1] = '\0';
	}
	buffer[buffer_length] = '\0';
	pclose(fp);

	return buffer;
}

void cli_exec(const char *cmd)
{
	system(cmd);
}

void print_file(const char *filename, const char *str)
{
	FILE *fd = fopen(filename, "w+");
	fputs(str, fd);
	fclose(fd);
}




/*
 * PART II: IPC SHM
 */
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
