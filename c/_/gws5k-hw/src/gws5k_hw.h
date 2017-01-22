/*
 * gwshw.h
 *
 *  Created on: Apr 28, 2016
 *  Updated on: July 20, 2016
 *  Updated on: Nov 9, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWS5K_HW_H_
#define GWS5K_HW_H_


#ifdef DEBUG_APP
#define DBG_HW(format, ...)			printf("-5k-: "format, ##__VA_ARGS__)
#else
#define DBG_HW(format, ...)			{}
#endif

//typedef unsigned int			uint;
//typedef unsigned char 			uchar;

#define R0FREQ(x)				(x ? 473+(x-14)*6 : 0)
#define R1FREQ(x)				(x ? 474+(x-21)*8 : 0)

enum GWS_ERR {
	GWS_HW_OK,
	GWS_HW_ERR_INIT = -199,
	GWS_HW_ERR_SHM,
	GWS_HW_ERR_MSG,
	GWS_HW_ERR_UART,
	GWS_HW_ERR_UART_READ,
	GWS_HW_ERR_UART_WRITE,
	GWS_HW_ERR_DATA,
	GWS_HW_ERR_DATA_LBB,
	GWS_HW_ERR_DATA_PARSER,
	GWS_HW_ERR_DATA_TOOSHORT,
};


/*
 * App Inforamtion/Version
 */
#define APP_DESC					("GWS5000 Hardware Controller (vQZ)")
#define APP_VERSION					("v10.0.091116. Buildtime: "__DATE__", "__TIME__)


#define GWS_CHK_PARAM_LENMAX		32

#define GWS_CONF_HW_IDLE		 	1
#define GWS_CONF_HW_FAILBAR			3

#define GWS_CONF_HW_CMD_LENGTH		128
#define GWS_CONF_HW_INITBAR			3
#define GWS_CONF_HW_INIT			"ifconfig wlan0 | grep wlan0 -B1; gws5001app getrssical; gws5001app rfinfo\n"
#define GWS_CONF_HW_QUERY			GWS_CONF_HW_INIT
//#define GWS_CONF_HW_TXON			"txon\n"
#define GWS_CONF_HW_TXPOWER			"gws5001app settxpwr 33\n"
#define GWS_CONF_HW_RXGAIN			"gws5001app setrxgain 12\n"

#define GWS5K_CONF_HW_TXPOWER_MAX		39
#define GWS5K_CONF_HW_TXPOWER_DFL		33
#define GWS5K_CONF_HW_RXGAIN_MAX		8
#define GWS5K_CONF_HW_RXGAIN_DFL		3

/*
 * idle time before read(uartfd)
 * unit: us
 */
#define GWS_CONF_HW_PARSE_IDLE		500000

#define GWS_CONF_HW_LINE_LENGTH		128
#define GWS_CONF_HW_LINE_MIN		8
#define GWS_CONF_HW_RFINFO_LENGTH	1024
#define GWS_CONF_HW_PARSE_LENGTH	512
#define GWS_CONF_HW_RFINFOS_LENGTH	55 //* TODO: strlen("rfinfo 1" result)

//#define GWS_CONF_HW_PARSE_HWV		"Harmonics"
//#define GWS_CONF_HW_PARSE_FWV		"Firmware"
//#define GWS_CONF_HW_PARSE_SN		"SNO"

#define GWS_CONF_HW_PARSE_REGION	"Region"
#define GWS_CONF_HW_PARSE_CHANNEL	"Chan:"
#define GWS_CONF_HW_PARSE_TXCHAIN	"wlan0"
#define GWS_CONF_HW_PARSE_TXPOWER	"Tx Power"

#define GWS_CONF_HW_PARSE_RXGAIN	"Rx Gain"

//#define GWS_CONF_HW_PARSE_TEMP		"Temp"

//#define GWS_CONF_HW_PARSE_AGCMODE		"AGCMode"
//#define GWS_CONF_HW_PARSE_IFOUT			"IFOUT"

//#define GWS_CONF_HW_PARSE_TXCAL			"TXCal"
//#define GWS_CONF_HW_PARSE_TXPOWER_MIN 	"MinTxPwr"
//#define GWS_CONF_HW_PARSE_TXPOWER_MAX 	"MaxTxPwr"
//#define GWS_CONF_HW_PARSE_BTXPOWER_MAX 	"BrdMaxPwr"
#define GWS_CONF_HW_PARSE_TXATTEN		"Tx Atten"
//#define GWS_CONF_HW_PARSE_ATTEN_MAX		"MaxAtten"

//#define GWS_CONF_HW_PARSE_RXCHAIN		"RX"
#define GWS_CONF_HW_PARSE_RXCAL			"rssi_cal"
//#define GWS_CONF_HW_PARSE_RXGAIN_MAX	"RxMaxGain"
#define GWS_CONF_HW_PARSE_RXFATTEN		"Rx Front Atten"
#define GWS_CONF_HW_PARSE_RXRATTEN		"Rx Rear Atten"


#define GWS_OFILE_HWREGION			"/var/run/gws_hwregion"
#define GWS_OFILE_HWCHANNEL			"/var/run/gws_hwchannel"
#define GWS_OFILE_HWTXCHAIN			"/var/run/gws_hwtxchain"
#define GWS_OFILE_HWTXPOWER			"/var/run/gws_hwtxpower"
#define GWS_OFILE_HWRXGAIN			"/var/run/gws_hwrxgain"

#endif /* GWS5K_HW_H_ */
