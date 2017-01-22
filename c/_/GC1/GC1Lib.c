/*
 * GC1Lib.c
 *
 *  Created on: May 17, 2016
 *      Author: qige
 */

#include <stdio.h>
#include <string.h>

#include "crc16.h"
#include "xml2.h"
#include "GC1Lib.h"

static unsigned int seq = 0;


int GCMsgPack(char *msg, const char *wrap, const char *cmd, const char *val)
{
	if (! wrap || strlen(wrap) < 1) {
		return GC_ERR_MSG_PACK_WRONG_WRAP;
	}
	if (! cmd || strlen(cmd) < 1) {
		return GC_ERR_MSG_PACK_WRONG_CMD;
	}

	if (val && strlen(val) > 0) {
		sprintf(msg, "<%s><%s val=\"%s\" /></%s>", wrap, cmd, val, wrap);
	} else {
		sprintf(msg, "<%s><%s /></%s>", wrap, cmd, wrap);
	}
	return GC_OK;
}
int GCPacketPack(void *conf, const char pType, const char *msg)
{
	unsigned short msgLength, bufLength;
	union US2C us2c;
	char buf[GC1_PACKET_SIZE];
	struct GC1_PACKET_TAIL tail;
	struct GC1_PACKET *p = (struct GC1_PACKET *) conf;

	//+ check msg & msgLength
	if (! msg) {
		return GC_ERR_PACKET_PACK_WRONG_MSG;
	}
	msgLength = strlen(msg);
	if (msgLength < GC1_PACKET_SIZE_MIN || msgLength > sizeof(p->msg)) {
		return GC_ERR_PACKET_PACK_MSG_LENGTH;
	}

	//+ make packet head
	switch(p->direction) {
	case GC1_PACKET_DL:
		p->header.h1 = GC1_PACKET_HEADER_BYTE1;
		p->header.h2 = GC1_PACKET_HEADER_BYTE2;
		break;
	case GC1_PACKET_UL:
	default:
		p->header.h1 = GC1_PACKET_HEADER_BYTE2;
		p->header.h2 = GC1_PACKET_HEADER_BYTE1;
		break;
	}

	us2c.num = seq; seq ++;
	p->header.seqLo = us2c.chars.lo;
	p->header.seqHi = us2c.chars.hi;

	p->header.pType = pType;

	us2c.num = msgLength;
	p->header.msgLengthLo = us2c.chars.lo;
	p->header.msgLengthHi = us2c.chars.hi;
	bufLength = GC1_PACKET_HEADER_SIZE;

	//+ copy msg
	strncpy(p->msg, msg, msgLength);
	bufLength += msgLength;

	//+ copy tail
	memset(&tail, 0, sizeof(tail));

	us2c.num = 0;
	memset(buf, 0, sizeof(buf));
	memcpy(buf, &p->header, GC1_PACKET_HEADER_SIZE);
	bufLength = GC1_PACKET_HEADER_SIZE;
	memcpy(&buf[bufLength], &p->msg, msgLength);
	bufLength += msgLength;

	us2c.num = crc16_calc(buf, bufLength);
	tail.crcLo = us2c.chars.lo;
	tail.crcHi = us2c.chars.hi;
	tail.end = GC1_PACKET_TAIL_BYTE;

	memcpy(&p->tail, &tail, GC1_PACKET_TAIL_SIZE);
	bufLength += GC1_PACKET_TAIL_SIZE;

	return GC_OK;
}

