/*INCLUDES *******************************************************************/
#include "ses_adc.h"
#include "ses_common.h"
#include "ses_led.h"
#include "ses_rotary.h"
#include "ses_lcd.h"
#include "ses_uart.h"
#include  "ses_scheduler.h"
/* DEFINES & MACROS **********************************************************/
#define A_ROTARY_PORT PORTB
#define A_ROTARY_PIN 5

#define B_ROTARY_PORT PORTG
#define B_ROTARY_PIN 2

#define ROTARY_NUM_DEBOUNCE_CHECKS 5
#define ROTARY_DEBOUNCE_POS_START 0x03
#define ROTARY_DEBOUNCE_POS_MID 0x00
#define ROTARY_DEBOUNCE_POS_CW 0x01
#define ROTARY_DEBOUNCE_POS_CCW 0x02
/* PRIVATE VARIABLES **************************************************/
volatile static pTypeRotaryCallback clockwiseCallback;
volatile static pTypeRotaryCallback counterClockwiseCallback;

struct taskDescriptor_s rotaryDebouncingTask;
/* FUNCTION DEFINITION *******************************************************/
void rotary_init()
{
    DDR_REGISTER(A_ROTARY_PORT) &= ~(1 << A_ROTARY_PIN); //define the rotary A button as input
    DDR_REGISTER(B_ROTARY_PORT) &= ~(1 << B_ROTARY_PIN); //define the rotary B button as input
}

void rotary_checkState(void *param)
{

    static uint8_t state[ROTARY_NUM_DEBOUNCE_CHECKS] = {};
    static uint8_t index = 0;
    static uint8_t debouncedState = 0;
    uint8_t debouncingCheckClockwise = 0;        //CW flag
    uint8_t debouncingCheckCounterClockwise = 0; //CCW flag
    uint8_t lastDebouncedState = debouncedState;
    // each bit in every state byte represents one button
    state[index] = 0;
    if (rotary_isClockwisePressed())
    {
        state[index] |= ROTARY_DEBOUNCE_POS_CW;
    }
    if (rotary_isCounterClockwisePressed())
    {
        state[index] |= ROTARY_DEBOUNCE_POS_CCW;
    }
    index++;
    if (index == ROTARY_NUM_DEBOUNCE_CHECKS)
    {
        index = 0;
    }
    // init compare value and compare with ALL reads, only if
    // we read BUTTON_NUM_DEBOUNCE_CHECKS consistent "1's" in the state
    // array, the button at this position is considered pressed
    uint8_t j = 0xFF;
    for (uint8_t i = 0; i < ROTARY_NUM_DEBOUNCE_CHECKS; i++)
    {
        j = j & state[i];
    }
    debouncedState = j;
    if (debouncedState != lastDebouncedState)
    {
        if (clockwiseCallback != NULL)
        { //for the CW the required sequence for pin A is (10011), for pin B (11001)
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_START && debouncedState == ROTARY_DEBOUNCE_POS_CW)
            { //first clockwise sequence
                debouncingCheckClockwise = 1;
                clockwiseCallback();
            }
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_CW && debouncedState == ROTARY_DEBOUNCE_POS_MID)
            { //second clockwise sequence
                debouncingCheckClockwise = 1;
                clockwiseCallback();
            }
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_MID && debouncedState == ROTARY_DEBOUNCE_POS_CCW)
            { //third clockwise sequence
                debouncingCheckClockwise = 1;
                clockwiseCallback();
            }
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_CCW && debouncedState == ROTARY_DEBOUNCE_POS_START)
            { //fourth clockwise sequence
                debouncingCheckClockwise = 1;
                clockwiseCallback();
            }
        }

        if (counterClockwiseCallback != NULL)
        { //for the CCW the required sequence for pin A is (11001), for pin B (10011)
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_START && debouncedState == ROTARY_DEBOUNCE_POS_CCW)
            { //first counter clockwise sequence
                debouncingCheckCounterClockwise = 1;
                counterClockwiseCallback();
            }
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_CCW && debouncedState == ROTARY_DEBOUNCE_POS_MID)
            { //second counter clockwise sequence
                debouncingCheckCounterClockwise = 1;
                counterClockwiseCallback();
            }
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_MID && debouncedState == ROTARY_DEBOUNCE_POS_CW)
            { //third counter clockwise sequence
                debouncingCheckCounterClockwise = 1;
                counterClockwiseCallback();
            }
            if (lastDebouncedState == ROTARY_DEBOUNCE_POS_CW && debouncedState == ROTARY_DEBOUNCE_POS_START)
            { //fourth counter clockwise sequence
                debouncingCheckCounterClockwise = 1;
                counterClockwiseCallback();
            }
        }
    }
    else
    {
        fprintf(uartout, "not valid");
        fprintf(lcdout, "not valid press");
        return;
    }
}

void check_rotary(void)
{
    static uint8_t p = 0;
    static bool sampling = false;
    bool a = PIN_REGISTER(A_ROTARY_PORT) & (1 << A_ROTARY_PIN);
    bool b = PIN_REGISTER(B_ROTARY_PORT) & (1 << B_ROTARY_PIN);
    if (a != b)
    {
        sampling = true;
    }
    if (sampling && p < 122)
    {
        lcd_setPixel((a) ? 0 : 1, p, true);
        lcd_setPixel((b) ? 4 : 5, p, true);
        p++;
    }
}

bool rotary_isClockwisePressed(void)
{

    uint8_t clockwise_rotary_pinValue;
    clockwise_rotary_pinValue = GET_BIT(B_ROTARY_PORT, B_ROTARY_PIN);

    if (!clockwise_rotary_pinValue)
    {
        return true;
    }
    return false;
}
bool rotary_isCounterClockwisePressed(void)
{

    uint8_t counterClockwise_rotary_pinValue;
    counterClockwise_rotary_pinValue = GET_BIT(A_ROTARY_PORT, A_ROTARY_PIN);

    if (!counterClockwise_rotary_pinValue)
    {
        return true;
    }
    return false;
}

void rotary_setClockwiseCallback(pTypeRotaryCallback callback)
{
    clockwiseCallback = callback;
}

void rotary_setCounterClockwiseCallback(pTypeRotaryCallback callback)
{
    counterClockwiseCallback = callback;
}

/**
 * Executes the debouncing for the rotary encoder
 * CW and CCW buttons every 5 ms
 */
static void task6()
{
    rotaryDebouncingTask.task = (void *)&check_rotary;
    rotaryDebouncingTask.param = NULL;
    rotaryDebouncingTask.execute = 0;
    rotaryDebouncingTask.expire = 0;
    rotaryDebouncingTask.period = 5;
}
