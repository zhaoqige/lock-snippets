/*
 * task.h
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 7, 2016
 *  Updated on: Nov 11, 2016
 *  Updated on: Jan 22, 2017 - Feb 1, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef TASK_H_
#define TASK_H_

#include "_def.h"

// ...
enum TASK_ERR {
	TASK_OK = 0,
	TASK_ERR_INIT = 1,
	TASK_ERR_JOB = 100,
	TASK_ERR_RES = 200,
	TASK_ERR_IPC = 300
};

// start with user input
int Task(const void *env);

#endif /* TASK_H_ */
