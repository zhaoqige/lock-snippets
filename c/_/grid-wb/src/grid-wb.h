/*
 * grid-wb.h
 *
 *  Created on: May 12, 2016
 *  Updated on: July 20, 2016
 *  Updated on: Nov 9. 2016
 *      Author: Qige Zhao
 */

#ifndef GRID_WB_H_
#define GRID_WB_H_

//#define QZ(format, ...)			printf("-dbg-: "format, ##__VA_ARGS__)
#define QZ(format, ...)			{}

enum GRID_ERR {
	GRID_WB_OK = 0,
	GRID_WB_ERR_UART = -50,
	GRID_WB_ERR_HW_SHM,
	GRID_WB_ERR_HW_MSG,
	GRID_WB_ERR_BB_SHM,
	GRID_WB_ERR_NO_DATA_RECV,
	GRID_WB_ERR_NO_CMD,
	GRID_WB_ERR_DATA_TOOSHORT,
	GRID_WB_ERR_CMD_BAD,
};

/*
 * App Inforamtion/Version
 */
#define APP_DESC						("GRID-WB: GWS Remote Interface Daemon (vQZ)")
#define APP_VERSION						("v10.0.091116. Buildtime: "__DATE__", "__TIME__)

#define GRID_WB_CONF_PARAM_LENMAX		32
#define GRID_WB_CONF_HW_UART			"/dev/ttyS0"
#define GRID_WB_CONF_HW_IDLE			1500 // unit: ms

#define GRID_WB_CONF_START_BYTE			'+'
#define GRID_WB_CONF_STOP_BYTE			'\n'
#define GRID_WB_CONF_CMD_LENGTH_MIN		11
#define GRID_WB_CONF_CMD_LENGTH_MAX		32

#define GRID_WB_CONF_CMD_MASK_SETCHN	"wbsetchn:"
#define GRID_WB_CONF_CMD_MASK_SETCHNBW	"wbsetchnbw:"
#define GRID_WB_CONF_CMD_MASK_SETRGN	"wbsetrgn:"
#define GRID_WB_CONF_CMD_MASK_SETPWR	"wbsetpwr:"
#define GRID_WB_CONF_CMD_MASK_SETMOD	"wbsetmod:"
#define GRID_WB_CONF_CMD_MASK_SETCMD	"wbcmd:"
#define GRID_WB_CONF_CMD_MASK_GETCHN	"wbgetchn"
#define GRID_WB_CONF_CMD_MASK_GETCHNS	"wbgetchns"
#define GRID_WB_CONF_CMD_MASK_GETSIG	"wbgetsig"
#define GRID_WB_CONF_CMD_MASK_GETTXPWRS	"wbgetpwrs"

#define GRID_WB_CONF_CMD_MASK_WBSCAN	"wbscan"


#endif /* GRID_WB_H_ */
