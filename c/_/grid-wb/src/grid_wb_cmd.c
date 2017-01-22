/*
 * grid_wb_cmd.c
 *
 *  Created on: May 23, 2016
 *  Updated on: July 15, 2016
 *      Author: qige
 */

#include <stdio.h>
#include <string.h>

#include "six.h"

#include "grid.h"
#include "grid-wb.h"

static struct lbb rlbb;
static const char wb_chan_list[] = "21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60";
static const char wb_txpower_list[] = "0,3,6,9,12,15,18,21,24,27,30,33";


static int wbcmd_find_exec(const int msgid, const void *gwsbb, const void *gwshw,
		const char *buf, char *result, unsigned int *move_bytes);


void grid_wb_cmd_init(void)
{
	lbb_init(&rlbb);
}


/*
 * WB Command Reply Hanlder:
 * 1. recv cmd & result from msg;
 * 2. write to sp/uart;
 */
int  grid_wb_reply_handler(const int uartfd, const int cmd_msgid)
{
	unsigned int msg_length;
	struct GWS_MSG msg;

	do {
		memset(&msg, 0, sizeof(msg));
		msg.mtype = GWS_MSG_COM_WB;
		msg_length = sizeof(msg);

		if (msg_recv(cmd_msgid, &msg, &msg_length, GWS_MSG_COM_WB)) {
			//QZ("---> no msg for %d\n", GWS_MSG_COM_WB);
			break;
		}

		QZ("---> msg recv(%d-%d:%s)\n",
				msg.msg.cmd, msg.msg.num, msg.msg.data);

		switch(msg.msg.cmd) {
		case GWS_MSG_CMD_SET_SCAN_ECHO:
		case GWS_MSG_CMD_SET_SCAN_RESULT:
			uart_write(uartfd, msg.msg.data, strlen(msg.msg.data));
			break;
		default:
			break;
		}

	} while(msg.msg.cmd > 0);
	return GRID_WB_OK;
}


/*
 * WB Command Handler:
 * 1. recv cmd from sp/uart;
 * 2. save cmd to lbb;
 * 3. read & find cmd in lbb;
 * 4. exec cmd & reply;
 */
int grid_wb_cmd_handler(const int uartfd, const int cmd_msgid, const void *gwsbb, const void *gwshw)
{
	int ret = GRID_WB_OK;

	int recv_length;
	unsigned int read_length, move_bytes;
	char buf[32], result[256];

	//+ save cmd from sp/uart
	do {
		recv_length = -1;
		memset(buf, 0, sizeof(buf));
		recv_length = uart_read(uartfd, buf, sizeof(buf)-1);
		if (recv_length < 1) break;

		lbb_save(&rlbb, buf, recv_length);
	} while(recv_length > 0);

	//+ read data from lbb
	do {
		memset(buf, 0, sizeof(buf));
		read_length = sizeof(buf) - 1;
		if (lbb_read(&rlbb, buf, &read_length, 0) || read_length < 1) {
			ret = GRID_WB_ERR_NO_DATA_RECV;
			break;
		}

		//+ find cmd
		move_bytes = 0;
		memset(result, 0, sizeof(result));
		if (wbcmd_find_exec(cmd_msgid, gwsbb, gwshw, buf, result, &move_bytes)) {
			ret = GRID_WB_ERR_NO_CMD;
			break;
		}

		//+ reply cmd result
		if (strlen(result) > 0) {
			uart_write(uartfd, result, strlen(result));
		}

		//+ discard data
		if (move_bytes > 0) lbb_move(&rlbb, move_bytes);
	} while(read_length > GRID_WB_CONF_CMD_LENGTH_MIN);

	return ret;
}

/*
 * parse cmd:
 * +wbsetchn:<m>
 * +wbsetpwr:<m>
 * +wbsetmod:<m>
 */
