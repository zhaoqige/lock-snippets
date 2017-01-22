/*
 * lp_cache.c
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: YY WANG, Qige
 *  Maintainer: Qige
 */
#include <stdio.h>
#include <string.h>
#include "lp_cache.h"


void lpc_init(LPCache *lpc)
{
	memset(lpc, 0, sizeof(LPCache));

	lpc->start = lpc->data;
	lpc->length = sizeof(lpc->data) - 1;

	lpc->data_head = lpc->start;
	lpc->data_length = 0;
}

/*
 * save data (str/hex) to lpc
 * - test data length;
 * - test bytes available;
 * - calc basic counters;
 * - copy
 */
int lpc_save(LPCache *lpc, const byte *data, const uint data_length)
{
	//+ buffer length = data length + data available length
	uint lpc_length, lpc_data_length, lpc_da_length;
	lpc_length = lpc->length;
	lpc_data_length = lpc->data_length;
	lpc_da_length = lpc_length - lpc_data_length;

	if (lpc_da_length < 1)
		return LPC_ERR_CACHE_IS_FULL;
	if (data_length < 1)
		return LPC_ERR_DATA_INVALID;
	if (data_length > lpc_da_length)
		return LPC_ERR_DATA_TOOLONG;

	//+ buffer length = data head to start + data head to tail
	uint dh_to_start, dh_to_tail;
	dh_to_start = lpc->data_head - lpc->start;
	dh_to_tail = lpc_length - dh_to_start;

	//+ data available
	uint da_tail;
	if (dh_to_tail >= lpc_data_length) {
		da_tail = dh_to_tail - lpc_data_length;
	} else {
		da_tail = 0;
	}

	//+ save
	if (lpc_data_length > 0) {
		if (da_tail >= data_length) {
			memcpy(lpc->data_head + lpc_data_length, data, data_length);
		} else {

			/*memcpy(lpc->data_head + lpc_data_length, data, da_tail);
			if (lpc_data_length > dh_to_tail) {
				memcpy(lpc->start+(lpc_data_length-dh_to_tail), &data[da_tail], data_length - da_tail);
			} else {
				memcpy(lpc->start, &data[da_tail], data_length - da_tail);
			}*/

			if (da_tail > 0) {
				memcpy(lpc->data_head + lpc_data_length, data, da_tail);
			}

			memcpy(lpc->start, data + da_tail, data_length - da_tail);
		}
	} else {
		memcpy(lpc->start, data, data_length);
	}
	lpc->data_length += data_length;

	return LPC_OK;
}

/*
 * read loopback cache data (ascii/hex)
 */
int lpc_read(LPCache *lpc, byte *buff, uint *buff_length)
{
	if (*buff_length < 1)
		return LPC_ERR_DATA_INVALID;

	//+ cache length = data length + data available length
	uint lpc_length, lpc_data_length;
	lpc_length = lpc->length;
	lpc_data_length = lpc->data_length;

	//+ cache length = data head to start + data head to tail
	uint dh_to_head, dh_to_tail;
	dh_to_head = lpc->data_head - lpc->start;
	dh_to_tail = lpc_length - dh_to_head;

	//+ calc bytes to read
	int b2read = MIN(*buff_length, lpc_data_length);
	*buff_length = b2read;

	if (dh_to_tail >= b2read) {
		memcpy(buff, lpc->data_head, b2read);
	} else {
		memcpy(buff, lpc->data_head, dh_to_tail);
		memcpy(&buff[dh_to_tail], lpc->start, b2read - dh_to_tail);
	}

#ifdef LPC_CLEAR_WHEN_READ
	lpc_move(lpc, *buff_length);
#endif

	return LPC_OK;
}

/*
 * discard data of b2move bytes
 * - data length <= b2move, init
 * -
 */
int lpc_move(LPCache *lpc, uint move)
{
	if (move < 1)
		return LPC_OK;

	//+ cache length = data length + data available length
	unsigned int lpc_length, lpc_data_length;
	lpc_length = lpc->length;
	lpc_data_length = lpc->data_length;

	//+ cache length = data head to start + data head to tail
	unsigned int dh_to_head, dh_to_tail;
	dh_to_head = lpc->data_head - lpc->start;
	dh_to_tail = lpc_length - dh_to_head;

	if (lpc_data_length > move) {
		if (dh_to_tail > move) {
#ifdef LPC_CLEAR_WHEN_MOVE
			int i;
			for(i = 0; i < move; i ++)
				*(lpc->data_head+i) = '\0';
#endif
			lpc->data_head += move;
		} else {
#ifdef LPC_CLEAR_WHEN_MOVE
			int i;
			for(i = 0; i <= dh_to_tail; i ++)
				*(lpc->data_head + i) = '\0';
			for(i = 0; i < move - dh_to_tail; i ++)
				*(lpc->start + i) = '\0';
#endif
			lpc->data_head = lpc->start + (move - dh_to_tail);
		}
		lpc->data_length -= move;
	} else {
		lpc_init(lpc);
	}

	return LPC_OK;
}

