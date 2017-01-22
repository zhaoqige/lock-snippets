/*
 * gwsbb_data.h
 *
 *  Created on: Apr 28, 2016
 *  Updated on: June 07, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWSBB_DATA_H_
#define GWSBB_DATA_H_

int gwsbb_data_update(const int bw, void *iw, void *bb, const char *ifname);
int gwsbb_data_publish(void *shm, void *bb);
void gwsbb_data_to_file(void *bb);

#endif /* GWSBB_DATA_H_ */
