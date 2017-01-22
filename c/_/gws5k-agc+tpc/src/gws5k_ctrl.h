/*
 * gws5k_ctrl.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: 2016.11.02
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWS5K_CTRL_H_
#define GWS5K_CTRL_H_


#define GWS5K_STANDBY_TXMCS             99
#define GWS5K_STANDBY_RXGAIN		8
#if defined(USE_8WATT)
#define GWS5K_STANDBY_TXPOWER		38
#else
#define GWS5K_STANDBY_TXPOWER		33
#endif


#define GWS5K_TPC_CONF_ADJ_STEP		3

#define GWS5K_CONF_TXMCS_WHEN_READY	99
#define GWS5K_CONF_TXMCS_WHEN_FAR	1

#define GWS5K_HW_CONF_TXPOWER_MIN	10
#define GWS5K_HW_CONF_TXPOWER_BAR	12
#if defined(USE_8WATT)
#define GWS5K_HW_CONF_TXPOWER_MAX	38
#else
#define GWS5K_HW_CONF_TXPOWER_MAX	33
#endif


#define GWS5K_HW_CONF_RXGAIN_MIN	-32
//#define GWS5K_HW_CONF_TXPOWER_RXGAIN_BAR	30
#define GWS5K_HW_CONF_RXGAIN_BAR	-20     // tpc mode, not agc mode
#define GWS5K_HW_CONF_RXGAIN_MAX	10

#define GWS5K_CMD_TXMCS_FMT			"/usr/sbin/txmcs %d > /dev/null\n"
#define GWS5K_CMD_TXPOWER_FMT		"gws5001app settxpwr %d\n"
#define GWS5K_CMD_RXGAIN_FMT		"gws5001app setrxgain %d\n"
#define GWS5K_CMD_RSSICAL			"gws5001app setrssical %d\n"


int gws5k_ctrl_range_max(const void *task_env);

int gws5k_set_mcs(const int tx_mcs);
int gws5k_set_txpower(const int current_txpwr, const int step, const int txpwr_max);
int gws5k_set_rxgain(const int current_rxgain, const int step, const int rxgain_max);
int gws5k_set_rssical(const int val);


#endif /* GWS5K_CTRL_H_ */
