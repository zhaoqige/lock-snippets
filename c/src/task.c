/*
 * task.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *  Updated on: Nov 11, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */
#include <stdio.h>
#include <signal.h>

#include "app.h"
#include "task.h"

#if defined(HW_GWS5K) && defined(USE_TPC)
#include "gws5k.h"
#endif


int FLAG_SIG_EXIT = 0;

static void task_init(void);
int (*task_core)(const void *);
void (*task_exit)(void);


// TODO: main logic/algorithm
// make your own algorithm here
// re-organize your calls here
int  task(APP_CONF *app_conf)
{
	task_init();

	(*task_core)(app_conf);
	(*task_exit)();

	return TASK_OK;
}


// set function pointer
static void task_init(void)
{
#if defined(_HW_GWS5K) && defined(_HW_SPI)
	task_core = &gws5k_run;
	task_idle = &gws5k_idle;
#endif
}


// mark FLAG_SIG_EXIT
void task_prepare_exit(void)
{
	printf("\n* SIGNAL caught, prepare to exit!\n");

	// TODO: syslog() signal exit
	FLAG_SIG_EXIT = 1;

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}
