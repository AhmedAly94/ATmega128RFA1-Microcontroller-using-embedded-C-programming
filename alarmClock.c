/* INCLUDES ******************************************************************/
#include "ses_common.h"
#include "ses_led.h"
#include "ses_lcd.h"
#include "ses_button.h"
#include "ses_timer.h"
#include "ses_alarmClock.h"
#include "ses_scheduler.h"
#include "ses_uart.h"

/* DEFINES & MACROS **********************************************************/
#define TRANSITION(newState) (fsm->state = newState, RET_TRANSITION)

#define HOURS_TO_SECONDS 3600
#define MINUTES_TO_SECONDS 60
#define SECONDS_TO_MILLIS 1000
#define HOURS_TO_MILLIS 3600000
#define MINUTES_TO_MILLIS 60000
#define HOURS_PER_DAY 24
#define MINUTES_PER_HOUR 60
/* PRIVATE VARIABLES **************************************************/
//Variables for taskDescriptor
struct taskDescriptor_s updateDisplayTask;
struct taskDescriptor_s timeOutCounterTask;
struct taskDescriptor_s flashRedLedTask;
struct taskDescriptor_s greenLedSynchronizeTask;
struct taskDescriptor_s checkAlarmTask;

static volatile uint8_t alarmHours = 0, alarmMinutes = 0;
struct fsm_s theFsm; //variable for the state machine
/* FUNCTION DEFINITION *******************************************************/

/*events dispatch functions, others handled in main c file*/
static void alarmTimeIsReachedDispatch(void)
{
    Event e = {.signal = ALARM_TIME_REACHED};
    fsm_dispatch(&theFsm, &e);

    fprintf(uartout, "ALAAAARMMMM DISPATCH\n");
}

static void fiveSecondsPassedDispatch(void *param)
{
    Event e = {.signal = FIVE_SECONDS_PASSED};
    fsm_dispatch(&theFsm, &e);

    fprintf(uartout, "Five seconds passed\n");
}

fsmReturnStatus running(Fsm *fsm, const Event *event)
{ ///normal opration mode of the  clock
    switch (event->signal)
    {
        //...handling of other events
    case ENTRY:
        fprintf(uartout, "running mode\n");
        scheduler_add(&checkAlarmTask); //check if alarm time is reached if the alarm is enabled
        scheduler_add(&updateDisplayTask); //update display on entry of the state
        fprintf(uartout, "print task added\n");
        led_redOff();           //turn off the red led in normal mode
        scheduler_add(&greenLedSynchronizeTask); //blink the green led with the seconds counter
        //fprintf(uartout, "greeen blink task added\n");
        return RET_HANDLED;

    case JOYSTICK_PRESSED:
        fprintf(uartout, "print task removing\n");
        scheduler_remove(&updateDisplayTask); // Stop the update display on exit of the state
        fprintf(uartout, "print task removed\n");
        return TRANSITION(setHourAlarm); //set alarm hours if the joystick is pressed

    case ROTARY_PRESSED:
        if (fsm->isAlarmEnabled)
        {                                //if the rotary is pressed and the alarm is enabled, disable the alarm
            fsm->isAlarmEnabled = false; //disable the alarm
            led_yellowOff();             //turn yellow led off if the alarm is disabled
            fprintf(uartout, "alarm disabled\n");
            return RET_HANDLED;
        }
        return TRANSITION(enableAlarm); //if rotary pressed in normal mode go to enable alarm state

    case ALARM_TIME_REACHED:
        fprintf(uartout, "alarm time is reached\n");
        scheduler_remove(&greenLedSynchronizeTask);       // stop the blink of the green led with the seconds counter
        led_greenOff();                  //turn green led off if the alarm is running
        scheduler_remove(&checkAlarmTask);       //remove the check for the alarm time when the alarm time reached
        return TRANSITION(runningAlarm); //if the alarm time was reached run the alarm

    default:
        return RET_IGNORED;
    }
}
fsmReturnStatus setHour(Fsm *fsm, const Event *event)
{ //sets clock hours and updates the display
    switch (event->signal)
    {
    //...handling of other events
    case ENTRY:
        //Initialize the timeSet variable with zero at start
        fsm->timeSet.hour = 0;
        fsm->timeSet.minute = 0;
        fsm->timeSet.second = 0;
        fsm->timeSet.milli = 0;
        fsm->isAlarmEnabled = false; //alarm disabled at start
        initialPrint();              //print un intialized clock and request to set clock at start
        //scheduler_add(&updateDisplayTask);      //update display after incrementing the hours
        return RET_HANDLED;

    case JOYSTICK_PRESSED:
        return TRANSITION(setMinutes); //if the joystick is pressed then set the clock minutes

    case ROTARY_PRESSED:
        fsm->timeSet.hour = (fsm->timeSet.hour + 1) % HOURS_PER_DAY; //increment hours if rotary is pressed, cover the 24 hour case
        updateSetDisplay();                                          //update display after incrementing the hours
        //scheduler_setTime(fsm->timeSet.hour * HOURS_TO_SECONDS * SECONDS_TO_MILLIS); //update the system time hours
        return RET_HANDLED;

    default:
        return RET_IGNORED;
    }
}

