/**
 * @file    state.c
 * @brief   State information functions
 *
 * @author  Mitchell Clay
 * @date    2/6/2021
**/

#include "node.h"
#include "file_output.h"
#include "mcu_emulation.h"
#include "state.h"

extern struct Settings settings;
extern struct State state;

int initialize_state() {
    state.start_time = clock();
    state.moving_nodes = 0;
    state.current_time = 0;
    state.collisions = 0;

    return 0;
}

int clock_tick(struct Node* nodes) {
    state.current_time += settings.time_resolution;
    
    if (settings.debug > 1) {
        printf("Clock tick: %f\n", state.current_time);
    }

    update_acceleration(nodes);
    update_velocity(nodes);
    update_position(nodes);
    update_mcu(nodes);
    if (settings.output) {
        check_write_interval(nodes);
    }

    return 0;
}