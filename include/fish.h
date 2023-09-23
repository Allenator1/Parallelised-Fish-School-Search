#ifndef FISH_H_
#define FISH_H_

typedef struct {
    float x;
    float y;
    float dx;
    float dy;
    float wt;          
    float df;       // change in fitness after random movement
} fish;

void swimfish(fish *f, unsigned int* randState, float step_ind);

void init_fish(fish *f, unsigned int* randState);

void feedfish(fish *f, float max_delta_f);

void collective_move(fish *f, unsigned int* randState, float xI, float yI, float xB, float yB, bool school_weight_improved, float step_vol);

void print_lake(fish *school, int grid_size);

#endif