//+ GWSCtrlV1: find packet in buf
int GCPacketFind(void *conf, const char *buf, const unsigned int bufLength, unsigned int *move)
{
	unsigned short crc;
	unsigned int i;
	int headStart, msgStart, msgEnd, msgLength, packetLength;
	union US2C us2c;

	//+ config before search job
	struct GC1_PACKET *p = (struct GC1_PACKET *) conf;
	switch(p->direction) {
	case GC1_PACKET_DL:
		p->header.h1 = GC1_PACKET_HEADER_BYTE1;
		p->header.h2 = GC1_PACKET_HEADER_BYTE2;
		break;
	case GC1_PACKET_UL:
	default:
		p->header.h1 = GC1_PACKET_HEADER_BYTE2;
		p->header.h2 = GC1_PACKET_HEADER_BYTE1;
		break;
	}

	//+ check buf length before any search job
	if (bufLength < GC1_PACKET_SIZE_MIN) {
		return GC_ERR_PACKET_FIND_DATA_TOO_SHORT;
	}

	//+ find head start position
	*move = 0; headStart = -1;
	for(i = 1; i <= bufLength - GC1_PACKET_SIZE_MIN + 1; i ++) {
		if (buf[i-1] == p->header.h1 && buf[i] == p->header.h2) {
			headStart = i - 1;
			break;
		}
	}
	*move = i - 1; //+ mark garbage bytes

	//+ check head start
	if (headStart < 0) {
		return GC_ERR_PACKET_FIND_DATA_WRONG_HEAD;
	}

	//+ check space left
	msgStart = headStart + GC1_PACKET_HEADER_SIZE;
	if (msgStart + GC1_PACKET_TAIL_SIZE > bufLength) {
		return GC_ERR_PACKET_FIND_DATA_NOT_ENOUGH_H;
	}
	packetLength = GC1_PACKET_HEADER_SIZE;

	//+ find msg length
	us2c.chars.lo = buf[headStart+6];
	us2c.chars.hi = buf[headStart+7];
	msgLength = us2c.num;
	msgEnd = msgStart + us2c.num - 1;
	packetLength += msgLength;

	//+ check space left
	if (msgEnd + GC1_PACKET_TAIL_SIZE > bufLength) {
		return GC_ERR_PACKET_FIND_DATA_NOT_ENOUGH_T;
	}

	//+ find crcLo/crcHi/tail bytes
	//+ check tail byte first
	if (buf[msgEnd+3] != (char) GC1_PACKET_TAIL_BYTE) {
		return GC_ERR_PACKET_FIND_DATA_WRONG_TAIL;
	}

	//+ check crcLo/crcHi bytes
	us2c.num = 0;
	us2c.chars.lo = buf[msgEnd+1];
	us2c.chars.hi = buf[msgEnd+2];
	crc = crc16_calc(&buf[headStart], packetLength);
	if (us2c.num != crc) {
		return GC_ERR_PACKET_FIND_DATA_WRONG_CRC;
	}

	//+ copy & set packetLength
	memcpy(&p->header, &buf[headStart], GC1_PACKET_HEADER_SIZE);
	memcpy(&p->msg, &buf[msgStart], msgLength);
	memcpy(&p->tail, &buf[msgEnd+1], GC1_PACKET_TAIL_SIZE);

	return GC_OK;
}
int GCPacketUnpack(void *conf, char *msg)
{
	union US2C us2c;
	struct GC1_PACKET *p = (struct GC1_PACKET *) conf;

	us2c.chars.lo = p->header.msgLengthLo;
	us2c.chars.hi = p->header.msgLengthHi;
	strncpy(msg, p->msg, MIN(us2c.num, strlen(p->msg)));
	return GC_OK;
}

