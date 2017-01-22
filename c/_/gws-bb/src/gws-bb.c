/*
 * gwsbb.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: June 07, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "iwinfo/iwinfo.h"

#include "six.h"

#include "grid.h"
#include "gwsbb.h"
#include "gwsbb_data.h"


static void mark_exit(void);
static void print_version(void);
static void print_help(const char *app);

static int  gwsbb_run(const int bw, const char *ifname, const int D);
static void gwsbb_idle(void);


/*
 * quit flag for main loop
 * SIGINT/SIGQUIT/SIGTERM will set it to true
 */
static int GWSBB_QUIT_FLAG = 0;

/*
 * struct gwsbb -> seq (mark data sequence)
 */
static unsigned long gwsbb_data_seq = 0;


/*
 * GWS Baseband Data Collector
 * - read data from libiwinfo
 * - save data via SHM
 * - save data via /tmp/run/gws/ files
 */
int main(int argc, char **argv)
{
	signal(SIGINT, 	(__sighandler_t) mark_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) mark_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) mark_exit); //+ "kill", not "kill -9"

	signal(SIGCHLD, SIG_IGN);


	int ret = GWS_OK;
	char opt = 0;
	int BW = GWS_CONF_BB_CHANBW, D = 0, V = 0, H = 0;
	char I[GRID_CONF_CLI_PARAM_SIZE];

	memset(I, 0x00, sizeof(I));
	while((opt = getopt(argc, argv, "K:b:i:k:vdh")) != -1) {
		switch(opt) {
		case 'b':
			BW = atoi(optarg);
			if (BW < GWS_CONF_BB_CHANBW_MIN) BW = GWS_CONF_BB_CHANBW_MIN;
			if (BW > GWS_CONF_BB_CHANBW_MAX) BW = GWS_CONF_BB_CHANBW_MAX;
			break;
		case 'h':
			H = 1;
			break;
		case 'v':
			V = 1;
			break;
		case 'd':
			D = 1;
			break;
		case 'i':
			snprintf(I, sizeof(I), "%s", optarg);
			break;
		case 'k':
		default:
			break;
		}
	}

	if (V) {
		print_version();
		return 0;
	}
	if (H) {
		//print_version();
		print_help(argv[0]);
		return 0;
	}

	//+ try default ifname: wlan0
	if (strlen(I) < 1) {
		strcpy(I, GWS_CONF_BB_IFNAME);
	}
	ret = gwsbb_run(BW, I, D);

	return ret;
}


static int gwsbb_run(const int bw, const char *ifname, const int D)
{
	int ret = GWS_OK;

	int i;

	int shmid = GRID_CONF_VAL_INVALID;
	struct gwsbb bb, *shm = NULL;

	int fail_counter = 0;
	const struct iwinfo_ops *iw;

	if (shm_init((char *) GWS_CONF_BB_SHM_KEY, &shmid, (void **) &shm, sizeof(struct gwsbb))) {
		printf("* quit (reason: fail to init shm)\n");
		return GWS_BB_ERR_SHM;
	}
	if (D) printf("> shm ok\n");

	iw = iwinfo_backend(ifname);
	if (!iw) {
		printf("no such wireless device: %s\n", ifname);
		return GWS_BB_ERR_IFNAME;
	}
	if (D) printf("> iwinfo ok\n");


	//+ TODO: apply chanbw
	if (D) printf("> changing bw to %d MHz\n", bw);



	for(i = 0; ; i ++) {
		if (GWSBB_QUIT_FLAG) break; //- SIGINT/SIGQUIT/SIGTERM

		if (gwsbb_data_update(bw, (void *) iw, (void *) &bb, ifname)) {
			QZ("data renew failed\n");
			fail_counter ++;
			if (fail_counter >= GWS_CONF_BB_FAIL_BAR) {
				printf("\n * NO DATA available!\n");

				//+ "wifi" affects nothing
				//+ but no "wifi down" before "gwsbb_run()"
				ret = GWS_BB_ERR_DATA;
				break;
			}
		} else {
			gwsbb_data_seq ++;
		}

		//+ save dat sequence
		bb.seq = gwsbb_data_seq;

		if (D) {
			printf("Signal: %d/%d dBm\n",
					bb.peers[0].signal, bb.peers[0].noise);
			printf("Mode: %d\n", bb.nl_mode);
			printf("-> (%05d) data updated & saved to shm/files.\n", i);
		}

		gwsbb_data_publish((void *) shm, (void *) &bb);
		gwsbb_data_to_file((void *) &bb);

		fail_counter = 0;
		gwsbb_idle();
	}

	if (D) printf("> clean up.\n");

	iwinfo_close();

	shm_free(shmid, shm);
	shmid = GRID_CONF_VAL_INVALID;
	shm = NULL;

	return ret;
}

static void gwsbb_idle(void)
{
	sleep(GWS_CONF_BB_IDLE);
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
	GWSBB_QUIT_FLAG = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

static void print_help(const char *app)
{
	printf("  usage: %s [-i ifname] [-v] [-h]\n", app);
}

