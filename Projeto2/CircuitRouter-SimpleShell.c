#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "lib/commandlinereader.h"
#include "CircuitRouter-SimpleShell.h"
/* Main */
int main(int argc, char** argv){
	int MAXCHILDREN = NOMAXCHILDREN, numargs, Pid, currentPid, finished = 0, status, i = 0;
	processlist PIDlist = NULL, current;
	char **line = (char**)malloc(MAXPARAMS*sizeof(char*));
	char *buffer = (char*)malloc(BUFFERSIZE* sizeof(char));
	if(argc == 2){
		MAXCHILDREN = atoi(argv[1]); /*If there's only one argument, it's MAXCHILDREN*/
	}
	prompt(MAXCHILDREN); /*Shows the beginning message to the user*/
	while(finished == 0){
		numargs = readLineArguments(line, MAXPARAMS, buffer, BUFFERSIZE); /*Reads the commands on the line*/
		if ((numargs == 1) && (strcmp(line[0],EXITCOMMAND) == 0)){/*If it detects only one arguments and it's exit, then it exits the program*/
			finished = 1;
		}
		else if((numargs == 2) && (strcmp(line[0],RUNCOMMAND) == 0)){ /*If it detects two arguments and the first argument is run, then it goes into run*/
			if ((MAXCHILDREN == NOMAXCHILDREN) || ((MAXCHILDREN != NOMAXCHILDREN)&&(activeProcess(PIDlist)<MAXCHILDREN))){
				Pid = runSeqSolver(line);
				PIDlist = insertEnd(PIDlist, Pid);
			}
			else{
				printf("%s\n%s\n","MAXCHILDREN reached", "Processing..."); /*Shows the use that MAXCHILDREN proceses have been used*/
				while(activeProcess(PIDlist) == MAXCHILDREN){
						/*It waits and registers when that process ends*/
				}
				printf("%s\n", "Processed! Processes will now be launched"); /*Runs the process*/
				Pid = runSeqSolver(line);
				PIDlist = insertEnd(PIDlist, Pid);
			}
		}
		else{
			printf("%s\n","Command not recognized");
		}
	}
	current = PIDlist;
	while(current != NULL){
		currentPid = current->PID;
		if (current->finished !=FINISHED){ /*If a process is not finished, wait for it*/
			waitpid(currentPid, &status,0);
			current->finished = FINISHED; /*When it ends mark it as finished*/
			current->finishedstatus = WEXITSTATUS(status); /*Saves the status it exited with*/
		}
		printf("CHILD EXITED(PID=%d; return %s)\n", currentPid, (current->finishedstatus==0)?"OK":"NOK"); /*If that status is equal to 0, it exited in an okay form, otherwise it ended*/
		current = current->next;
	}
	freelist(PIDlist); /*Frees the PIDLIST*/
	free(buffer); /*Frees the buffer*/
	for(i = 0; i < MAXPARAMS;i++){ /*Frees the components of line*/
		if(line[i] != NULL){
			free(line[i]);
		}
	}
	free(line); /*Frees line*/
	printf("%s\n", "END.");
	return 0;
}
int runSeqSolver(char** args){
	int SeqPid = fork();
	char* argumentos[] = {FILENAME, args[1], NULL};
	if(SeqPid == 0){ /*Child Process*/
		execv(PATH, argumentos);/*Executes the program*/
	}
	return SeqPid;/*The parent returns the child's PID*/
}

int activeProcess(processlist list){ 
	processlist current = list;
	int currentPid, counter = 0, status;
	while(current != NULL){ /*While the list hasn't finished*/
		if (current->finished != FINISHED){ /*If the current PID hasn't been marked as finished yet*/
			currentPid = current->PID;
			waitpid(currentPid, &status, WNOHANG);
			if(WIFEXITED(status)){  /*If it exited, then it marks this PID as finished and saves it's exit status*/
				current->finished = FINISHED;
				current->finishedstatus = WEXITSTATUS(status);
			}
			else{
				counter++; /*If it hasn't exited it increments to the counter*/
			}
		}
		current = current->next; /*Proceeds to the next element*/
	}
	return counter; /*Returns the counter*/
}

void prompt(int MAXCHILDREN){ /*Shows the beginning message*/
	printf("Welcome to CircuitRouter-SimpleShell.\n");
	if (MAXCHILDREN == NOMAXCHILDREN){
		printf("MAXCHILDREN: Unlimited\n");
	}
	else{
		printf("MAXCHILDREN: %d\n", MAXCHILDREN);
	}
}