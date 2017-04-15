/*!
* \file configuration.h
* \brief File containing constants
*/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define PRESS_DELAY 100000			/*delay in uS*/
#define REST_DELAY 20000 			/*delay in uS*/
#define STYLUS_CLICK_POSITION 73
#define STYLUS_REST_POSITION 90
#define PWM_PIN 1


#define PWM_RANGE 2000
#define PWM_CLOCK 192
#define MIN_PWM_PULSE_DURATION 80
#define MAX_PWM_PULSE_DURATION 200

#define NB_PIXEL_THRESHOLD 10
#define BIRD_CIRCLE_DIAMETER 15

#define CAPTURE_WIDTH 960/2		/* Must be a multiple of 32 */
#define CAPTURE_HEIGHT 720/2	/* Must be a multiple of 32 */

#define TRACKED_OBJECT_DEFAULT_COLOR CV_RGB(255, 0, 0)

enum {CIRCLE,RECTANGLE};
enum boolean{false = 0,true = 1,False = 0,True = 1, FALSE = 0,TRUE = 1};

/* Tolerances for color analysis (HSV format)*/
const int Htolerance = 5;
const int Stolerance = 30;


#endif /*CONFIGURATION_H*/
