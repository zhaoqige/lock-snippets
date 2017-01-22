/*
 * gwsbb_data.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: June 07, 2016
 *  Updated on: Nov 9, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <string.h>

#include "grid.h"
#include "gwsbb.h"
#include "gwsbb_iwinfo.h"
#include "gwsbb_data.h"


static void print_file(const char *filename, const char *str);
static char *get_bbmode(const int bbmode);

static int gwsbb_data_read(const int bw, void *iw, void *bb, const char *ifname);


/*
 * clean struct gwsbb
 * read data from libiwinfo
 */
int gwsbb_data_update(const int bw, void *iw, void *bb, const char *ifname)
{
	memset(bb, 0x00, sizeof(struct gwsbb));
	if (gwsbb_data_read(bw, iw, bb, ifname)) {
		QZ("* update bb failed(iwinfo)\n");
		return GWS_BB_ERR_IWINFO;
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
static int gwsbb_data_read(const int bw, void *iw, void *bb, const char *ifname)
{
	int ret = GWS_OK;

	int snr, signal, noise;
	struct gwsbb *gwsbb = bb;
	gwsbb->chanbw = bw; // v10.03091116
	gwsbb->nl_mode = bb_mode(iw, ifname);

	noise = bb_noise(iw, ifname);
	signal = bb_signal(iw, ifname);

	snr = signal - noise;
	if (snr < 0) {
		snr = 0;
	}
	if (noise < GWS_CONF_BB_8M_NOISE_MIN) {
		noise = GWS_CONF_BB_8M_NOISE_MIN;
	}
	signal = noise + snr;

	if (snr > 0) {
		gwsbb->avg.signal = signal;
	} else {
		gwsbb->avg.signal = GWS_CONF_BB_UNKNOWN;
	}
	gwsbb->avg.noise = noise;


	gwsbb->peers_qty = bb_assoclist(bw, &gwsbb->peers, iw, ifname);
	if (gwsbb->peers_qty < 0) {
		gwsbb->peers_qty = 0;
		QZ("* update assoclist failed(iwinfo)\n");
		//ret = GWS_BB_ERR_ASSOCLIST;
	}

	return ret;
}

/*
 * publish to shm
 * - pay attention to data write lock
 */
int gwsbb_data_publish(void *shm, void *bb)
{
	// FIXME: sem_p() before memcpy()?
	memcpy(shm, bb, sizeof(struct gwsbb));

	QZ("data saved to shm\n");
	return 0;
}

void gwsbb_data_to_file(void *bb)
{
	char str[16];
	struct gwsbb *gbb =bb;
	QZ("saving data to files\n");
	QZ("mode   		= %d\n", gbb->nl_mode);
	QZ("signal(avg)	= %d\n", gbb->avg.signal);
	QZ("noise(avg)	= %d\n", gbb->avg.noise);

	snprintf(str, sizeof(str), "%s\n", get_bbmode(gbb->nl_mode));
	print_file(GWS_OFILE_BBMODE, str);

	if (gbb->avg.signal != GWS_CONF_BB_UNKNOWN) {
		snprintf(str, sizeof(str), "%d\n", gbb->avg.signal);
	} else {
		snprintf(str, sizeof(str), "unknown\n");
	}
	print_file(GWS_OFILE_BBSIGNAL, str);

	if (gbb->avg.noise != GWS_CONF_BB_UNKNOWN) {
		snprintf(str, sizeof(str), "%d\n", gbb->avg.noise);
	} else {
		snprintf(str, sizeof(str), "unknown\n");
	}
	print_file(GWS_OFILE_BBNOISE, str);
}

/*
 * GWS BB Mode:
 * 0: mesh point
 * 1: EAR (WDS STA)
 * 2: CAR (WDS AP)
 */
static char *get_bbmode(const int bbmode)
{
	switch(bbmode) {
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
static void print_file(const char *filename, const char *str)
{
	FILE *fd = fopen(filename, "w+");
	fputs(str, fd);
	fclose(fd);
}
