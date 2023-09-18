#ifndef FISH_H_
#define FISH_H_

typedef struct {
    float x;
    float y;
    float wt;          
    float fitness;  
    float delta_f;  // change in fitness between ith and i+1th iteration
} fish;

void swimfish(fish *f, float rand_x, float rand_y, float step_size, int lake_size);

void feedfish(fish *f, float max_delta_f, float inital_wt);

void print_lake(fish *school, int grid_size, float lake_size, int num_fish);

#endif