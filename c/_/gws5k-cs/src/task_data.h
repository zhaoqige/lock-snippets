/*
 * task_data.h
 *
 *  Created on: Jun 8, 2016
 *  Updated on: July 13, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef TASK_DATA_H_
#define TASK_DATA_H_

int task_data_update(const void *conf, void *iw, void *nf);
//int task_data_publish(void *shm, void *nf);
void task_data_to_file(void *nf);

#endif /* TASK_DATA_H_ */
