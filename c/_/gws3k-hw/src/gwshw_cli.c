/*
 * gwshw_cli.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: May 16, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdlib.h>

void exec_cmd(const char *cmd)
{
	system(cmd);
}
