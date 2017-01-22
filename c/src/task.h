/*
 * task.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 7, 2016
 *  Updated on: Nov 11, 2016
 *  Updated on: Jan 22, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef TASK_H_
#define TASK_H_

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

// ...
enum TASK_ERR {
	TASK_OK = 0,
	TASK_ERR_INIT = 1,
	TASK_ERR_JOB = 100,
	TASK_ERR_RES = 200,
	TASK_ERR_IPC = 300
};

// start with user input
int  task(APP_CONF *app_conf);

// clean up before exit
void task_prepare_exit(void);

#endif /* TASK_H_ */
