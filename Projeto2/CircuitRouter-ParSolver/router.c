#include <assert.h>
#include <stdlib.h>
#include "coordinate.h"
#include "grid.h"
#include "lib/queue.h"
#include "router.h"
#include "lib/vector.h"
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

typedef enum momentum {
    MOMENTUM_ZERO = 0,
    MOMENTUM_POSX = 1,
    MOMENTUM_POSY = 2,
    MOMENTUM_POSZ = 3,
    MOMENTUM_NEGX = 4,
    MOMENTUM_NEGY = 5,
    MOMENTUM_NEGZ = 6
} momentum_t;

typedef struct point {
    long x;
    long y;
    long z;
    long value;
    momentum_t momentum;
} point_t;

point_t MOVE_POSX = { 1,  0,  0,  0, MOMENTUM_POSX};
point_t MOVE_POSY = { 0,  1,  0,  0, MOMENTUM_POSY};
point_t MOVE_POSZ = { 0,  0,  1,  0, MOMENTUM_POSZ};
point_t MOVE_NEGX = {-1,  0,  0,  0, MOMENTUM_NEGX};
point_t MOVE_NEGY = { 0, -1,  0,  0, MOMENTUM_NEGY};
point_t MOVE_NEGZ = { 0,  0, -1,  0, MOMENTUM_NEGZ};

pthread_mutex_t queuepoplock   = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t listinsertlock = PTHREAD_MUTEX_INITIALIZER;
/* =============================================================================
 * router_alloc
 * =============================================================================
 */
router_t* router_alloc (long xCost, long yCost, long zCost, long bendCost){
    router_t* routerPtr;

    routerPtr = (router_t*)malloc(sizeof(router_t));
    if (routerPtr) {
        routerPtr->xCost = xCost;
        routerPtr->yCost = yCost;
        routerPtr->zCost = zCost;
        routerPtr->bendCost = bendCost;
    }

    return routerPtr;
}


/* =============================================================================
 * router_free
 * =============================================================================
 */
void router_free (router_t* routerPtr){
    free(routerPtr);
}

/* =============================================================================
 * expandToNeighbor
 * =============================================================================
 */
static void expandToNeighbor (grid_t* myGridPtr, long x, long y, long z, long value, queue_t* queuePtr){
    if (grid_isPointValid(myGridPtr, x, y, z)) { /*Verifica se o point é valido, ou seja se nao saiu dos limites*/
        long* neighborGridPointPtr = grid_getPointRef(myGridPtr, x, y, z); /*Retira a refrencia do ponto atual*/
        long neighborValue = *neighborGridPointPtr;
        if (neighborValue == GRID_POINT_EMPTY) { /*Se esta vazio, atribui o valor e adiciona ao queque*/
            (*neighborGridPointPtr) = value; /*Atribui o valor*/
            queue_push(queuePtr, (void*)neighborGridPointPtr); /*Adiciona ao queque*/
        } 
        else if (neighborValue != GRID_POINT_FULL) {
            /* We have expanded here before... is this new path better? */
            if (value < neighborValue) {
                (*neighborGridPointPtr) = value;
                queue_push(queuePtr, (void*)neighborGridPointPtr);
            }
        }
    }
}


/* =============================================================================
 * doExpansion
 * =============================================================================
 */
