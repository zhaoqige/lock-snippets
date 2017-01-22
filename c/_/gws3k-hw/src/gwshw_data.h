/*
 * gwshw_data.h
 *
 *  Created on: Apr 29, 2016
 *  Updated on: May 16, 2016
 *      Author: Qige Zhao <qige@6harmonics.com>
 */

#ifndef GWSHW_DATA_H_
#define GWSHW_DATA_H_

int gwshw_data_update(const int uartfd, void *hw, void *b, const char *cmd);
int gwshw_data_save(void *src, void *des);
int gwshw_data_clean(void *hw);
void gwshw_data_save2file(void *hw);
void gwshw_data_print(void *hw);

void gwshw_data_selfcheck(const int uartfd);

#endif /* GWSHW_DATA_H_ */
