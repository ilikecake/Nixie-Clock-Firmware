/*   This file is part of Nixie Clock Firmware
*
*	 Nixie Clock Firmware is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/** \file
*	\brief		Header for TCA9538 IO expander hardware functions
*	\author		Pat Satyshur
*	\version	1.0
*	\date		8/3/2019
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	hardware
*
*	@{
*/

#ifndef _TCA9538_H_
#define _TCA9538_H_

//TODO: Should I move this to a hardware.c file?
#define TCA9538_RESET_PORT		PORTF
#define TCA9538_RESET_PIN_NUM	6
#define TCA9538_RESET_DDR		DDRF
#define TCA9538_RESET_PIN		PINF

//Slave addresses of TCA9538 devices. Set to zero to disable.
#define TCA9538_SLA_ADDR_1		0x70
#define TCA9538_SLA_ADDR_2		0
#define TCA9538_SLA_ADDR_3		0
#define TCA9538_SLA_ADDR_4		0

#define TCA9538_REG_INPUT		0x00
#define TCA9538_REG_OUTPUT		0x01
#define TCA9538_REG_POL_INV		0x02
#define TCA9538_REG_CONFIG		0x03

void TCA9538_Reset (uint8_t TCA9538_num, uint8_t reset);
uint8_t TCA9538_ReadReg (uint8_t TCA9538_num, uint8_t RegisterToRead, uint8_t *RegVal);
uint8_t TCA9538_WriteReg (uint8_t TCA9538_num, uint8_t RegisterToWrite, uint8_t RegVal);



#endif