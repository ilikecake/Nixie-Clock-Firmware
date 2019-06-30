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
*	\brief		Basic UART driver header for Atmel AVR8 CPUs
*	\author		Pat Satyshur
*	\version	1.0
*	\date		1/21/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	common
*
*	@{
*/
#ifndef _MAIN_H_
#define _MAIN_H_

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <time.h>


#include "DigitDefs.h"
#include "command.h"
#include "twi.h"
#include "UART.h"
#include "TCA9538.h"
#include "ds3231m.h"
#include "XA1110.h"

//#define TIME_ZONE_OFFSET					-6

//Make this internal to main?
//extern uint8_t EEMEM NV_TimeZone;




//System status defines. A one in the corresponding bit indicates the system module is physically present and detected by the CPU.
//TODO: Change this to subsystem status, have a system status byte that indicates that the system is ready or initializing??
#define SYSTEM_STATUS_RTC_PRESENT				0x01
#define SYSTEM_STATUS_BUTTON_BOARD_PRESENT		0x02
#define SYSTEM_STATUS_GPS_PRESENT				0x04
//#define SYSTEM_STATUS_GPS_LOCK					0x08	//TODO: remove this...
#define SYSTEM_STATUS_RTC_VALID					0x10

//Clock status defines
#define CLOCK_STATUS_INITIALIZING			0x00
#define CLOCK_STATUS_NORMAL					0x01
#define CLOCK_STATUS_DEBUG					0x02
#define CLOCK_STATUS_SETING					0x03
#define CLOCK_STATUS_STANDBY				0x04
#define CLOCK_STATUS_DEMO					0x05

//Clock display options
#define TIME_MODE_24_HOURS					0x01
#define TIME_MODE_AMPM						0x00
#define CLOCK_SHOWN_LEADING_ZERO			0x02
#define CLOCK_HIDE_LEADING_ZERO				0x00



#define HV_PWR_NUM			0x01
#define GPS_PWR_NUM			0x02
#define IOEXP_RESET_NUM		0x04
//TODO: Make some sort of OFFVAL register to indicate if high is on or off?

#define PWR_STATUS_HIGH		0x01
#define PWR_STATUS_LOW		0x00
#define PWR_STATUS_ERROR	0x02
#define PWR_STATUS_NOSUB	0xFF

#define GPS_TWI_ADDRESS		0x50	//TODO: look this up later...

//TODO: Move these to a different file?
void BlankDigit(uint8_t DigitToBlank);
void SetDigit(uint8_t Digit, uint8_t NumberToDisplay);
void SetDP(uint8_t DPLocation, uint8_t DPState);

void HVPower(uint8_t PWRState);	//TODO: Combine these functions for power switching?
void GPSPower(uint8_t GPSState);
void IOExpReset(uint8_t ResetState);	//TODO: Set up the IO Expander when the reset is off?
void LED(uint8_t LEDNumber, uint8_t state);
uint8_t PWRState(uint8_t SystemToQuery);

uint8_t GetSystemStatus(void);
uint8_t GetClockStatus(void);

void SetSystemStatus(uint8_t StatusToSet);
void SetClockStatus(uint8_t StatusToSet);

void UpdateDisplay(void);

void SetDimming(uint16_t DimVal);
uint8_t GetDigitStatus(uint8_t DigitToQuery);

uint8_t SetSystemTime(struct tm *TimeToSet, int8_t TimeZoneToSet, int32_t LatInSeconds, int32_t LongInSeconds);

int8_t GetTimeZone(void);
int32_t GetLat(void);
int32_t GetLong(void);
void SetTimeZone(int8_t TimeZoneToSet);
void SetLat(int32_t LatToSet);
void SetLong(int32_t LongToSet);

int8_t GetTimeFromGPS(void);

//uint8_t GetGPSTime(struct tm *TimeToSet, int32_t LatInSeconds, int32_t LongInSeconds);


#endif