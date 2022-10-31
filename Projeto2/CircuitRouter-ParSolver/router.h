#ifndef ROUTER_H
#define ROUTER_H 1


#include "grid.h"
#include "maze.h"
#include "lib/vector.h"

typedef struct router {
    long xCost;
    long yCost;
    long zCost;
    long bendCost;

} router_t;

typedef struct router_solve_arg {
    router_t* routerPtr;
    maze_t* mazePtr;
    list_t* pathVectorListPtr;
} router_solve_arg_t;


/* =============================================================================
 * router_alloc
 * =============================================================================
 */
router_t* router_alloc (long xCost, long yCost, long zCost, long bendCost);


/* =============================================================================
 * router_free
 * =============================================================================
 */
void router_free (router_t* routerPtr);


/* =============================================================================
 * router_solve
 * =============================================================================
 */
void router_solve (void* argPtr, int threadNum);


#endif /* ROUTER_H */


/* =============================================================================
 *
 * End of router.h
 *
 * =============================================================================
 */
