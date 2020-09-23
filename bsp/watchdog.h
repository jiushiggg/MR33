/*
 * watchdog.h
 *
 *  Created on: 2018��12��12��
 *      Author: ggg
 */

#ifndef BSP_WATCHDOG_H_
#define BSP_WATCHDOG_H_

#define WD_RESET_TIME_S 	350
#define LOAD_TIME_S(n)	(n*1000)

extern void watchdog_init(void);
extern void watchdog_clear(void);

#endif /* BSP_WATCHDOG_H_ */
