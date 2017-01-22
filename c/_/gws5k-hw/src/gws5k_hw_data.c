/*
 * task_data.c
 *
 *  Created on: Apr 29, 2016
 *  Updated on: July 5, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include <unistd.h>

#include "grid.h"
#include "six.h"

#include "gws5k_hw.h"
#include "gws5k_hw_data.h"



static void str2file(const char *filename, const char *str);
static char *getval(const char *kvpair, const char split);

static int task_data_parser(void *hw, const char *buffer);



/*
 * force init when running
 * - turn on txchain
 * - set max txpower
 * - set rxgain to 12
 */
void task_data_selfcheck(const int uartfd)
{
	//cli_exec(GWS_CONF_HW_TXON);
	cli_exec(GWS_CONF_HW_TXPOWER);
	cli_exec(GWS_CONF_HW_RXGAIN);
}


/*
 * 1. send cmd
 * 2. read data from uart/spi, then save to (struct lbb *) b;
 * 3. parse data into struct gwshw;
 * 4. save to shm;
 * 5. save to file;
 */
int task_data_update(void *hw, const char *cmd)
{
	char result[GWS_CONF_HW_RFINFO_LENGTH];

	memset(result, 0, sizeof(result));
	snprintf(result, sizeof(result)-1, "%s", cli_read(cmd));

	DBG_HW("data/parse > parsing %d bytes\n", strlen(result));
	task_data_parser(hw, result);

	return GWS_HW_OK;
}

/*
 * parse rfinfo output into struct gwshw
 * - split by \r or \n
 * - get string
 * - sscanf()
 * - save b2move
 */
static int task_data_parser(void *hw, const char *buffer)
{
	unsigned int buffer_length;

	struct gwshw *ghw = (struct gwshw *) hw;

	// TODO: split str by "\r" or "\n"
	int i, j, read_offset;
	char str[GWS_CONF_HW_LINE_LENGTH];

	buffer_length = strlen(buffer);

	if (buffer_length < 1)
		return GWS_HW_ERR_DATA;

	//* start parse
	char param[GWS_CONF_HW_LINE_LENGTH];
	read_offset = 0;
	do { // GOTO
		j = 0;
		memset(str, 0x00, sizeof(str));
		for(i = read_offset; i < buffer_length; i ++) {
			if (isprint(buffer[i])) {
				str[j] = buffer[i];
				j ++;
			} else {
				if (buffer[i] == '\r' || buffer[i] == '\n') {
					if (strlen(str) > 0) {
						read_offset = i;
						break;
					}
				}
			}
		}
		if (i >= buffer_length) break;


		DBG_HW("data/parse/line: %s (%d/%d/%d)\n", str, i, j, read_offset);

		do { // goto
			memset(param, 0, sizeof(param));
			if (strlen(str) < GWS_CONF_HW_LINE_MIN)
				break;

			if (strstr(str, GWS_CONF_HW_PARSE_REGION)) {
				strcpy(param, getval(str, ':'));
				ghw->region = atoi(param);
				DBG_HW("data/parser/change > region=%d\n", ghw->region);
				break;
			}

			if (strstr(str, GWS_CONF_HW_PARSE_CHANNEL)) {
				strcpy(param, getval(str, ':'));
				ghw->channel = atoi(param);
				DBG_HW("data/parser/change > channel=%d\n", ghw->channel);
				break;
			}

			if (strstr(str, GWS_CONF_HW_PARSE_TXCHAIN)) {
				ghw->txchain = 1;
				DBG_HW("data/parser/change > txchain=%d\n", ghw->txchain);
				break;
			}

			if (strstr(str, GWS_CONF_HW_PARSE_TXPOWER)) {
				strcpy(param, getval(str, ':'));
				ghw->txpower = atoi(param);
				DBG_HW("data/parser/change > txpower=%d\n", ghw->txpower);
				break;
			}

			if (strstr(str, GWS_CONF_HW_PARSE_RXGAIN)) {
				strcpy(param, getval(str, ':'));
				ghw->rxgain = atoi(param);
				DBG_HW("data/parser/change > rxgain=%d\n", ghw->rxgain);
				break;
			}
			if (strstr(str, GWS_CONF_HW_PARSE_RXCAL)) {
				strcpy(param, getval(str, ':'));
				if (strstr(param, "ON")) {
					ghw->adv_rxcal = 1;
				} else {
					ghw->adv_rxcal = 0;
				}
				DBG_HW("data/parser/change > rxcal=%d\n", ghw->adv_rxcal);
				break;
			}

			if (strstr(str, GWS_CONF_HW_PARSE_TXATTEN)) {
				strcpy(param, getval(str, ':'));
				ghw->adv_txatten = atoi(param);
				DBG_HW("data/parser/change > txatten=%d\n", ghw->adv_txatten);
				break;
			}


			if (strstr(str, GWS_CONF_HW_PARSE_RXFATTEN)) {
				strcpy(param, getval(str, ':'));
				ghw->adv_rxfatten = atoi(param);
				DBG_HW("data/parser/change > rxfatten=%d\n", ghw->adv_rxfatten);
				break;
			}

			if (strstr(str, GWS_CONF_HW_PARSE_RXRATTEN)) {
				strcpy(param, getval(str, ':'));
				ghw->adv_rxratten = atoi(param);
				DBG_HW("data/parser/change > rxratten=%d\n", ghw->adv_rxratten);
				break;
			}

		} while(0); // goto end
	} while(strlen(str) >= GWS_CONF_HW_LINE_MIN);

	return GWS_HW_OK;
}


