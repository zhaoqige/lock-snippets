/*
 * gws.c: for gws5000
 *
 *  Created on: May 20, 2016
 *  Updated on: July 15, 2016
 *  Updated on: Oct 27, 2017
 *  Updated on: Nov 9, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include "six.h"


#include "grid.h"
#include "gws.h"

//#include "grid3k_cmd.h"
//#include "grid3k_rpt.h"



static void task_prepare_exit(void);
static void print_version(void);
static void print_help(const char *app);

static int  task_run(const int times, const int intl, const int cmd, const int val, const int D);
static void task_idle(const int intl);

static void task_bb_print(void *gwsbb);
static void task_hw_print(void *gwshw);

static char *get_bbmode(const int bbmode);

static int FLAG_APP_QUIT = 0;


/*
 * GWS3000 Controller
 * - read user input
 * - init shm/msg
 * - task main loop
 * -- send cmd
 * -- read status
 * -- idle
 * - free & exit
 */
int main(int argc, char **argv)
{
	signal(SIGINT, 	(__sighandler_t) task_prepare_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) task_prepare_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) task_prepare_exit); //+ "kill", not "kill -9"

	signal(SIGCHLD, SIG_IGN);


	int ret = ERR_NONE;
	char opt = 0;
	int cmd = 0, val = 0;
	unsigned int D = 0, V = 0, H = 0, T = 1, S = GRID_CONF_TASK_IDLE;

	while((opt = getopt(argc, argv, "C:R:P:G:T:t:s:vdh")) != -1) {
		switch(opt) {
		case 'C':
			cmd = GWS_MSG_CMD_SET_HW_CHANNEL;
			val = atoi(optarg);
			break;
		case 'R':
			cmd = GWS_MSG_CMD_SET_HW_REGION;
			val = atoi(optarg);
			break;
		case 'P':
			cmd = GWS_MSG_CMD_SET_HW_TXPOWER;
			val = atoi(optarg);
			break;
		case 'G':
			cmd = GWS_MSG_CMD_SET_HW_RXGAIN;
			val = atoi(optarg);
			break;
		case 'T':
			cmd = GWS_MSG_CMD_SET_HW_TXCHAIN;
			val = atoi(optarg);
			break;
		case 's':
			S = atoi(optarg);
			break;
		case 't':
			T = atoi(optarg);
			break;
		case 'v':
			V = 1;
			break;
		case 'h':
			H = 1;
			break;
		case 'd':
			D = 1;
			break;
		default:
			break;
		}
	}

	if (H) {
		//print_version();
		print_help(argv[0]);
		return 0;
	}

	if (V) {
		print_version();
		return 0;
	}

	if (S < GRID_CONF_TASK_IDLE)
		S = GRID_CONF_TASK_IDLE;

	//+ enter main task
	ret = task_run(T, S, cmd, val, D);

	return ret;
}


static int  task_run(const int times, const int intl,
		const int cmd, const int val, const int D)
{
	int i, bb_seq;

	int bb_shmid = GRID_CONF_VAL_INVALID;
	int hw_shmid = GRID_CONF_VAL_INVALID;
	int cmd_msgid = GRID_CONF_VAL_INVALID;

	struct gwsbb *bb_shm;
	struct gwshw *hw_shm;

	char ip[18],mac[18];
	memset(ip, 0, sizeof(ip));
	memset(mac, 0, sizeof(mac));
	snprintf(ip, sizeof(ip), "%s", get_ip(GRID_CONF_IF_IP));
	snprintf(mac, sizeof(mac), "%s", get_mac(GRID_CONF_IF_WMAC));
	printf("> local: %s/%s\n\n", ip, mac);


	//+ init bb shm
	bb_shm = NULL;
	if (shm_init((char *) GWS_CONF_BB_SHM_KEY, &bb_shmid, (void **) &bb_shm, sizeof(struct gwsbb))) {
		printf("* quit (reason: fail to init bb shm)\n");
		return ERR_IPC_SHM_BB;
	}
	if (D)
		printf("-> bb shm ok (%d)\n", bb_shmid);


	//+ init hw shm
	hw_shm = NULL;
	if (shm_init((char *) GWS_CONF_HW_SHM_KEY, &hw_shmid, (void **) &hw_shm, sizeof(struct gwshw))) {
		printf("* quit (reason: fail to init hw shm)\n"); //perror("shmget");
		return ERR_IPC_SHM_RF;
	}
	if (D)
		printf("-> hw shm ok (%d)\n", hw_shmid);

	if (msg_init((char *) GWS_CONF_MSG_KEY, &cmd_msgid)) {
		printf("* quit (reason: fail to init hw msg)\n");
		return ERR_IPC_MSG;
	}
	if (D) printf("-> hw msg ok (%d)\n", cmd_msgid);

	//+ send command before anything
	if (cmd > 0) {
		struct GWS_MSG gmsg;
		memset(&gmsg, 0, sizeof(gmsg));
		gmsg.mtype = GWS_MSG_COM_HW;
		gmsg.msg.cmd = cmd;
		gmsg.msg.num = val;
		msg_send(cmd_msgid, &gmsg, sizeof(gmsg));
	}


	//+ main loop
	i = 0; bb_seq = 0;
	while(i < times) {

		if (FLAG_APP_QUIT)
			break;

		if (bb_shm->seq != bb_seq) {
			if (D)
				printf("--> bb data updated (%d|%ld)\n", i, bb_shm->seq);
			task_bb_print(bb_shm);
		} else {
			printf("--> bb data not refreshed (%d)\n", i);
		}
		bb_seq = bb_shm->seq;

		if (hw_shm->seq != 0) {
			if (D)
				printf("--> hw data updated (%d|%ld)\n", i, hw_shm->seq);
			task_hw_print(hw_shm);
		} else {
			printf("--> hw data not refreshed (%d)\n", i);
		}

		i ++;
		if (i >= times)
			break;

		printf("-------- -------- %d --------\n\n", i);
		task_idle(intl);
	}

	//+ free up
	msg_free(GRID_CONF_VAL_INVALID);
	shm_free(GRID_CONF_VAL_INVALID, bb_shm);
	shm_free(GRID_CONF_VAL_INVALID, hw_shm);

	return ERR_NONE;
}

