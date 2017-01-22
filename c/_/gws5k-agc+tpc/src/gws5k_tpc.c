/*
 * gws5k_tpc.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "six.h"

#include "grid.h"

#include "app.h"
#include "task.h"
#include "gws5k_ctrl.h"

#include "gws5k_tpc.h"


//static int gws5k_tpc_conf_check(const void *task_env);

static int gws5k_tpc_standby(const void *task_env);
static int gws5k_tpc_single(const void *task_env);
static int gws5k_tpc_multi(const void *task_env);
static int gws5k_tpc_adjust(const void *task_env);
static int gws5k_tpc_report(const void *task_env);

static int global_remote_snr, global_remote_txpwr, global_remote_rxgain;


/*
 * judge online/offline
 * judge vs 1 or vs multi
 *
 * by Qige Zhao <qige@6harmonics.com>
 * Algorithm I (No AGC):
 * conditions:
 * 1. very near (snr >= 40 && txpwr = txpwr_min):
 * 		free tx mcs;
 * 		turn rxgain until -30 (step -6, min is -32);
 *
 * 2. normal:
 * 		free tx mcs;
 * 		snr >= 50, turn remote txpower, step -12;
 * 		snr >= 40, turn remote txpower, step -6;
 * 		snr >= 30, keep it;
 * 		snr >= 20, turn remote txpower, step 6;
 *
 * 		snr >= 10, turn remote txpower, step 12, set tx mcs = 1;
 *
 * 	3. too far (snr <= 20 && remote txpwr == txpwr_max):
 * 		turn rxgain until -10 (step 12, max is -10);
 * 		set tx mcs = 1;
 *
 *
 * Algorithm III (No AGC):
 * conditions:
 * 1. very near (remote txpwr <= 11)
 * 		free tx mcs;
 * 		turn rxgain until -30 (step -6, min is -32);
 *
 * 2. normal (remote txpwr <= 36, snr >= 20):
 * 		free tx mcs;
 * 		remote txpwr up step 10;
 * 		remote txpwr down step -6;
 * 		turn rxgain until -10 (step 3/-3);
 *
 * 3. far (remote txpwr is max, snr < 20)
 * 		set tx mcs = 1;
 * 		turn rxgain until 10 (step 10, max is 10);
 */
int gws5k_tpc_run(const void *task_env)
{
	struct TASK_ENV *tpc_env = (struct TASK_ENV *) task_env;

	//gws5k_tpc_conf_check(task_env);

	DBG_TPC(" bb.seq = %ld, hw.seq = %ld\n",
			tpc_env->data.bb.seq,
			tpc_env->data.hw.seq);

	if (tpc_env->data.bb.peers_qty > 0 ||
			tpc_env->data.bb.avg.signal > tpc_env->data.bb.avg.noise)
	{
		switch(tpc_env->data.bb.nl_mode) {
			case GWS_MODE_ARN_EAR:
				gws5k_tpc_single(task_env);
				break;
			case GWS_MODE_ARN_CAR:
			case GWS_MODE_MESH:
			default:
				gws5k_tpc_multi(task_env);
				break;
		}
	} else {
		gws5k_tpc_standby(task_env);
	}

	return TPC_OK;
}

static int gws5k_tpc_standby(const void *task_env)
{
	DBG_TPC(" enter STANDBY\n");

	if (gws5k_ctrl_range_max(task_env))
		return TPC_ERR_STANDBY;

	return TPC_OK;
}

static int gws5k_tpc_single(const void *task_env)
{
	int ret = TPC_OK;
	DBG_TPC(" enter SINGLE\n");

	struct TASK_ENV *tpc_env = (struct TASK_ENV *) task_env;


	// FIXME: recv & adjust first
	if (gws5k_tpc_adjust(task_env)) {
		printf("(warning) NO data received.\n");
	}

	if (gws5k_tpc_report(task_env)) {
		printf("(warning) NO data sent.\n");
		ret = TPC_ERR_REPORT;
	}

	// FIXME: when traffic sucks, turn up txpwr, turn up rxgain
	if (ret != TPC_OK) {
		gws5k_set_txpower(tpc_env->data.hw.txpower, 10, tpc_env->conf.txpwr_max);
	}

	return ret;
}

static int gws5k_tpc_multi(const void *task_env)
{
	DBG_TPC(" enter MULTI2MULTI\n");

	// TODO: handle 1 vs multi EARs/STAs/Mesh Nodes
	gws5k_tpc_single(task_env);

	return TPC_OK;
}

/*
 * recv data via socket
 * parse data into values
 * verify if for "me"
 * compare & adjust
 */
