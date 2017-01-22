/*
 * app.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: 2016.11.11
 *  Updated on: Jan 22, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef APP_H_
#define APP_H_

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

#define APP_DESC		("App Name (vQZ)")
#define APP_VERSION		("v1.0.220117. Buildtime: "__DATE__", "__TIME__)

typedef struct {
	struct {
		int version;
		int help;
		int debug;
	} flag;
	uint port;
} APP_CONF;

#endif /* APP_H_ */
