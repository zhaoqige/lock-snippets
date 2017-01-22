/*
 * chscan.h: channel scan
 *
 *  Created on: June 8, 2016
 *  Updated on: July 20, 2016
 *  Updated on: Nov 9, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWSCS_H_
#define GWSCS_H_



#ifdef DEBUG_APP
#define DBG_CS(format, ...)			printf("-dbg-: "format, ##__VA_ARGS__)
#else
#define DBG_CS(format, ...)			{}
#endif


enum GWS_ERR {
	GWS_OK,
	GWS_CS_ERR_SHM = -99,
	GWS_CS_ERR_IFNAME,
	GWS_CS_ERR_IWINFO,
	GWS_CS_ERR_NL_MODE_EAR,
	GWS_CS_ERR_DATA,
};

/*
 * App Inforamtion/Version
 */
#define APP_DESC					("GWS Channel Scanner (vQZ)")
#define APP_VERSION					("v10.0.091116. Buildtime: "__DATE__", "__TIME__)


/*
 * App Task Config
 */
struct GWS_CS_CONF {
	char ifname[32];
	int  bw;
	int  region_old;
	int  channel_old;

	int  region;
	int  channel_start;
	int  channel_toscan;
	int  intl;
};


/*
 * App Task Data
 */
struct gwscs_data {
	unsigned long seq;

	int  ongoing;			// "1:scanning" or "0:idle"
	int	 region;
	int  channel;

	int  nl_mode;
	int  noise;				// current region/channel noise

	int  r0nf[47];			// 60-14+1
	int  r1nf[40];			// 60-21+1
};


#define GWS_CONF_CS_IFNAME			"wlan0"
#define GWS_CONF_CS_BW				5

#define GWS_CONF_NL_INTL			1000
#define GWS_CONF_NL_INTL_MIN		600
#define GWS_CONF_NL_INTL_MAX		2000



#define GWS_CONF_HW_R0CHAN_MIN		14
#define GWS_CONF_HW_R0CHAN_MAX		51
#define GWS_CONF_HW_R1CHAN_MIN		21
#define GWS_CONF_HW_R1CHAN_MAX		51


#define GWS_CONF_NL_START			"gws5001app setrssical 0; gws5001app setrxgain 0; echo 'scan enable' > /sys/kernel/debug/ieee80211/phy0/ath9k/chanscan"
#define GWS_CONF_NL_STOP			"gws5001app setrssical 1; echo 'scan disable' > /sys/kernel/debug/ieee80211/phy0/ath9k/chanscan"


#define GWS_CONF_CS_IDLE			1
#define GWS_CONF_CS_FAIL_BAR		3
#define GWS_CONF_CS_INACTIVE_BAR 	4000
#define GWS_CONF_CS_UNKNOWN			-9999

#define GWS_OFILE_CS_RUN			"/var/run/gwscs_run"


#endif /* GWSCS_H_ */
