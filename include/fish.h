#ifndef FISH_H_
#define FISH_H_

typedef struct {
    float x;
    float y;
    float wt;          
    float fitness;  
    float df;  // change in fitness between ith and i+1th iteration
} fish;

void swimfish(fish *f, unsigned int* randState, float step_ind);

void init_fish(fish *f, unsigned int* randState);

void feedfish(fish *f, float max_delta_f);

void print_lake(fish *school, int grid_size);

#endif