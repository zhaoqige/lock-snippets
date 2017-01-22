/*
 * gwshw_data.c
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
#include "uart.h"
#include "lbb.h"

#include "gwshw.h"
#include "gwshw_data.h"



static void str2file(const char *filename, const char *str);
static char *getval(const char *kvpair, const char split);

static int gwshw_data_parser(void *hw, const char *rfinfo, unsigned int *b2move);



/*
 * force init when running
 * - turn on txchain
 * - set max txpower
 * - set rxgain to 12
 */
void gwshw_data_selfcheck(const int uartfd)
{
	uart_write(uartfd, GWS_CONF_HW_TXON, strlen(GWS_CONF_HW_TXON));
	uart_write(uartfd, GWS_CONF_HW_TXPOWER, strlen(GWS_CONF_HW_TXPOWER));
	uart_write(uartfd, GWS_CONF_HW_RXGAIN, strlen(GWS_CONF_HW_RXGAIN));
}


/*
 * 1. send cmd
 * 2. read data from uart/spi, then save to (struct lbb *) b;
 * 3. parse data into struct gwshw;
 * 4. save to shm;
 * 5. save to file;
 */
int gwshw_data_update(const int uartfd, void *hw, void *b, const char *cmd)
{
	struct lbb *lb = (struct lbb *) b;
	struct gwshw *ghw = (struct gwshw *) hw;

	int flag_clean;
	unsigned int line_length, b2move;
	char line[GWS_CONF_HW_LINE_LENGTH];

	int b2rw, total_read = 0;
	char buf[GWS_CONF_HW_RFINFO_LENGTH];


	b2rw = uart_write(uartfd, cmd, strlen(cmd));
	if (b2rw < strlen(cmd)) {
		QZ("data/renew > uart_write failed\n");
		return GWS_HW_ERR_UART_WRITE;
	}
	QZ("data/renew > cmd (%d) sent to radio\n", strlen(cmd));


	usleep(GWS_CONF_HW_PARSE_IDLE); //* wait for response


	//* recv
	QZ("data/renew > reading result from radio\n");
	memset(buf, 0x00, sizeof(buf));
	while((b2rw = uart_read(uartfd, buf, sizeof(buf)-1)) > 0) {
		total_read += b2rw;
		QZ("data/read > [%s]\n", buf);
		if (lbb_save(lb, buf, strlen(buf))) {
			printf("(warning) * lbb_save failed\n");
			//* FIXME: when lbb is full, discard all previous data?
			lbb_init(lb);
			total_read = 0;
		}
		memset(buf, 0x00, sizeof(buf));
	}
	QZ("data > read %d bytes\n", total_read);


	//* parse data per line
	do {
		QZ("data/parse > read rfinfo\n");
		flag_clean = 0;
		memset(line, 0x00, sizeof(line));
		line_length = sizeof(line)-1;
		if (lbb_read(lb, line, &line_length, flag_clean) || line_length < 1) {
			return GWS_HW_ERR_DATA_LBB;
		}

		if (line_length < GWS_CONF_HW_LINE_MIN) break;

		QZ("data/parse > parsing %d bytes\n", line_length);
		gwshw_data_parser(ghw, line, &line_length);

		b2move = line_length;
		QZ("data/parse > delete data from lbb (%d bytes)\n", b2move);
		lbb_move(lb, b2move);
	} while(line_length > GWS_CONF_HW_LINE_MIN);

	return GWS_HW_OK;
}

/*
 * parse rfinfo output into struct gwshw
 * - split by \r or \n
 * - get string
 * - sscanf()
 * - save b2move
 */
