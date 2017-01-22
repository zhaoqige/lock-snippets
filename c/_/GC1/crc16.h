/*
 * crc16.h
 *
 *  Created on: May 6, 2016
 *      Author: qige
 */

#ifndef CRC16_H_
#define CRC16_H_

#ifdef __cplusplus
extern "C" {
#endif


unsigned short crc16_calc(const char *msg, const unsigned int msg_length);


#ifdef __cplusplus
}
#endif

#endif /* CRC16_H_ */