fsmReturnStatus setMinutes(Fsm *fsm, const Event *event)
{
    switch (event->signal)
    {
    //...handling of other events
    case JOYSTICK_PRESSED:
        return TRANSITION(running);

    case ROTARY_PRESSED:
        fsm->timeSet.minute = (fsm->timeSet.minute + 1) % MINUTES_PER_HOUR; //increment minutes if rotary is pressed, cover the 60 minutes case
        updateSetDisplay();                                                 //update display after incrementing the minutes
        return RET_HANDLED;

    case EXIT:
        scheduler_setTime((fsm->timeSet.hour * HOURS_TO_MILLIS) +
                          (fsm->timeSet.minute * MINUTES_TO_MILLIS)); //update the system time
        return RET_HANDLED;

    default:
        return RET_IGNORED;
    }
}

fsmReturnStatus setHourAlarm(Fsm *fsm, const Event *event)
{
    switch (event->signal)
    {
    //...handling of other events
    case ENTRY:
        fprintf(uartout, "set alarm hours");
        //scheduler_remove(&updateDisplayTask);//remove update display after clock is set
        displayAlarm(); //display the un-initialized alarm
        return RET_HANDLED;

    case JOYSTICK_PRESSED:
        return TRANSITION(setMinutesAlarm); //if the joystick is pressed go to set alarm minutes state

    case ROTARY_PRESSED:
        alarmHours = (alarmHours + 1) % HOURS_PER_DAY; //increment hours if rotary is pressed, cover the 24 hour case
        displayAlarm();                                //display the set alarm hours
        return RET_HANDLED;

    case EXIT:
        fprintf(uartout, "set alarm minutes");
        return RET_HANDLED;

    default:
        return RET_IGNORED;
    }
}

fsmReturnStatus setMinutesAlarm(Fsm *fsm, const Event *event)
{
    switch (event->signal)
    {
        //...handling of other events
    case ROTARY_PRESSED:
        alarmMinutes = (alarmMinutes + 1) % MINUTES_PER_HOUR; //increment minutes if rotary is pressed, cover the 60 minutes case
        displayAlarm();                                       //display the set alarm minutes
        return RET_HANDLED;

    case JOYSTICK_PRESSED:
        //display the set alarm once on the screen line 1
        lcd_setCursor(2, 1);
        fprintf(lcdout, "alarm time: %02d:%02d", alarmHours, alarmMinutes);
        fprintf(uartout, "alarm time: %02d:%02d\n", alarmHours, alarmMinutes);
        return TRANSITION(running); //if alarm minutes is set go to the running state

    default:
        return RET_IGNORED;
    }
}

fsmReturnStatus enableAlarm(Fsm *fsm, const Event *event)
{
    switch (event->signal)
    {
    //...handling of other events
    case ENTRY:
        fsm->isAlarmEnabled = true; ///enable alarm
        led_yellowOn();             //yellow led on if the alarm is enabled
        fprintf(uartout, "alarm enabled\n");
        return TRANSITION(running); //enable alarm then reaturn back to clock's normal operation
    default:
        return RET_IGNORED;
    }
}

fsmReturnStatus runningAlarm(Fsm *fsm, const Event *event)
{ //alarm operation mode
    switch (event->signal)
    {
        //...handling of other events
    case ENTRY:
        fprintf(uartout, "ALAAAARRMMMM");
        scheduler_add(&timeOutCounterTask); //start checking for the five seconds
        scheduler_add(&flashRedLedTask); //toggle the red led on entry
        return RET_HANDLED;

    //if any button was pressed go to the normal mode
    case JOYSTICK_PRESSED:
        return TRANSITION(running);

    case ROTARY_PRESSED:
        return TRANSITION(running);

    //if 5 seconds have passed go to the normal mode
    case FIVE_SECONDS_PASSED:
        return TRANSITION(running);

    case EXIT:
        fsm->isAlarmEnabled = false; //disable the alarm after running it
        led_yellowOff();             //turn the yelllow led off if the alarm is disabled
        scheduler_remove(&flashRedLedTask);   //stop the toggling of the red led on entered
        return RET_HANDLED;

    default:
        return RET_IGNORED;
    }
}

/*Converts from time_t to systemTime_t*/
static systemTime_t toSystemTime(struct time_t time)
{ ///accumulate the hours, minutes, seconds, and millis
    return ((time.hour) * HOURS_TO_SECONDS + (time.minute) * MINUTES_TO_SECONDS + (time.second)) * SECONDS_TO_MILLIS + (time.milli);
}

