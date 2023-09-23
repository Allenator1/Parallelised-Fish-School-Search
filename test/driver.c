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

#define NUM_THREADS_MIN 2

#define MIN_CHUNK_SIZE 1
#define CHUNK_SIZE_NUM_STEPS 10

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
    int program = 0;
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

    fclose(resfile);
    if (logfile != NULL) fclose(logfile);
}


void test_sequential_fss(FILE *logfile, FILE *resfile) {
    fprintf(resfile, "Num.Fish,Num.Rounds,Fitness.Function,Execution.Time\n");

    for (int n_p = 4; n_p <= 17; n_p++) {
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

    for (int n_p = 4; n_p <= 17; n_p++) {
        int n = pow(2, n_p);
        printf("NUMBER OF FISHES: %d\n", n);
        int num_iters = 0;
        int num_iters_total =  3 * (nt_max - NUM_THREADS_MIN + 1) * NUM_REPEATS;

        for (int f = 1; f <= 3; f++) {

            for (int nt = NUM_THREADS_MIN; nt <= nt_max; nt++) {
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
    int nt = omp_get_max_threads();
    int n = pow(2, 17);
    int chunk_step = (n / nt - MIN_CHUNK_SIZE) / CHUNK_SIZE_NUM_STEPS;
    int num_iters = 0;
    int num_iters_total =  3 * 4 * CHUNK_SIZE_NUM_STEPS * NUM_REPEATS;

    for (int f = 1; f <= 3; f++) {

        for (int s = 1; s <= 4; s++) {

            for (int c = MIN_CHUNK_SIZE; c <= n / nt; c += chunk_step) {
                float exec_time_avg = 0;

                for (int i = 0; i < NUM_REPEATS; i++) {
                    char cmdline[1000];
                    sprintf(cmdline, "../src/parallel_fss -t%d -n%d -r%d -s%d -c%d -f%d", 
                            nt, n, NROUNDS, s, c, f);
                    exec_time_avg += exec_program(logfile, cmdline, n, NROUNDS, f);
                    num_iters++;

                    int bar_progress = (float)num_iters / num_iters_total  * 77;
                    printf("[%s]\r", &bar[77 - (int)bar_progress]);
                    fflush(stdout);
                }
                exec_time_avg /= NUM_REPEATS;
                fprintf(resfile, "%d,%d,%d,%d,%d,%d,%f\n", nt, n, NROUNDS, s, c, f, exec_time_avg);

            }
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