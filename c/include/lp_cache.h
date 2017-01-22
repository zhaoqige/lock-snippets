/*
 * lp_cache.h
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: YY Wang, Qige
 *  Maintainer: Qige
 */

#ifndef LP_CACHE_H_
#define LP_CACHE_H_

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

#define MIN(x, y)					(x < y ? x : y)

// empty debug print out
#ifdef ENV_DEBUG_LPCACHE
#define LPC_DBG(format, ...)		{printf("<lp cache> "format, ##__VA_ARGS__);}
#else
#define LPC_DBG(fmt, ...)			{}
#endif


// features
#define LPC_CLEAR_WHEN_MOVE
#define LPC_CLEAR_WHEN_READ

// @returns
enum LPC_ERR {
	LPC_OK = 0,
	LPC_ERR_CACHE_IS_FULL = 1,
	LPC_ERR_DATA_INVALID = 100,
	LPC_ERR_DATA_TOOLONG
};


#ifdef DEBUG_LPC
#define LPC_CONF_BF_LENGTH		16
#else
#define LPC_CONF_BF_LENGTH		1024
#endif

typedef struct {
	char *start;
	uint length;
	char *data_head;
	uint data_length;
	byte data[LPC_CONF_BF_LENGTH];
} LPCache;


/*
 * init loopback cache
 */
void lpc_init(LPCache *lpc);

/*
 * return bytes that saved
 * return value < 0, error number
 */
int lpc_save(LPCache *lpc, const byte *data, const uint data_length);

/*
 * return bytes that read from cache
 * return value < 0, error number
 */
int lpc_read(LPCache *lpc, byte *buff, uint *buff_length);

/*
 * return bytes that cache discarded
 * return value = 0, init loopback cache when "move" > "data_length"
 * return value < 0, error number
 */
int lpc_move(LPCache *lpc, uint move);


#endif /* LP_CACHE_H_ */
