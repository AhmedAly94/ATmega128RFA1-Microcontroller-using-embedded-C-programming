#ifndef SES_ROTARY_H
#define SES_ROTARY_H

/*INCLUDES *******************************************************************/

#include <inttypes.h>
#include <avr/io.h>
#include "ses_common.h"

/* DEFINES & MACROS **********************************************************/
/**
 * Definition of the rotary fuction pointer
 */
typedef void (*pTypeRotaryCallback)();

/* FUNCTION PROTOTYPES *******************************************************/
/**
 * Initializes rotary button's right and left
 * buttons
 */
void rotary_init();

/**
 * Sets call back for the Rotary encoder's (Clockwise) right button
 *
 * @param pTypeRotaryCallback function pointer
 */
void rotary_setClockwiseCallback(pTypeRotaryCallback);

/**
 *Sets call back for the Rotary encoder's (CountereClockwise) left button
 *
 * @param pTypeRotaryCallback function pointer
 */
void rotary_setCounterClockwiseCallback(pTypeRotaryCallback);

/*
* Callback-procedure to plot the samples of the rotary pins on the LCD after first change
*/
void check_rotary(void);

/**
 * Get the state of the rotary's Clockwise button.
 */
bool rotary_isClockwisePressed(void);

/**
 * Get the state of the rotary's CounterClockwise button.
 */
bool rotary_isCounterClockwisePressed(void);

/**
 * handles debouncing of the rotaries clockwise and counter Buttons
 *
 * @param param void pointer
 */
void rotary_checkState(void *param);

#endif /* SES_ROTARY_H */
