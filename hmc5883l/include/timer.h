#ifndef TIMER_H
# define TIMER_H

/***********************************************************************
 * 
 * Small custom Timer library for ATmega328PB.
 * 
 * ATmega328PB, 16 MHz
 *
 * Václav Kubeš
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/

#include <avr/io.h>

/* Defines -----------------------------------------------------------*/

/**
 * @name  Definitions for 16-bit Timer/Counter1
 */

/** @brief Stop timer, prescaler 000 --> STOP */
#define TIM1_stop()      TCCR1B &= ~((1<<CS12) | (1<<CS11) | (1<<CS10));

/** @brief Set overflow 500ms, ! works only if COMPA inerrupt is on !, prescaler 101 --> 1024, OCR1A = 7813 */
#define TIM1_500ms_ovf() OCR1A = 7813; TCCR1A = 0x00; TCCR1B &= ~(1<<CS11); TCCR1B |= ((1<<CS10) | (1<<CS12) | (1<<WGM12));

/** @brief Set overflow 1s, prescaler 100 --> 256 */
#define TIM1_ovf_1sec()  TCCR1B &= ~((1<<CS11) | (1<<CS10)); TCCR1B |= (1<<CS12);

/** @brief Set overflow 4s, prescaler // 101 --> 1024 */
#define TIM1_ovf_4sec()  TCCR1B &= ~(1<<CS11); TCCR1B |= (1<<CS12) | (1<<CS10);

/** @brief Enable timer 1 ovf. and compare match interrupts */
#define TIM1_ovf_enable() TIMSK1 |= (1<<TOIE1);// | (1<<OCIE1A);

/** @brief Disable interrupts by Timer 1*/
#define TIM1_ovf_disable() TIMSK1 &= ~((1<<TOIE1) |(1<<OCIE1A));

/** @brief Stop timer, prescaler 000 --> STOP */
#define TIM3_stop()      TCCR3B &= ~((1<<CS32) | (1<<CS31) | (1<<CS30));

/** @brief Set compare match reg. A to 15500 and prescaler to 101 --> 1024*/
#define TIM3_ovf_50ms()  TCCR3B &= ~(1<<CS32); TCCR3B |= (1<<CS31) | (1<<CS30) | (1<<WGM32); OCR3A = 15500;

/** @brief Enable timer 3 compare match interrupt */
#define TIM3_ovf_enable() TIMSK3 |= (1<<OCIE3A);

/** @brief Disable interrupts by Timer 3*/
#define TIM3_ovf_disable() TIMSK3 &= ~(1<<OCIE3A);
#endif