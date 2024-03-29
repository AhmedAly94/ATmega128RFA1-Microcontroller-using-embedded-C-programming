/*INCLUDES *******************************************************************/
#include "ses_common.h"
#include "ses_lcd.h"
#include "ses_button.h"
#include "ses_uart.h"
#include "ses_led.h"
#include "ses_pwm.h"
#include "ses_motorFrequency.h"
#include "ses_scheduler.h"
#include "ses_timer.h"
#include "ses_alarmClock.h"
#include "ses_rotary.h"
/* DEFINES & MACROS **********************************************************/
#define BAUDRATE 57600
/* GLOBAL VARIABLES *******************************************************/
struct fsm_s theFsm; //Global variable for the state machine
/* FUNCTION PROTOTYPES *******************************************************/
/**
 * Sets and calls initial state of state machine
 *
 * @param fsm   the state machine
 * @param init  the initial state of the state machine
 */

inline static void fsm_init(Fsm *fsm, State init);

/**
 * Dispatches an event when the joystick is pressed; JOYSTICK_PRESSED
 *
 * @param param void pointer
 */
static void joystickPressedDispatch(void *param);

/**
 * Dispatches an event when the rotary is pressed; ROTARY_PRESSED
 *
 * @param param void pointer
 */
static void rotaryPressedDispatch(void *param);
/*FUNCTION DEFINITION ********************************************************/

inline static void fsm_init(Fsm *fsm, State init)
{
    //...other initialization
    //intial state
    Event entryEvent = {.signal = ENTRY};
    fsm->state = init;
    fsm->state(fsm, &entryEvent);
}

static void joystickPressedDispatch(void *param)
{
    Event e = {.signal = JOYSTICK_PRESSED};
    fsm_dispatch(&theFsm, &e);
}

static void rotaryPressedDispatch(void *param)
{
    Event e = {.signal = ROTARY_PRESSED};
    fsm_dispatch(&theFsm, &e);
}

int main(void)
{ //initialize leds
    led_redInit();
    led_greenInit();
    led_yellowInit();

    lcd_init(); //initialize lcd

    uart_init(BAUDRATE); //initialize uart

    button_init(true); //debouncing is active

    //callbacks are set to the event dispatch functions
    button_setRotaryButtonCallback(&rotaryPressedDispatch);
    button_setJoystickButtonCallback(&joystickPressedDispatch);

    //Scheduler's event queue
    task1();
    task2();
    task3();
    task4();
    task5();

     //Initialize the fsm
    fsm_init(&theFsm, setHour);

    //Initialize scheduler
    scheduler_init();

    sei(); //Globally Enable Interrupts

    //run the scheduler
    scheduler_run();

    return 0;
}
