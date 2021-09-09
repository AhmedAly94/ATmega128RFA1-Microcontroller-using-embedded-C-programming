# ATmega128RFA1-Microcontroller-using-embedded-C-programming
Software for embedded systems project at Technische Universität Hamburg (TUHH) 

An alarm clock is implemented using scheduler.The scheduler adds and removes tasks to be implemented by a certain order.

When the microcontroller is powered up, the actual time is not known. The user has to set the time
manually using the buttons. At this stage, the display shows an uninitialized clock using the format
HH:MM. The second display line may show a request for the user to set time. First, the hour has to be
set by repeatedly pressing the Rotary Button. After pressing the Joystick Button, the minutes have to
be set via the Rotary Button. Pressing the Joystick Button again updates the system time and starts the
clock. In this normal operation mode the time is shown in the format HH:MM:SS. In this state, the user
can press the Rotary Button to enable the alarm or to disable the alarm. If the Joystick Button is pressed,
the alarm time can be set. In this mode line 1 shows the alarm time instead of the current system time
using the format HH:MM. If the alarm is enabled and the actual time matches the alarm time, the red
LED shall toggle with 4 Hz. The alarm shall stop, if any button is pressed or 5 seconds have passed. The
alarm must only be triggered if the clock is in its normal operating mode, i.e., it must not be triggered
while the alarm time is being modified.

In addition, the LEDs are used for the following functionality:
 The green LED blinks synchronously with the counter of the seconds.
 The yellow LED is on, if and only if the alarm is enabled.
 The red LED is flashing with 4 Hz during alarm, and it is off otherwise.

Task1: Prints and updates the system time every 1000 ms
Task2: Calculates 5 seconds after the runningAlarm state is entered and then fires the FIVE_second_PASSED signal
Task3: Blinks the red led at 4 Hz
Task4: blinks the green led with the seconds counter
Task5: Checks if the alarm is reached every 1000 ms

An UML statechart matching the previous description is attached 
