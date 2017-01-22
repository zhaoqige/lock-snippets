/*
 * gwsbb_iwinfo.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: Aug 3, 2016
 *  Updated on: Nov 11, 2016 (v10.0-40)
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include "iwinfo/iwinfo.h"
#include "grid.h"
#include "gwsbb.h"


int bb_mode(const void *bbiw, const char *ifname)
{
	const struct iwinfo_ops *iw = bbiw;
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
int bb_signal(const void *bbiw, const char *ifname)
{
	static int signal;
	const struct iwinfo_ops *iw = bbiw;

	//if (iw->signal(ifname, &signal) || signal < GWS_CONF_BB_UNKNOWN) {
	if (iw->signal(ifname, &signal)) {
		signal = GWS_CONF_BB_UNKNOWN;
	}
	return signal;
}
int bb_noise(const void *bbiw, const char *ifname)
{
	static int noise;
	const struct iwinfo_ops *iw = bbiw;

	//if (iw->noise(ifname, &noise) || noise < GWS_CONF_BB_8M_NOISE_MIN) {
	if (iw->noise(ifname, &noise)) {
		noise = GWS_CONF_BB_UNKNOWN;
	}
	return noise;
}

int bb_assoclist(const int bw, void *assoclist, const void *bbiw, const char *ifname)
{
	int j, k, peer_qty;
	int snr, signal, noise;
	struct gwsbb_assoclist *al;

	const struct iwinfo_ops *iw = bbiw;
	int i, len, inactive = 0, inactive_min = 0;

	char buf[IWINFO_BUFSIZE];
	struct iwinfo_assoclist_entry *e;
	struct iwinfo_rate_entry *br;

	if (iw->assoclist(ifname, buf, &len)) {
		QZ("no assoclist information available\n");
		return GWS_BB_ERR_ASSOCLIST_STA_INVALID;
	} else if (len <= 0) {
		QZ("no station connected\n");
		return GWS_BB_ERR_ASSOCLIST_STA_NONE;
	}

	//QZ("assoclist len=%d\n", len);
	peer_qty = 0;
	memset(assoclist, 0, sizeof(struct gwsbb_assoclist) * GWS_CONF_BB_STA_MAX); // v10.0-39.111116
	for(i = j = k = 0; i < len && k < GWS_CONF_BB_STA_MAX; i += sizeof(struct iwinfo_assoclist_entry)) {
		snr = 0;
		e = (struct iwinfo_assoclist_entry *) &buf[i];
		inactive = e->inactive;

		//QZ(" assoclist inactive=%d ms\n", inactive);
		if (inactive < GWS_CONF_BB_INACTIVE_BAR) {
			al = (struct gwsbb_assoclist *) &assoclist[j];

			//if (e->signal != 0) {
			sprintf(al->mac, "%02X:%02X:%02X:%02X:%02X:%02X",
					e->mac[0], e->mac[1], e->mac[2],
					e->mac[3], e->mac[4], e->mac[5]);

			signal = e->signal;
			noise = e->noise;
			snr = signal - noise;
			if (snr < 0) {
				snr = 0;
			}
			if (noise < GWS_CONF_BB_8M_NOISE_MIN) {
				noise = GWS_CONF_BB_8M_NOISE_MIN;
			}

			al->noise = noise;
			if (snr > 0) {
				al->signal = noise + snr;
			} else {
				al->signal = GWS_CONF_BB_UNKNOWN;
			}
			al->inactive = inactive;

			//+ TODO: save mac/rx_br/rx_mcs/tx_br/tx_mcs/rx_pckts/tx_pckts/tx_failed
			br = (struct iwinfo_rate_entry *) &e->rx_rate;
			al->rx_mcs = br->mcs;
			al->rx_br = br->rate * bw / GWS_CONF_BB_BR_DFL;
			al->rx_pckts = e->rx_packets;

			br = (struct iwinfo_rate_entry *) &e->tx_rate;
			al->tx_mcs = br->mcs;
			al->tx_br = br->rate * bw / GWS_CONF_BB_BR_DFL;
			al->tx_pckts = e->tx_packets;
			al->tx_failed = e->tx_failed;

			j += sizeof(struct gwsbb_assoclist);
			k ++;
			peer_qty ++;
		}

		QZ("min_active=%d:%d\n", i, inactive);
		if (!inactive_min || inactive_min > inactive) {
			inactive_min = inactive;
		}
	}

	QZ("assoclist min_active=%d\n", inactive_min);
	return (inactive_min < GWS_CONF_BB_INACTIVE_BAR ? peer_qty : GWS_BB_ERR_ASSOCLIST_STA_TIMEOUT);
}
