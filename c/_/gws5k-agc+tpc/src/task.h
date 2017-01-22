/*
 * task.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 7, 2016
 *  Updated on: Nov 11, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef TASK_H_
#define TASK_H_

#include "grid.h"

//+ MIN intl: second(s)
#define TASK_CONF_INTL_MIN		1       // v10.0.111116
#define TASK_CONF_WMAC_IFNAME		"wlan0"


enum TASK_ERR {
	TASK_OK = 0,
	TASK_ERR_INIT = -10,
	TASK_ERR_SOCKET,
	TASK_ERR_BB_SHM,
	TASK_ERR_HW_SHM,
	TASK_ERR_MSG,
	TASK_ERR_SOCKET_BIND = -500,
	TASK_ERR_SOCKET_SETSOCKOPT,
	TASK_ERR_,
	TASK_ERR_x
};

struct TASK_ENV {
	char wls_mac[18];
	struct {
		int bb_shm_id, hw_shm_id, cmd_msg_id;
	} ipc;
	struct {
		int sockfd;
		char remote[GWS_CONF_BB_STA_MAX][16];
		int remote_port;
	} comm;
	struct {
		int snr;
		struct gwsbb bb, *bb_shm;
		struct gwshw hw, *hw_shm;
	} data;
	struct APP_CONF conf;
};



int  task_run(void *app_conf);
void task_prepare_exit(void);


#endif /* TASK_H_ */
