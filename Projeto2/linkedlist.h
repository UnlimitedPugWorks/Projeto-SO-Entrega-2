typedef struct node{
	int PID;
	int finished;
	int finishedstatus;
	int finishedPid;
	struct node* next;
}*processlist;
#define NOTFINISHED 0
#define FINISHED 1
#define NOEXITSTATUS -1
processlist insertEnd(processlist head, int i);
void freelist(processlist head);

