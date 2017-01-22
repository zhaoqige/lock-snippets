/*
 * _base.c
 *
 *  Created on: Jan 19, 2017
 *      Author: Qige
 */

#include "_base.h"


// 0: Big-endian, 1: Little-endian
int env_little_endian(void)
{
	ushort x = 1;
	//uint y = (uint) x;
	return ((uint) x);
}
