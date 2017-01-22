/*
 * app.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>

#include "app.h"
#include "task.h"


static void ui_print_version(void);
static void ui_print_help(const char *app);



/*
 * read user input
 * parse & save to config struct
 * call main task_run()
 */
int main(int argc, char **argv)
{
	signal(SIGINT, 	(__sighandler_t) task_prepare_exit); //+ "^C"
	signal(SIGQUIT, (__sighandler_t) task_prepare_exit); //+ "^\"
	signal(SIGTERM,	(__sighandler_t) task_prepare_exit); //+ "kill", not "kill -9"

	signal(SIGCHLD, SIG_IGN);


	int ret = APP_OK;
	char opt = 0;
	struct APP_CONF app_conf;

	memset(&app_conf, 0, sizeof(app_conf));
	while((opt = getopt(argc, argv, "A:s:vdhp:P:x:X:G:y:Y:")) != -1) {
		switch(opt) {
		case 'A':
			app_conf.algorithm = atoi(optarg);
			break;
		case 's':
			app_conf.intl = atoi(optarg); // second(s)
			break;
		case 'v':
			app_conf.echo_version = 1;
			break;
		case 'h':
			app_conf.echo_help = 1;
			break;
		case 'd':
			app_conf.debug = 1;
			break;
		case 'p':
			app_conf.comm_port = atoi(optarg);
			break;
		case 'P':
			app_conf.txpwr_normal = atoi(optarg);
			break;
		case 'x':
			app_conf.txpwr_min = atoi(optarg);
			break;
		case 'X':
			app_conf.txpwr_max = atoi(optarg);
			break;
		case 'G':
			app_conf.rxgain_normal = atoi(optarg);
			break;
		case 'y':
			app_conf.rxgain_max_near = atoi(optarg);
			break;
		case 'Y':
			app_conf.rxgain_max = atoi(optarg);
			break;
		default:
			break;
		}
	}

	if (app_conf.echo_help) {
		//ui_print_version();
		ui_print_help(argv[0]);
		return 0;
	}

	if (app_conf.echo_version) {
		ui_print_version();
		return 0;
	}

	ret = task_run(&app_conf);

	return ret;
}


static void ui_print_version(void)
{
	printf("%s\n(%s)\n", APP_DESC, APP_VERSION);
}

static void ui_print_help(const char *app)
{
	printf("  usage: %s [-s intl] [-p port]\n", app);
	printf("         %s [-d] [-v] [-h]\n", app);
}