static bool_t doExpansion (router_t* routerPtr, grid_t* myGridPtr, queue_t* queuePtr, coordinate_t* srcPtr, coordinate_t* dstPtr){
    long xCost = routerPtr->xCost; /*Variaveis locais*/
    long yCost = routerPtr->yCost; /*Variaveis locais*/
    long zCost = routerPtr->zCost; /*Variaveis locais*/
    queue_clear(queuePtr); /*Esvazia o queque*/
    long* srcGridPointPtr = grid_getPointRef(myGridPtr, srcPtr->x, srcPtr->y, srcPtr->z); /*Refrencia do ponto de fonte*/
    queue_push(queuePtr, (void*)srcGridPointPtr);/*Mete a refrencia da fonte no queque*/
    grid_setPoint(myGridPtr, srcPtr->x, srcPtr->y, srcPtr->z, 0);/*Defenir a fonte como uma fonte*/
    grid_setPoint(myGridPtr, dstPtr->x, dstPtr->y, dstPtr->z, GRID_POINT_EMPTY);/*Esvaziar o destino*/
    long* dstGridPointPtr = grid_getPointRef(myGridPtr, dstPtr->x, dstPtr->y, dstPtr->z);/*Reeceber refrencia do destino*/
    bool_t isPathFound = FALSE; /* Diz que um path não foi encontrado*/
    while (!queue_isEmpty(queuePtr)) { /*Enquanto o queque não esta vazio*/
        long* gridPointPtr = (long*)queue_pop(queuePtr); /*Tirar do queque a ultimo elemento*/
        if (gridPointPtr == dstGridPointPtr) { /*Se já tiver encontreado o destino, said do loop e retorna que encontrou o path*/
            isPathFound = TRUE;
            break;
        }
        long x;
        long y;
        long z;
        grid_getPointIndices(myGridPtr, gridPointPtr, &x, &y, &z); /*Obtem as coordenadas*/
        long value = (*gridPointPtr); /*Obtem os valor do ponto do queque*/
        expandToNeighbor(myGridPtr, x+1, y,   z,   (value + xCost), queuePtr);
        expandToNeighbor(myGridPtr, x-1, y,   z,   (value + xCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y+1, z,   (value + yCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y-1, z,   (value + yCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y,   z+1, (value + zCost), queuePtr);
        expandToNeighbor(myGridPtr, x,   y,   z-1, (value + zCost), queuePtr);

    } /* iterate over work queue */
    return isPathFound;
}


/* =============================================================================
 * traceToNeighbor
 * =============================================================================
 */
static void traceToNeighbor (grid_t* myGridPtr, point_t* currPtr, point_t* movePtr, bool_t useMomentum, long bendCost, point_t* nextPtr){
    /*Adiciona as coordenadas*/
    long x = currPtr->x + movePtr->x;
    long y = currPtr->y + movePtr->y;
    long z = currPtr->z + movePtr->z;
    /*Verifica se o ponto em que está é valido, se não está vazio e se não está cheio ou é uma parede*/
    if (grid_isPointValid(myGridPtr, x, y, z) && !grid_isPointEmpty(myGridPtr, x, y, z) &&!grid_isPointFull(myGridPtr, x, y, z)){
        /*Obtem o value do ponto*/
        long value = grid_getPoint(myGridPtr, x, y, z);
        long b = 0;
        /*Verifica se usa momentum ou não e se o momentum é diferente*/
        if (useMomentum && (currPtr->momentum != movePtr->momentum)) {
            b = bendCost;
        }
        if ((value + b) <= nextPtr->value) { /* '=' favors neighbors over current */
            nextPtr->x = x;
            nextPtr->y = y;
            nextPtr->z = z;
            nextPtr->value = value;
            nextPtr->momentum = movePtr->momentum;
        }
    }
}


/* =============================================================================
 * doTraceback
 * =============================================================================
 */
static vector_t* doTraceback (grid_t* gridPtr, grid_t* myGridPtr, coordinate_t* dstPtr, long bendCost){
    vector_t* pointVectorPtr = vector_alloc(1); /*Faz alloc de vetor*/
    assert(pointVectorPtr);/*NULL*/
    point_t next;  /*Cria um point next*/
    next.x = dstPtr->x; /*Mete as coordenadas do destino*/
    next.y = dstPtr->y;
    next.z = dstPtr->z;
    next.value = grid_getPoint(myGridPtr, next.x, next.y, next.z);/*E o seu valor tambem*/
    next.momentum = MOMENTUM_ZERO; /*MOMENTUM_ZERO*/
    while (1) {
        long* gridPointPtr = grid_getPointRef(gridPtr, next.x, next.y, next.z); /*Guarda o ponteiro do ponto seguinte*/
        vector_pushBack(pointVectorPtr, (void*)gridPointPtr);/*Guarda o ponteiro do ponto no vector*/
        grid_setPoint(myGridPtr, next.x, next.y, next.z, GRID_POINT_FULL); /*Mete o ponto atual cheio*/
        /* Check if we are done */
        if (next.value == 0) { /*Verifica se chegou a fonte*/
            break;
        } 
        point_t curr = next;
        traceToNeighbor(myGridPtr, &curr, &MOVE_POSX, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_POSY, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_POSZ, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_NEGX, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_NEGY, TRUE, bendCost, &next);
        traceToNeighbor(myGridPtr, &curr, &MOVE_NEGZ, TRUE, bendCost, &next);
        if ((curr.x == next.x) && (curr.y == next.y) &&(curr.z == next.z)){
            next.value = curr.value;
            traceToNeighbor(myGridPtr, &curr, &MOVE_POSX, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_POSY, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_POSZ, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_NEGX, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_NEGY, FALSE, bendCost, &next);
            traceToNeighbor(myGridPtr, &curr, &MOVE_NEGZ, FALSE, bendCost, &next);

            if ((curr.x == next.x) && (curr.y == next.y) && (curr.z == next.z)){
                vector_free(pointVectorPtr);
                return NULL; /* cannot find path */
            }
        }
    }
    return pointVectorPtr;
}
/* ==============================================================================*/
/* valid_path                                                                    */
/*===============================================================================*/
bool_t valid_path(grid_t* gridPtr, vector_t* pointVectorPtr, int* c){
    long i,j, *gridPointPtr, x, y, z, index;
    unsigned long n = vector_getSize(pointVectorPtr);
    struct timespec random;
    random.tv_sec = 0;
    int error, unlockerror;
    for(i = 1; i < n-1; i++){
        gridPointPtr = (long*)vector_at(pointVectorPtr, i);
        grid_getPointIndices(gridPtr, gridPointPtr, &x, &y, &z);
        index = grid_getLockIndex( gridPtr, x, y, z);
        error = pthread_mutex_trylock(&gridPtr->pointMutexPtr[index]);
        if (error == EBUSY){
            (*c)++;
            random.tv_nsec = ((rand() % 900) +100)*((*c)) ;
            nanosleep(&random, NULL);
            for(j = i-1 ; j > 0 ; j--){
                long* gridPointPtr = (long*)vector_at(pointVectorPtr, j);
                grid_getPointIndices(gridPtr, gridPointPtr, &x, &y, &z);
                index = grid_getLockIndex(gridPtr, x, y, z);
                unlockerror = pthread_mutex_unlock(&gridPtr->pointMutexPtr[index]);
                assert(unlockerror == 0);
            }
            return FALSE;
        }
        else if(error != 0){
            exit(EXIT_FAILURE);
        }
    }
    grid_addPath_Ptr( gridPtr, pointVectorPtr);
    return TRUE;
}
/* =============================================================================
 * parSolver_thread
 * =============================================================================
 */

void *parSolver_thread(void* args){
    router_solve_arg_t* routerArgPtr = (router_solve_arg_t*)args; /*Carrega os args*/
    router_t* routerPtr = routerArgPtr->routerPtr;/*Carrega o router dos args*/
    maze_t* mazePtr = routerArgPtr->mazePtr;/*Carrega o maze do router*/
    vector_t* myPathVectorPtr = vector_alloc(1); /*Cria um vector de caminhos. Variavel local*/
    grid_t* gridPtr = mazePtr->gridPtr; /*Carrega a grid da maze*/
    queue_t* workQueuePtr = mazePtr->workQueuePtr;/*Carrega um queue de aonde vai buscar os caminhos que tem de fazer*/
    grid_t* myGridPtr = grid_alloc(gridPtr->width, gridPtr->height, gridPtr->depth); /*Cria um grid que é um variavel local*/
    long bendCost = routerPtr->bendCost; /*Carrega o bend cost*/
    queue_t* myExpansionQueuePtr = queue_alloc(-1);
    int c;
    int lock, unlock;
    while (1){
        c=0;
        pair_t* coordinatePairPtr;/*Variavel local*/
        lock = pthread_mutex_lock(&queuepoplock);
        assert(lock == 0);
        if (queue_isEmpty(workQueuePtr)) { /*Se o queue tá vazia, acabou*/
            coordinatePairPtr = NULL;
        } else {
            /*pthread_mutex_lock(&queuepoplock);*/
            coordinatePairPtr = (pair_t*)queue_pop(workQueuePtr); /*Carrega o par no coordinatePairPtr*/
            /*pthread_mutex_unlock(&queuepoplock);*/
        }
        unlock = pthread_mutex_unlock(&queuepoplock);
        assert(unlock == 0);
        if (coordinatePairPtr == NULL) { /*Verifica se o que foi carregado é vazio ou não e se sim, sai do loop*/
            break;
        }
        /*printf("%s\n","consegui buscar a coordenada" );*/
        coordinate_t* srcPtr = coordinatePairPtr->firstPtr;
        coordinate_t* dstPtr = coordinatePairPtr->secondPtr;
        pair_free(coordinatePairPtr);
        bool_t success = FALSE;
        vector_t* pointVectorPtr = NULL;
        grid_copy(myGridPtr, gridPtr);
        /*printf("%s\n", "copiou a grid");*/
        while(!success){
            if (doExpansion(routerPtr, myGridPtr, myExpansionQueuePtr,srcPtr, dstPtr)){ /*Does the expansion*/
                /*printf("%s\n", "fez expansion");*/
                pointVectorPtr = doTraceback(gridPtr, myGridPtr, dstPtr, bendCost); /*Does the traceback e retorna um caminho*/
                if (pointVectorPtr){/*Se um caminho foi encontrado*/
                    if(valid_path(gridPtr, pointVectorPtr, &c)){
                        /*pathsCalculated++;*/
                        /*printf("O Caminho %ld foi valido\n", pathsCalculated);*/
                        success = TRUE; 
                    }
                    else{
                        /*printf("O caminho %ld nao foi valido, logo vamos tentar outra vez \n", pathsCalculated);*/
                        grid_copy(myGridPtr, gridPtr);
                        pointVectorPtr = NULL;
                    }          
                } 
                else if(pointVectorPtr == NULL){
                    break;
                }
            }
            else{
                break;  
            }
        } 
        if (success){/*Se houve sucesso*/
            /*printf("%s\n", "insere" );*/
            bool_t status = vector_pushBack(myPathVectorPtr,(void*)pointVectorPtr);
            assert(status);
        }
    }
    /*
     * Add my paths to global list
     */
    list_t* pathVectorListPtr = routerArgPtr->pathVectorListPtr;
    lock = pthread_mutex_lock(&listinsertlock);
    assert(lock == 0);
    list_insert(pathVectorListPtr, (void*)myPathVectorPtr);
    unlock = pthread_mutex_unlock(&listinsertlock);
    assert(unlock == 0);
    grid_free(myGridPtr);
    queue_free(myExpansionQueuePtr);
    pthread_exit(NULL);
}


/* =============================================================================
 * router_solve
 * =============================================================================
 */
void router_solve (void* argPtr, int threadNum){
    int i, create, join;
    pthread_t trabalhadoras[threadNum];
    for(i = 0; i < threadNum; i++){
        create = pthread_create(&trabalhadoras[i],0,parSolver_thread, argPtr); /*Cria NumTarefas numero de Threads*/
        assert(create == 0);
    }
    for(i = 0; i < threadNum; i++){
        join = pthread_join(trabalhadoras[i],NULL);/*Espera que as tarefas todas acabem*/
        assert(join == 0);

    }
}
/* =============================================================================
 *
 * End of router.c
 *
 * =============================================================================
 */
