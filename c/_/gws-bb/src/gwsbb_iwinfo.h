/*
 * gwsbb_iwinfo.h
 *
 *  Created on: Apr 28, 2016
 *  Updated on: June 07, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWSBB_IWINFO_H_
#define GWSBB_IWINFO_H_

int bb_mode(const void *bbiw, const char *ifname);
int bb_signal(const void *bbiw, const char *ifname);
int bb_noise(const void *bbiw, const char *ifname);
int bb_assoclist(const int bw, void *assoclist, const void *bbiw, const char *ifname);

#endif /* GWSBB_IWINFO_H_ */