static int gwshw_data_parser(void *hw, const char *line, unsigned int *line_length)
{
	struct gwshw *ghw = (struct gwshw *) hw;

	// TODO: split str by "\r" or "\n"
	int i, j = 0, b2move;
	char str[GWS_CONF_HW_LINE_LENGTH];
	memset(str, 0x00, sizeof(str));
	for(i = 0; i < *line_length; i ++) {
		if (isprint(line[i])) {
			str[j] = line[i];
			j ++;
		} else {
			if (line[i] == '\r' || line[i] == '\n') {
				b2move = i;
				if (strlen(str) > 0) {
					break;
				}
			}

			memset(str, 0x00, sizeof(str));
			j = 0;
		}
	}

	//* save last \n or \r pos
	*line_length = b2move;

	//* start parse
	char param[GWS_CONF_HW_LINE_LENGTH];
	do { // GOTO
		memset(param, 0, sizeof(param));

		QZ("data/parse/line: %s\n", str);
		if (strlen(str) < GWS_CONF_HW_LINE_MIN) break;

		if (strstr(str, GWS_CONF_HW_PARSE_REGION)) {
			strcpy(param, getval(str, ':'));
			ghw->region = atoi(param);
			QZ("data/parser/change > region=%d\n", ghw->region);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_CHANNEL)) {
			strcpy(param, getval(str, ':'));
			ghw->channel = atoi(param);
			QZ("data/parser/change > channel=%d\n", ghw->channel);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TXCAL)) {
			strcpy(param, getval(str, ':'));
			if (strstr(param, "ON")) {
				ghw->adv_txcal = 1;
			} else {
				ghw->adv_txcal = 0;
			}
			QZ("data/parser/change > txcal=%d\n", ghw->adv_txcal);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TXCHAIN)) {
			strcpy(param, getval(str, ':'));
			if (strstr(param, "ON")) {
				ghw->txchain = 1;
			} else {
				ghw->txchain = 0;
			}
			QZ("data/parser/change > txchain=%d\n", ghw->txchain);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TXPOWER)) {
			strcpy(param, getval(str, ':'));
			ghw->txpower = atoi(param);
			QZ("data/parser/change > txpower=%d\n", ghw->txpower);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXGAIN)) {
			strcpy(param, getval(str, ':'));
			ghw->rxgain = atoi(param);
			QZ("data/parser/change > rxgain=%d\n", ghw->rxgain);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TEMP)) {
			strcpy(param, getval(str, ':'));
			ghw->temp = atoi(param);
			QZ("data/parser/change > temp=%d\n", ghw->temp);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_AGCMODE)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_agcmode = atoi(param);
			QZ("data/parser/change > agcmode=%d\n", ghw->adv_agcmode);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_IFOUT)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_ifout = atoi(param);
			QZ("data/parser/change > ifout=%d\n", ghw->adv_ifout);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TXPOWER_MIN)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_mintxpwr = atoi(param);
			QZ("data/parser/change > txpower(min)=%d\n", ghw->adv_mintxpwr);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TXPOWER_MAX)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_maxtxpwr = atoi(param);
			QZ("data/parser/change > txpower(max)=%d\n", ghw->adv_maxtxpwr);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_BTXPOWER_MAX)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_brdmaxpwr = atoi(param);
			QZ("data/parser/change > brdpower(max)=%d\n", ghw->adv_brdmaxpwr);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_TXATTEN)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_txatten = atoi(param);
			QZ("data/parser/change > txcal=%d\n", ghw->adv_txatten);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_ATTEN_MAX)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_maxatten = atoi(param);
			QZ("data/parser/change > maxatten=%d\n", ghw->adv_maxatten);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXCAL)) {
			strcpy(param, getval(str, ':'));
			if (strstr(param, "ON")) {
				ghw->adv_rxcal = 1;
			} else {
				ghw->adv_rxcal = 0;
			}
			QZ("data/parser/change > rxcal=%d\n", ghw->adv_rxcal);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXCHAIN)) {
			strcpy(param, getval(str, ':'));
			ghw->rxgain = atoi(param);
			QZ("data/parser/change > rxgain=%d\n", ghw->rxgain);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXGAIN_MAX)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_rxmaxgain = atoi(param);
			QZ("data/parser/change > rxgain(max)=%d\n", ghw->adv_rxmaxgain);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXFATTEN)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_rxfatten = atoi(param);
			QZ("data/parser/change > rxfatten=%d\n", ghw->adv_rxfatten);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXFATTEN)) {
			strcpy(param, getval(str, ':'));
			ghw->adv_rxratten = atoi(param);
			QZ("data/parser/change > rxratten=%d\n", ghw->adv_rxratten);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_RXCHAIN)) {
			strcpy(param, getval(str, ':'));
			if (strstr(param, "ON")) {
				ghw->adv_rx = 1;
			} else {
				ghw->adv_rx = 0;
			}
			QZ("data/parser/change > rx=%d\n", ghw->adv_rx);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_SN)) {
			strcpy(ghw->hw_sn, getval(str, ':'));
			QZ("data/parser/change > hwsn=%s\n", ghw->hw_sn);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_FWV)) {
			memset(ghw->fw_ver, 0x00, sizeof(ghw->fw_ver));
			strcpy(ghw->fw_ver, str);
			QZ("data/parser/change > fwv=%s\n", ghw->fw_ver);
			break;
		}

		if (strstr(str, GWS_CONF_HW_PARSE_HWV)) {
			memset(ghw->hw_ver, 0x00, sizeof(ghw->hw_ver));
			strcpy(ghw->hw_ver, str);
			break;
		}
	} while(0);

	return GWS_HW_OK;
}


/*
 * save data to shm
 */
int gwshw_data_save(void *src, void *des)
{
	memcpy(des, src, sizeof(struct gwshw));
	return GWS_HW_OK;
}

int gwshw_data_clean(void *hw)
{
	memset(hw, 0x00, sizeof(struct gwshw));
	return GWS_HW_OK;
}

void gwshw_data_print(void *hw)
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

	printf("%s\n", ghw->hw_ver);
	printf("%s\n\n", ghw->fw_ver);
	printf("      S/N: %s\n", ghw->hw_sn);

	printf("   Region: %d\n", ghw->region);
	printf("  Channel: %d (%d MHz)\n", ghw->channel, ghw->freq);
	printf("       TX: %d (TxCal: %d)\n", ghw->txchain, ghw->adv_txcal);
	printf(" Tx Power: %d dBm (step 0.5 dBm)\n", ghw->txpower);
	printf("       Rx: %d (RxCal: %d)\n\n", ghw->rxgain, ghw->adv_rxcal);

	printf("     Temp: %d Degree\n\n", ghw->temp);

	printf(" AGC & IF: %d & %d\n", ghw->adv_agcmode, ghw->adv_ifout);
	printf("   Adv Tx: Min %d/ Max %d/ BMax %d dBm, Atten %d/ MaxAtten %d\n",
			ghw->adv_mintxpwr, ghw->adv_maxtxpwr, ghw->adv_brdmaxpwr,
			ghw->adv_txatten, ghw->adv_maxatten);

	printf("   Adv Rx: MaxGain %d/ FAtten %d/ RAtten %d\n",
			ghw->adv_rxmaxgain, ghw->adv_rxfatten, ghw->adv_rxratten);
}
/*
 * write to /var/run/gws_hw* files
 */
void gwshw_data_save2file(void *hw)
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
