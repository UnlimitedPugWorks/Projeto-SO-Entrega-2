#include "linkedlist.h"
#include <stdio.h>
#include <stdlib.h>
processlist insertEnd(processlist head, int i){
	processlist end = (processlist) malloc(sizeof(struct node));
	processlist current = head;
	end->PID = i;
	end->next = NULL;
	end->finished = NOTFINISHED;
	end->finishedstatus = NOEXITSTATUS;
	end->finishedPid = NOEXITSTATUS;
	if (head == NULL){
		return end;
	}
	else{
		while(current != NULL){
			if (current->next == NULL){
				current->next = end;
				return head;
			}
			current = current->next;
		}
	}
	return head;
}
void freelist(processlist head){
	processlist current = head;
	processlist temp;
	while(current != NULL){
		temp = current;
		free(temp);
		current = current->next;
	}
}