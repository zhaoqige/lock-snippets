/*
 * iwinfo.c
 *
 *  Created on: Jun 8, 2016
 *  Updated on: July 13, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include "iwinfo/iwinfo.h"
#include "grid.h"


int bb_mode(const void *nfiw, const char *ifname)
{
	const struct iwinfo_ops *iw = nfiw;
	static int mode, wifi_mode;
	if (iw->mode(ifname, &wifi_mode)) {
		mode = -1;
	}

	switch(wifi_mode) {
	case NL80211_MODE_MASTER: //- Master to CAR
		mode = GWS_MODE_ARN_CAR;
		break;
	case NL80211_MODE_CLIENT: //- Client to EAR
		mode = GWS_MODE_ARN_EAR;
		break;
	case NL80211_MODE_MESH: //- Meshpoint to Mesh
		mode = GWS_MODE_MESH;
		break;
	default: // Ad-Hoc, etc.
		mode = GWS_MODE_OTHER;
		break;
	}
	return mode;
}
int bb_noise(const void *nfiw, const char *ifname)
{
	static int noise;
	const struct iwinfo_ops *iw = nfiw;

	if (iw->noise(ifname, &noise)) {
		noise = GWS_CONF_BB_SIGNAL_UNKNOW;
	}
	return noise;
}
