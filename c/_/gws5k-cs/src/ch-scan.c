/*
 * thread_chscan.c: channel scan
 *
 *  Created on: Jun 8, 2016
 *  Updated on: July 13, 2016
 *  Updated on: Oct 27, 2016
 *  Updated on: Nov 9, 2016
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
//#include "msg.h"

#include "grid.h"
#include "task_data.h"

#ifdef GWS3K
#include "hw_gws3k.h"
#endif

#ifdef GWS5K
#include "hw_gws5k.h"
#endif

#include "ch-scan.h"



static void task_prepare_exit(void);
static void print_help(const char *app);

static int  task_run(const void *conf, const int QUIET, const int D);
static void task_idle(void);


static int  getFreqByResionChannel(int region, int channel);

/*
 * quit flag for main loop
 * SIGINT/SIGQUIT/SIGTERM will set it to true
 */
static int GWSBB_QUIT_FLAG = 0;

/*
 * struct gwscs -> seq (mark data sequence)
 */
//static unsigned long gwscs_data_seq = 0;


/*
 * GWS Channel Scanner
 * - set channel
 * - write "debugfs" to start
 * - read noise
 * - write "debugfs" to stop
 * - next channel
 */
int main(int argc, char **argv)
{
	signal(SIGINT, 	(__sighandler_t) task_prepare_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) task_prepare_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) task_prepare_exit); //+ "kill", not "kill -9"

	signal(SIGCHLD, SIG_IGN);


	int ret = GWS_OK;
	char opt = 0;

	struct GWS_CS_CONF csconf;
	memset(&csconf, 0, sizeof(csconf));
	csconf.channel_toscan = 1;
	csconf.intl = GWS_CONF_NL_INTL;
	csconf.region = -1;

	int channel_stop = 0;
	int Q = 0, D = 0, V = 0, H = 0; //+ debug, version, help

	while((opt = getopt(argc, argv, "qr:b:e:c:s:vdh")) != -1) {
		switch(opt) {
		case 'q':
			Q = 1;
			D = 0;
			break;
		case 'r': // region
			csconf.region = atoi(optarg);
			break;
		case 'b': // begin channel
			csconf.channel_start = atoi(optarg);
			break;
		case 'e': // end channel
			channel_stop = atoi(optarg);
			break;
		case 'c': // how many channels
			csconf.channel_toscan = atoi(optarg);
			break;

		case 's': // interval
			csconf.intl = atoi(optarg);
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
		default:
			break;
		}
	}

	if (V) {
		printf("%s (%s)\n", APP_DESC, APP_VERSION);
		return 0;
	}
	if (H) {
		print_help(argv[0]);
		return 0;
	}

	//+ check before run
	if (strlen(csconf.ifname) < 1) {
		snprintf(csconf.ifname, sizeof(csconf.ifname),
				"%s", GWS_CONF_CS_IFNAME);
	}
	if (csconf.bw < GWS_CONF_CS_BW)
		csconf.bw = GWS_CONF_CS_BW;

	if (csconf.intl < GWS_CONF_NL_INTL_MIN)
		csconf.intl = GWS_CONF_NL_INTL_MIN;
	if (csconf.intl > GWS_CONF_NL_INTL_MIN)
		csconf.intl = GWS_CONF_NL_INTL_MAX;

	if (channel_stop) {
		if (channel_stop >= csconf.channel_start) {
			csconf.channel_toscan = channel_stop - csconf.channel_start + 1;
		} else {
			if (csconf.region) {
				csconf.channel_toscan = (GWS_CONF_HW_R1CHAN_MAX - csconf.channel_start + 1)
						+ (channel_stop - GWS_CONF_HW_R1CHAN_MIN + 1);
			} else {
				csconf.channel_toscan = (GWS_CONF_HW_R0CHAN_MAX - csconf.channel_start + 1)
						+ (channel_stop - GWS_CONF_HW_R0CHAN_MIN + 1);
			}
		}
	}
	if (csconf.channel_toscan < 1)
		csconf.channel_toscan = 1;

	ret = task_run(&csconf, Q, D);

	return ret;
}


