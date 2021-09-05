#ifndef SES_alarmClock_H_
#define SES_alarmClock_H_

/*INCLUDES *******************************************************************/

#include "ses_common.h"
#include "ses_scheduler.h"
/*PROTOTYPES *****************************************************************/
typedef struct fsm_s Fsm;

//< typedef for alarm clock state machine
typedef struct event_s Event; //< event type for alarm clock fsm

typedef uint8_t fsmReturnStatus; //< typedef to be used with above enum

/** typedef for state event handler functions */
typedef fsmReturnStatus (*State)(Fsm *, const Event *);

/** return values */
enum
{
    RET_HANDLED,   //< event was handled
    RET_IGNORED,   //< event was ignored; not used in this implementation
    RET_TRANSITION //< event was handled and a state transition occurred
};

/*Signals used by the alarmClock FSM*/
enum
{
    ENTRY,
    EXIT,
    JOYSTICK_PRESSED,
    ROTARY_PRESSED,
    FIVE_SECONDS_PASSED,
    ALARM_TIME_REACHED
};

struct fsm_s
{
    State state;

    //<current state, pointer to event handler
    bool isAlarmEnabled;

    //<flag for the alarm status
    struct time_t timeSet; //< multi-purpose var for system time and alarm time
};

struct event_s
{
    uint8_t signal; //< identifies the type of event
};

/**
 * Dispatches events to state machine, called in application
 *
 * @param fsm   the state machine
 * @param event the event to be dispatched
 */
inline static void fsm_dispatch(Fsm *fsm, const Event *event)
{
    static Event entryEvent = {.signal = ENTRY};
    static Event exitEvent = {.signal = EXIT};
    State s = fsm->state;
    fsmReturnStatus r = fsm->state(fsm, event);
    if (r == RET_TRANSITION)
    {
        s(fsm, &exitEvent);

        //< call exit action of  last state
        fsm->state(fsm, &entryEvent); //< call entry action of new state
    }
}

/**
 * Dispatches an event when the actual system time is equal
 * to the alarm time; ALARM_TIME_REACHED
 */
static void alarmTimeIsReachedDispatch(void);

/**
 * Dispatches an event when five seconds
 * have passed; FIVE_SECONDS_PASSED
 */
static void fiveSecondsPassedDispatch(void *param);

/**
 * The normal operation mode state of the alarm clock
 * where most of the clock operations are handled;
 * updates the Lcd display, contiuously check if the alarm time
 * was reached, blinks the green led with seconds counter, controls enabling and
 * disabling of the alarm
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus running(Fsm *fsm, const Event *event);

/**
 * Sets the clock's hours using the rotary, displays the un-intialized clock,
 * requests from the user to set the clock
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus setHour(Fsm *fsm, const Event *event);

/**
 * Sets the clock's minutes using the rotary, displays the set clock,
 * sets the system time to start the clock operation
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus setMinutes(Fsm *fsm, const Event *event);

/**
 * Sets the alarm clock's hours using the rotary,
 * and displays the set alarm hours
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus setHourAlarm(Fsm *fsm, const Event *event);

/**
 * Sets the alarm clock's minutes using the rotary,
 * and displays the set alarm clock
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus setMinutesAlarm(Fsm *fsm, const Event *event);

/**
 * Enables the alarm mode by setting the flag isALarmEnabled
 * to true, turns the led yellow on
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus enableAlarm(Fsm *fsm, const Event *event);

/**
 * Run's the alarm if the alarm time is reached,
 * toggles the red led at 4 Hz
 *
 * @param fsm   the state machine
 * @param event  the event to be handled
 * @return the event handled status; either handled, ignored, or
 * a state transiton
 * */
fsmReturnStatus runningAlarm(Fsm *fsm, const Event *event);

/**
 * Converts from time_t to systemTime_t
 *
 * @param  time time in time_t type
 * @return the time in the type systemTime_t
 *
static systemTime toSystemTime(SystemTime_t time);

/**
 * Converts from systemTime_t to time_t in milliseconds
 *
 * @param systemTime time in systemTime_t type
 * @return the time in millis in the type time_t
 *
static systemTime fromSystemTime(time_t systemTime);
*/

/**
 * Gets the updated system time and converts it to time_t
 * and prints the clock in 24-hour format on the Lcd
 *
 * @param param void pointer
 */
void updateDisplay(void *param);

/**
 * Gets the updated system time and converts it to time_t
 * and prints the clock in 24-hour format on the Lcd,
 * checks if alarm time is reached in the running mode
 * and if reached call the correponding dispatch function
 *
 * @param param void pointer

void updateDisplayCheckAlarm(void *param);
*/

/**
 * Prints the clock on the Lcd while setting
 * the hours and the minutes and incrementing
 * them
 */
void updateSetDisplay(void);

/**
 * Display the set alarm time on the Lcd
 */
void displayAlarm(void);

/**
 * Prints the un initialize clock "HH:MM:SS"
 * and requests the user to set the clock on the Lcd
 */
void initialPrint(void);

/**
 * Checks if the aLarm time is reached,
 * and fires a check  ALARM_IS_REACHED signal if it is time out
 *
 * @param param void pointer
 */
void checkAlarm(void *param);

/**
 * Prints and updates the system time every 1000 ms,
 */
void task1(void);

/**
 * Calculates 5 seconds after the runningAlarm state is entered
 * and then fires the FIVE_second_PASSED signal
 */
void task2(void);

/**
 * Blinks the red led at 4 Hz
 */
void task3(void);

/**
 * Blinks the green led with the seconds counter
 *
 */
void task4(void);

/**
 * Checks if the alarm is reached every 1000 ms
 *
 */
void task5(void);

#endif /* SES_alarmClock_H_ */
