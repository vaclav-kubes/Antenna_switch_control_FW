#ifndef TIMER_H
# define TIMER_H

#include <avr/io.h>

#define TIM1_500ms_ovf() TCNT1 = 57723; TCCR1A = 0x00; TCCR1B &= ~(1<<CS10) | (1<<CS12);

#define TIM1_1s_ovf() TCNT1 = 49911; TCCR1A = 0x00; TCCR1B &= ~(1<<CS10) | (1<<CS12);

#define TIM1_4s_ovf() TCNT1 = 3036; TCCR1A = 0x00; TCCR1B &= ~(1<<CS10) | (1<<CS12);

#define TIM1_ovf_enable() TIMSK1 |= (1<<TOIE1);

#endif