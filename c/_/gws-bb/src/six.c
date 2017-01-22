/*
 * six.c
 *
 *  Created on: July 13, 2016
 *      Author: Qige Zhao
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>

#include <termios.h>
#include <fcntl.h>


#include "conf.h"
#include "six.h"


/*
 * PART I: pipe/cli
 */
const char *get_ip(const char *ifname)
{
	char cmd[128];
	memset(cmd, 0x00, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),
			"ifconfig %s | grep 'inet addr' | cut -d ':' -f2 | cut -d ' ' -f1",
			ifname);
	return cli_read(cmd);
}

const char *get_mac(const char *ifname)
{
	char cmd[128];
	memset(cmd, 0x00, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),
			"ifconfig %s | grep 'HWaddr' | cut -d 'W' -f2 | cut -d ' ' -f2",
			ifname);
	return cli_read(cmd);
}

char *cli_read_line(const char *cmd)
{
	FILE *fp;
	unsigned int buffer_length;
	static char buffer[128];

	fp = popen(cmd, "r");
	memset(buffer, 0x00, sizeof(buffer));
	fgets(buffer, sizeof(buffer)-1, fp);
	buffer_length = strlen(buffer);
	if (buffer_length > 0) {
		if (buffer[buffer_length-1] == '\n') buffer[buffer_length-1] = '\0';
	}
	buffer[buffer_length] = '\0';
	pclose(fp);

	return buffer;
}

char *cli_read(const char *cmd)
{
	FILE *fp;
	unsigned int buffer_length;
	static char buffer[128];

	fp = popen(cmd, "r");
	memset(buffer, 0x00, sizeof(buffer));
	fread(buffer, sizeof(buffer)-1, sizeof(char), fp);
	buffer_length = strlen(buffer);
	if (buffer_length > 0) {
		if (buffer[buffer_length-1] == '\n') buffer[buffer_length-1] = '\0';
	}
	buffer[buffer_length] = '\0';
	pclose(fp);

	return buffer;
}

void cli_exec(const char *cmd)
{
	system(cmd);
}

void print_file(const char *filename, const char *str)
{
	FILE *fd = fopen(filename, "w+");
	fputs(str, fd);
	fclose(fd);
}




/*
 * PART II: IPC SHM
 */
int shm_init(const char *key, int *shmid, void **shm, const unsigned int size)
{
#ifdef GWS_USE_KEYFILE
	*shmid = shmget(ftok(key, 1), size, 0666 | IPC_CREAT);
#else
	*shmid = shmget((key_t) key, size, 0666 | IPC_CREAT);
#endif
	if (*shmid == -1) {
		SHM_DBG("-> (init) * init failed (shmget).\n");
		return SHM_ERR_GET;
	}

	*shm = (void *) shmat(*shmid, NULL, 0);
	if (*shm && *shm == (void *) -1) {
		SHM_DBG("-> (init) * init failed (shmat)\n");
		return SHM_ERR_AT;
	}

	return SHM_OK;
}

int shm_free(const int shmid, void *shm)
{
	if (shmdt(shm)) {
		SHM_DBG("-> (quit) * warning: close (shmdt)\n");
	}

	if (shmid != -1) {
		if (shmctl(shmid, IPC_RMID, NULL) < 0) {
			SHM_DBG("-> (quit) * warning: close (shmctl)\n");
		}
	}

	return SHM_OK;
}








/*
 * PART III: IPC MSG
 */

int msg_init(const char *key, int *msgid)
{
#ifdef GWS_USE_KEYFILE
	*msgid = msgget(ftok(key, 1), 0666 | IPC_CREAT);
#else
	*msgid = msgget((key_t) key, 0666 | IPC_CREAT);
#endif
	if (*msgid == -1) {
		MSG_DBG("-> (init) * init failed (msgget)\n");
		return MSG_ERR_GET;
	}
	return MSG_OK;
}

int msg_send(const int msgid, const void *msg, const unsigned int msg_length)
{
	if (msgsnd(msgid, msg, msg_length, 0) < 0) {
		return MSG_ERR_SEND_FAILED;
	}
	return MSG_OK;
}
int msg_recv(const int msgid, void *data, unsigned int *msg_length, const int msg_type)
{
	if (msgrcv(msgid, data, *msg_length, msg_type, IPC_NOWAIT) < 0) {
		return MSG_ERR_RECV_FAILED;
	}
	return MSG_OK;
}

int msg_free(const int msgid)
{
	if (msgid != -1) {
		if (msgctl(msgid, IPC_RMID, NULL)) {
			MSG_DBG("-> (quit) * wanring: close (msgctl)\n");
			return MSG_ERR_RMID;
		}
	}
	return MSG_OK;
}







/*
 * PART IV: SP/UART
 */


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



/*
 * PART V: LBB
 */

void lbb_init(void *lbb)
{
	struct lbb *b = (struct lbb *) lbb;
	memset(b, 0, sizeof(struct lbb));

	b->lbb_start = b->lbb_data;
	b->lbb_length = sizeof(b->lbb_data) - 1;

	b->lbb_data_head = b->lbb_start;
	b->lbb_data_length = 0;
}

