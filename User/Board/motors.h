/* motors.h */
#ifndef MOTORS_H
#define MOTORS_H

#include "Device/motor/motor_bdc.h"
#include "Device/motor/motor_servo_pwm.h"
#include "Device/encoder/encoder_base.h"

extern struct motor_bdc *g_motor_left;
extern struct motor_bdc *g_motor_right;
extern struct motor_servo_pwm *g_motor_steering;
extern struct encoder_base *g_encoder_left;
extern struct encoder_base *g_encoder_right;

int motor_board_init(void);

#endif /* MOTORS_H */
