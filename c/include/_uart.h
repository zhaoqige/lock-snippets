/*
 * _uart.h
 *
 *  Created on: July 13, 2016
 *  Updated on: Jan 22, 2017
 *      Author: Qige
 */

#ifndef _UART_H_
#define _UART_H_

// data types
typedef unsigned int			uint;
typedef unsigned char			uchar;
typedef char					byte;
typedef unsigned short 			ushort;

// empty debug print out
#ifndef _DBG
#define _DBG(fmt, ...)			{}
#endif

#ifndef
#define UART_SPEED_DEFAULT    B9600 // B9600, B115200
#endif
#define UART_FD_INVALID_BAR		2

enum UART_ERR {
	UART_OK,
	UART_ERR_OPEN,
};

int  uart_open(const char *dev);
int  uart_read(const int fd, char *buff, const uint buff_length);
int  uart_write(const int fd, const char *data, const uint data_length);
void uart_close(int *fd);


#endif /* _UART_H_ */
