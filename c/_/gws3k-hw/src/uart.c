/*
 * uart.c
 *
 *  Created on: Apr 28, 2016
 *  Updated on: Jun 30, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <termios.h>
#include <fcntl.h>

#include "uart.h"

/*
 * default is 9600/8/N/1
 */
int uart_open(const char *dev)
{
	int invalid_uartfd = 2;
	static int uartfd = -1;

	uartfd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	//uartfd = open(dev, O_RDWR | O_NOCTTY);
	if (uartfd < invalid_uartfd) {
		return UART_ERR_OPEN;
	}

	struct termios opt;
	tcgetattr(uartfd, &opt);
	//cfsetispeed(&opt, UART_SPEED);
	//cfsetospeed(&opt, UART_SPEED);
	cfsetispeed(&opt, B9600);
	cfsetospeed(&opt, B9600);
	//cfsetispeed(&opt, B115200);
	//cfsetospeed(&opt, B115200);

    opt.c_cflag |= (CLOCAL | CREAD);
    opt.c_cflag &= ~PARENB;
    opt.c_cflag &= ~CSTOPB;
    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |=  CS8;
    //opt.c_cflag &= ~INPCK;
    opt.c_cflag |= INPCK;
    opt.c_cflag &= ~CRTSCTS;

    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    //opt.c_lflag |= (ICANON | ECHO | ECHOE | ISIG);

    //opt.c_oflag &= ~OPOST;
    //opt.c_oflag &= ~(INLCR | IGNCR | ICRNL | ONLCR | OCRNL);

    //opt.c_iflag &= ~IXON;	// v1.5 WYY
    //opt.c_iflag &= ~IXOFF;	// v1.5 WYY
    //opt.c_iflag &= ~INLCR;
    //opt.c_iflag &= ~IGNCR;

    //opt.c_cc[VTIME] = 150;
    //opt.c_cc[VMIN] = 0;

	tcsetattr(uartfd, TCSANOW, &opt);
	tcflush(uartfd, TCIFLUSH);

	while(uart_read(uartfd, (void *) 0, 1) > 0);

	return uartfd;
}

int uart_read(const int fd, char *buf, const unsigned int buf_length)
{
	int b2rw = 0;
	b2rw = read(fd, buf, buf_length);
	//printf("uart > read %d bytes\n", b2rw);
	return b2rw;
}
int uart_write(const int fd, const char *data, const unsigned int data_length)
{
	int b2rw = 0;
	b2rw = write(fd, data, data_length);
	if (b2rw < data_length) {
		return -1;
	}
	return b2rw;
}


void uart_close(const int uartfd)
{
	if (uartfd)
		close(uartfd);
}
