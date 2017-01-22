/*
 * gws.h
 *
 *  Created on: July 20, 2016
 *  Updated on: Nov 9, 2016
 *      Author: Qige Zhao
 */

#ifndef GWS3K_H_
#define GWS3K_H_


#ifdef DEBUG_APP
#define QZ(format, ...)			printf("-dbg-: "format, ##__VA_ARGS__)
#else
#define QZ(format, ...)			{}
#endif

enum GRID3K_ERR {
	ERR_NONE = 0,
	ERR_IPC_SHM_BB,
	ERR_IPC_SHM_RF,
	ERR_IPC_MSG,
};

/*
 * App Inforamtion/Version
 */
#define APP_DESC						("GWS5K User Commandline (vQZ)")
#define APP_VERSION						("v10.0.091116. Buildtime: "__DATE__", "__TIME__)

#define GRID_CONF_TASK_IDLE				1000 // unit: ms

#define GRID_CONF_IF_IP					"br-lan"
#define GRID_CONF_IF_WMAC				"wlan0"

#endif /* GWS3K_H_ */
