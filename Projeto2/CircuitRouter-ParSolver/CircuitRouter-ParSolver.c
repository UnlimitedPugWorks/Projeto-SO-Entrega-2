#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"
#include <pthread.h>

#define UNITIALIZED -1
enum param_types {
    PARAM_BENDCOST = (unsigned char)'b',
    PARAM_XCOST    = (unsigned char)'x',
    PARAM_YCOST    = (unsigned char)'y',
    PARAM_ZCOST    = (unsigned char)'z',
};

enum param_defaults {
    PARAM_DEFAULT_BENDCOST = 1,
    PARAM_DEFAULT_XCOST    = 1,
    PARAM_DEFAULT_YCOST    = 1,
    PARAM_DEFAULT_ZCOST    = 2,
};

/*bool_t global_doPrint = FALSE;*/
char* global_inputFile = NULL;
int NUMTarefas = UNITIALIZED;
long global_params[256]; /* 256 = ascii limit */

int exists(char*);
/* =============================================================================
 * displayUsage
 * =============================================================================
 */
/*static void displayUsage (const char* appName){
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                            (defaults)\n");
    printf("    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
    printf("    p          [p]rint routed maze  (false)\n");
    printf("    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
    printf("    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
    printf("    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
    printf("    h          [h]elp message       (false)\n");
    exit(1);
}*/


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams (){
    global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
    global_params[PARAM_XCOST]    = PARAM_DEFAULT_XCOST;
    global_params[PARAM_YCOST]    = PARAM_DEFAULT_YCOST;
    global_params[PARAM_ZCOST]    = PARAM_DEFAULT_ZCOST;
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static void parseArgs (long argc, char* const argv[]){
    /*long i;*/
    long opt;
    opterr = 0;
    setDefaultParams();
    while ((opt = getopt(argc, argv, "hb:x:y:z:t:")) != -1) {
        switch (opt) {
            case 'b':
            case 'x':
            case 'y':
            case 'z':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case 't':
                NUMTarefas = atoi(optarg);
                if (NUMTarefas <= 0) exit(EXIT_FAILURE);
                break;
            case '?':
            case 'h':
            default:
                opterr++;
                break;
        }
    }
    /*for (i = optind; i < argc; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }
    if (opterr){
        displayUsage(argv[0]);
    }*/
}


/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
    /*
     * Initialization
     */
    long i;
    long error;
    unsigned long gridsize;
    if(argc < 2){
        printf("%s\n","No file detected");
        exit(1);
    }
    parseArgs(argc, (char** const)argv);
    FILE *inputFile, *outputFile; /*Initializes de FILE Pointers*/
    char *filename = (char*) malloc(strlen(argv[argc-1]+5)*sizeof(char));
    char *oldfile = (char*) malloc(strlen(argv[argc-1]+9)*sizeof(char));
    char *buffer = (char*) malloc((PATH_MAX+1)*sizeof(char));
    pthread_mutex_t *pointmutex;
    strcpy(filename, argv[argc-1]); /*Copia o nome do ficheiro*/
    inputFile = fopen(argv[argc-1],"r");
    if (inputFile == NULL){
        printf("%s\n","Wasn't able to open the file.");
        exit(1);
    }
    strcat(filename,".res"); 
    if (exists(filename)){ /*Verifies if the file already exists*/
        strcpy(oldfile, filename);
        strcat(oldfile, ".old");
        rename(realpath(filename, buffer), oldfile);
    }
    outputFile = fopen(filename, "w"); /*If it doesn't exist it creates one, if it does then it rewrites it*/
    maze_t* mazePtr = maze_alloc();
    assert(mazePtr);
    long numPathToRoute = maze_fileread(inputFile, outputFile, mazePtr); /*Le a matriz*/
    fclose(inputFile); /*Fecha o inputfile*/
    router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],global_params[PARAM_YCOST],global_params[PARAM_ZCOST],global_params[PARAM_BENDCOST]);
    assert(routerPtr);
    list_t* pathVectorListPtr = list_alloc(NULL); /*Inicia a lista dos paths*/
    assert(pathVectorListPtr);/*Verifica se a list nao deu null*/
    gridsize = grid_getSize(mazePtr->gridPtr);
    pointmutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)* gridsize);
    assert(pointmutex);
    for(i = 0; i < gridsize; i++){
        error = pthread_mutex_init(&pointmutex[i], NULL);
        assert(error == 0);
    }
    grid_t* local = mazePtr->gridPtr;
    local->pointMutexPtr = pointmutex;
    router_solve_arg_t routerArg = {routerPtr, mazePtr, pathVectorListPtr};
    TIMER_T startTime;
    TIMER_READ(startTime);
    router_solve((void *)&routerArg, NUMTarefas);
    TIMER_T stopTime;
    TIMER_READ(stopTime);
    long numPathRouted = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        numPathRouted += vector_getSize(pathVectorPtr);
	}
    fprintf(outputFile,"Paths routed    = %li\n", numPathRouted);
    fprintf(outputFile,"Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    /*
     * Check solution and clean up
     */
    assert(numPathRouted <= numPathToRoute);
    bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, outputFile); /*SEGMENTATION FAULT*/
    assert(status == TRUE);
    free(pointmutex);
    fprintf(outputFile,"Verification passed.");
    fclose(outputFile);
    maze_free(mazePtr);
    router_free(routerPtr);
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        vector_t* v;
        while((v = vector_popBack(pathVectorPtr))) {
            // v stores pointers to longs stored elsewhere; no need to free them here
            vector_free(v);
        }
        vector_free(pathVectorPtr);
    }
    list_free(pathVectorListPtr);
    return 0;
}

int exists(char* filename){
    FILE *file;
    file = fopen(filename, "r");
    if (file != NULL){
        fclose(file);
        return 1;
    }
    return 0;
}

/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */
