/*
 * grid-wb.c
 *
 *  Created on: May 12, 2016
 *  Updated on: July 15, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include "grid-wb.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include "six.h"


#include "grid.h"

#include "grid_wb_cmd.h"
#include "grid_wb_rpt.h"



static void task_prepare_exit(void);
static void print_version(void);
static void print_help(const char *app);

static int  task_run(const char *uart, const int intl, int D);
static void task_idle(const int intl);


static int FLAG_GRID_WB_QUIT = 0;


/*
 * GWS Hardware Data Collector
 * - publish data via SHM
 * - publish data via /tmp/run/gws_hw* files
 */
int main(int argc, char **argv)
{
	signal(SIGINT, 	(__sighandler_t) task_prepare_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) task_prepare_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) task_prepare_exit); //+ "kill", not "kill -9"

	signal(SIGCHLD, SIG_IGN);


	int ret = GRID_WB_OK;
	char opt = 0;
	unsigned int D = 0, V = 0, H = 0, I = 1000;
	char uart[GRID_WB_CONF_PARAM_LENMAX];

	memset(uart, 0x00, sizeof(uart));
	while((opt = getopt(argc, argv, "s:vD:dh")) != -1) {
		switch(opt) {
		case 's':
			I = atoi(optarg);
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
		case 'D':
			strncpy(uart, optarg, sizeof(uart)-1);
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

	if (!strlen(uart)) {
		snprintf(uart, sizeof(uart), "%s", GRID_WB_CONF_HW_UART);
	}

	if (I < GRID_WB_CONF_HW_IDLE) I = GRID_WB_CONF_HW_IDLE;


	ret = task_run(uart, I, D);

	return ret;
}


static int task_run(const char *uart, const int intl, int D)
{
	int i, j;

	int bb_shmid = GRID_CONF_VAL_INVALID;
	int hw_shmid = GRID_CONF_VAL_INVALID;
	int cmd_msgid = GRID_CONF_VAL_INVALID;

	struct gwsbb bb, *bb_shm;
	struct gwshw hw, *hw_shm;

	int uartfd = GRID_CONF_VAL_INVALID;

	//+ init uart/sp
	uartfd = uart_open(uart); // FIXME: uartfd=1?
	if (uartfd < 1) {
		printf("failed to open device: %s\n", uart);
		return GRID_WB_ERR_UART;
	}
	if (D) printf("-> uart ok (%s/%d)\n", uart, uartfd);


	//+ init bb shm
	bb_shm = NULL;
	if (shm_init((char *) GWS_CONF_BB_SHM_KEY, &bb_shmid, (void **) &bb_shm, sizeof(struct gwsbb))) {
		printf("* quit (reason: fail to init bb shm)\n");
		return GRID_WB_ERR_BB_SHM;
	}
	if (D) printf("-> bb shm ok (%d)\n", bb_shmid);


	//+ init hw shm
	hw_shm = NULL;
	if (shm_init((char *) GWS_CONF_HW_SHM_KEY, &hw_shmid, (void **) &hw_shm, sizeof(struct gwshw))) {
		printf("* quit (reason: fail to init hw shm)\n"); //perror("shmget");
		return GRID_WB_ERR_HW_SHM;
	}
	if (D) printf("-> hw shm ok (%d)\n", hw_shmid);

	if (msg_init((char *) GWS_CONF_MSG_KEY, &cmd_msgid)) {
		printf("* quit (reason: fail to init hw msg)\n");
		return GRID_WB_ERR_HW_MSG;
	}
	if (D) printf("-> hw msg ok (%d)\n", cmd_msgid);


	//+ main loop
	grid_wb_cmd_init(); //+ init lbb for sp/uart recv
	memset(&bb, 0, sizeof(bb));
	memset(&hw, 0, sizeof(hw));
	for(i = 0, j = 0; ; i ++) {
		if (FLAG_GRID_WB_QUIT) break;

		if (bb_shm->seq != bb.seq) {
			//bb.seq = bb_shm->seq;
			memcpy(&bb, bb_shm, sizeof(bb));
			if (D) printf("--> bb data updated (%d)\n", i);
		} else {
			printf("--> bb data not refreshed (%d)\n", i);
		}
		if (hw_shm->seq != hw.seq) {
			//hw.seq = hw_shm->seq;
			memcpy(&hw, hw_shm, sizeof(hw));
			if (D) printf("--> hw data updated (%d)\n", i);
		} else {
			printf("--> hw data not refreshed (%d)\n", i);
		}

		//+ handle cmd from "ipc/msg"
		grid_wb_cmd_handler(uartfd, cmd_msgid, &bb, &hw);
		grid_wb_reply_handler(uartfd, cmd_msgid);

		//+ send +wbsts
		grid_wb_rpt_sts(uartfd, bb_shm, hw_shm, D);
		if (D) printf("--> +wbsts sent (%d)\n", i);
		//+ send +wbsigs
		grid_wb_rpt_sigs(uartfd, bb_shm, hw_shm, D);
		if (D) printf("--> +wbsigs sent (%d)\n", i);

		j ++;

		task_idle(intl);
	}

	//+ free up
	msg_free(GRID_CONF_VAL_INVALID);
	shm_free(GRID_CONF_VAL_INVALID, bb_shm);
	shm_free(GRID_CONF_VAL_INVALID, hw_shm);

	return GRID_WB_OK;
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
	// TODO: syslog() signal exit
	FLAG_GRID_WB_QUIT = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

static void print_version(void)
{
	printf("%s\n(%s)\n", APP_DESC, APP_VERSION);
}
static void print_help(const char *app)
{
	printf("  usage: %s [-s intl] [-D dev]\n", app);
	printf("         %s [-d] [-v] [-h]\n", app);
}
