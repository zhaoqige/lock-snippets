/*
 * grid.h
 *
 *  Created on: Jun 17, 2016
 *  Updated on: July 8, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GRID_H_
#define GRID_H_


#include "conf.h"


#define GRID_R0FREQ(x)				(473+(x-14)*6)
#define GRID_R1FREQ(x)				(474+(x-21)*8)

#define GRID_CONF_CLI_PARAM_SIZE	32
#define GRID_CONF_VAL_INVALID		-1


/*
 * gwsbb/gwshw/msg
 */
#define GWS_CONF_MSG_LENGTH			256

// FIXME: tested, CAR can connect 10 EARs
#define GWS_CONF_BB_STA_MAX			10
#define GWS_CONF_BB_INACTIVE_BAR	4000


#ifdef GWS_USE_KEYFILE
#define GWS_CONF_BB_SHM_KEY		"/usr/sbin/gwsbb"
#define GWS_CONF_HW_SHM_KEY		"/usr/sbin/gwshw"
#define GWS_CONF_MSG_KEY		"/usr/sbin/"
#else
#define GWS_CONF_BB_SHM_KEY		0x1351105
#define GWS_CONF_HW_SHM_KEY		0x3556153
#define GWS_CONF_MSG_KEY		0x1091915
#endif


enum NL80211_BB_MODE {
	NL80211_MODE_MASTER = 1,
	NL80211_MODE_CLIENT = 3,
	NL80211_MODE_MESH = 7,
};
enum GWS_BB_MODE {
	GWS_MODE_UNKNOWN = -1,
	GWS_MODE_MESH = 0,
	GWS_MODE_ARN_EAR = 1,
	GWS_MODE_ARN_CAR = 2,
	GWS_MODE_ADHOC,
	GWS_MODE_OTHER = 99
};


enum GWS_MSG_COM {
	GWS_MSG_COM_BB = 1,
	GWS_MSG_COM_HW,
	GWS_MSG_COM_SCAN,

	GWS_MSG_COM_TPC = 20,
	GWS_MSG_COM_RARPD,

	GWS_MSG_COM_WB = 50,
	GWS_MSG_COM_SP,
	GWS_MSG_COM_GRID_LITE,
	GWS_MSG_COM_GRID_ADV
};

enum GWS_MSG_CMD {
	GWS_MSG_CMD_SET_HW_REGION = 1,
	GWS_MSG_CMD_SET_HW_CHANNEL,
	GWS_MSG_CMD_SET_HW_TXCHAIN,
	GWS_MSG_CMD_SET_HW_TXPOWER,
	GWS_MSG_CMD_SET_HW_RXGAIN,
	GWS_MSG_CMD_SET_HW_MODE,

	GWS_MSG_CMD_TPC = 20,
	GWS_MSG_CMD_ADJ_HW_TXPWR,
	GWS_MSG_CMD_ADJ_HW_RXGAIN,
	GWS_MSG_CMD_DFL_HW_TXPOWER,
	GWS_MSG_CMD_DFL_HW_RXGAIN,

	GWS_MSG_CMD_RARPD_RPT = 30,

	GWS_MSG_CMD_SET_SCAN_START 	= 50,
	GWS_MSG_CMD_SET_SCAN_ECHO,
	GWS_MSG_CMD_SET_SCAN_RESULT,

	GWS_MSG_CMD_SET_BB_CHANBW 	= 100,
	GWS_MSG_CMD_SET_BB_MODE,

};

struct GWS_MSG {
	long mtype; // msg to
	struct {
		enum GWS_MSG_COM from;
		enum GWS_MSG_CMD cmd;
		int  num;
		char data[GWS_CONF_MSG_LENGTH];
	} msg;
};

#define GWS_CONF_BB_CHANBW			8		// 5/8/10/16/20
#define GWS_CONF_BB_CHANBW_MIN		5
#define GWS_CONF_BB_CHANBW_MAX		16
#define GWS_CONF_BB_SNR_SELF		99
#define GWS_CONF_BB_SIGNAL_UNKNOW	-9999
#define GWS_CONF_BB_NOISE_MIN		-101
#define GWS_CONF_BB_BR_DFL			20


/*
 * check user input
 */

struct gwsbb_assoclist {
	char mac[18]; //"00:00:00:00:00:00"; @ 2016.06.07 18:12
	int signal;
	int noise;
	float rx_br;
	float tx_br;
	int rx_mcs;
	int tx_mcs;
	unsigned long long rx_pckts;
	unsigned long long tx_pckts;
	unsigned long long tx_failed;
	unsigned int inactive; //+ unit: ms
};

struct gwsbb {
	unsigned long seq;
	char vers[4];			//- "1.0\0"
	char kw[32];			//- "gwsctrlv1: bb data collector"

	int nl_mode;				//- 0: mesh, 1: wds sta, 2: wds ap
	struct {
		int signal;
		int noise;
		int br;
	} avg;

	int peers_qty;		//+ peer 0 is average
	struct gwsbb_assoclist peers[GWS_CONF_BB_STA_MAX];
};

/*
 * parse output from "rfinfo"
 * items in order:
 *
 */
struct gwshw {
	unsigned long seq;
	char ver[4];			// "1.0\0"
	char kw[32];			// "gwsctrlv1: bb data collector"

	char hw_ver[64];
	char fw_ver[64];
	char hw_sn[32];

	int  region;			// 0: 6 MHz 1: 8 MHz
	int  channel;
	int  freq;
	int  txchain;
	int  txpower;

	int  rxgain;

	int  temp;

	int  adv_agcmode;
	int  adv_ifout;

	int  adv_txcal;
	int  adv_mintxpwr;
	int  adv_maxtxpwr;
	int  adv_brdmaxpwr;
	int  adv_txatten;
	int  adv_maxatten;

	int  adv_rx;
	int  adv_rxcal;
	int  adv_rxmaxgain;
	int  adv_rxfatten;
	int  adv_rxratten;
};


#endif /* GRID_H_ */
