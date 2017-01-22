/*
 * task_data.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: July 13, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */


#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "six.h"

#include "grid.h"
#include "iwinfo.h"
#include "ch-scan.h"

#ifdef GWS3K
#include "hw_gws3k.h"
#endif

#ifdef GWS5K
#include "hw_gws5k.h"
#endif

#include "task_data.h"


static char *get_csmode(const int csmode);

static void task_data_set_next_channel(const void *conf);
static int task_data_read(const void *conf, void *iw, void *cs_data);


/*
 * clean struct gwscs
 * read data from libiwinfo
 */
int task_data_update(const void *conf, void *iw, void *cs_data)
{
	//return 0;
	if (task_data_read(conf, iw, cs_data)) {
		DBG_CS("* update cs failed(iwinfo)\n");
		return GWS_CS_ERR_IWINFO;
	}
	return GWS_OK;
}

/*
 * FIXME: MAX 5 STATIONS !!!
 *
 * read data from libiwinfo
 * |
 * + libiwinfo
 *      |
 *      + nl-tiny (netlink-tiny)
 */
static int task_data_read(const void *conf, void *iw, void *cs_data)
{
	struct gwscs_data *gwscs_data = cs_data;
	struct GWS_CS_CONF *csconf = (struct GWS_CS_CONF *) conf;

	int i, noise, noise_sum;

	//+ don't support cs scan in EAR mode
	gwscs_data->nl_mode = bb_mode(iw, csconf->ifname);
	if (gwscs_data->nl_mode == GWS_MODE_ARN_EAR) {
		printf("--warning--: gwscs data cannot work on EAR\n");
		return GWS_CS_ERR_NL_MODE_EAR;
	}

	if (csconf->channel_toscan > 0) {
		noise = 0;
		noise_sum = 0;
		task_hw_prepare(csconf->channel_start, csconf->intl*0.6);

		gwscs_data->region = csconf->region;
		gwscs_data->channel = csconf->channel_start;
		for(i = 0; i < 50; i ++) {
			noise = bb_noise(iw, csconf->ifname);
			noise_sum += noise;
			usleep((int) (csconf->intl*0.4/50*1000));
		}

		//task_hw_revert(0, 0);

		gwscs_data->region = csconf->region;
		gwscs_data->channel = csconf->channel_start;
		gwscs_data->noise = noise_sum/i;
		DBG_CS("noise of region/channel %d:%d is %d\n",
				csconf->region, csconf->channel_start,
				gwscs->noise);

		task_data_set_next_channel(csconf);
	}

	return GWS_OK;
}

/*
 * publish to shm
 * - pay attention to data write lock
 */
int task_data_publish(void *shm, void *cs_data)
{
	// FIXME: sem_p() before memcpy()?
	memcpy(shm, cs_data, sizeof(struct gwscs_data));

	DBG_CS("data saved to shm\n");
	return 0;
}

void task_data_to_file(void *cs)
{
	//struct gwscs *gcs =cs;
	DBG_CS("saving data to files\n");
	DBG_CS("mode			= %s\n", get_csmode(gcs->nl_mode));
	DBG_CS("region/channel 	= %d:%d\n", gcs->region, gcs->channel);
	DBG_CS("noise(avg)		= %d\n", gcs->noise);

	/*
	char str[16];
	snprintf(str, sizeof(str), "%s\n", get_csmode(gcs->nl_mode));
	print_file(GWS_OFILE_BBMODE, str);

	if (gcs->avg.signal != GWS_CONF_BB_UNKNOWN) {
		snprintf(str, sizeof(str), "%d\n", gcs->avg.signal);
	} else {
		snprintf(str, sizeof(str), "unknown\n");
	}
	print_file(GWS_OFILE_BBSIGNAL, str);

	if (gcs->avg.noise != GWS_CONF_BB_UNKNOWN) {
		snprintf(str, sizeof(str), "%d\n", gcs->avg.noise);
	} else {
		snprintf(str, sizeof(str), "unknown\n");
	}
	print_file(GWS_OFILE_BBNOISE, str);*/
}

static void task_data_set_next_channel(const void *conf)
{
	struct GWS_CS_CONF *csconf = (struct GWS_CS_CONF *) conf;
	if (csconf->region) {
		csconf->channel_start ++;
		if (csconf->channel_start > GWS_CONF_HW_R1CHAN_MAX)
			csconf->channel_start = GWS_CONF_HW_R1CHAN_MIN;
	} else {
		csconf->channel_start ++;
		if (csconf->channel_start > GWS_CONF_HW_R0CHAN_MAX)
			csconf->channel_start = GWS_CONF_HW_R0CHAN_MIN;
	}
	//csconf->channel_toscan --;
}
/*
 * GWS BB Mode:
 * 0: mesh point
 * 1: EAR (WDS STA)
 * 2: CAR (WDS AP)
 */
static char *get_csmode(const int csmode)
{
	switch(csmode) {
	case GWS_MODE_ARN_CAR:
		return "car";
		break;
	case GWS_MODE_ARN_EAR:
		return "ear";
		break;
	case GWS_MODE_MESH:
		return "mesh";
		break;
	default:
		break;
	}
	return "unknown";
}
