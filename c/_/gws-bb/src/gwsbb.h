/*
 * gwsbb.h
 *
 *  Created on: Apr 28, 2016
 *  Updated on: Aug 3, 2016
 *  Updated on: Oct 27, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWSBB_H_
#define GWSBB_H_



#ifdef DEBUG_APP
#define QZ(format, ...)			printf("-dbg-: "format, ##__VA_ARGS__)
#else
#define QZ(format, ...)			{}
#endif


enum GWS_ERR {
	GWS_OK,
	GWS_BB_ERR_SHM = -99,
	GWS_BB_ERR_IFNAME,
	GWS_BB_ERR_IWINFO,
	GWS_BB_ERR_ASSOCLIST,
	GWS_BB_ERR_ASSOCLIST_STA_INVALID,
	GWS_BB_ERR_ASSOCLIST_STA_NONE,
	GWS_BB_ERR_ASSOCLIST_STA_TIMEOUT,
	GWS_BB_ERR_DATA,
};

/*
 * App Inforamtion/Version
 */
#define APP_DESC					("GWS Baseband (vQZ)")
#define APP_VERSION					("v10.0.091116. Buildtime: "__DATE__", "__TIME__)


#define GWS_CONF_BB_IFNAME			"wlan0"
#define GWS_CONF_BB_IDLE			1
#define GWS_CONF_BB_FAIL_BAR		3
//#define GWS_CONF_BB_INACTIVE_BAR 	4000
#define GWS_CONF_BB_8M_NOISE_MIN	-105
#define GWS_CONF_BB_UNKNOWN			-9999

#define GWS_OFILE_BBMODE			"/var/run/gwsbb_mode"
#define GWS_OFILE_BBSIGNAL			"/var/run/gwsbb_signal"
#define GWS_OFILE_BBNOISE			"/var/run/gwsbb_noise"
#define GWS_OFILE_BBASSOCIATED		"/var/run/gwsbb_associated"


#endif /* GWSBB_H_ */
