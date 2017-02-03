/*
 * task.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *  Updated on: Nov 11, 2016
 *  Updated on: Jan 22, 2017 - Feb 1, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include <syslog.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "_env.h"
#include "app.h"
#include "task.h"

#include "abb_iwinfo.h"


static void task_init(void);
static void task_prepare_exit(void);

int (*task_run)(const void *);
void (*task_signal)(void);


// MAIN Calling sequences.
// - init()
// - run()
//
// if you want to fork a process, do it here;
int Task(const void *env)
{
	int ret;

	task_init();
	ret = (*task_run)(env);

	return ret;
}


// set function pointer
// handle signal()
static void task_init(void)
{
	// hook function
#if (defined(_ABB_SRC) && (_ABB_SRC == IWINFO))
	task_run = &Abb_iwinfo_run;
	task_signal = &Abb_iwinfo_signal;
#endif

	// release before interrupt/quit/terminated
	DBG("handling signal(SIGINT, SIGQUIT, SIGTERM)\n");
	signal(SIGINT, 	(__sighandler_t) task_prepare_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) task_prepare_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) task_prepare_exit); //+ "kill", not "kill -9"

	// release zombies
	// comment next line if "wait()/waitpid()"
	DBG("handling signal(SIGCHLD): SIG_IGN\n");
	signal(SIGCHLD, SIG_IGN);
}


// call & reset signal(), tell your real executor, it's time to exit
// so hit ^c twice will interrupt right away
static void task_prepare_exit(void)
{
	LOG("signal(SIGINT, SIGQUIT, SIGTERM) detected, exiting...\n");
	(*task_signal)();

	signal(SIGQUIT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
}