static int gws5k_tpc_adjust(const void *task_env)
{
	struct TASK_ENV *tpc_env = (struct TASK_ENV *) task_env;

	int recv_bytes;
	char buffer[128];

	int addr_length;
	struct sockaddr_in addr;

	int txpwr_step, txmcs_val, txmcs_val_last;
	char dmac[17], smac[18];

	do {
		recv_bytes = 0;
		memset(buffer, 0, sizeof(buffer));

		addr_length = sizeof(addr);
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(tpc_env->comm.remote_port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		recv_bytes = recvfrom(tpc_env->comm.sockfd, buffer, sizeof(buffer)-1, MSG_DONTWAIT,
				(struct sockaddr *) &addr, (size_t *) &addr_length);
		printf("--> recv %d bytes\n", recv_bytes);
		if (recv_bytes <= 0) {
			break;
		}

		//sprintf(buffer, "AC:EE:3B:D0:02:17 AC:EE:3B:D0:00:26 55 33 -15");
		printf("---> (remote to local) report: %s\n", buffer);

		global_remote_snr = 0; global_remote_txpwr = 0; global_remote_rxgain = 0;
		memset(dmac, 0, sizeof(dmac));
		memset(smac, 0, sizeof(smac));
		if (sscanf(buffer, "%s %s %d %d %d", dmac, smac, &global_remote_snr, &global_remote_txpwr, &global_remote_rxgain) == -1) {
			continue;
		}

		txpwr_step = 0;
		txmcs_val = 0;
		txmcs_val_last = GWS5K_CONF_TXMCS_WHEN_READY;


		if (strstr(dmac, tpc_env->wls_mac) > 0) {
			printf("### Remote: %d, (rxgain %d)\n", global_remote_snr, global_remote_rxgain);
			printf("###  Local: %d dBm\n", tpc_env->data.hw.txpower);

			switch(global_remote_snr/10) {
			case 2:
			case 1:
			case 0:
				txmcs_val = GWS5K_CONF_TXMCS_WHEN_FAR;
				break;
			default:
				txmcs_val = GWS5K_CONF_TXMCS_WHEN_READY;
				break;
			}

			//+ save smac, save snr
			switch(global_remote_rxgain/10) {
			case 2:
			case 1:
			case 0:
				printf("----> Too LOW! (remote snr = %d)\n", global_remote_snr);
				txpwr_step = GWS5K_TPC_CONF_ADJ_STEP * 3;
				break;
			case -1:
			case -2:
				printf("----> Too STRONG! (remote snr = %d)\n", global_remote_snr);
				txpwr_step = 0 - GWS5K_TPC_CONF_ADJ_STEP * 2;
				break;
			default:
				break;
			}


			if (txpwr_step != 0) {
				gws5k_set_txpower(tpc_env->data.hw.txpower, txpwr_step, tpc_env->conf.txpwr_max);
			}

			if (txmcs_val_last != txmcs_val) {
				gws5k_set_mcs(txmcs_val);
			}

			txmcs_val_last = txmcs_val;
		}

	} while(recv_bytes > 0);

	return TPC_OK;
}

static int gws5k_tpc_report(const void *task_env)
{
	int i, snr[GWS_CONF_BB_STA_MAX], snr_min, snr_max;
	char data[64], remote[16];

	struct TASK_ENV *tpc_env = (struct TASK_ENV *) task_env;

	int sent_bytes;
	struct sockaddr_in addr;

	memset(&snr, 0, sizeof(snr));
	snr_min = 0;
	snr_max = 0;
	for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
		if (strlen(tpc_env->data.bb.peers[i].mac) > 0) {

			snr[i] = tpc_env->data.bb.peers[i].signal - tpc_env->data.bb.peers[i].noise;
			DBG_TPC(" peer%d: %s, %d/%d/%d, %d ms\n",
					i, tpc_env->data.bb.peers[i].mac,
					tpc_env->data.bb.peers[i].signal, tpc_env->data.bb.peers[i].noise,
					snr[i],
					tpc_env->data.bb.peers[i].inactive);

			//+ find ip address: "rarp <wmac>"
			memset(data, 0, sizeof(data));
			memset(remote, 0, sizeof(remote));
			snprintf(data, sizeof(data)-1, GWS5K_RARP_FORMAT, tpc_env->data.bb.peers[i].mac);
			snprintf(remote, sizeof(remote)-1, "%s", cli_read_line(data));

			DBG_TPC(" peer%d: %s\n", i, remote);

			//+ send report
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(tpc_env->comm.remote_port);
			addr.sin_addr.s_addr = inet_addr(remote);
			if (addr.sin_addr.s_addr == INADDR_NONE) {
				return TPC_ERR_REPORT_SENDTO_TARGET;
			}

			memset(data, 0, sizeof(data));
			snprintf(data, sizeof(data)-1, "%s %s %d %d %d",
					tpc_env->data.bb.peers[i].mac,
					tpc_env->wls_mac,
					snr[i],
					tpc_env->data.hw.txpower,
					tpc_env->data.hw.rxgain);

			printf("---> (local to remote) report: %s\n", data);
			sent_bytes = sendto(tpc_env->comm.sockfd, data, strlen(data),
					0, (struct sockaddr *) &addr, sizeof(addr));
			if (sent_bytes <= 0) {
				printf(" (warning) send report to remote failed (%d/%s:%d)\n",
						tpc_env->comm.sockfd, remote, tpc_env->comm.remote_port);
				return TPC_ERR_REPORT_SENDTO;
			}
			printf("--> sent %d bytes to remote %s:%d\n",
					sent_bytes, remote, tpc_env->comm.remote_port);

			if (snr_min == 0 || snr[i] < snr_min) {
				snr_min = snr[i];
			}
			if (snr[i] > snr_max) {
				snr_max = snr[i];
			}
		}
		break;
	}

	return TPC_OK;
}


void gws5k_tpc_idle(const int intl_sec)
{
	sleep(intl_sec);
}