int GCMsgUnpack(const char *msg, const char *wrap, char *cmd, char *val)
{
	int ret = GC_OK;

	XDOC pDoc = NULL;
	XNODE pRootNode = NULL, pCurrentNode = NULL;
	XCHAR *pAttr = NULL;
	char *pRoot = NULL, *pNode = NULL;

	do {
		pDoc = xml2ParseMemory(msg, strlen(msg));
		if (! pDoc) {
			ret = GC_ERR_MSG_UNPACK_WRONG_FORMAT;
			break;
		}

		pRootNode = xml2GetRootNode(pDoc);
		if (pRootNode->type != XML_ELEMENT_NODE) {
			ret = GC_ERR_MSG_UNPACK_WRONG_ROOT;
			break;
		}

		pRoot = xml2GetNodeName(pRootNode);
		if (! pRoot || strcmp(pRoot, wrap)) {
			ret = GC_ERR_MSG_UNPACK_WRONG_WRAP;
			break;
		}

		pCurrentNode = xml2GetChildNode(pRootNode);
		if (pCurrentNode->type != XML_ELEMENT_NODE) {
			ret = GC_ERR_MSG_UNPACK_NODE_EMPTY;
			break;
		}

		pNode = xml2GetNodeName(pCurrentNode);

		strcpy(cmd, pNode);

		pAttr = xml2GetNodeAttrValue(pCurrentNode, (XCHAR *) "val");
		if (pAttr)
			strcpy(val, (char *) pAttr);

	} while(0);

	if (pAttr)
		xml2FreeAttr(pAttr);

	if (pDoc)
		xml2FreeDoc(pDoc);

	return ret;
}

int GCMsgUnpackFind(const char *msg, const char *wrap, char *cmd, char *val)
{
	int ret = GC_OK;

	XDOC pDoc = NULL;
	XNODE pRootNode = NULL, pCurrentNode = NULL;
	XCHAR *pAttr = NULL;
	char *pRoot = NULL, *pNode = NULL;

	do {
		pDoc = xml2ParseMemory(msg, strlen(msg));
		if (! pDoc) {
			ret = GC_ERR_MSG_UNPACK_WRONG_FORMAT;
			break;
		}

		pRootNode = xml2GetRootNode(pDoc);
		if (pRootNode->type != XML_ELEMENT_NODE) {
			ret = GC_ERR_MSG_UNPACK_WRONG_ROOT;
			break;
		}

		pRoot = xml2GetNodeName(pRootNode);
		if (! pRoot || strcmp(pRoot, wrap)) {
			ret = GC_ERR_MSG_UNPACK_WRONG_WRAP;
			break;
		}

		pCurrentNode = xml2GetChildNode(pRootNode);
		if (pCurrentNode->type != XML_ELEMENT_NODE) {
			ret = GC_ERR_MSG_UNPACK_NODE_EMPTY;
			break;
		}

		pNode = xml2GetNodeName(pCurrentNode);
		while(pNode) {
			if (strstr(cmd, pNode)) {
				pAttr = xml2GetNodeAttrValue(pCurrentNode, (XCHAR *) "val");
				if (pAttr)
					strcpy(val, (char *) pAttr);
				break;
			}
			pCurrentNode = xml2GetNodeNext(pCurrentNode);
			pNode = xml2GetNodeName(pCurrentNode);
		}

	} while(0);

	if (pAttr)
		xml2FreeAttr(pAttr);

	if (pDoc)
		xml2FreeDoc(pDoc);

	return ret;
}

void GCPacketToBuffer(const void *conf, char *buffer, unsigned int *bufferLength)
{
	union US2C us2c;
	struct GC1_PACKET *packet = (struct GC1_PACKET *) conf;

	memcpy(buffer, &packet->header, GC1_PACKET_HEADER_SIZE);
	*bufferLength = GC1_PACKET_HEADER_SIZE;

	us2c.chars.lo = packet->header.msgLengthLo;
	us2c.chars.hi = packet->header.msgLengthHi;
	memcpy(&buffer[*bufferLength], &packet->msg,
			MIN(us2c.num, GC1_PACKET_SIZE - GC1_PACKET_HEADER_SIZE - GC1_PACKET_TAIL_SIZE));
	*bufferLength += us2c.num;

	memcpy(&buffer[*bufferLength], &packet->tail, GC1_PACKET_TAIL_SIZE);
	*bufferLength += GC1_PACKET_TAIL_SIZE;
}
