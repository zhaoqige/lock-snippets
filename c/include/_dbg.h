/*
 * _dbg.h
 *
 *  Created on: Jan 19, 2017
 *      Author: Qige
 */

#ifndef DBG_H_
#define DBG_H_


#ifndef _DBG
#define _DBG(fmt, ...)			(printf("<dbg> "fmt, ##__VA_ARGS__))
#else
#define _DBG(fmt, ...)      {}
#endif


#endif /* DBG_H_ */
