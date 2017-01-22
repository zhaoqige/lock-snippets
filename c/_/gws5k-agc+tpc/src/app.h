/*
 * app.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: 2016.11.11
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef APP_H_
#define APP_H_


enum APP_ERR {
	APP_OK = 0,
	APP_PARAM_BAD = -1,

	APP_TASK_RUN = -100,
};

struct APP_CONF {
	int debug;
	int echo_help;
	int echo_version;

	int comm_port;
	int ear_qty_max;

	//int txpwr_init;
	int txpwr_normal;
	int txpwr_min;
	int txpwr_max;

	//int rxgain_init;
	int rxgain_normal;
	//int rxgain_min;
	int rxgain_max_near; // 39: <= -10; 33: <= 10
	int rxgain_max; // 39: <= 10; 33: <= 10

	int algorithm;

	int intl;
};


#ifdef DEBUG
#define DBG_TPC(format, ...)			printf("-dbg-: "format, ##__VA_ARGS__)
#else
#define DBG_TPC(format, ...)			{}
#endif


#define APP_DESC		("GWS TPC (with AGC) (vQZ)")
#define APP_VERSION		("v10.0.111116. Buildtime: "__DATE__", "__TIME__)


#define APP_VAL_INVALID					-1
#define APP_CONF_PARAM_LENGTH_MAX		32


#define APP_COMM_PORT_DEFAULT			6638

#endif /* APP_H_ */
