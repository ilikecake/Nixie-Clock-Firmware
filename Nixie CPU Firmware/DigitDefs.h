/*   This file is part of Nixie Clock Firmware
 *	 
 *	Nixie Clock Firmware is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */ 

/** \file
*	\brief		Definitions for the digits on the Nixie Tube Board
*	\author		Pat Satyshur
*	\version	1.0
*	\date		8/13/2018
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	nixie_clock_main
*
*	@{
*/

//Digit 4 is on the far left of the board, digit 1 is on the far right

#ifndef DIGITDEFS_H_
#define DIGITDEFS_H_

//Digit 1:
#define DIGIT_1_0_PORT		PORTE
#define DIGIT_1_0_NUM		4
#define DIGIT_1_0_DDR		DDRE

#define DIGIT_1_1_PORT		PORTE
#define DIGIT_1_1_NUM		3
#define DIGIT_1_1_DDR		DDRE

#define DIGIT_1_2_PORT		PORTE
#define DIGIT_1_2_NUM		2
#define DIGIT_1_2_DDR		DDRE

#define DIGIT_1_3_PORT		PORTE
#define DIGIT_1_3_NUM		1
#define DIGIT_1_3_DDR		DDRE

#define DIGIT_1_4_PORT		PORTB
#define DIGIT_1_4_NUM		3
#define DIGIT_1_4_DDR		DDRB

#define DIGIT_1_5_PORT		PORTB
#define DIGIT_1_5_NUM		2
#define DIGIT_1_5_DDR		DDRB

#define DIGIT_1_6_PORT		PORTB
#define DIGIT_1_6_NUM		1
#define DIGIT_1_6_DDR		DDRB

#define DIGIT_1_7_PORT		PORTE
#define DIGIT_1_7_NUM		7
#define DIGIT_1_7_DDR		DDRE

#define DIGIT_1_8_PORT		PORTB
#define DIGIT_1_8_NUM		0
#define DIGIT_1_8_DDR		DDRB

#define DIGIT_1_9_PORT		PORTE
#define DIGIT_1_9_NUM		5
#define DIGIT_1_9_DDR		DDRE

//Digit 2:
#define DIGIT_2_0_PORT		PORTG
#define DIGIT_2_0_NUM		5
#define DIGIT_2_0_DDR		DDRG

#define DIGIT_2_1_PORT		PORTF
#define DIGIT_2_1_NUM		2
#define DIGIT_2_1_DDR		DDRF

#define DIGIT_2_2_PORT		PORTF
#define DIGIT_2_2_NUM		0
#define DIGIT_2_2_DDR		DDRF

#define DIGIT_2_3_PORT		PORTF
#define DIGIT_2_3_NUM		1
#define DIGIT_2_3_DDR		DDRF

#define DIGIT_2_4_PORT		PORTG
#define DIGIT_2_4_NUM		3
#define DIGIT_2_4_DDR		DDRG

#define DIGIT_2_5_PORT		PORTG
#define DIGIT_2_5_NUM		4
#define DIGIT_2_5_DDR		DDRG

#define DIGIT_2_6_PORT		PORTB
#define DIGIT_2_6_NUM		6
#define DIGIT_2_6_DDR		DDRB

#define DIGIT_2_7_PORT		PORTB
#define DIGIT_2_7_NUM		4
#define DIGIT_2_7_DDR		DDRB

#define DIGIT_2_8_PORT		PORTB
#define DIGIT_2_8_NUM		5
#define DIGIT_2_8_DDR		DDRB

#define DIGIT_2_9_PORT		PORTE
#define DIGIT_2_9_NUM		0
#define DIGIT_2_9_DDR		DDRE

//Digit 3:
#define DIGIT_3_0_PORT		PORTA
#define DIGIT_3_0_NUM		2
#define DIGIT_3_0_DDR		DDRA

#define DIGIT_3_1_PORT		PORTA
#define DIGIT_3_1_NUM		3
#define DIGIT_3_1_DDR		DDRA

#define DIGIT_3_2_PORT		PORTA
#define DIGIT_3_2_NUM		4
#define DIGIT_3_2_DDR		DDRA

#define DIGIT_3_3_PORT		PORTA
#define DIGIT_3_3_NUM		5
#define DIGIT_3_3_DDR		DDRA

#define DIGIT_3_4_PORT		PORTC
#define DIGIT_3_4_NUM		0
#define DIGIT_3_4_DDR		DDRC

#define DIGIT_3_5_PORT		PORTG
#define DIGIT_3_5_NUM		1
#define DIGIT_3_5_DDR		DDRG

#define DIGIT_3_6_PORT		PORTG
#define DIGIT_3_6_NUM		0
#define DIGIT_3_6_DDR		DDRG

#define DIGIT_3_7_PORT		PORTD
#define DIGIT_3_7_NUM		6
#define DIGIT_3_7_DDR		DDRD

#define DIGIT_3_8_PORT		PORTD
#define DIGIT_3_8_NUM		7
#define DIGIT_3_8_DDR		DDRD

#define DIGIT_3_9_PORT		PORTA
#define DIGIT_3_9_NUM		1
#define DIGIT_3_9_DDR		DDRA

//Digit 4:
#define DIGIT_4_0_PORT		PORTA
#define DIGIT_4_0_NUM		7
#define DIGIT_4_0_DDR		DDRA

#define DIGIT_4_1_PORT		PORTC
#define DIGIT_4_1_NUM		6
#define DIGIT_4_1_DDR		DDRC

#define DIGIT_4_2_PORT		PORTG
#define DIGIT_4_2_NUM		2
#define DIGIT_4_2_DDR		DDRG

#define DIGIT_4_3_PORT		PORTC
#define DIGIT_4_3_NUM		7
#define DIGIT_4_3_DDR		DDRC

#define DIGIT_4_4_PORT		PORTC
#define DIGIT_4_4_NUM		4
#define DIGIT_4_4_DDR		DDRC

#define DIGIT_4_5_PORT		PORTC
#define DIGIT_4_5_NUM		5
#define DIGIT_4_5_DDR		DDRC

#define DIGIT_4_6_PORT		PORTC
#define DIGIT_4_6_NUM		3
#define DIGIT_4_6_DDR		DDRC

#define DIGIT_4_7_PORT		PORTC
#define DIGIT_4_7_NUM		1
#define DIGIT_4_7_DDR		DDRC

#define DIGIT_4_8_PORT		PORTC
#define DIGIT_4_8_NUM		2
#define DIGIT_4_8_DDR		DDRC

#define DIGIT_4_9_PORT		PORTA
#define DIGIT_4_9_NUM		6
#define DIGIT_4_9_DDR		DDRA

//Decimal Points
#define DP_TOP_PORT			PORTD
#define DP_TOP_NUM			5
#define DP_TOP_DDR			DDRD

#define DP_BOTTOM_PORT		PORTA
#define DP_BOTTOM_NUM		0
#define DP_BOTTOM_DDR		DDRA

#endif /* DIGITDEFS_H_ */

/** @} */