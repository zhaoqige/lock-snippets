/*
 * gws5k_tpc.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 4, 2016
 *  Updated on: Nov 2, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWS5K_TPC_H_
#define GWS5K_TPC_H_

enum GWS5K_TPC_ERR {
	TPC_OK = 0,
	TPC_ERR_REPORT = -100,
	TPC_ERR_REPORT_SENDTO,
	TPC_ERR_REPORT_SENDTO_TARGET,
	TPC_ERR_STANDBY = -200,
	TPC_ERR_SINGLE_ADJUST,
	TPC_ERR_MULTI_ADJUST,
};

#define GWS5K_RARP_FORMAT			"rarp %s\n"


int gws5k_tpc_run(const void *task_env);
void gws5k_tpc_idle(const int intl_sec);


#endif /* GWS5K_TPC_H_ */
