/*
 * gws5k-hw.c
 *
 *  Created on: July 11, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include "six.h"

#include "grid.h"
#include "gws5k_hw.h"
#include "gws5k_hw_data.h"


static void mark_exit(void);
static void print_version(void);
static void print_help(const char *app);

static int  task_run(int F, const int D);
static int  task_msg_handle(const int msgid, int obj, const void *hw);
static void task_idle(void);

static int  task_setmode(const int mode);


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

	while((opt = getopt(argc, argv, "s:k:dFhv")) != -1) {
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

	ret = task_run(F, D);

	return ret;
}


static int task_run(int F, const int D)
{
	int ret = GWS_HW_OK;

	int f = 0;

	int hw_shmid = GRID_CONF_VAL_INVALID, msgid = GRID_CONF_VAL_INVALID;
	struct gwshw hw, *hw_shm = NULL;

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
		task_msg_handle(msgid, GWS_MSG_COM_HW, hw_shm);

		//* quit when user press ^C
		if (GWSHW_QUIT_FLAG)
			break;

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
		//* - read/save/fetch data from cli
		//* - parse/save rfinfo data into shm/ofile
		if (task_data_update(&hw, cmd)) {
			printf("\n * NO DATA available!\n");
			DBG_HW("-> * data renew failed\n");
			fail_counter ++;
			if (fail_counter >= GWS_CONF_HW_FAILBAR) {
				// FIXME: clear when cannot parse result?
				task_data_clean(&hw);
				ret = GWS_HW_ERR_DATA;
			}
		} else {
			printf("-> hardware data updated! (seq %ld)\n", gwshw_data_seq);
			gwshw_data_seq ++;
			hw.seq = gwshw_data_seq;

			task_data_save((void *) &hw, (void *) hw_shm);
			if (D) printf("core > data updated\n");
			fail_counter = 0;
		}

		task_data_save2file((void *) hw_shm);
		if (D)
			task_data_print((void *) hw_shm);

		if (F > 0) {
			task_data_selfcheck();
			F --;
		}

		printf("-------- -------- %05d -------- --------\n", i);
		f ++;
		task_idle(); //break;
	}

	//* clean up
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
static int task_msg_handle(const int msgid, int obj, const void *hw)
{
	//* read all msg for hw
	unsigned int msg_length;
	struct GWS_MSG msg;

	struct gwshw *ghw = (struct gwshw *) hw;

	int x, step;
	char cmd[128];

	do {
		memset(&msg, 0, sizeof(msg));
		msg_length = sizeof(msg);

		if (msg_recv(msgid, &msg, &msg_length, obj)) {
			break;
		}

		printf("---> @MSG/CMD Recv: %d(%d/%s)\n",
				msg.msg.cmd, msg.msg.num, msg.msg.data);

		memset(cmd, 0, sizeof(cmd));
		switch(msg.msg.cmd) {
		case GWS_MSG_CMD_SET_HW_REGION:
			if (msg.msg.num > 0) {
				sprintf(cmd, "uci set gws-hw.v1.region=1; uci commit gws-hw; "
					"gws5001app setregion 1\n");
			} else {
				sprintf(cmd, "uci set gws-hw.v1.region=0; uci commit gws-hw; "
					"gws5001app setregion 0\n");
			}
			cli_exec(cmd);
			break;
		case GWS_MSG_CMD_SET_HW_CHANNEL:
			sprintf(cmd, "uci set gws-hw.v1.channel=%d; uci commit gws-hw; "
					"gws5001app setchan %d\n", 
					msg.msg.num, msg.msg.num);
			cli_exec(cmd);
			break;
		case GWS_MSG_CMD_SET_HW_TXCHAIN:
			if (msg.msg.num > 0) {
				sprintf(cmd, "wifi up\n");
			} else {
				sprintf(cmd, "wifi down\n");
			}
			cli_exec(cmd);
			break;
		case GWS_MSG_CMD_SET_HW_TXPOWER:
			if (msg.msg.num > 0) {
				sprintf(cmd, "gws5001app settxpwr %d\n", msg.msg.num);
			} else {
				sprintf(cmd, "gws5001app settxpwr %d\n", 0);
			}
			cli_exec(cmd);
			break;
		case GWS_MSG_CMD_SET_HW_RXGAIN:
			if (msg.msg.num > 0) {
				sprintf(cmd, "setrxgain %d\n", msg.msg.num);
				cli_exec(cmd);
			}
			break;

		case GWS_MSG_CMD_SET_BB_MODE:
			task_setmode(msg.msg.num);
			break;

		case GWS_MSG_CMD_ADJ_HW_TXPWR:
			x = ghw->txpower+msg.msg.num;
			if (x > GWS5K_CONF_HW_TXPOWER_MAX)
				x = GWS5K_CONF_HW_TXPOWER_MAX;
			if (x < 0)
				x = 1;
			if (x != ghw->txpower && x > 0 && x <= GWS5K_CONF_HW_TXPOWER_MAX) {
				sprintf(cmd, "gws5001app settxpwr %d\n", x);
				cli_exec(cmd);
			}
			break;
		case GWS_MSG_CMD_ADJ_HW_RXGAIN:
			x = ghw->rxgain+msg.msg.num;
			if (x < 0)
				x = 0;
			if (x > GWS5K_CONF_HW_RXGAIN_MAX)
				x = GWS5K_CONF_HW_RXGAIN_MAX;
			if (x != ghw->rxgain && x >= 0 && x <= GWS5K_CONF_HW_RXGAIN_MAX) {
				sprintf(cmd, "gws5001app setrxgain %d\n", x);
				cli_exec(cmd);
			}
			break;

		case GWS_MSG_CMD_DFL_HW_TXPOWER:
			x = GWS5K_CONF_HW_TXPOWER_DFL;
			sprintf(cmd, "gws5001app settxpwr %d\n", x);
			cli_exec(cmd);
			break;

		case GWS_MSG_CMD_DFL_HW_RXGAIN:
			step = msg.msg.num;
			x = ghw->rxgain;

			//+ current rxgain between 12+-3, set to 12;
			//+ else, +-3 to close 12.
			if (x <= GWS5K_CONF_HW_RXGAIN_DFL - step) {
				x += step;
			} else if (x >= GWS5K_CONF_HW_RXGAIN_DFL + step) {
				x -= step;
			} else {
				x = GWS5K_CONF_HW_RXGAIN_DFL;
			}

			if (x != ghw->rxgain && x >= 0 && x <= GWS5K_CONF_HW_RXGAIN_MAX) {
				sprintf(cmd, "gws5001app setrxgain %d\n", x);
				cli_exec(cmd);
			}
			break;

		default:
			break;
		}
	} while(msg.msg.cmd > 0);

	return GWS_HW_OK;
}

static int task_setmode(const int mode)
{
	switch(mode) {
	case GWS_MODE_MESH:
		cli_exec("arn_mesh\n");
		break;
	case GWS_MODE_ARN_CAR:
		cli_exec("arn_car\n");
		break;
	case GWS_MODE_ARN_EAR:
		cli_exec("arn_ear\n");
		break;
	}
	return GWS_HW_OK;
}

static void task_idle(void)
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
	printf("  usage: %s [-s idle] [-k key] [-F] [-v] [-h]\n", app);
}
