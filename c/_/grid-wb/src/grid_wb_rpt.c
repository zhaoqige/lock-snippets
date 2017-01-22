/*
 * grid_wb_rpt.c
 *
 *  Created on: Jun 23, 2016
 *  Updated on: July 15, 2016
 *      Author: qige@6harmonics.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "six.h"

#include "grid.h"


void grid_wb_rpt_sts(const int uartfd, void *gwsbb, void *gwshw, const int D)
{
	char data[32];
	struct gwsbb *bb = (struct gwsbb *) gwsbb;
	struct gwshw *hw = (struct gwshw *) gwshw;

	memset(data, 0, sizeof(data));
	snprintf(data, sizeof(data), "+wbsts:%d,%d,%d,%d,%d,%d,%d\n",
			hw->channel,
			bb->avg.noise - GWS_CONF_BB_NOISE_MIN,
			hw->txchain ? hw->txpower : 0,
			bb->nl_mode,
			hw->region,
			bb->chanbw,
			hw->rxgain
			);

	if (D) printf("---> %s\n", data);
	uart_write(uartfd, data, strlen(data));
}

void grid_wb_rpt_sigs(const int uartfd, void *gwsbb, void *gwshw, const int D)
{
	int i;
	int wbsigs_snr[GWS_CONF_BB_STA_MAX], wbsigs_non, wbsigs_self;
	char data[32];
	struct gwsbb *bb = (struct gwsbb *) gwsbb;
	//struct gwshw *hw = (struct gwshw *) gwshw;

	wbsigs_non = 0;
	wbsigs_self = GWS_CONF_BB_SNR_SELF;

	for(i = 0; i < GWS_CONF_BB_STA_MAX; i ++) {
		if (bb->avg.signal == GWS_CONF_BB_SIGNAL_UNKNOW) {
			wbsigs_snr[i] = 0;
		} else {
			wbsigs_snr[i] = bb->peers[i].signal - bb->peers[i].noise;
		}
	}
	memset(data, 0, sizeof(data));

	switch(bb->nl_mode) {
		case GWS_MODE_MESH:
			snprintf(data, sizeof(data), "+wbsigs:%d,%d,%d,%d,%d\n",
					wbsigs_self,
					wbsigs_snr[0],
					wbsigs_snr[1],
					wbsigs_snr[2],
					wbsigs_snr[3]
					);
			break;
		case GWS_MODE_ARN_CAR:
			snprintf(data, sizeof(data), "+wbsigs:%d,%d,%d,%d,%d\n",
					wbsigs_self,
					wbsigs_snr[0],
					wbsigs_snr[1],
					wbsigs_snr[2],
					wbsigs_snr[3]
					);
			break;
		case GWS_MODE_ARN_EAR:
		default:
			snprintf(data, sizeof(data), "+wbsigs:%d,%d,%d,%d,%d\n",
					wbsigs_snr[0],
					wbsigs_self,
					wbsigs_non, wbsigs_non, wbsigs_non
					);
			break;
	}

	if (D) printf("---> %s\n", data);
	uart_write(uartfd, data, strlen(data));
}

