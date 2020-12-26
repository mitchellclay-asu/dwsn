/**
 * @file    node.h
 * @brief   Node specific functions for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define XYACCELDELTAMAX 0.005
#define DRAGVARIANCE 0.05

#ifndef node_H
#define node_H

struct Node {
    double terminal_velocity;
    double x_pos;
    double y_pos;
    double z_pos;
    double x_velocity;
    double y_velocity;
    double z_velocity;
    double x_acceleration;
    double y_acceleration;
    double z_acceleration;
    double power_output;
    int transmit_active;
    int active_channel;
    int current_function;
    double busy_remaining;
    double* received_signals;
    int* group_list;
};

int initialize_nodes(struct Node*, int, double, double, double, double, double, double, int, char*, int, int); 
int update_acceleration(struct Node*, int, double, double, int);
int update_velocity(struct Node*, int, double, int);
int update_position(struct Node*, int, double, int);
int update_signals(struct Node*, int, double, int, int, char*, double, double);
int write_node_data(struct Node*, int, int, double, FILE*);

#endif