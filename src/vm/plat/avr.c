/*
 * PyMite - A flyweight Python interpreter for 8-bit microcontrollers and more.
 * Copyright 2006 Dean Hall
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#undef __FILE_ID__
#define __FILE_ID__ 0x51
/**
 * PyMite platform-specific routines for AVR target
 *
 * Log
 * ---
 *
 * 2007/01/10   Added time tick service for desktop (POSIX) and AVR. (P.Adelt)
 * 2006/12/26   #65: Create plat module with put and get routines
 */


#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "../pm.h"

/***************************************************************
 * Defines
 **************************************************************/
 
/***************************************************************
 * Globals
 **************************************************************/

volatile uint32_t plat_ticks = 0;
volatile uint8_t plat_switchThreads = 0;

/***************************************************************
 * Functions
 **************************************************************/


/*
 * AVR target shall use stdio for I/O routines.
 * The UART or USART must be configured for the interactive interface to work.
 */
PmReturn_t
plat_init(void)
{
    /* PORT BEGIN: Set these UART/USART SFRs properly for your AVR */

    /* Set the baud rate register */
    UBRR = (F_CPU / (16UL * UART_BAUD)) - 1;

    /* Enable the transmit and receive pins */
    UCR = _BV(TXEN) | _BV(RXEN);
    /* PORT END */
    
    /* PORT BEGIN: Configure a timer that fits your needs. */
    /* Use T/C0 in synchronous mode, aim for a tick rate of
     * several hundred Hz */
    #if (TARGET_MCU == atmega103) || (TARGET_MCU == atmega128)
    /* set T/C0 to use synchronous clock */
    ASSR &= ~(1<<AS0);
    /* set prescaler to /8 */
    TCCR0 &= ~0x07;
    TCCR0 |= (1<<CS01);
    #else
    #error No timer configuration is known for this AVR.
    #endif 
    /* PORT END */

    return PM_RET_OK;
}

ISR(TIMER0_OVF_vect) 
{
	plat_ticks++;
	if (plat_ticks % THREAD_SWITCH_TICKS == 0) {
		plat_switchThreads = 1;
	}
}


/*
 * UART receive char routine MUST return exactly and only the received char;
 * it should not translate \n to \r\n.
 * This is because the interactive interface uses binary transfers.
 */
PmReturn_t
plat_getByte(uint8_t *b)
{
    PmReturn_t retval = PM_RET_OK;

    /* PORT BEGIN: Set these UART/USART SFRs properly for your AVR */
    /* Loop until serial receive is complete */
    loop_until_bit_is_set(USR, RXC);
    
    /* If a framing error or data overrun occur, raise an IOException */
    if (UCR & (_BV(FE) | _BV(DOR)))
    {
        PM_RAISE(retval, PM_RET_EX_IO);
        return retval;
    }
    *b = UDR;
    /* PORT END */

    return retval;
}


/*
 * UART send char routine MUST send exactly and only the given char;
 * it should not translate \n to \r\n.
 * This is because the interactive interface uses binary transfers.
 */
PmReturn_t
plat_putByte(uint8_t b)
{
    /* PORT BEGIN: Set these UART/USART SFRs properly for your AVR */
    /* Loop until serial data reg is empty (from previous transfer) */
    loop_until_bit_is_set(USR, UDRE);
    
    /* Put the byte to send into the serial data register */
    UDR = b;
    /* PORT END */

    return PM_RET_OK;
}

/* remember that 32bit-accesses are non-atomic on AVR */
uint32_t
plat_getTicks(void)
{
	uint32_t result;
	cli();
	result = plat_ticks;
	sei();
	return result;
}