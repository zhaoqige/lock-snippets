/*
 * task.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *  Updated on: Nov 11, 2016 - v10.0.111116
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "six.h"

#include "grid.h"
#include "app.h"
#include "task.h"

#ifdef HW_GWS3K
#include "gws3k.h"
#endif
#if defined(HW_GWS5K) && defined(USE_TPC)
#include "gws5k_tpc.h"
#endif
#if defined(HW_GWS5K) && defined(USE_AGC)
#include "gws5k_agc.h"
#endif

int FLAG_APP_EXIT = 0;


static int task_init(const void *conf, void *env);
static int task_check_config(void *app_conf);


int (*task_core)(const void *);
void (*task_idle)(const int);


static void task_func_init(void) {
#ifdef HW_GWS3K
		task_core = &gws3k_tpc_run;
		task_idle = &gws3k_tpc_idle;
#endif
#if defined(HW_GWS5K) && defined(USE_TPC)
		task_core = &gws5k_tpc_run;
		task_idle = &gws5k_tpc_idle;
#endif
#if defined(HW_GWS5K) && defined(USE_AGC)
		task_core = &gws5k_agc_run;
		task_idle = &gws5k_agc_idle;
#endif
}

static int task_check_config(void *app_conf)
{
	struct APP_CONF *task_conf = (struct APP_CONF *) app_conf;

	if (task_conf->comm_port < 1)
		task_conf->comm_port = APP_COMM_PORT_DEFAULT;

	return TASK_OK;
}



int task_run(void *app_conf)
{
	int i;

	task_check_config(app_conf);

	struct APP_CONF *task_conf = (struct APP_CONF *) app_conf;
	struct TASK_ENV task_env;

	//+ init functions
	task_func_init();

	//+ init env
	if (task_init(task_conf, &task_env)) {
		printf("(error) init failed.\n");
		return TASK_ERR_INIT;
	}

	//+ main loop
	for(i = 0; ; i ++) {

		if (FLAG_APP_EXIT)
			break;

		if (task_env.data.bb_shm->seq != task_env.data.bb.seq) {
			memcpy(&task_env.data.bb, task_env.data.bb_shm, sizeof(task_env.data.bb));
			if (task_conf->debug)
				printf("1> Baseband: Updated (%d)\n", i);
		} else {
			printf("1> @ Baseband: NOT updated (%d)\n", i);
		}
		if (task_env.data.hw_shm->seq != task_env.data.hw.seq) {
			memcpy(&task_env.data.hw, task_env.data.hw_shm, sizeof(task_env.data.hw));
			if (task_conf->debug)
				printf("1> Hardware: Updated (%d)\n", i);
		} else {
			printf("1> @ Hardware: NOT updated (%d)\n", i);
		}

		//+ enter TPC algorithm
		(*task_core)(&task_env);


		if (task_conf->debug)
			printf("-------- -------- -------- -------- (%d) --------\n", i);

		//+ enter TPC idle
		(*task_idle)(task_conf->intl);
	}


	//+ free up
	if (task_conf->debug)
		printf("* clean up before exit\n");

	msg_free(APP_VAL_INVALID);
	shm_free(APP_VAL_INVALID, task_env.data.bb_shm);
	shm_free(APP_VAL_INVALID, task_env.data.hw_shm);

	return TASK_OK;
}



/*
 * init socket
 * init bb shm, bb shm id
 * init hw shm, hw shm id
 * init cmd msg id
 */
static int task_init(const void *conf, void *env)
{
	struct APP_CONF *task_conf = (struct APP_CONF *) conf;
	struct TASK_ENV *task_env = (struct TASK_ENV *) env;

	//memcpy(&task_env->conf, conf, sizeof(struct APP_CONF));


	//+ clear before use
	memset(task_env, 0, sizeof(struct TASK_ENV));

	if (task_env->conf.intl < TASK_CONF_INTL_MIN)
		task_env->conf.intl = TASK_CONF_INTL_MIN;
	task_env->conf.algorithm = task_conf->algorithm;
	task_env->conf.txpwr_min = task_conf->txpwr_min;
	task_env->conf.txpwr_max = task_conf->txpwr_max;
	task_env->conf.txpwr_normal = task_conf->txpwr_normal;

	task_env->conf.rxgain_max_near = task_conf->rxgain_max_near;
	task_env->conf.rxgain_max = task_conf->rxgain_max;
	task_env->conf.rxgain_normal = task_conf->rxgain_normal;

	snprintf(task_env->wls_mac, sizeof(task_env->wls_mac),
			"%s", get_mac(TASK_CONF_WMAC_IFNAME));

	task_env->ipc.bb_shm_id = APP_VAL_INVALID;
	task_env->ipc.hw_shm_id = APP_VAL_INVALID;
	task_env->ipc.cmd_msg_id = APP_VAL_INVALID;


	//+ init socket
	int num = 1;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(task_conf->comm_port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	task_env->comm.remote_port = task_conf->comm_port;
	task_env->comm.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (task_env->comm.sockfd < 0) {
		printf("#### failed to init sock: port = %d ####\n", task_conf->comm_port);
		return TASK_ERR_SOCKET;
	}

	if (bind(task_env->comm.sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		return TASK_ERR_SOCKET_BIND;
	}
	if (setsockopt(task_env->comm.sockfd, SOL_SOCKET, SO_REUSEADDR,
			&num, sizeof(int)) < 0) {
		return TASK_ERR_SOCKET_SETSOCKOPT;
	}
	if (task_conf->debug)
		printf("0> socket ok (%d)\n", task_env->comm.sockfd);

	//+ init bb shm
	if (shm_init((char *) GWS_CONF_BB_SHM_KEY, &task_env->ipc.bb_shm_id,
			(void **) &task_env->data.bb_shm, sizeof(struct gwsbb))) {
		printf("#### quit (reason: fail to init bb shm) ####\n");
		return TASK_ERR_BB_SHM;
	}
	if (task_conf->debug)
		printf("0> bb shm ok (%d)\n", task_env->ipc.bb_shm_id);


	//+ init hw shm
	if (shm_init((char *) GWS_CONF_HW_SHM_KEY, &task_env->ipc.hw_shm_id,
			(void **) &task_env->data.hw_shm, sizeof(struct gwshw))) {
		printf("#### quit (reason: fail to init hw shm) ####\n"); //perror("shmget");
		return TASK_ERR_HW_SHM;
	}
	if (task_conf->debug)
		printf("0> hw shm ok (%d)\n", task_env->ipc.hw_shm_id);

	if (msg_init((char *) GWS_CONF_MSG_KEY, &task_env->ipc.cmd_msg_id)) {
		printf("#### quit (reason: fail to init hw msg) ####\n");
		return TASK_ERR_MSG;
	}
	if (task_conf->debug)
		printf("0> hw msg ok (%d)\n", task_env->ipc.cmd_msg_id);

	return TASK_OK;
}


/*
 * SIGINT, SIGQUIT, SIGTERM: clean first
 */
void task_prepare_exit(void)
{
	printf("\n* SIGNAL caught, prepare to exit!\n");

	// TODO: syslog() signal exit
	FLAG_APP_EXIT = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}


