/**
 * @file    mcu_emulation.h
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "node.h"

#ifndef mcuemulation_H
#define mcuemulation_H

int update_mcu(struct Node*, int, double, int, int, double*, int);
int mcu_run_function(struct Node*, int, int, double, int, int, double*, int);
int mcu_update_busy_time(struct Node*, int, double, int);
int mcu_call(struct Node*, int, int, int, int);
int mcu_return(struct Node*, int, int, int);

#endif