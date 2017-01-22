/*
 * uart.h
 *
 *  Created on: Apr 28, 2016
 *  Updated on: June 07, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef UART_H_
#define UART_H_

#define UART_SPEED B9600

enum UART_ERR {
	UART_OK,
	UART_ERR_OPEN,
};

int uart_open(const char *dev);
int uart_read(const int fd, char *buf, const unsigned int buf_length);
int uart_write(const int fd, const char *data, const unsigned int data_length);
void uart_close(const int fd);

#endif /* UART_H_ */
