#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "../include/fish.h"
#include "../include/util.h"
#include "../include/constants.h"


float rand_range(float min, float max, unsigned int* randState) {
    return (float)rand_r(randState) / RAND_MAX * (max - min) + min;
}


// initialise the fish with random parameters
void init_fish(fish *f, unsigned int* randState) {
    f->x = rand_range(-lake_width / 2, lake_width / 2, randState);
    f->y = rand_range(-lake_width / 2, lake_width / 2, randState);
    f->wt = INITIAL_WT;
    f->df = -1;
    f->dx = 0;
    f->dy = 0;
}


// random individual movement of the fish
void swimfish(fish *f, unsigned int* randState, float step_ind) {
    float new_x = f->x + rand_range(-1, 1, randState) * step_ind;
    float new_y = f->y + rand_range(-1, 1, randState) * step_ind;
    check_bounds(&new_x, &new_y);
    float delta_f = fitness_function(new_x, new_y) - fitness_function(f->x, f->y);
    if (MINIMISE_FITNESS_FN) delta_f = -delta_f;
    
    if (delta_f >= 0) {
        f->dx = new_x - f->x;
        f->x = new_x;
        f->dy = new_y - f->y;
        f->y = new_y;
        f->df = delta_f;
    }
}

// movement of the fish dependent on the school
void collective_move(fish *f, unsigned int* randState, float xI, float yI, float xB, float yB, bool school_weight_improved, float step_vol) {
    if (isnan(xI) || isnan(yI) || isnan(xB) || isnan(yB)) return;

    // instinctive movement
    float new_x = f->x + xI;
    float new_y = f->y + yI;
    check_bounds(&new_x, &new_y);

    // volitive movement
    float dist2barycenter = dist(f->x, f->y, xB, yB);
    float xV = step_vol * rand_range(0, 1, randState) * (f->x - xB) / dist2barycenter;
    float yV = step_vol * rand_range(0, 1, randState) * (f->y - yB) / dist2barycenter;
    if (school_weight_improved) {
        xV = -xV;
        yV = -yV;
    }
    new_x += xV;
    new_y += yV;
    check_bounds(&new_x, &new_y);

    f->x = new_x;
    f->y = new_y;
}


// weight gain after random movement of the fish
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