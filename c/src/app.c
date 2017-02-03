/*
 * app.c
 *
 *  Created on: Jul 25, 2016
 *  Updated on: Aug 6, 2016
 *  Updated on: Jan 31, 2017 - Feb 1, 2017
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include <signal.h>

#include "_env.h"
#include "app.h"
#include "task.h"


static void app_version(void);
static void app_help(const char *app);
static void app_daemon(void);

/*
 * read user input
 * parse & save to app_conf
 * call main task()
 *
 * version 1.1.310117
 * todo: getopt(), getopt_long()
 */
int main(int argc, char **argv)
{
	// read & parse user cli input
	int ret = 0;
	const char *app = argv[0];
	APP_ENV env;
	memset(&env, 0, sizeof(env));

#ifdef USE_GETOPT
	DBG("read command line params (getopt())\n");

	// verified by Qige @ 2017.01.31
	int c = 0;
	while((c = getopt(argc, argv, "Dvhdi:b:k:")) != -1) {
		switch(c) {
		case 'i':
			snprintf(env.conf.ifname, ABB_IFNAME_LENGTH, "%s", optarg);
			break;
		case 'h':
			env.flag.help = 1;
			break;
		case 'v':
			env.flag.version = 1;
			break;
		case 'd':
			env.flag.debug = 1;
			break;
		case 'D':
			env.flag.daemon = 1;
			break;
		default:
			break;
		}
	}
#endif

#ifdef USE_GETOPT_LONG
	DBG("read command line params (getopt_long())\n");

	// todo: verify in gdbserver
	for(;;) {
		int option_index = 0, c = 0;
		static struct option long_options[] = {
				{ "h", 			no_argument, 0, 0 },
				{ "help", 		no_argument, 0, 0 },
				{ "v", 			no_argument, 0, 0 },
				{ "version", 	no_argument, 0, 0 },
				{ "d", 			no_argument, 0, 0 },
				{ "debug", 		no_argument, 0, 0 },
				{ "D", 			no_argument, 0, 0 },
				{ "daemon", 	no_argument, 0, 0 },
				{ "i", 			required_argument, 0, 0 },
				{ "ifname", 	required_argument, 0, 0 },
				{ 0, 			no_argument, 0, 0 }
		};

		//c = getopt_long(argc, argv, "", long_options, &option_index);
		c = getopt_long_only(argc, argv, "", long_options, &option_index);

		// no more params
		if (c == -1) break;

		// unknown param
		if (c == '?') continue;

		// handle param
		switch(option_index) {
		case 0:
		case 1:
			env.flag.help = 1;
			return 0;
			break;
		case 2:
		case 3:
			env.flag.version = 1;
			return 0;
			break;
		case 4:
		case 5:
			env.flag.debug = 1;
			break;
		case 6:
		case 7:
			env.flag.daemon = 1;
			break;
		case 9:
		case 10:
			snprintf(env.conf.ifname, ABB_IFNAME_LENGTH, "%s", optarg);
			break;
		default: // running with default values
			break;
		}
	}
#endif


	DBG("check flags\n");
	if (env.flag.help) {
		//app_version();
		app_help(app);
		return 0;
	}

	if (env.flag.version) {
		app_version();
		return 0;
	}

	if (env.flag.debug) {
		app_version();
	}

	env.conf.pid = getpid();
	snprintf(env.conf.app, APP_NAME_LENGTH, "%s", app);
	if (env.flag.daemon) {
		app_daemon();
		env.conf.pid = getpid();
		LOG("running daemon (%s, pid=%d)\n", env.conf.app, env.conf.pid);
	}

	LOG("started (%s, pid=%d)\n", env.conf.app, env.conf.pid);
	ret = Task(&env);
	return ret;
}

// todo: verify with gdbserver
// detach terminal
// run in backgrund
static void app_daemon(void)
{
	int fr=0;

	fr = fork();
	if( fr < 0 ) {
		fprintf(stderr, "fork() failed\n");
		exit(1);
	}
	if ( fr > 0 ) {
		exit(0);
	}

	if( setsid() < 0 ) {
		fprintf(stderr, "setsid() failed\n");
		exit(1);
	}

	fr = fork();
	if( fr < 0 ) {
		fprintf(stderr, "fork() failed\n");
		exit(1);
	}
	if ( fr > 0 ) {
		//fprintf(stderr, "forked to background (%d)\n", fr);
		exit(0);
	}

	umask(0);

	chdir("/");
	close(0);
	close(1);
	close(2);

	open("/dev/null", O_RDWR);
	dup(0);
	dup(0);
}


static void app_version(void)
{
	printf("%s\n(%s)\n", APP_DESC, APP_VERSION);
}

static void app_help(const char *app)
{
#ifdef USE_GETOPT
	printf("  usage: %s [-D] [-i ifname]\n", app);
	printf("         %s [-d] [-v] [-h]\n", app);
#endif
#ifdef USE_GETOPT_LONG
	printf(" usage: %s [-D|--daemon] [-i|--ifname ifname]\n", app);
	printf("        %s [-d|--debug] [-v|--version|--ver] [-h|--help]\n", app);
#endif
}
