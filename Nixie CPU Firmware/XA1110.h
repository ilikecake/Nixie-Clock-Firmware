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
*	\brief		Header file for XA1110 GPS functions
*	\author		Pat Satyshur
*	\version	1.0
*	\date		2/17/2019
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	hardware
*
*	@{
*/

#ifndef XA1110_H_
#define XA1110_H_

#define XA1110_SLA_ADDR		0x10

//TODO: Make a function that only gets the RMC string?
uint8_t GPS_GetPacket( char *DataString);
uint8_t GPS_GetStrings(char *GPGGA_STRING, char *GPRMC_STRING);

uint8_t GPS_ValidateRMC(char *GPRMC_STRING, struct tm *RMC_Time, int32_t *LatInSeconds, int32_t *LongInSeconds);



#endif /* XA1110_H_ */