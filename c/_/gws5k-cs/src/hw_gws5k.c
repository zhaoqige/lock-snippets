/*
 * hw_gws5k.c: gws5000 hardware control
 *
 *  Created on: Jun 8, 2016
 *      Author: qige
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "six.h"

#include "grid.h"
#include "ch-scan.h"


/*
 * channel = 0, scan current channel;
 * channel != 0, check channel range;
 * setchan & set chanscan
 */
void task_hw_prepare(const int channel, const int intl_ms)
{
	char cmd[128];

	if (channel != 0 && (channel < GWS_CONF_HW_R0CHAN_MIN || channel > GWS_CONF_HW_R1CHAN_MAX))
		return;

	// TODO: set channel before scan
	print_file(GWS_OFILE_CS_RUN, "1");

	if (channel > 0) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd)-1, "gws5001app setchan %d\n", channel);
		cli_exec(cmd);
		DBG_CS("%s\n", cmd);
	}

	cli_exec(GWS_CONF_NL_START);
	usleep(intl_ms*1000);
}

/*
 *
 */
void task_hw_revert(const int last_region, const int last_channel)
{
	char cmd[128];

	if (last_channel != 0 && (last_channel < GWS_CONF_HW_R0CHAN_MIN || last_channel > GWS_CONF_HW_R1CHAN_MAX))
		return;

	// TODO: set to original channel when done scan
	print_file(GWS_OFILE_CS_RUN, "1");

	if (last_region > 0) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd)-1, "gws5001app setregion %d\n", last_region);
		cli_exec(cmd);
		DBG_CS("%s\n", cmd);
	}

	if (last_channel > 0) {
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd)-1, "gws5001app setchan %d\n", last_channel);
		cli_exec(cmd);
		DBG_CS("%s\n", cmd);
	}

	cli_exec(GWS_CONF_NL_STOP);
	print_file(GWS_OFILE_CS_RUN, "0");
}
