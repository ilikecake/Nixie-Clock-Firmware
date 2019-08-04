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
*	\brief		TCA9538 IO expander hardware functions
*	\author		Pat Satyshur
*	\version	1.0
*	\date		8/3/2019
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	hardware
*
*	@{
*/

#include "main.h"

void TCA9538_Reset (uint8_t TCA9538_num, uint8_t reset)
{

	return;	
}

uint8_t TCA9538_ReadReg (uint8_t TCA9538_num, uint8_t RegisterToRead, uint8_t *RegVal)
{
	//uint8_t ReturnCode;
	//Make sure the register is valid
	if((RegisterToRead >= 0) && (RegisterToRead <= 3))
	{
		if ((TCA9538_num == 1) && (TCA9538_SLA_ADDR_1 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_1, &RegisterToRead, RegVal, 1, 1));
		}
		else if ((TCA9538_num == 2) && (TCA9538_SLA_ADDR_2 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_2, &RegisterToRead, RegVal, 1, 1));
		}
		else if ((TCA9538_num == 3) && (TCA9538_SLA_ADDR_3 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_3, &RegisterToRead, RegVal, 1, 1));
		}
		else if ((TCA9538_num == 4) && (TCA9538_SLA_ADDR_4 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_4, &RegisterToRead, RegVal, 1, 1));
		}
	}

	//Invalid register to read or invalid TCA9538 number
	return 0xFF;	//TODO: is this a valid return code?
}

uint8_t TCA9538_WriteReg (uint8_t TCA9538_num, uint8_t RegisterToWrite, uint8_t RegVal)
{
	uint8_t BytesToSend[2];
	
	//uint8_t ReturnCode;
	//Make sure the register is valid
	if((RegisterToWrite >= 0) && (RegisterToWrite <= 3))
	{
		BytesToSend[0] = RegisterToWrite;
		BytesToSend[1] = RegVal;
		
		if ((TCA9538_num == 1) && (TCA9538_SLA_ADDR_1 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_1, BytesToSend, NULL, 2, 0));
		}
		else if ((TCA9538_num == 2) && (TCA9538_SLA_ADDR_2 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_2, BytesToSend, NULL, 2, 0));
		}
		else if ((TCA9538_num == 3) && (TCA9538_SLA_ADDR_3 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_3, BytesToSend, NULL, 2, 0));
		}
		else if ((TCA9538_num == 4) && (TCA9538_SLA_ADDR_4 > 0))
		{
			return(TWIRW(TCA9538_SLA_ADDR_4, BytesToSend, NULL, 2, 0));
		}
	}
	
	return 0xFF;	//TODO: is this a valid return code?
}