#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "../include/fish.h"
#include "../include/util.h"
#include "../include/constants.h"


void init_fish(fish *f, unsigned int* randState) {
    f->x = (float)rand_r(randState) / RAND_MAX * (lake_width) - lake_width / 2;
    f->y = (float)rand_r(randState) / RAND_MAX * (lake_width) - lake_width / 2;
    f->wt = INITIAL_WT;
    f->df = 0;
    f->fitness = fitness_function(f->x, f->y);
}


// random movement
void swimfish(fish *f, unsigned int* randState, float step_ind) {
    float new_x = f->x + ((float)rand_r(randState) / RAND_MAX * 2 - 1) * step_ind;
    float new_y = f->y + ((float)rand_r(randState) / RAND_MAX * 2 - 1) * step_ind;
    check_bounds(&new_x, &new_y);

    float new_fitness = fitness_function(new_x, new_y);
    float delta_f = new_fitness - f->fitness;
    
    if (delta_f > 0) {
        f->x = new_x;
        f->y = new_y;
        f->fitness = new_fitness;
        f->df = delta_f;
    } else {
        f->df = 0;
    }
}


// gain weight after swimming
void feedfish(fish *f, float max_delta_f) {
    float new_wt = f->wt + f->df / max_delta_f;
    f->wt = new_wt < 2 * INITIAL_WT? new_wt : 2 * INITIAL_WT;
}


void print_lake(fish *school, int grid_width) {
    int num_grid = grid_width * grid_width;
    int *occupancy = calloc(num_grid, sizeof(int));

    for (int i = 0; i < number_of_fish; i++) {
        int grid_x = round( (school[i].x + lake_width / 2) / lake_width * (grid_width - 1) );
        int grid_y = round( (school[i].y + lake_width / 2) / lake_width * (grid_width - 1) );
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