/*
 * save data (str/hex) to lbb
 * - test data length;
 * - test bytes available;
 * - calc basic counters;
 * - copy
 */
int lbb_save(void *lbb, const void *data, const unsigned int data_length)
{
	const char *d = (char *) data;
	struct lbb *b = (struct lbb *) lbb;

	//+ buffer length = data length + data available length
	unsigned int lbb_length, lbb_data_length, lbb_da_length;
	lbb_length = b->lbb_length;
	lbb_data_length = b->lbb_data_length;
	lbb_da_length = lbb_length - lbb_data_length;

	if (data_length < 1)
		return LBB_ERR_DATAINVALID;
	if (data_length > lbb_da_length)
		return LBB_ERR_NOSPACE;

	//+ buffer length = data head to start + data head to tail
	unsigned int dh_to_start, dh_to_tail;
	dh_to_start = b->lbb_data_head - b->lbb_start;
	dh_to_tail = lbb_length - dh_to_start;

	//+ data available
	unsigned int da_tail;
	if (dh_to_tail >= lbb_data_length) {
		da_tail = dh_to_tail - lbb_data_length;
	} else {
		da_tail = 0;
	}

	//+ save
	if (lbb_data_length > 0) {
		if (da_tail >= data_length) {
			memcpy(b->lbb_data_head+lbb_data_length, d, data_length);
		} else {

			/*memcpy(b->lbb_data_head+lbb_data_length, d, da_tail);
			if (lbb_data_length > dh_to_tail) {
				memcpy(b->lbb_start+(lbb_data_length-dh_to_tail), &d[da_tail], data_length - da_tail);
			} else {
				memcpy(b->lbb_start, &d[da_tail], data_length - da_tail);
			}*/

			if (da_tail > 0) {
				memcpy(b->lbb_data_head+lbb_data_length, d, da_tail);
			}

			memcpy(b->lbb_start, d+da_tail, data_length - da_tail);
		}
	} else {
		memcpy(b->lbb_start, d, data_length);
	}
	b->lbb_data_length += data_length;

	return LBB_OK;
}

/*
 * read loopback buffer data (ascii/hex)
 */
int lbb_read(void *lbb, void *buf, unsigned int *buf_length, const int flag_clean)
{
	if (*buf_length < 1)
		return LBB_ERR_NOSPACE;

	char *d = (char *) buf;
	struct lbb *b = (struct lbb *) lbb;


	//+ buffer length = data length + data available length
	unsigned int lbb_length, lbb_data_length;
	lbb_length = b->lbb_length;
	lbb_data_length = b->lbb_data_length;

	//+ buffer length = data head to start + data head to tail
	unsigned int dh_to_head, dh_to_tail;
	dh_to_head = b->lbb_data_head - b->lbb_start;
	dh_to_tail = lbb_length - dh_to_head;

	//+ calc bytes to read
	int b2read = MIN(*buf_length, lbb_data_length);
	*buf_length = b2read;

	if (dh_to_tail >= b2read) {
		memcpy(d, b->lbb_data_head, b2read);
	} else {
		memcpy(d, b->lbb_data_head, dh_to_tail);
		memcpy(&d[dh_to_tail], b->lbb_start, b2read - dh_to_tail);
	}

	if (flag_clean)
		lbb_move(lbb, *buf_length);

	return LBB_OK;
}

/*
 * discard data of b2move bytes
 * - data length <= b2move, init
 * -
 */
int lbb_move(void *lbb, unsigned int b2move)
{
	if (b2move < 1) return LBB_OK;

	struct lbb *b = (struct lbb *) lbb;

	//+ buffer length = data length + data available length
	unsigned int lbb_length, lbb_data_length;
	lbb_length = b->lbb_length;
	lbb_data_length = b->lbb_data_length;

	//+ buffer length = data head to start + data head to tail
	unsigned int dh_to_head, dh_to_tail;
	dh_to_head = b->lbb_data_head - b->lbb_start;
	dh_to_tail = lbb_length - dh_to_head;

	if (lbb_data_length > b2move) {
		if (dh_to_tail > b2move) {
#ifdef CLEAR_WHEN_MOVE
			int i;
			for(i = 0; i < b2move; i ++) *(b->lbb_data_head+i) = '\0';
#endif
			b->lbb_data_head += b2move;
		} else {
#ifdef CLEAR_WHEN_MOVE
			int i;
			for(i = 0; i <= dh_to_tail; i ++) *(b->lbb_data_head+i) = '\0';
			for(i = 0; i < b2move - dh_to_tail; i ++) *(b->lbb_start+i) = '\0';
#endif
			b->lbb_data_head = b->lbb_start + (b2move - dh_to_tail);
		}
		b->lbb_data_length -= b2move;
	} else {
		lbb_init(lbb);
	}

	return LBB_OK;
}
