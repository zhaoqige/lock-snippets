/*
 * _env.h
 *
 *  Created on: Jan 31, 2017
 *  Updated on: Feb 1, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef ENV_H_
#define ENV_H_

// call syslog()
#define LOG(fmt, ...) 	{ \
							char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, fmt, ##__VA_ARGS__); \
							fprintf(stderr, "> %s", _bf); syslog(LOG_INFO, "%s", _bf); \
						}

// -DDEBUG symbol to control debug msg
#ifdef DEBUG
#define DBG(fmt, ...)	{ \
							fprintf(stderr, "dbg> "fmt, ##__VA_ARGS__); \
						}
#else
#define DBG(fmt, ...)	{}
#endif


// AND: &&
// OR: ||
#if (!defined(USE_GETOPT) && !defined(USE_GETOPT_LONG))
#define USE_GETOPT
#endif

// determine Analog Baseband source
#if (!defined(_ABB_SRC))
#define _ABB_SRC		IWINFO
#endif

// determine radio platform
#if (!defined(_RADIO_MODEL))
#define _RADIO_MODEL	GWS5K
#endif

#endif /* ENV_H_ */
