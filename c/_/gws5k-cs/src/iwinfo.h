/*
 * iwinfo.h
 *
 *  Created on: Jun 8, 2016
 *  Updated on: July 13, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef IWINFO_H_
#define IWINFO_H_

int bb_mode(const void *nfiw, const char *ifname);
int bb_noise(const void *nfiw, const char *ifname);

#endif /* IWINFO_H_ */
