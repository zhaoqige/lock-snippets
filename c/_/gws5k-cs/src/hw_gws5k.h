/*
 * hw_gws5k.h
 *
 *  Created on: Jun 8, 2016
 *  Updated on: July 13, 2016
 *      Author: Qige Zhao
 */

#ifndef HW_GWS5K_H_
#define HW_GWS5K_H_

void task_hw_prepare(const int channel, const int intl_ms);
void task_hw_revert(const int region, const int channel);

#endif /* HW_GWS5K_H_ */
