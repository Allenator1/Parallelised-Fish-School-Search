#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "../include/fish.h"
#include "../include/util.h"
#include "../include/constants.h"


void init_fish(fish *f, unsigned int* randState, float lake_w, int fitness_fn) {
    f->x = (float)rand_r(randState) / RAND_MAX * (lake_w) - lake_w / 2;
    f->y = (float)rand_r(randState) / RAND_MAX * (lake_w) - lake_w / 2;
    f->wt = INITIAL_WT;
    f->delta_f = 0;
    f->fitness = fitness_function(f->x, f->y, fitness_fn);
}


// random movement
void swimfish(fish *f, unsigned int* randState, float lake_w, int fitness_fn) {
    float new_x = f->x + ((float)rand_r(randState) / RAND_MAX * 2 - 1) * STEP_IND;
    float new_y = f->y + ((float)rand_r(randState) / RAND_MAX * 2 - 1) * STEP_IND;
    check_bounds(&new_x, &new_y, lake_w);

    float new_fitness = fitness_function(new_x, new_y, fitness_fn);
    float delta_f = new_fitness - f->fitness;
    
    if (delta_f > 0) {
        f->x = new_x;
        f->y = new_y;
        f->fitness = new_fitness;
        f->delta_f = delta_f;
    } else {
        f->delta_f = 0;
    }
}


// gain weight after swimming
void feedfish(fish *f, float max_delta_f) {
    float new_wt = f->wt + f->delta_f / max_delta_f;
    f->wt = new_wt < 2 * INITIAL_WT? new_wt : 2 * INITIAL_WT;
}


void print_lake(fish *school, int grid_width, float lake_w, int num_fish) {
    int num_grid = grid_width * grid_width;
    int *occupancy = calloc(num_grid, sizeof(int));

    for (int i = 0; i < num_fish; i++) {
        int grid_x = round( (school[i].x + lake_w / 2) / lake_w * (grid_width - 1) );
        int grid_y = round( (school[i].y + lake_w / 2) / lake_w * (grid_width - 1) );
        occupancy[grid_y * grid_width + grid_x] += 1;
    }

    int *qs = malloc(4 * sizeof(int));
    quantiles(occupancy, num_grid, qs);

    for (int i = 0; i < grid_width * 3 + 2; i++) printf("-");
    printf("\n");
    for (int i = 0; i < grid_width; i++) {
        printf("|");
        for (int j = 0; j < grid_width; j++) {
            int o = occupancy[i * grid_width + j];
            char *s;
            if (o == 0) {
                s = "   ";
            } else if (o <= qs[0]) {
                s = " \033[0;31mo\033[0m ";     // red
            } else if (o <= qs[1]) {
                s = " \033[0;34mo\033[0m ";     // blue
            } else if (o <= qs[2]) {
                s = " \033[0;32mo\033[0m ";     // green
            } else if (o <= qs[3]) {
                s = " \033[0;35mo\033[0m ";     // purple
            } else {
                s = " \033[0;37mo\033[0m ";     // white
            }
            printf("%s", s);
        }
        printf("|\n");
    }
    for (int i = 0; i < grid_width * 3 + 2; i++) printf("-");
    printf("\n");

    free(occupancy);
}