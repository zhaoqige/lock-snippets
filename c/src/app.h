/*
 * app.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: 2016.11.11
 *  Updated on: Jan 31, 2017 - Feb 1, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef APP_H_
#define APP_H_

#include "_gws.h"
#include "_def.h"


#define APP_DESC				("GWS Analog Baseband")
#define APP_VERSION				("v11.0.010217. Buildtime: "__DATE__", "__TIME__)


// size define
#define APP_NAME_LENGTH			16

// ABB source is IWINFO
#if (defined(_ABB_SRC) && (_ABB_SRC == IWINFO))
#define ABB_IFNAME_LENGTH		16
#endif


// user input struct
typedef struct {
	struct {
		int version;
		int help;
		int debug;
		int daemon;
	} flag;
	struct {
		int pid;
		char app[APP_NAME_LENGTH];
		char key[GWS_APP_KEY_LENGTH];

#if (defined(_ABB_SRC) && (_ABB_SRC == IWINFO))
		int bw;
		char ifname[ABB_IFNAME_LENGTH];
#endif

	} conf;
} APP_ENV;

#endif /* APP_H_ */
