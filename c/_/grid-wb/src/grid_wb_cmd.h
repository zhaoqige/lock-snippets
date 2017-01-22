/*
 * grid_wb_cmd.h
 *
 *  Created on: May 23, 2016
 *      Author: qige
 */

#ifndef GRID_WB_CMD_H_
#define GRID_WB_CMD_H_

void grid_wb_cmd_init(void);
int  grid_wb_cmd_handler(const int uartfd, const int cmd_msgid, const void *gwsbb, const void *gwshw);
int  grid_wb_reply_handler(const int uartfd, const int cmd_msgid);

#endif /* GRID_WB_CMD_H_ */
