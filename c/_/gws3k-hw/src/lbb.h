/*
 * lbb.h
 * - Loopback Buffer
 *
 *  Created on: Apr 29, 2016
 *  Updated on: June 07, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef LBB_H_
#define LBB_H_


//#define DEBUG_LBB				// TODO: comment this line in real app
//#define CLEAR_WHEN_MOVE


enum LBB_ERR {
	LBB_OK,
	LBB_ERR_NOSPACE,
	LBB_ERR_DATATOOLONG,
	LBB_ERR_DATAINVALID
};


#define MIN(x, y)					(x < y ? x : y)

#ifdef DEBUG_LBB
#define LBB_CONF_DATA_LENGTH		16
#else
#define LBB_CONF_DATA_LENGTH		1024
#endif

struct lbb {
	char 		*lbb_start;
	unsigned int lbb_length;
	char		*lbb_data_head;
	unsigned int lbb_data_length;
	char 		 lbb_data[LBB_CONF_DATA_LENGTH];

};


/*
 * init loopback buffer
 */
void lbb_init(void *ptr);

/*
 * return bytes that saved
 * return value < 0, error number
 */
int lbb_save(void *lbb, const void *data, const unsigned int data_length);

/*
 * return bytes that read from buffer
 * return value < 0, error number
 */
int lbb_read(void *lbb, void *buf, unsigned int *buf_length, const int flag_clean);

/*
 * return bytes that buffer discarded
 * return value = 0, init loopback buffer when "move" > "data_length"
 * return value < 0, error number
 */
int lbb_move(void *lbb, unsigned int move);

#endif /* LBB_H_ */
