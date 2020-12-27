/**
 * @file    main.c
 * @brief   Dynamic Wireless Networking Simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "node.h"
#include "mcu_emulation.h"

int clock_tick(struct Node* nodes, 
               int node_count, 
               double* current_time, 
               double time_resolution, 
               double gravity,
               double spread_factor,
               int debug,
               int output,
               char* output_dir,
               int write_interval,
               int group_max) {
    *current_time += time_resolution; 
    char file_path[100];

    update_acceleration(nodes, node_count, time_resolution, spread_factor, debug);
    update_velocity(nodes, node_count, time_resolution, debug);
    update_position(nodes, node_count, time_resolution, debug);
    update_mcu(nodes, node_count, time_resolution, group_max, debug);

    // Update output files if output option specified
    if (output) {
        for (int i = 0; i < node_count; i++) {
            if (fmod(*current_time, write_interval) < time_resolution) {
                sprintf(file_path, "%s/%d%s", output_dir, i, ".txt");
                FILE *fp;
                fp  = fopen (file_path, "a");
                write_node_data(nodes, node_count, i, *current_time, fp);
                fclose(fp);
            }
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    // Initialization and defaults
    clock_t t1, t2;
    int node_count = 10;
    int moving_nodes = 0; 
    int ret = 0;
    double gravity = 9.80665;
    double start_x = 0;
    double start_y = 0;
    double start_z = 30000;
    double current_time = 0;
    double time_resolution = 0.001;
    double terminal_velocity = 8.0;
    double spread_factor = 20;
    double default_power_output = 400;
    double write_interval = 1.0;
    int group_max = 5;
    int random_seed = -1;
    int debug = 0;
    int verbose = 1;
    int output = 0;
    char output_dir[50];

    // get command line switches
    int c;
    while ((c = getopt(argc, argv, "d:v:c:g:r:z:t:s:e:p:o:m:")) != -1)
    switch (c) {
        case 'd':
            debug = atoi(optarg);
            break;
        case 'v':
            verbose = atoi(optarg);
            break;
        case 'c':
            node_count = atoi(optarg);
            break;
        case 'g':
            gravity = atof(optarg);
            break;
        case 'r':
            time_resolution = atof(optarg);
            break;
        case 'z':
            start_z = atof(optarg);
            break;
        case 't':
            terminal_velocity = atof(optarg);
            break;
        case 's': 
            spread_factor = atof(optarg);
            break;
        case 'e':
            random_seed = atoi(optarg);
            break;
        case 'p':
            default_power_output = atof(optarg);
            break;
        case 'o':
            output = atoi(optarg);
            break;
        case 'm':
            group_max = atoi(optarg);
            break;    
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else {
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
                return 1;
            }
        default:
            abort ();
    }
    
    // Seed random number generator if seed isn't specified
    if (random_seed < 0) {
        srand(time(NULL)); 
        if (verbose) {
            printf("Seeded random number generator\n");
        }
    }

    // Output parameters
    if (verbose) {
        printf("Number of nodes: %d\n", node_count);
        printf("Gravity: %f m/(s^2)\n", gravity);
        printf("Time resolution: %f secs/tick\n", time_resolution); 
        printf("Starting height: %f meters\n", start_z);
        printf("Terminal velocity: %f meters/second\n", terminal_velocity);
        printf("Spread factor: %f\n", spread_factor);
        printf("Default power output: %f\n", default_power_output);
    }
    
    // Make log directory is output option is turned on
    if (output) {
        struct tm *timenow;
        time_t now = time(NULL);
        timenow = gmtime(&now);
        strftime(output_dir, sizeof(output_dir), "output/run/%Y-%m-%d-%H-%M-%S", timenow);
        if (verbose) {
            printf("Creating output directory \"%s\": ", output_dir);
        }

        // Make path for timestamped directory if it doesn't already exist
        struct stat st = {0};
        if (stat("output", &st) == -1) {
           mkdir("output", 0777);
        }
        if (stat("output/run", &st) == -1) {
           mkdir("output/run", 0777);
        }

        // Make directory just for this run
        ret = mkdir(output_dir,0777); 
  
        // check if directory is created or not 
        if (!ret) {
            if (verbose) {
                printf("OK\n"); 
            }
        }
        else { 
            if (verbose) {
                printf("Unable to create directory, exiting\n"); 
            }
            exit(1); 
        } 
    }

    // Get nodes ready
    if (verbose) {
        printf("Initializing nodes\n");
    }
    struct Node nodes[node_count];
    ret = initialize_nodes(nodes, node_count, terminal_velocity, 
                           start_x, start_y, start_z, gravity, 
                           default_power_output, output, output_dir, 
                           group_max, debug);
    if (ret == 0) {
        if (verbose) {
            printf("Initialization OK\n");
        }
        moving_nodes = node_count;
    }
    
    // Run until all nodes reach z = 0;
    if (verbose) {
        printf("Running simulation\n");
    }
    t1 = clock();

    while (moving_nodes != 0) {
        clock_tick(nodes, node_count, &current_time, time_resolution, gravity,
                   spread_factor, debug, output, output_dir, write_interval, 
                   group_max);
        moving_nodes = 0; 
        for (int i = 0; i < node_count; i++) {
            if (nodes[i].z_pos > 0) {
                moving_nodes++;
            }
        }
    }

    // Calculate simulation time
    t2 = clock();
    double runTime = (double)(t2 - t1) / CLOCKS_PER_SEC;

    // Print summary information
    if (verbose) {
        printf("Simulation complete\n");
        printf("Simulation time: %f seconds\n", runTime);        
    }

    if (debug) {
        for (int i = 0; i < node_count; i++) {
            printf("Node %d final velocity: %f %f %f m/s, final position: %f %f %f\n", 
                i, 
                nodes[i].x_velocity, 
                nodes[i].y_velocity, 
                nodes[i].z_velocity, 
                nodes[i].x_pos, 
                nodes[i].y_pos, 
                nodes[i].z_pos);
        }
    }
    if (verbose) {
        printf("Final clock time: %f seconds\n", current_time);
    }
    
    return 0;
}