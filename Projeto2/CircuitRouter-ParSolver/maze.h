#ifndef MAZE_H
#define MAZE_H 1


#include "coordinate.h"
#include "grid.h"
#include "lib/list.h"
#include "lib/pair.h"
#include "lib/queue.h"
#include "lib/types.h"
#include "lib/vector.h"
#include <pthread.h>

typedef struct maze {
    grid_t* gridPtr;
    queue_t* workQueuePtr;   /* contains source/destination pairs to route */
    vector_t* wallVectorPtr; /* obstacles */
    vector_t* srcVectorPtr;  /* sources */
    vector_t* dstVectorPtr;  /* destinations */
} maze_t;


/* =============================================================================
 * maze_alloc
 * =============================================================================
 */
maze_t* maze_alloc ();


/* =============================================================================
 * maze_free
 * =============================================================================
 */
void maze_free (maze_t* mazePtr);


/* =============================================================================
 * maze_read
 * -- Return number of path to route
 * =============================================================================
 */
long maze_read (maze_t* mazePtr);

long maze_fileread(FILE* Istream, FILE* Ostream, maze_t* mazePtr);


/* =============================================================================
 * maze_checkPaths
 * =============================================================================
 */
bool_t maze_checkPaths (maze_t* mazePtr, list_t* pathListPtr, FILE* outputfile);


#endif /* MAZE_H */


/* =============================================================================
 *
 * End of maze.h
 *
 * =============================================================================
 */
