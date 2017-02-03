/*
 * abb_iwinfo.c
 *
 *  Created on: Feb 1, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "_env.h"
//#include "iwinfo/iwinfo.h"

// check in main loop
// 0: continue; 1: break & stop.
int FLAG_SIG_EXIT = 0;

static void abb_iwinfo_stop(void);

int Abb_iwinfo_run(const void *env)
{
	DBG("Abb_iwinfo_run()\n");

	int i, sum = 0;
	for(i = 1; i <=10; i ++) {
		if (FLAG_SIG_EXIT)
			break;

		sum += i;
		DBG("> loop %d\n", i);
		sleep(1);
	}

	abb_iwinfo_stop();

	return 0;
}


static void abb_iwinfo_stop(void)
{
	DBG("abb_iwinfo_stop()\n");
}

// mark FLAG, to break main loop
void Abb_iwinfo_signal(void)
{
	DBG("Abb_iwinfo_signal(): mark FLAG_SIG_EXIT\n");
	FLAG_SIG_EXIT = 1;
}