static int task_run(const void *conf, const int QUIET, const int D)
{
	int ret = GWS_OK;
	struct GWS_CS_CONF *csconf = (struct GWS_CS_CONF *) conf;

	int hw_shmid = -1;
	struct gwscs_data nf_data; //, *shm = NULL;
	struct gwshw *hw_shm;

	int fail_counter = 0;
	const struct iwinfo_ops *iw;


	//+ init hw shm
	hw_shm = NULL;
	if (shm_init((char *) GWS_CONF_HW_SHM_KEY, &hw_shmid,
			(void **) &hw_shm, sizeof(struct gwshw))) {
		printf("* quit (reason: fail to init hw shm)\n"); //perror("shmget");
		return GWS_CS_ERR_SHM;
	}
	if (!QUIET && D) 
		printf("-> hw shm ok (%d)\n", hw_shmid);

	// TODO: read current region/channel and save to *conf
	csconf->region_old = hw_shm->region;
	csconf->channel_old = hw_shm->channel;
	if (csconf->region < 0)
		csconf->region = csconf->region_old;
	if (csconf->channel_start < 1)
		csconf->channel_start = csconf->channel_old;

	if (!QUIET) {
		if (D) {
			printf("> Current: Region %d CH %d\n"
					"> Region %d, %d channel(s) to scan, first CH %d\n",
					csconf->region_old, csconf->channel_old,
					csconf->region, csconf->channel_toscan, csconf->channel_start);
		} else {
			printf(" Start with  Region %d CH %d ...\n",
					csconf->region, csconf->channel_start);
		}
	}

	iw = iwinfo_backend(csconf->ifname);
	if (!iw) {
		printf("no such wireless device: %s\n", csconf->ifname);
		return GWS_CS_ERR_IFNAME;
	}
	if (!QUIET && D)
		printf("> iwinfo ok\n");


	while(csconf->channel_toscan > 0) {
		if (task_data_update(csconf, (void *) iw, (void *) &nf_data)) {
			DBG_CS("data renew failed\n");
			fail_counter ++;
			if (fail_counter >= GWS_CONF_CS_FAIL_BAR) {
				printf("\n * NO DATA available!\n");

				//+ "wifi" affects nothing
				//+ but no "wifi down" before "task_run()"
				ret = GWS_CS_ERR_DATA;
				break;
			}
		}


		//+ save dat sequence
		//nf.seq = gwscs_data_seq;

		if (QUIET) {
			printf("%d,%d,%d,%d\n", nf_data.region, nf_data.channel, 						getFreqByResionChannel(nf_data.region, nf_data.channel),		
					nf_data.noise);
		} else {
			printf(" - Rg%d - C%d/F%d: %d/%d dBm (min -105)\n",
					nf_data.region, nf_data.channel,
					getFreqByResionChannel(nf_data.region, nf_data.channel),
					nf_data.noise + 105, nf_data.noise);
			//printf("-> (%05d) data updated & saved to shm/files.\n", i);
		}

		//task_data_publish((void *) shm, (void *) &bb);
		task_data_to_file((void *) &nf_data);

		if (GWSBB_QUIT_FLAG)
			break; //- SIGINT/SIGQUIT/SIGTERM

		csconf->channel_toscan --;

		//fail_counter = 0;
		task_idle();
	}

	task_hw_revert(csconf->region_old, csconf->channel_old);
	if (!QUIET && D)
		printf("> clean up.\n");

	iwinfo_close();
	shm_free(-1, hw_shm);
	hw_shmid = -1;
	hw_shm = NULL;

	return ret;
}


static int  getFreqByResionChannel(int region, int channel)
{
	static int freq = 0;
	switch(region) {
		case 0:
			freq = 473 + (channel - 14) * 6;
			break;
		case 1:
		default:
			freq = 474 + (channel - 21) * 8;
			break;
	}
	if (freq < 0)	freq = 0;
	return freq;
}

static void task_idle(void)
{
	sleep(GWS_CONF_CS_IDLE);
}


/*
 * SIGINT, SIGQUIT, SIGTERM: clean first
 */
static void task_prepare_exit(void)
{
	printf("\n exiting %s ...\n", APP_DESC);
	GWSBB_QUIT_FLAG = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}

static void print_help(const char *app)
{
	printf("  usage: %s [-q] [-r region] [-b begin] [-c next_n] [-e end]\n", app);
	printf("         %s [-d] [-v] [-h]\n", app);
	printf("   note: the scan value, the smaller, the better\n");
}