/*
 * save data to shm
 */
int task_data_save(void *src, void *des)
{
	memcpy(des, src, sizeof(struct gwshw));
	return GWS_HW_OK;
}

int task_data_clean(void *hw)
{
	memset(hw, 0x00, sizeof(struct gwshw));
	return GWS_HW_OK;
}

void task_data_print(void *hw)
{
	int region, channel, frequency;
	struct gwshw *ghw = (struct gwshw *) hw;

	region = ghw->region;
	channel = ghw->channel;
	if (region) {
		frequency = R1FREQ(channel);
	} else {
		frequency = R0FREQ(channel);
	}
	ghw->freq = frequency;

	printf("   Region: %d\n", ghw->region);
	printf("  Channel: %d (%d MHz)\n", ghw->channel, ghw->freq);
	printf(" Tx Power: %d dBm (step 1 dBm)\n", ghw->txpower);
	printf("    TX/Rx: TxCal: %d, RxCal: %d\n\n", ghw->adv_txcal, ghw->adv_rxcal);
}
/*
 * write to /var/run/gws_hw* files
 */
void task_data_save2file(void *hw)
{
	char str[16];
	struct gwshw *ghw = (struct gwshw *) hw;

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%d\n", ghw->region);
	str2file(GWS_OFILE_HWREGION, str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%d\n", ghw->channel);
	str2file(GWS_OFILE_HWCHANNEL, str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%d\n", ghw->txchain);
	str2file(GWS_OFILE_HWTXCHAIN, str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%d\n", ghw->txpower);
	str2file(GWS_OFILE_HWTXPOWER, str);

	memset(str, 0, sizeof(str));
	snprintf(str, sizeof(str), "%d\n", ghw->rxgain);
	str2file(GWS_OFILE_HWRXGAIN, str);
}
static void str2file(const char *filename, const char *str)
{
	FILE *fd = fopen(filename, "w+"); // FIXME: truncate when open
	fputs(str, fd);
	fclose(fd);
}

static char *getval(const char *kvpair, const char split)
{
	int i, j, flag_start, kvp_length = strlen(kvpair);
	static char val[16];

	j = 0;
	flag_start = 0;
	memset(val, 0x00, sizeof(val));
	for(i = 0; i < kvp_length; i ++) {
		if (flag_start && isprint(kvpair[i])) {
			if (strlen(val) || kvpair[i] != ' ') {
				val[j] = kvpair[i];
				j ++;
			}
		} else {
			if (kvpair[i] == split) {
				flag_start = i;
			}
		}
	}

	return val;
}
