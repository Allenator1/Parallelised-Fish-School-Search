#ifndef FISH_H_
#define FISH_H_

typedef struct {
    float x;
    float y;
    float wt;          
    float fitness;  
    float delta_f;  // change in fitness between ith and i+1th iteration
} fish;

void swimfish(fish *f, unsigned int* randState, float lake_size, int fitness_fn);

void init_fish(fish *f, unsigned int* randState, float lake_size, int fitness_fn);

void feedfish(fish *f, float max_delta_f);

void print_lake(fish *school, int grid_size, float lake_size, int num_fish);

#endif