#ifndef TIMER_H
# define TIMER_H

#include <avr/io.h>
#define TIM1_stop()      TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));
#define TIM1_500ms_ovf() TCNT1 = 34286; TCCR1A = 0x00; TCCR1B &= ~(1<<CS10) | (1<<CS12); TCCR1B |= (1<<CS10);

/** @brief Set overflow 1s, prescaler 100 --> 256 */
#define TIM1_ovf_1sec()  TCCR1B &= ~((1<<CS11) | (1<<CS10)); TCCR1B |= (1<<CS12);
/** @brief Set overflow 4s, prescaler // 101 --> 1024 */
#define TIM1_ovf_4sec()  TCCR1B &= ~(1<<CS11); TCCR1B |= (1<<CS12) | (1<<CS10);

#define TIM1_ovf_enable() TIMSK1 |= (1<<TOIE1);
#define TIM1_ovf_disable() TIMSK1 &= ~(1<<TOIE1);

#define TIM3_stop()      TCCR3B &= ~((1<<CS32) | (1<<CS31) | (1<<CS30));
#define TIM3_ovf_50ms()  TCCR3B &= ~(1<<CS32); TCCR3B |= (1<<CS31) | (1<<CS30); TCNT3 = 50036;
#define TIM3_ovf_enable() TIMSK3 |= (1<<TOIE3);
#define TIM3_ovf_disable() TIMSK3 &= ~(1<<TOIE3);
#endif