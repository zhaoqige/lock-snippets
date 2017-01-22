/*
 * gws5k_ctrl.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 4, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <string.h>

#include "six.h"

#include "app.h"
#include "task.h"
#include "gws5k_ctrl.h"


static int gws5k_do_set(const char *format, const int val);


/*
 * set default rxgain
 * set default txpower
 */
int gws5k_ctrl_range_max(const void *task_env)
{
	struct TASK_ENV *env = (struct TASK_ENV *) task_env;

	DBG_TPC("base> enter ctrl STANDBY\n");

	gws5k_set_mcs(GWS5K_STANDBY_TXMCS);
	gws5k_set_txpower(GWS5K_STANDBY_TXPOWER, 1, env->conf.txpwr_max);

	return 0;
}


int gws5k_set_mcs(const int tx_mcs)
{
	int x = 0;
	static int tx_mcs_last;

	switch(tx_mcs) {
	case 7:
	case 6:
	case 5:
	case 4:
	case 3:
	case 2:
	case 1:
	case 0:
		x = tx_mcs;
		break;
	default:
		x = 99;
		break;
	}

	printf("TPC 9> txmcs -> %d (last %d)\n", tx_mcs, tx_mcs_last);

	if (tx_mcs_last != x) {
		tx_mcs_last = x;
		return gws5k_do_set(GWS5K_CMD_TXMCS_FMT, x);
	} else {
		tx_mcs_last = x;
		return 0;
	}
}
int gws5k_set_txpower(const int current_txpwr, const int step, const int txpwr_max)
{
	int txpwr = 0;

	txpwr = current_txpwr + step;
	if (txpwr > GWS5K_HW_CONF_TXPOWER_MAX) {
		txpwr = GWS5K_HW_CONF_TXPOWER_MAX;
	}
	if (txpwr < GWS5K_HW_CONF_TXPOWER_MIN) {
		txpwr = GWS5K_HW_CONF_TXPOWER_MIN;
	}

	if (txpwr_max && txpwr > txpwr_max)
		txpwr = txpwr_max;

	printf("TPC 9> txpower -> %d (max %d)\n", txpwr, txpwr_max);

	//if (txpwr != current_txpwr) {
		return gws5k_do_set(GWS5K_CMD_TXPOWER_FMT, txpwr);
	//} else {
		//return 0;
	//}
}

int gws5k_set_rxgain(const int current_rxgain, const int step, const int rxgain_max)
{
	int rxgain = 0;
	rxgain = current_rxgain + step;

	if (rxgain > GWS5K_HW_CONF_RXGAIN_MAX) {
		rxgain = GWS5K_HW_CONF_RXGAIN_MAX;
	}

	if (rxgain < GWS5K_HW_CONF_RXGAIN_MIN) {
		rxgain = GWS5K_HW_CONF_RXGAIN_MIN;
	}

	if (rxgain_max && rxgain > rxgain_max)
		rxgain = rxgain_max;

	printf("TPC 9> rxgain -> %d (current %d, step %d, max %d)\n",
			rxgain, current_rxgain, step, rxgain_max);

	if (rxgain != current_rxgain) {
		return gws5k_do_set(GWS5K_CMD_RXGAIN_FMT, rxgain);
	} else {
		return 0;
	}
}

int gws5k_set_rssical(const int val)
{
	return 0;
	//return gws5k_do_set(GWS5K_CMD_RSSICAL, val);
}

static int gws5k_do_set(const char *format, const int val)
{
	char cmd[128];
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd)-1, format, val);

	//printf("-=-=-=->  CMD(%s) <-=-=-=-\n", cmd);
	cli_exec(cmd);
	return 0;
}
