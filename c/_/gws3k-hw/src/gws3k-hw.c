/*
 * gws3k-hw.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: July 8, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "uart.h"
#include "lbb.h"
#include "shm.h"
#include "msg.h"
#include "gwshw_cli.h"

#include "grid.h"
#include "gwshw.h"
#include "gwshw_data.h"


static void mark_exit(void);
static void print_version(void);
static void print_help(const char *app);

static int  gwshw_run(const char *uart, int F, const int D);
static int  gwshw_msg_handle(const int uartfd, const int msgid, int obj, const void *hw);
static void gwshw_idle(void);

static int  gwshw_setmode(const int mode);


static int GWSHW_QUIT_FLAG = 0;

/*
 * struct gwsbb -> seq (mark data sequence)
 */
static unsigned long gwshw_data_seq = 0;


/*
 * GWS Hardware Data Collector
 * - publish data via SHM
 * - publish data via /tmp/run/gws_hw* files
 */
int main(int argc, char **argv)
{
	signal(SIGINT, 	(__sighandler_t) mark_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) mark_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) mark_exit); //+ "kill", not "kill -9"

	signal(SIGCHLD, SIG_IGN);


	int ret = GWS_HW_OK;
	char opt = 0;
	int D = 0, V = 0, F = 0, H = 0;
	char SP[GWS_CHK_PARAM_LENMAX];

	memset(SP, 0x00, sizeof(SP));
	while((opt = getopt(argc, argv, "k:dD:Fhv")) != -1) {
		switch(opt) {
		case 'k':
			printf("key = %s\n", optarg);
			break;
		case 'd':
			D = 1;
			break;
		case 'h':
			H = 1;
			break;
		case 'v':
			V = 1;
			break;
		case 'F':
			F = 2;
			break;
		case 'D':
			if (strlen(optarg) <= GWS_CHK_PARAM_LENMAX) {
				strcpy(SP, optarg);
			}
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

	if (!strlen(SP)) {
		snprintf(SP, sizeof(SP), "%s", GWS_CONF_HW_UART);
	}
	ret = gwshw_run(SP, F, D);

	return ret;
}


static int gwshw_run(const char *uart, int F, const int D)
{
	int ret = GWS_HW_OK;

	int f = 0;

	int hw_shmid = GRID_CONF_VAL_INVALID, msgid = GRID_CONF_VAL_INVALID;
	struct gwshw hw, *hw_shm = NULL;

	int uartfd = GRID_CONF_VAL_INVALID;
	uartfd = uart_open(uart); // FIXME: uartfd=1?
	if (uartfd < 1) {
		printf("failed to open device: %s\n", uart);
		return GWS_HW_ERR_UART;
	}
	if (D) printf("-> uart ok (%s/%d)\n", uart, uartfd);

	if (shm_init((char *) GWS_CONF_HW_SHM_KEY, &hw_shmid, (void **) &hw_shm,
			sizeof(struct gwshw))) {
		printf("* quit (reason: fail to init shm)\n"); perror("shmget");
		return GWS_HW_ERR_SHM;
	}
	if (D) printf("-> hw_shm ok (%d)\n", hw_shmid);

	if (msg_init((char *) GWS_CONF_MSG_KEY, &msgid)) {
		printf("* quit (reason: fail to init msg)\n");
		return GWS_HW_ERR_MSG;
	}
	if (D) printf("-> msg ok (%d)\n", msgid);


	// TODO: syslog() start

	int i, fail_counter = 0;
	char cmd[GWS_CONF_HW_CMD_LENGTH];
	static struct lbb lb; //* loopback buffer for uart_recv();

	lbb_init(&lb);

	memset(&hw, 0x00, sizeof(struct gwshw));
	memset(hw_shm, 0x00, sizeof(struct gwshw));

	for(i = 0;; i ++) {
		gwshw_msg_handle(uartfd, msgid, GWS_MSG_COM_HW, hw_shm);

		//* quit when user press ^C
		if (GWSHW_QUIT_FLAG) break;

		//* check init values of rfinfo
		memset(cmd, 0x00, sizeof(cmd));
		if (strlen(hw_shm->fw_ver) < 1
				|| i < GWS_CONF_HW_INITBAR*3
				|| i % 8 < GWS_CONF_HW_INITBAR)
		{
			snprintf(cmd, sizeof(cmd), "%s", GWS_CONF_HW_INIT);
			printf("-> query init\n");
		} else {
			snprintf(cmd, sizeof(cmd), "%s", GWS_CONF_HW_QUERY);
			printf("-> query minimum\n");
		}

		//* data renew process:
		//* - read/save/fetch data from uart
		//* - parse/save rfinfo data into shm/ofile
		if (gwshw_data_update(uartfd, &hw, &lb, cmd)) {
			printf("\n * NO DATA available!\n");
			QZ("-> * data renew failed\n");
			fail_counter ++;
			if (fail_counter >= GWS_CONF_HW_FAILBAR) {
				// FIXME: clear when cannot parse result?
				gwshw_data_clean(&hw);
				ret = GWS_HW_ERR_DATA;
			}
		} else {
			printf("-> hardware data updated! (seq %ld)\n", gwshw_data_seq);
			gwshw_data_seq ++;
			hw.seq = gwshw_data_seq;

			gwshw_data_save((void *) &hw, (void *) hw_shm);
			if (D) printf("core > data updated\n");
			fail_counter = 0;
		}

		gwshw_data_save2file((void *) hw_shm);
		if (D) gwshw_data_print((void *) hw_shm);

		if (F > 0) {
			gwshw_data_selfcheck(uartfd);
			F --;
		}

		printf("-------- -------- %05d -------- --------\n", i);
		f ++;
		gwshw_idle(); //break;
	}

	//* clean up
	uart_close(uartfd); 
	uartfd = GRID_CONF_VAL_INVALID;

	shm_free(hw_shmid, hw_shm); 
	hw_shmid = GRID_CONF_VAL_INVALID; 
	hw_shm = NULL;

	msg_free(msgid); 
	msgid = GRID_CONF_VAL_INVALID;

	// TODO: syslog() exit

	return ret;
}


/*
 * read and exec cmd from msg
 */
static int gwshw_msg_handle(const int uartfd, const int msgid, int obj, const void *hw)
{
	//* read all msg for hw
	unsigned int msg_length;
	struct GWS_MSG msg;

	struct gwshw *ghw = (struct gwshw *) hw;

	int x, step;
	char cmd[32];

	do {
		memset(&msg, 0, sizeof(msg));
		msg_length = sizeof(msg);

		if (msg_recv(msgid, &msg, &msg_length, obj)) {
			break;
		}

		printf("---> !! MSG/CMD Recv: %d(%d/%s)\n",
				msg.msg.cmd, msg.msg.num, msg.msg.data);

		memset(cmd, 0, sizeof(cmd));
		switch(msg.msg.cmd) {
		case GWS_MSG_CMD_SET_HW_REGION:
			if (msg.msg.num > 0) {
				sprintf(cmd, "setregion 1\n");
			} else {
				sprintf(cmd, "setregion 0\n");
			}
			uart_write(uartfd, cmd, strlen(cmd));
			break;
		case GWS_MSG_CMD_SET_HW_CHANNEL:
			sprintf(cmd, "setchan %d\n", msg.msg.num);
			uart_write(uartfd, cmd, strlen(cmd));
			break;
		case GWS_MSG_CMD_SET_HW_TXCHAIN:
			if (msg.msg.num > 0) {
				sprintf(cmd, "txon\n");
			} else {
				sprintf(cmd, "txoff\n");
			}
			uart_write(uartfd, cmd, strlen(cmd));
			break;
		case GWS_MSG_CMD_SET_HW_TXPOWER:
			if (msg.msg.num > 0) {
				sprintf(cmd, "settxpwr %d\ntxon\n", msg.msg.num*2);
			} else {
				sprintf(cmd, "settxpwr %d\ntxoff\n", 0);
			}
			uart_write(uartfd, cmd, strlen(cmd));
			break;
		case GWS_MSG_CMD_SET_HW_RXGAIN:
			if (msg.msg.num > 0) {
				sprintf(cmd, "setrxgain %d\n", msg.msg.num*2);
				uart_write(uartfd, cmd, strlen(cmd));
			}
			break;

		case GWS_MSG_CMD_SET_BB_MODE:
			gwshw_setmode(msg.msg.num);
			break;

		case GWS_MSG_CMD_ADJ_HW_TXPWR:
			x = ghw->txpower+msg.msg.num;
			if (x > 33)
				x = 33;
			if (x < 0)
				x = 1;
			if (x != ghw->txpower && x > 0 && x <= 33) {
				sprintf(cmd, "settxpwr %d\n", x*2);
				uart_write(uartfd, cmd, strlen(cmd));
			}
			break;
		case GWS_MSG_CMD_ADJ_HW_RXGAIN:
			x = ghw->rxgain+msg.msg.num;
			if (x < 0)
				x = 0;
			if (x > 14)
				x = 14;
			if (x != ghw->rxgain && x >= 0 && x <= 14) {
				sprintf(cmd, "setrxgain %d\n", x*2);
				uart_write(uartfd, cmd, strlen(cmd));
			}
			break;

		case GWS_MSG_CMD_DFL_HW_TXPOWER:
			x = GWS_CONF_FW_DFL_TXPOWER;
			sprintf(cmd, "settxpwr %d\n", x*2);
			uart_write(uartfd, cmd, strlen(cmd));
			break;

		case GWS_MSG_CMD_DFL_HW_RXGAIN:
			step = msg.msg.num;
			x = ghw->rxgain;

			//+ current rxgain between 12+-3, set to 12;
			//+ else, +-3 to close 12.
			if (x <= GWS_CONF_FW_DFL_RXGAIN - step) {
				x += step;
			} else if (x >= GWS_CONF_FW_DFL_RXGAIN + step) {
				x -= step;
			} else {
				x = GWS_CONF_FW_DFL_RXGAIN;
			}

			if (x != ghw->rxgain && x >= 0 && x <= 14) {
				sprintf(cmd, "setrxgain %d\n", x*2);
				uart_write(uartfd, cmd, strlen(cmd));
			}
			break;

		default:
			break;
		}
	} while(msg.msg.cmd > 0);

	return GWS_HW_OK;
}

static int gwshw_setmode(const int mode)
{
	switch(mode) {
	case GWS_MODE_MESH:
		exec_cmd("arn_mesh\n");
		break;
	case GWS_MODE_ARN_CAR:
		exec_cmd("arn_car\n");
		break;
	case GWS_MODE_ARN_EAR:
		exec_cmd("arn_ear\n");
		break;
	}
	return GWS_HW_OK;
}

static void gwshw_idle(void)
{
	sleep(GWS_CONF_HW_IDLE);
}



static void print_version(void)
{
	printf("%s\n(%s)\n", APP_DESC, APP_VERSION);
}

/*
 * SIGINT, SIGQUIT, SIGTERM: clean first
 */
static void mark_exit(void)
{
	printf("\n exiting %s ...\n", APP_DESC);
	GWSHW_QUIT_FLAG = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

static void print_help(const char *app)
{
	printf("  usage: %s [-i ifname] [-v] [-h]\n", app);
}
