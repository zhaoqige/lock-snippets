/*
 * six.h
 *
 *  Created on: July 13, 2016
 *      Author: Qige Zhao
 */

#ifndef SIX_H_
#define SIX_H_


//#define DEBUG_SHM

#define MIN(x, y)			(x < y ? x : y)
#define MAX(x, y)			(x > y ? x : y)

/*
 * PART I: pipe/cli
 */
const char *get_ip(const char *ifname);
const char *get_mac(const char *ifname);
char *cli_read_line(const char *cmd);
char *cli_read(const char *cmd);
void cli_exec(const char *cmd);
void print_file(const char *filename, const char *str);



/*
 * PART II: IPC SHM
 */
enum SHM_ERR {
	SHM_OK = 0,
	SHM_ERR_GET = -50,
	SHM_ERR_AT,
};

#ifdef DEBUG_SHM
#define SHM_DBG(format, ...)		{printf("-shm-: "format, ##__VA_ARGS__);}
#else
#define SHM_DBG(fmt, ...)			{}
#endif

int shm_init(const char *keyfile, int *shmid, void **shm, const unsigned int size);
int shm_free(const int shmid, void *shm);



/*
 * PART III: IPC MSG
 */
enum MSG_ERR {
	MSG_OK = 0,
	MSG_ERR_GET = -50,
	MSG_ERR_SEND_FAILED,
	MSG_ERR_RECV_FAILED,
	MSG_ERR_RMID,
};

#define MSG_DBG(format, ...)		{printf("-msg-: "format, ##__VA_ARGS__);}
//#define MSG_DBG(fmt, ...)			{}

int msg_init(const char *key, int *msgid);
int msg_send(const int msgid, const void *msg, const unsigned int msg_length);
int msg_recv(const int msgid, void *data, unsigned int *msg_length, const int msg_type);
int msg_free(const int msgid);




/*
 * PART IV: SP/UART
 */
#define UART_SPEED B9600

enum UART_ERR {
	UART_OK,
	UART_ERR_OPEN,
};

int uart_open(const char *dev);
int uart_read(const int fd, char *buf, const unsigned int buf_length);
int uart_write(const int fd, const char *data, const unsigned int data_length);
void uart_close(const int fd);


/*
 * PART V: LBB
 */

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


#endif /* SIX_H_ */
