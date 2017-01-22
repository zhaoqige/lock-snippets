/*
 * GC1Lib.h
 *
 *  Created on: May 17, 2016
 *      Author: qige
 */

#ifndef GC1LIB_H_
#define GC1LIB_H_


//#define __OS_IS_X86__

#define MIN(x, y)					(x < y ? x : y)
#define MAX(x, y)					(x > y ? x : y)

#define GC1_PACKET_HEADER_BYTE1		0x55
#define GC1_PACKET_HEADER_BYTE2		0xee
#define GC1_PACKET_TAIL_BYTE		0xee

#define GC1_PACKET_SIZE				512
#define GC1_PACKET_SIZE_MIN			11

#define GC1_PACKET_HEADER_SIZE		8
#define GC1_PACKET_TAIL_SIZE		3

#define GC1_MSG_SIZE				(GC1_PACKET_SIZE - GC1_PACKET_HEADER_SIZE - GC1_PACKET_TAIL_SIZE)


#define GC1_PACKET_HEARTBEAT 		0xff
#define GC1_PACKET_GET_REQ			0x11
#define GC1_PACKET_GET_RESP			0x12
#define GC1_PACKET_SET_REQ			0x21
#define GC1_PACKET_SET_RESP			0x22
#define GC1_PACKET_NOTIFY			0xf1
#define GC1_PACKET_NOTIFY_ACK		0xf2


enum GC1_PACKET_DIRECTION {
	GC1_PACKET_DL = 1,
	GC1_PACKET_UL
};

enum GC1_ERROR {
	GC_OK = 0,

	GC_ERR_PACKET_PACK = -10,
	GC_ERR_PACKET_PACK_WRONG_MSG,
	GC_ERR_PACKET_PACK_MSG_LENGTH,

	GC_ERR_MSG_PACK = -20,
	GC_ERR_MSG_PACK_WRONG_WRAP,
	GC_ERR_MSG_PACK_WRONG_CMD,

	GC_ERR_PACKET_FIND = -30,
	GC_ERR_PACKET_FIND_DATA_TOO_SHORT,
	GC_ERR_PACKET_FIND_DATA_WRONG_HEAD,
	GC_ERR_PACKET_FIND_DATA_NOT_ENOUGH_H,
	GC_ERR_PACKET_FIND_DATA_NOT_ENOUGH_T,
	GC_ERR_PACKET_FIND_DATA_WRONG_TAIL,
	GC_ERR_PACKET_FIND_DATA_WRONG_CRC,

	GC_ERR_PACKET_UNPACK = -40,

	GCC_ERR_MSG_UNPACK = -50,
	GC_ERR_MSG_UNPACK_WRONG_FORMAT,
	GC_ERR_MSG_UNPACK_WRONG_ROOT,
	GC_ERR_MSG_UNPACK_WRONG_WRAP,
	GC_ERR_MSG_UNPACK_NODE_EMPTY,
};

union US2C {
	unsigned short num;
	struct {
#ifdef __OS_IS_X86__
		unsigned char lo, hi;
#else
		unsigned char hi, lo;
#endif
	} chars;
};

struct GC1_PACKET_HEADER {
	char h1, h2;
	char seqLo, seqHi;
	char pType, pFlag;
	char msgLengthLo, msgLengthHi;
};
struct GC1_PACKET_TAIL {
	char crcLo;
	char crcHi;
	char end;
};

struct GC1_PACKET {
	enum GC1_PACKET_DIRECTION direction;
	struct GC1_PACKET_HEADER header;
	char msg[GC1_PACKET_SIZE];
	struct GC1_PACKET_TAIL tail;
};



int GCPacketFind(void *conf, const char *buf, const unsigned int bufLength, unsigned int *move);

int GCMsgPack(char *msg, const char *wrap, const char *cmd, const char *val);
int GCPacketPack(void *conf, const char pType, const char *msg);

int GCPacketUnpack(void *conf, char *msg);
int GCMsgUnpack(const char *msg, const char *wrap, char *cmd, char *val);
int GCMsgUnpackFind(const char *msg, const char *wrap, char *cmd, char *val);

void GCPacketToBuffer(const void *conf, char *buffer, unsigned int *bufferLength);


#endif /* GC1LIB_H_ */
