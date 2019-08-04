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
*	\brief		Command interpreter application specific functions
*	\author		Pat Satyshur
*	\version	1.0
*	\date		8/3/2019
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	nixie_clock_main
*
*	@{
*/
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

//Command setup
#define COMMAND_PROMPT	">"							//Define the character(s) to use for the command prompt
#define COMMAND_USE_ARROWS							//If this is defined, arrow keys will be identified by the command interpreter
#undef COMMAND_EX_COMMAND_IN_INPUT					//If this is defined, the command will be executed in the CommandGetInput function. If not, the RunCommand function must be called to run the command. Further characters recieved on CommandGetInput will be ignored untill RunCommand is complete.

#define MAX_COMMAND_DESCRIPTION_LENGTH		32		//The maximum length of the description and help strings
#define MAX_COMMAND_LENGTH 					10		//The maximum length of the command string
#define MAX_ARGS							6		//The maximum number of arguments allowed

#define CLI_Size 	(MAX_COMMAND_DESCRIPTION_LENGTH*2+MAX_COMMAND_LENGTH*1+6)	//Size (in bytes) of the CommandListItem struct

#include <stdint.h>
#include "command.h"
extern const uint8_t NumCommands;
extern const CommandListItem AppCommandList[];

#endif

/** @} */