static int wbcmd_find_exec(const int msgid, const void *gwsbb, const void *gwshw,
		const char *buf, char *result, unsigned int *move_bytes)
{
	int i, j, num;
	unsigned int cmd_start_pos, cmd_stop_pos;
	unsigned int buf_length, cmd_length;
	char start_byte = GRID_WB_CONF_START_BYTE, stop_byte = GRID_WB_CONF_STOP_BYTE;
	char cmd[GRID_WB_CONF_CMD_LENGTH_MAX], ok[] = "\r\n+wb:OK\r\n", err[] = "\r\n+wb:ERR\r\n";
	char pcmd[32];

	struct GWS_MSG gmsg;

	struct gwsbb *bb = (struct gwsbb *) gwsbb;
	struct gwshw *hw = (struct gwshw *) gwshw;


	buf_length = strlen(buf);
	if (buf_length < GRID_WB_CONF_CMD_LENGTH_MIN) return GRID_WB_ERR_DATA_TOOSHORT;

	cmd_start_pos = 0; cmd_stop_pos = 0;
	cmd_length = 0;
	do { // throw/catch

		//+ find head
		for(i = 0; i < buf_length; i ++) {
			if (buf[i] == start_byte) {
				cmd_start_pos = i;
				break;
			}
		}
		*move_bytes = i;

		//+ find tail
		for(j = i; j < buf_length; j ++) {
			if (buf[j] == stop_byte) {
				cmd_stop_pos = j;
				break;
			}
		}

		cmd_length = cmd_stop_pos - cmd_start_pos;
		if (cmd_length > GRID_WB_CONF_CMD_LENGTH_MAX || cmd_length < GRID_WB_CONF_CMD_LENGTH_MIN) return GRID_WB_ERR_CMD_BAD;

		//+ parse cmd
		memset(cmd, 0, sizeof(cmd));
		memcpy(cmd, &buf[cmd_start_pos], cmd_length);
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_SETCHN) > 0) {
			num = 0;
			if (sscanf(cmd, "+wbsetchn:%d", &num) != -1) {
				QZ("---> wbsetchn(%d)\n", num);

				gmsg.mtype = GWS_MSG_COM_HW;
				gmsg.msg.from = GWS_MSG_COM_WB;
				gmsg.msg.cmd = GWS_MSG_CMD_SET_HW_CHANNEL;
				gmsg.msg.num = num;

				msg_send(msgid, &gmsg, sizeof(gmsg));

				strcpy(result, ok);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_SETCHNBW) > 0) {
			num = 0;
			memset(pcmd, 0, sizeof(pcmd));
			if (sscanf(cmd, "+wbsetchnbw:%d", &num) != -1) {
				QZ("---> wbsetchnbw(%d)\n", num);
				sprintf(pcmd, "BW %d\n", num);
				cli_exec(pcmd);

				strcpy(result, ok);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_SETCMD) > 0) {
			QZ("---> wbcmd(%s)\n", cmd);
			memset(pcmd, 0, sizeof(pcmd));
			if (sscanf(cmd, "+wbcmd:%s", pcmd) != -1) {
				QZ("---> wbcmd(%s)\n", pcmd);
				cli_exec(pcmd);

				strcpy(result, ok);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_SETRGN) > 0) {
			num = 0;
			if (sscanf(cmd, "+wbsetrgn:%d", &num) != -1) {
				QZ("---> wbsetrgn(%d)\n", num);

				gmsg.mtype = GWS_MSG_COM_HW;
				gmsg.msg.from = GWS_MSG_COM_WB;
				gmsg.msg.cmd = GWS_MSG_CMD_SET_HW_REGION;
				gmsg.msg.num = num;

				msg_send(msgid, &gmsg, sizeof(gmsg));

				strcpy(result, ok);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_SETPWR) > 0) {
			num = 0;
			if (sscanf(cmd, "+wbsetpwr:%d", &num) != -1) {
				QZ("---> wbsetpwr(%d)\n", num);

				gmsg.mtype = GWS_MSG_COM_HW;
				gmsg.msg.from = GWS_MSG_COM_WB;
				gmsg.msg.cmd = GWS_MSG_CMD_SET_HW_TXPOWER;
				gmsg.msg.num = num;

				msg_send(msgid, &gmsg, sizeof(gmsg));

				strcpy(result, ok);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_SETMOD) > 0) {
			num = 0;
			if (sscanf(cmd, "+wbsetmod:%d", &num) != -1) {
				QZ("---> wbsetmod(%d)\n", num);

				gmsg.mtype = GWS_MSG_COM_HW;
				gmsg.msg.from = GWS_MSG_COM_WB;
				gmsg.msg.cmd = GWS_MSG_CMD_SET_BB_MODE;
				gmsg.msg.num = num;

				msg_send(msgid, &gmsg, sizeof(gmsg));

				strcpy(result, ok);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_WBSCAN) > 0) {
			QZ("---> wbscan\n");

			gmsg.mtype = GWS_MSG_COM_SCAN;
			gmsg.msg.from = GWS_MSG_COM_WB;
			gmsg.msg.cmd = GWS_MSG_CMD_SET_SCAN_START;
			gmsg.msg.num = num;

			msg_send(msgid, &gmsg, sizeof(gmsg));

			strcpy(result, ok);
			break;
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_GETCHN) > 0) {
			QZ("---> wbgetchn\n");
			sprintf(result, "\r\n+wb:%d\r\n", hw->channel);
			break;
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_GETCHNS) > 0) {
			QZ("---> wbgetchns\n");
			sprintf(result, "\r\n+wb:%s\r\n", wb_chan_list);
			break;
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_GETSIG) > 0) {
			num = 0;
			if (sscanf(cmd, "+wbgetsig%d", &num) != -1) {
				QZ("---> wbgetsig(%d)\n", num);

				sprintf(result, "\r\n+wb:%d\r\n", bb->avg.signal);
				break;
			}
		}
		if (strstr(cmd, GRID_WB_CONF_CMD_MASK_GETTXPWRS) > 0) {
			num = 0;
			if (sscanf(cmd, "+wbgetpwrs%d", &num) != -1) {
				QZ("---> wbgetpwrs(%d)\n", num);

				sprintf(result, "\r\n+wb:%s\r\n", wb_txpower_list);
				break;
			}
		}

		strcpy(result, err);

	} while(0);

	*move_bytes += cmd_length;
	return GRID_WB_OK;
}
