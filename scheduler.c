/*INCLUDES ************************************************************/
#include "ses_timer.h"
#include "ses_uart.h"
#include "ses_scheduler.h"
#include "util/atomic.h"

/* DEFINES & MACROS **********************************************************/
#define HOURS_TO_SECONDS 3600
#define MINUTES_TO_SECONDS 60
#define SECONDS_TO_MILLIS 1000

/* PRIVATE VARIABLES **************************************************/
/** list of scheduled tasks */
static taskDescriptor *taskList = NULL;

static volatile systemTime_t systemTime; //system clock counter

/*FUNCTION DEFINITION *************************************************/
static void scheduler_update(void)
{
	taskDescriptor *currentTask = taskList;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{				  //thisblockisexecutedatomically
		systemTime++; //update systemTime every 1 ms
	}
	while (currentTask != NULL)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{ //thisblockisexecutedatomically
			if (currentTask->expire == 0)
			{ //waits some ms to execute task

				currentTask->execute = 1; //set execute flag

				if (currentTask->period > 0) //check if a perodic task
				{
					currentTask->expire = currentTask->period; //repeat task periodically
				}
			}

			//scheduler updating
			(currentTask->expire)--;		 //decrement the expire
			currentTask = currentTask->next; //move pointer through the task list to update all tasks
		}
	}
}

void scheduler_init()
{
	pTimerCallback schedulerCallback = &scheduler_update;
	timer2_setCallback(schedulerCallback);
	timer2_start(); //Initializes Timer2
}

void scheduler_run()
{
	taskDescriptor *currentTask = taskList;
	while (1) //superloop
	{
		sei(); //enable interrupts globally

		currentTask = taskList; //reset pointer to taskList head
		while (currentTask != NULL)
		{

			if (currentTask->execute == 1) //task is ready for execution
			{
				//executing task
				currentTask->task(currentTask->param); //enable passing of parameters to executed task

				if (currentTask->period == 0) //execution of a non periodic task
				{
					scheduler_remove(currentTask); //remove currentTask from the tasklist
				}

				if (currentTask->period > 0) //execution of a periodic task
				{
					currentTask->execute = 0; // clear flag to re-execute the periodic task after the given period
				}
			}
			currentTask = currentTask->next; //move to the next task in the taskList
		}
	}
}

bool scheduler_add(taskDescriptor *toAdd)
{
	taskDescriptor *head = taskList; //begining of the linked list

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{ //thisblockisexecutedatomically

		if (toAdd != NULL)
		{
			if (taskList == NULL)
			{ //adding in an empty list case
				taskList = toAdd;
				return true;
			}

			else
			{
				while (head->next != NULL)
				{ //move pointer to the end of the linked list
					if (head == toAdd)
					{ //check if task already added
						return false;
					}
					head = head->next; //move through the list
				}
				if (head == toAdd)
				{ //check if last entry in the list is already added
					return false;
				}
				else
				{
					head->next = toAdd;		 //add node at the end of the linked list
					head->next->next = NULL; //define the new end of the linked list
					return true;
				}
			}
		}
	}
}

void scheduler_remove(taskDescriptor *toRemove)
{
	taskDescriptor *head = taskList; //begining of the linked list
	taskDescriptor *rightTask;
	taskDescriptor *leftTask;

	if (taskList == NULL)
	{ //case of removing in an empty list
		return;
	}

	else
	{
		if (toRemove != NULL && taskList != NULL)
		{
			if (taskList == toRemove)
			{ //if removing the first task
				taskList = toRemove->next;
			}
			else
			{
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
				{ //thisblockisexecutedatomically
					while (head->next != toRemove)
					{ //move till toRemove on the pointer's right
						head = head->next;
						if (head->next == NULL)
						{ //if list was traversed and toRemove wasn't found break
							return;
						}
					}
					leftTask = head;			//define left of the toRemove
					rightTask = toRemove->next; //define right of the toRemove
					leftTask->next = rightTask; //remove the node toRemove from the linked list
				}
			}
		}
	}
}

systemTime_t scheduler_getTime()
{
	return systemTime;
}

void scheduler_setTime(systemTime_t time)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{ //thisblockisexecutedatomically
		systemTime = time;
	}
}
