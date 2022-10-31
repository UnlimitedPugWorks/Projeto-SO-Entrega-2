#include "linkedlist.h"
/* DEFINED CONSTANTS*/
#define PATH "./CircuitRouter-SeqSolver/CircuitRouter-SeqSolver"
#define BUFFERSIZE 120
#define MAXPARAMS 3
#define EXITCOMMAND "exit"
#define RUNCOMMAND "run"
#define FILENAME "CircuitRouter-SeqSolver"
#define FILENAMESIZE 25
#define NOMAXCHILDREN -1
/* Functions*/
int activeProcess(processlist);
int runSeqSolver(char**);
void prompt(int);