/*Converts from systemTime_t to time_t in milliseconds*/
static struct time_t fromSystemTime(systemTime_t systemTime)
{
    struct time_t time;

    time.hour = systemTime / HOURS_TO_MILLIS;                //get the hours first
    systemTime = systemTime - (HOURS_TO_MILLIS * time.hour); //subtract hours from the systemTime

    time.minute = systemTime / MINUTES_TO_MILLIS;                //get the minutes
    systemTime = systemTime - (MINUTES_TO_MILLIS * time.minute); //subtract minutes from the systemTime

    time.second = systemTime / SECONDS_TO_MILLIS;                //get the seconds
    systemTime = systemTime - (SECONDS_TO_MILLIS * time.second); //subtract seconds from systemTime;finally to get the millis

    time.milli = systemTime; //get the millis

    return time;
}

void updateDisplay(void *param)
{
    //fprintf(uartout, "updatingLCD...\n");
    struct time_t currentTime = fromSystemTime(scheduler_getTime());

    lcd_clear();

    lcd_setCursor(4, 2);
    fprintf(lcdout, "%02d:%02d:%02d", currentTime.hour, currentTime.minute, currentTime.second);

    fprintf(uartout, "%02d:%02d:%02d\n", currentTime.hour, currentTime.minute, currentTime.second);
}

void updateSetDisplay(void)
{
    lcd_clear();

    lcd_setCursor(4, 2);
    fprintf(lcdout, "%02d:%02d", theFsm.timeSet.hour, theFsm.timeSet.minute);
    fprintf(uartout, "%02d:%02d\n", theFsm.timeSet.hour, theFsm.timeSet.minute);
}

void displayAlarm(void)
{
    lcd_clear();

    lcd_setCursor(2, 1);
    fprintf(lcdout, "alarm time: %02d:%02d", alarmHours, alarmMinutes);
    fprintf(uartout, "alarm time: %02d:%02d\n", alarmHours, alarmMinutes);
}

void initialPrint(void)
{
    lcd_clear();

    lcd_setCursor(3, 1);
    fprintf(lcdout, "HH:MM:SS"); //display the uninitialized clock on start
    fprintf(uartout, "HH:MM:SS\n");

    lcd_setCursor(0, 2);
    fprintf(lcdout, "Please set the clock"); //request from the user to set the clock at start
    fprintf(uartout, "Please set the clock\n");
}

void checkAlarm(void *param)
{
    struct time_t currentTime = fromSystemTime(scheduler_getTime());

    //fprintf(uartout, "checking...\n");
    if (alarmHours == currentTime.hour && alarmMinutes == currentTime.minute && theFsm.isAlarmEnabled)
    {
        alarmTimeIsReachedDispatch(); //fire ALARM_TIME_REACHED signal if alarm time is equal to actual time

        //fprintf(uartout, "ALAAAARMMMM\n");
    }
}

/**
 * Prints and updates the system time every 1000 ms
 */
void task1()
{ //Create task
    updateDisplayTask.task = (void *)&updateDisplay;
    updateDisplayTask.param = NULL;
    updateDisplayTask.execute = 0;
    updateDisplayTask.expire = 0;
    updateDisplayTask.period = 1000;
}

/**
 * Calculates 5 seconds after the runningAlarm state is entered
 * and then fires the FIVE_second_PASSED signal
 */
void task2()
{ //Create task
    timeOutCounterTask.task = (void *)&fiveSecondsPassedDispatch;
    timeOutCounterTask.param = NULL;
    timeOutCounterTask.execute = 0;
    timeOutCounterTask.expire = 5000;
    timeOutCounterTask.period = 0;
}

/**
 * Blinks the red led at 4 Hz
 */
void task3()
{ //Create task
    flashRedLedTask.task = &led_redToggle;
    flashRedLedTask.param = NULL;
    flashRedLedTask.execute = 0;
    flashRedLedTask.expire = 0;
    flashRedLedTask.period = 125; //blinks 8 times every second
}

/**
 * blinks the green led with the seconds counter
 *
 */
void task4()
{ //Create task
    greenLedSynchronizeTask.task = &led_greenToggle;
    greenLedSynchronizeTask.param = NULL;
    greenLedSynchronizeTask.execute = 0;
    greenLedSynchronizeTask.expire = 0;
    greenLedSynchronizeTask.period = 1000 - (scheduler_getTime() % SECONDS_TO_MILLIS); //blinks the led in synchronization with the seconds counter
}

/**
 * Checks if the alarm is reached every 1000 ms
 *
 */
void task5()
{ //Create task
    checkAlarmTask.task = (void *)&checkAlarm;
    checkAlarmTask.param = NULL;
    checkAlarmTask.execute = 0;
    checkAlarmTask.expire = 0;
    checkAlarmTask.period = 1000;
}
