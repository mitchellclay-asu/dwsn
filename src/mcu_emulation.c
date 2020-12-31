/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"

int update_mcu(struct Node* nodes,
               int node_count,
               double time_resolution,
               int group_max,
               int debug) {
    for (int i = 0; i < node_count; i++) {
        // To-do!!! check to make sure nodes aren't on ground
        mcu_run_function(nodes, node_count, i, time_resolution, group_max, debug);
    }
    return 0;
}

int mcu_run_function(struct Node* nodes,
                     int node_count,
                     int id,
                     double time_resolution,
                     int group_max,
                     int debug) {
    double busy_time = 0.0;

    mcu_update_busy_time(nodes, id, time_resolution, debug);

    if (nodes[id].busy_remaining <= 0) {
        switch (nodes[id].current_function) {
            case 0:                         // initial starting point for all nodes
                mcu_function_main(nodes, node_count, id, group_max, debug);
                break;
            case 1:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50 ms listen time per channel, update later
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_scan_lfg(nodes, node_count, id, group_max, debug);
                }
                break;
            case 2:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 2.0;        // broadcast LFG for 2.0 seconds
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_broadcast_lfg(nodes, id, group_max, debug);
                }
                break;
            case 3:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50ms listen time per channel
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_find_clear_channel(nodes, node_count, id, debug);
                }
                break;
            case 4:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50ms listen time per channel
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_check_channel_busy(nodes, node_count, id, debug);
                }
                break;
            default:
                abort ();
        }
    }
    return 0;
}

int mcu_function_main(struct Node* nodes,
                     int node_count,
                     int id,
                     int group_max,
                     int debug) {
    if (id % 2 == 0) {          // send half to function 1 and half to function 2
        mcu_call(nodes, id, 0, 0, 1);
    }
    else {
        mcu_call(nodes, id, 0, 1, 2);
    }
    return 0;
}

int mcu_function_scan_lfg(struct Node* nodes,
                          int node_count,
                          int id,
                          int group_max,
                          int debug) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].transmit_active && nodes[id].active_channel == nodes[i].active_channel) {
            update_signal(nodes, id, i, debug);
        }
    }
    if (nodes[id].active_channel < 64) {       // go to next channel if less than max_channels (using 64 for now)
        nodes[id].active_channel++;
    }
    else {                           // for now, if done scanning, reset node
        nodes[id].busy_remaining = -1;    
        nodes[id].current_function = 0;
        nodes[id].active_channel = 0;
    }
    return 0;
}

int mcu_function_broadcast_lfg(struct Node* nodes,
                               int id,
                               int group_max,
                               int debug) {
    if (nodes[id].transmit_active) { 
                nodes[id].busy_remaining = 0;
                nodes[id].transmit_active = 0;
    }
    else {                           // look for clear channel
        mcu_call(nodes, id, 2, 0, 3);
    }
    return 0;
}

int mcu_function_find_clear_channel(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    int debug) {
    for (int i = 0; i < node_count; i++) {
        if (i != id) {                  // don't check own id
            if (nodes[id].active_channel == nodes[i].active_channel && nodes[i].transmit_active) {
                if (nodes[id].active_channel < 64) {
                    nodes[id].active_channel++;
                    return 0;
                }
                else {
                    nodes[id].active_channel = 0;
                    return 0;
                }
            }
        }
    }
    nodes[id].transmit_active = 1;
    mcu_return(nodes, id, 0, 3);
    return 0;
}

int mcu_function_check_channel_busy(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    int debug) {
    for (int i = 0; i < node_count; i++) {
        if (i != id) {                  // don't check own id
            if (nodes[id].active_channel == nodes[i].active_channel && nodes[i].transmit_active) {
                mcu_return(nodes, id, 1, 4);
                return 0;
            }
        }
    }
    mcu_return(nodes, id, 0, 4);
    return 0;    
}

int mcu_update_busy_time(struct Node* nodes,
                         int id,
                         double time_resolution,
                         int debug) {
    if (nodes[id].busy_remaining > 0) {
        if (nodes[id].busy_remaining - time_resolution > 0) {
            nodes[id].busy_remaining -= time_resolution;
        }
        else {
            nodes[id].busy_remaining = 0;
        } 
    }
    return 0;
}

int mcu_call(struct Node* nodes, int id, int caller, int return_to_label, int function_number) {
    fs_push(caller, return_to_label, &nodes[id].function_stack);
    nodes[id].busy_remaining = -1;
    nodes[id].current_function = function_number;
    return 0;
}

int mcu_return(struct Node* nodes, int id, int return_value, int function_number) {
    nodes[id].current_function = nodes[id].function_stack->caller; 
    rs_push(function_number, nodes[id].function_stack->return_to_label, &nodes[id].return_stack);
    fs_pop(&nodes[id].function_stack);
    nodes[id].return_value = return_value;
    nodes[id].busy_remaining = -1;
    return 0;
}