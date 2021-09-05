#ifndef SCHEDULER_H_
#define SCHEDULER_H_

/*INCLUDES *******************************************************************/
#include "ses_common.h"

/* TYPES ********************************************************************/

/**type of function pointer for tasks */
typedef void (*task_t)(void *);

/**type for system clock calculations*/
typedef uint32_t systemTime_t;

/** Task structure to schedule tasks
 */
typedef struct taskDescriptor_s
{
	task_t task;				   ///< function pointer to call
	void *param;				   ///< pointer, which is passed to task when executed
	uint16_t expire;			   ///< time offset in ms, after which to call the task
	uint16_t period;			   ///< period of the timer after firing; 0 means exec once
	uint8_t execute : 1;		   ///< for internal use
	uint8_t reserved : 7;		   ///< reserved
	struct taskDescriptor_s *next; ///< pointer to next task, internal use
} taskDescriptor;

/**
 * Wrapper functions to calculate systemTime
 */
struct time_t
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint16_t milli;
};
/* FUNCTION PROTOTYPES *******************************************************/

/**
 * Initializes the task scheduler. Uses hardware timer2 of the AVR.
 */
void scheduler_init();

/**
 * Runs scheduler in an infinite loop.
 */
void scheduler_run();

/**
 * Adds a new task to the scheduler.
 * May be called from any context (interrupt or main program)
 *
 * @param td   Pointer to taskDescriptor structure. The scheduler takes
 *             possesion of the memory pointed at by td until the task
 *             is removed by scheduler_remove or -- in case of a
 *             non-periodic task -- the task is executed. td->expire
 *             and td->execute are changed to by the task scheduler.
 *
 * @return     false, if task is already scheduled or invalid (NULL)
 *             true, if task was successfully added to scheduler and
 *             will be executed after td->expire ms.
 */
bool scheduler_add(taskDescriptor *td);

/**
 * Removes a task from the scheduler.
 * May be called from any context (interrupt or main program)
 *
 * @param td	pointer to task descriptor to remove
 * */
void scheduler_remove(taskDescriptor *td);

/**
 * Gets the current system time from the created clock
 *
 * @return systemTime in ms
 */
systemTime_t scheduler_getTime();

/**
 * Sets the systemTime to value and then the created clock starts
 * calculation of time from it and is updated by a resolution of
 * 1 ms by the scheduler ISR
 *
 * @param time the time in which the created clock starts counting from
 */
void scheduler_setTime(systemTime_t time);

#endif /* SCHEDULER_H_ */