static void task_bb_print(void *gwsbb)
{
	int i;
	struct gwsbb *bb = (struct gwsbb *) gwsbb;

	if (bb->avg.signal != GWS_CONF_BB_SIGNAL_UNKNOW) {
		printf(" Signal: %d/%d dBm (SNR %d), Mode: %s, BW: %d\n\n", 
			bb->avg.signal, bb->avg.noise, 
			bb->avg.signal - bb->avg.noise,
			get_bbmode(bb->nl_mode), bb->chanbw);
	} else {
		printf(" Signal: unknown/%d dBm (SNR 0), Mode: %s, BW: %d\n\n", 
			bb->avg.noise, get_bbmode(bb->nl_mode), bb->chanbw);
	}

	for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
		// 6.2-36
		if (bb->peers[i].noise && bb->peers[i].signal != GWS_CONF_BB_SIGNAL_UNKNOW) {
			printf("   STA%d: %s, %d/%d dBm (SNR %d, %u ms), \n"
					"     Rx: %d/%.2f/%llu, Tx: %d/%.2f/%llu\n",
					i,
					bb->peers[i].mac, bb->peers[i].signal, bb->peers[i].noise,
					bb->peers[i].signal - bb->peers[i].noise, bb->peers[i].inactive,
					bb->peers[i].rx_mcs, bb->peers[i].rx_br/1024, bb->peers[i].rx_pckts,
					bb->peers[i].tx_mcs, bb->peers[i].tx_br/1024, bb->peers[i].tx_pckts);
		}
	}
	printf("\n");
}

static void task_hw_print(void *gwshw)
{
	struct gwshw *hw = (struct gwshw *) gwshw;

	printf(" Region: %d\n", hw->region);
	printf("Channel: %d (%d MHz)\n", hw->channel,
			hw->region ? GRID_R1FREQ(hw->channel) : GRID_R0FREQ(hw->channel));
	printf("TxPower: %d dBm (TxAtten: %d)\n", hw->txpower, hw->adv_txatten);
	printf(" RxGain: %d dBm (AGC: %s, RxFAtten: %d, RxRAtten: %d)\n", 
		hw->rxgain, (hw->adv_rxcal ? "ON" : "OFF"),
		hw->adv_rxfatten, hw->adv_rxratten);
}

static void task_idle(const int intl)
{
	usleep(intl*1000);
}

/*
 * SIGINT, SIGQUIT, SIGTERM: clean first
 */
static void task_prepare_exit(void)
{
	FLAG_APP_QUIT = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

static void print_version(void)
{
	printf("%s\n(%s)\n", APP_DESC, APP_VERSION);
}
static void print_help(const char *app)
{
	printf("  usage: %s [-R region] [-C channel] [-T 1|0] [-P txpwr] [-G rxgain]\n", app);
	printf("         %s [-t times] [-s intl]\n", app);
	printf("         %s [-d] [-v] [-h]\n", app);
}


static char *get_bbmode(const int bbmode)
{
	switch(bbmode) {
	case GWS_MODE_ARN_CAR:
		return "CAR";
		break;
	case GWS_MODE_ARN_EAR:
		return "EAR";
		break;
	case GWS_MODE_MESH:
		return "Mesh Point";
		break;
	default:
		break;
	}
	return "unknown";
}
