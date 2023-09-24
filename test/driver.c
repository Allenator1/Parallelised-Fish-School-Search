#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <omp.h>


#define NROUNDS 500

#define MAX_THREAD_STEPS 8
#define NUM_REPEATS 10

static char bar[] = "======================================="
                    "======================================>";


void test_sequential_fss(FILE *logfile, FILE *resfile);
void test_parallel_fss(FILE *logfile, FILE *resfile);
void test_parallel_schedules(FILE *logfile, FILE *resfile);
float exec_program(FILE *logfile, char *cmd, int n, int r, int f);


int main(int argc, char *argv[]) {
    char* resfile_path = NULL;
    char* logfile_path = NULL;
    bool test_schedules = false;
    int program = 1;
    int opt;

    while((opt = getopt(argc, argv, "f:l:p:s")) != -1) {
    switch(opt) {
        case 'f':
            resfile_path = optarg;
            break;
        case 'l':
            logfile_path = optarg;
            break;
        case 'p':
            program = atoi(optarg);
            break;
        case 's':
            test_schedules = true;
            break;
        default:
            fprintf(stderr, "Usage: [-f RESULTS_FILENAME] [-l LOGFILE] [-p PROGRAM]\n");
            exit(EXIT_FAILURE);
        }
    }

    FILE* logfile = NULL;
    if (logfile_path != NULL) {
        FILE* logfile = fopen(logfile_path, "w");
    }
    FILE* resfile = fopen(resfile_path, "w");

    if (program == 1) {
        test_sequential_fss(logfile, resfile);
    } 
    else if (program == 2) {
        if (test_schedules) {
            test_parallel_schedules(logfile, resfile);
        }
        else {
            test_parallel_fss(logfile, resfile);
        }
    } 
    else {
        fprintf(stderr, "Please enter a valid program. Usage: [-p PROGRAM]\n");
        exit(EXIT_FAILURE);
    }

    fclose(resfile);
    if (logfile != NULL) fclose(logfile);
}


void test_sequential_fss(FILE *logfile, FILE *resfile) {
    fprintf(resfile, "Num.Fish,Num.Rounds,Fitness.Function,Execution.Time\n");

    for (int n_p = 4; n_p <= 18; n_p++) {
        int n = pow(2, n_p);
        printf("NUMBER OF FISHES: %d\n", n);
        int num_iters = 0;

        for (int f = 1; f <= 3; f++) {
            float exec_time_avg = 0;

            for (int i = 0; i < NUM_REPEATS; i++) {
                char cmdline[1000];
                sprintf(cmdline, "../src/seq_fss -n%d -r%d -f%d", n, NROUNDS, f);
                exec_time_avg += exec_program(logfile, cmdline, n, NROUNDS, f);
                num_iters++;

                int bar_progress = (float)num_iters / (NUM_REPEATS * 3) * 77;
                printf("[%s]\r", &bar[77 - (int)bar_progress]);
                fflush(stdout);
            }
            exec_time_avg /= NUM_REPEATS;
            fprintf(resfile, "%d,%d,%d,%f\n", n, NROUNDS, f, exec_time_avg);
        }
        printf("\n");
    }
}


void test_parallel_fss(FILE *logfile, FILE *resfile) {
    fprintf(resfile, "Num.Threads,Num.Fish,Num.Rounds,Schedule,Num.Chunks,Fitness.Function,Execution.Time\n");
    int nt_max = omp_get_max_threads();
    int t_step = 1;
    if (nt_max >= MAX_THREAD_STEPS) {
        t_step = nt_max / MAX_THREAD_STEPS;
    }

    for (int n_p = 4; n_p <= 18; n_p++) {
        int n = pow(2, n_p);
        printf("NUMBER OF FISHES: %d\n", n);
        int num_iters = 0;
        int num_iters_total =  3 * (nt_max / t_step) * NUM_REPEATS;

        for (int f = 1; f <= 3; f++) {

            for (int nt = t_step; nt <= nt_max; nt += t_step) {
                float exec_time_avg = 0;

                for (int i = 0; i < NUM_REPEATS; i++) {
                    char cmdline[1000];
                    sprintf(cmdline, "../src/parallel_fss -t%d -n%d -r%d -f%d", 
                            nt, n, NROUNDS, f);
                    exec_time_avg += exec_program(logfile, cmdline, n, NROUNDS, f);
                    num_iters++;

                    int bar_progress = (float)num_iters / num_iters_total  * 77;
                    printf("[%s]\r", &bar[77 - (int)bar_progress]);
                    fflush(stdout);
                }
                exec_time_avg /= NUM_REPEATS;
                fprintf(resfile, "%d,%d,%d,4,NA,%d,%f\n", nt, n, NROUNDS, f, exec_time_avg);
            }
        }
        printf("\n");
    }
}


void test_parallel_schedules(FILE *logfile, FILE *resfile) {
    fprintf(resfile, "Num.Threads,Num.Fish,Num.Rounds,Schedule,Num.Chunks,Fitness.Function,Execution.Time\n");
    int max_nt = omp_get_max_threads();
    double closest_pow_2 = floor(log2(max_nt));
    int nt = pow(2, closest_pow_2);
    int n = pow(2, 17);
    int max_chunk_pow = 17 - (int)closest_pow_2;
    int num_iters = 0;
    int num_iters_total =  4 * (max_chunk_pow - 2 + 1) * NUM_REPEATS;
    int f = 1;

    for (int s = 1; s <= 4; s++) {

        for (int c = 2; c <= max_chunk_pow; c++) {
            int chunk_size = pow(2, c);
            float exec_time_avg = 0;

            for (int i = 0; i < NUM_REPEATS; i++) {
                char cmdline[1000];
                sprintf(cmdline, "../src/parallel_fss -t%d -n%d -r%d -s%d -c%d -f%d", 
                        nt, n, NROUNDS, s, chunk_size, f);
                exec_time_avg += exec_program(logfile, cmdline, n, NROUNDS, f);
                num_iters++;

                int bar_progress = (float)num_iters / num_iters_total  * 77;
                printf("[%s]\r", &bar[77 - (int)bar_progress]);
                fflush(stdout);
            }
            exec_time_avg /= NUM_REPEATS;
            fprintf(resfile, "%d,%d,%d,%d,%d,%d,%f\n", nt, n, NROUNDS, s, chunk_size, f, exec_time_avg);

            if (s == 4) break;  // run only once for 'automatic' schedule
        }
    }
    printf("\n");
}


float exec_program(FILE *logfile, char *cmd, int n, int r, int f) {
    float exec_time = 0;
    FILE *fp;
    if ((fp = popen(cmd, "r")) == NULL) {
        perror("popen failure");
        exit(EXIT_FAILURE);
    }

    char completionTime[20];
    fgets(completionTime, 20, fp);
    int status = pclose(fp);
    if (!WIFEXITED(status) && logfile != NULL) {
        fprintf(logfile, "Error: %s with parameters n=%d, r=%d, f=%d\n", 
            strerror(status), n, r, f);
    } 
    else {
        exec_time = atof(completionTime);
    }
    return exec_time;
}