/*
 * gws5k_data.h
 *
 *  Created on: Apr 29, 2016
 *  Updated on: May 16, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWS5K_HW_DATA_H_
#define GWS5K_HW_DATA_H_

int task_data_update(void *hw, const char *cmd);
int task_data_save(void *src, void *des);
int task_data_clean(void *hw);
void task_data_save2file(void *hw);
void task_data_print(void *hw);

void task_data_selfcheck();

#endif /* GWS5K_HW_DATA_H_ */
