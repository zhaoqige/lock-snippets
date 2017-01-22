/*
 * app.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include "app.h"
#include "task.h"


static void app_version(void);
static void app_help(const char *app);

/*
 * read user input
 * parse & save to app_conf
 * call main task()
 */
int main(int argc, char **argv)
{
	// release before interrupt/quit/terminated
	signal(SIGINT, 	(__sighandler_t) task_prepare_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) task_prepare_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) task_prepare_exit); //+ "kill", not "kill -9"

	// no zombies
	signal(SIGCHLD, SIG_IGN);


	int ret = 0;
	char opt = 0;
	APP_CONF app_conf;

	memset(&app_conf, 0, sizeof(app_conf));
	while((opt = getopt(argc, argv, "vhdP:")) != -1) {
		switch(opt) {
		case 'P':
			app_conf.port = atoi(optarg);
			break;
		case 'h':
			app_conf.flag.help = 1;
			break;
		case 'v':
			app_conf.flag.version = 1;
			break;
		case 'd':
			app_conf.flag.debug = 1;
			break;
		default:
			break;
		}
	}

	if (app_conf.flag.help) {
		//app_version();
		app_help(argv[0]);
		return 0;
	}

	if (app_conf.flag.version) {
		app_version();
		return 0;
	}

	ret = task(&app_conf);
	return ret;
}


static void app_version(void)
{
	printf("%s\n(%s)\n", APP_DESC, APP_VERSION);
}

static void app_help(const char *app)
{
	printf("  usage: %s [-P port]\n", app);
	printf("         %s [-d] [-v] [-h]\n", app);
}
