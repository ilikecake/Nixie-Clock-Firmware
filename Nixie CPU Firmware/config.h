/*   This program is free software: you can redistribute it and/or modify
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
*	\brief		Project specific config file
*	\author		Pat Satyshur
*	\version	1.0
*	\date		1/2/2019
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	common
*
*	@{
*/

#define F_CPU 16000000UL						//16 MHz

#define BACK_BOARD_PRESENT	1					//Set to 0 if the backer board is not present. PB7 (and possibly others) have different functions if the backer board is present or absent. Make sure to correctly set the DIP switch on the CPU board as well.

//Set up TWI module
#define TWI_USER_CONFIG							//Define this in your user code to disable the above error.
#undef TWI_USE_ISR								//Define this to enable interrupt driven TWI interface (this code does not work).
#define _TWI_DEBUG								//Define this to enable the output of debug messages.
#define TWI_USE_INTERNAL_PULLUPS				//Define this to use the internal pull-up resistors of the device.
#define TWI_SCL_FREQ_HZ				100000		//The SCL frequency in Hz (100000 is a good value)

//Set up UART module
#define UART_USER_CONFIG				//Must be set to enable the UART module
#define UART_NUMBER				1		//For MCUs with more than one UART, define the UART to be used here
#define UART_BAUD				9600	//UART baud rate
#undef  UART_ENABLE_LOOPBACK			//Define this to enable an interrupt based loopback (for testing, generally)

//Set up Command command interpreter module
#define COMMAND_USER_CONFIG								//Define this in your user code to disable the above error.
#define COMMAND_STAT_SHOW_COMPILE_STRING		1		//Set to 1 to output the compile date/time string in the stat function					
#define COMMAND_STAT_SHOW_MEM_USAGE				0		//Set to 1 to show the memory usage in the stat function. NOTE: if this is enabled, the mem_usage.c must be included in the makefile

//Based on the setup above
#if COMMAND_STAT_SHOW_COMPILE_STRING == 1
extern const char fwCompileDate[];				//The compile date/time string. This must be a string in flash called fwCompileDate.
#endif
 
#if COMMAND_STAT_SHOW_MEM_USAGE == 1
#include "mem_usage.h"									//The header that contains StackCount()
#endif

