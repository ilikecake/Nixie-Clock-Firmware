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

#include "main.h"
//#include "commands.h"


//The number of commands
const uint8_t NumCommands = 13;

//Handler function declerations

//LED control function
static int _F1_Handler (void);
const char _F1_NAME[] PROGMEM 			= "led";
const char _F1_DESCRIPTION[] PROGMEM 	= "Turn LED on or off";
const char _F1_HELPTEXT[] PROGMEM 		= "led <number>";

//Jump to DFU bootloader
static int _F2_Handler (void);
const char _F2_NAME[] PROGMEM 			= "dfu";
const char _F2_DESCRIPTION[] PROGMEM 	= "Jump to bootloader";
const char _F2_HELPTEXT[] PROGMEM 		= "'dfu' has no parameters";

//Get time from RTC
static int _F3_Handler (void);
const char _F3_NAME[] PROGMEM 			= "gettime";
const char _F3_DESCRIPTION[] PROGMEM 	= "Show the current date and time";
const char _F3_HELPTEXT[] PROGMEM 		= "'gettime' has no parameters";

//Set time on RTC
static int _F4_Handler (void);
const char _F4_NAME[] PROGMEM 			= "settime";
const char _F4_DESCRIPTION[] PROGMEM 	= "Set the time";
const char _F4_HELPTEXT[] PROGMEM 		= "settime <month> <day> <year> <hr> <min> <sec> <dow>";

//Read a register from the ADC
static int _F5_Handler (void);
const char _F5_NAME[] PROGMEM 			= "digit";
const char _F5_DESCRIPTION[] PROGMEM 	= "Manually set a digit";
const char _F5_HELPTEXT[] PROGMEM 		= "digit <digit number> <number to display>";

//Write a register to the ADC
static int _F6_Handler (void);
const char _F6_NAME[] PROGMEM 			= "ioexp";
const char _F6_DESCRIPTION[] PROGMEM 	= "Read or write to the IO expander";
const char _F6_HELPTEXT[] PROGMEM 		= "ioexp <reg> <value>";

//Show internal CPU setup stuff
static int _F7_Handler (void);
const char _F7_NAME[] PROGMEM 			= "iostate";
const char _F7_DESCRIPTION[] PROGMEM 	= "Show the state of the I/O registers";
const char _F7_HELPTEXT[] PROGMEM 		= "'iostate' has no parameters";

//Test the buzzer
static int _F8_Handler (void);
const char _F8_NAME[] PROGMEM 			= "pwr";
const char _F8_DESCRIPTION[] PROGMEM 	= "Control power to subsystems";
const char _F8_HELPTEXT[] PROGMEM 		= "pwr <sub> <state>";

//Turn the relay on or off
static int _F9_Handler (void);
const char _F9_NAME[] PROGMEM 			= "rtc";
const char _F9_DESCRIPTION[] PROGMEM 	= "Control the RTC";
const char _F9_HELPTEXT[] PROGMEM 		= "rtc <cmd> <data>";

//Manual calibration of the ADC
static int _F10_Handler (void);
const char _F10_NAME[] PROGMEM 			= "sys";
const char _F10_DESCRIPTION[] PROGMEM 	= "Calibrate the ADC";
const char _F10_HELPTEXT[] PROGMEM 		= "'cal' has no parameters";

//Get temperatures from the ADC
static int _F11_Handler (void);
const char _F11_NAME[] PROGMEM 			= "gps";
const char _F11_DESCRIPTION[] PROGMEM 	= "GPS functions";
const char _F11_HELPTEXT[] PROGMEM 		= "gps <1> <2>";

//Scan the TWI bus for devices
static int _F12_Handler (void);
const char _F12_NAME[] PROGMEM 			= "twiscan";
const char _F12_DESCRIPTION[] PROGMEM 	= "Scan for TWI devices";
const char _F12_HELPTEXT[] PROGMEM 		= "'twiscan' has no parameters";

//Dataflash functions
static int _F13_Handler (void);
const char _F13_NAME[] PROGMEM 			= "mem";
const char _F13_DESCRIPTION[] PROGMEM 	= "dataflash functions";
const char _F13_HELPTEXT[] PROGMEM 		= "mem <1> <2> <3>";

//Command list
const CommandListItem AppCommandList[] PROGMEM =
{
	{ _F1_NAME,		1,  2,	_F1_Handler,	_F1_DESCRIPTION,	_F1_HELPTEXT	},		//led
	{ _F2_NAME, 	0,  0,	_F2_Handler,	_F2_DESCRIPTION,	_F2_HELPTEXT	},		//dfu
	{ _F3_NAME, 	0,  0,	_F3_Handler,	_F3_DESCRIPTION,	_F3_HELPTEXT	},		//gettime
	{ _F4_NAME, 	7,  7,	_F4_Handler,	_F4_DESCRIPTION,	_F4_HELPTEXT	},		//settime
	{ _F5_NAME, 	1,  2,	_F5_Handler,	_F5_DESCRIPTION,	_F5_HELPTEXT	},		//digit
	{ _F6_NAME, 	1,  2,	_F6_Handler,	_F6_DESCRIPTION,	_F6_HELPTEXT	},		//ioexp
	{ _F7_NAME, 	0,  0,	_F7_Handler,	_F7_DESCRIPTION,	_F7_HELPTEXT	},		//iostate
	{ _F8_NAME,		0,  2,	_F8_Handler,	_F8_DESCRIPTION,	_F8_HELPTEXT	},		//pwr
	{ _F9_NAME,		1,  2,	_F9_Handler,	_F9_DESCRIPTION,	_F9_HELPTEXT	},		//rtc
	{ _F10_NAME,	0,  3,	_F10_Handler,	_F10_DESCRIPTION,	_F10_HELPTEXT	},		//sys
	{ _F11_NAME,	0,  0,	_F11_Handler,	_F11_DESCRIPTION,	_F11_HELPTEXT	},		//gps
	{ _F12_NAME,	0,  1,	_F12_Handler,	_F12_DESCRIPTION,	_F12_HELPTEXT	},		//twiscan
	{ _F13_NAME,	1,  3,	_F13_Handler,	_F13_DESCRIPTION,	_F13_HELPTEXT	},		//mem
};

//Command functions

//LED control function
static int _F1_Handler (void)
{
	uint8_t LEDNumber;
	uint8_t LEDState;
	
	LEDNumber = argAsInt(1);
	
	if( NumberOfArguments() == 1 )
	{
		//Turn on the specified LED, will also work for all off
		LED(LEDNumber, 1);
	}
	else
	{
		LEDState = argAsInt(2);
		LED(LEDNumber, LEDState);
	}
	return 0;
}

//Jump to DFU bootloader
static int _F2_Handler (void)
{
	printf_P(PSTR("Jumping to bootloader. A manual reset will be required\nPress 'y' to continue..."));
	//
	//if(WaitForAnyKey() == 'y')
	//{
	//printf_P(PSTR("Jump\n"));
	//DelayMS(100);
	//Jump_To_Bootloader();
	//}
	//
	//printf_P(PSTR("Canceled\n"));
	return 0;
}

//Get time from RTC
static int _F3_Handler (void)
{

	//TimeAndDate CurrentTime;
	//DS3232M_GetTime(&CurrentTime);
	/*if(DS1390GetTime(&CurrentTime) != 0)
	{
	printf_P(PSTR("Error\n"));
	return 0;
	}*/
	
	//printf_P(PSTR("%02d/%02d/20%02d %02d:%02d:%02d\n"), CurrentTime.month, CurrentTime.day, CurrentTime.year, CurrentTime.hour, CurrentTime.min, CurrentTime.sec);
	return 0;
}

//Set time on RTC
static int _F4_Handler (void)
{
	//TimeAndDate CurrentTime;
	
	////CurrentTime.month	= argAsInt(1);
	//CurrentTime.day		= argAsInt(2);
	//CurrentTime.year	= argAsInt(3);
	//CurrentTime.hour	= argAsInt(4);
	//CurrentTime.min		= argAsInt(5);
	//CurrentTime.sec		= argAsInt(6);
	//CurrentTime.dow		= argAsInt(7);
	
	//DS3232M_SetTime(&CurrentTime);
	/*if(DS1390SetTime(&CurrentTime) != 0)
	{
	printf_P(PSTR("Error\n"));
	return 0;
	}*/
	
	printf_P(PSTR("Done\n"));
	return 0;
}

//Manually set digit display
static int _F5_Handler (void)
{
	if( NumberOfArguments() == 1 )
	{
		if(argAsInt(1) == 0)
		{
			BlankDigit(1);
			BlankDigit(2);
			BlankDigit(3);
			BlankDigit(4);
		}
	}
	else
	{
		if((argAsInt(1) > 0) && (argAsInt(1) < 5))
		{
			SetDigit(argAsInt(1), argAsInt(2));
		}
		else if(argAsInt(1) == 5)
		{
			SetDP(1, argAsInt(2));
		}
		else if(argAsInt(1) == 6)
		{
			SetDP(2, argAsInt(2));
		}
		else if (argAsInt(1) == 7)
		{
			printf_P(PSTR("Dimming Value: %d\n"), argAsInt(2));
			SetDimming(argAsInt(2));
		}
		else if (argAsInt(1) == 8)
		{
			printf_P(PSTR("Display State: %d\n"), argAsInt(2));
			SetClockStatus(argAsInt(2));
		}
		else if (argAsInt(1) == 9)
		{
			printf_P(PSTR("Digit 1: %d\n"), GetDigitStatus(1));
			printf_P(PSTR("Digit 2: %d\n"), GetDigitStatus(2));
			printf_P(PSTR("Digit 3: %d\n"), GetDigitStatus(3));
			printf_P(PSTR("Digit 4: %d\n"), GetDigitStatus(4));

		}
	}
	return 0;
}

//Read or write to the IO expander
//	ioexp <reg> <value>
//TODO: check register valid
static int _F6_Handler (void)
{
	uint8_t RegAddr = argAsInt(1);
	uint8_t RegVal;
	uint8_t TWI_Stat;
	
	if( NumberOfArguments() == 1 )
	{
		//Read a register
		TWI_Stat = TCA9538_ReadReg (1, RegAddr, &RegVal);
		if(TWI_Stat == 0x00)
		{
			//Read succeeded, display result
			printf("Register(0x%02X): 0x%02X\n", RegAddr, RegVal);
		}
		else
		{
			//Something bad happened...
			printf("Read failed with error code 0x%02X\n", TWI_Stat);
		}
	}
	else
	{
		//Write a register
		RegVal = argAsInt(2);
		TWI_Stat = TCA9538_WriteReg (1, RegAddr, RegVal);
		if(TWI_Stat == 0x00)
		{
			//write succeeded
			printf("0x%02X written to register 0x%02X successfully\n", RegVal, RegAddr);
		}
		else
		{
			//Something bad happened...
			printf("Write failed with error code 0x%02X\n", TWI_Stat);
		}
	}

	

	return 0;
}

//iostate
static int _F7_Handler (void)
{
	printf("IO Block   PORT   DDR\n");
	printf("   A       0x%02X   0x%02X\n", PORTA, DDRA);
	printf("   B       0x%02X   0x%02X\n", PORTB, DDRB);
	printf("   C       0x%02X   0x%02X\n", PORTC, DDRC);
	printf("   D       0x%02X   0x%02X\n", PORTD, DDRD);
	printf("   E       0x%02X   0x%02X\n", PORTE, DDRE);
	printf("   F       0x%02X   0x%02X\n", PORTF, DDRF);
	printf("   G       0x%02X   0x%02X\n", PORTG, DDRG);
	
	printf("-------------\n");
	printf("System Status: 0x%02X\n", GetSystemStatus());
	printf("Clock Status: 0x%02X\n", GetClockStatus());
	
	
	return 0;
}

//pwr: Show status of power/reset to the subsystems
static int _F8_Handler (void)
{
	uint8_t SubsystemNumber;
	uint8_t SubsystemState;
	
	if(NumberOfArguments() == 0)
	{
		//Display status of subsystem power and reset
		printf_P(PSTR("Power/Reset Status:\n"));
		printf_P(PSTR("%d:  %d\n"), HV_PWR_NUM, PWRState(HV_PWR_NUM));
		printf_P(PSTR("%d:  %d\n"), GPS_PWR_NUM, PWRState(GPS_PWR_NUM));
		printf_P(PSTR("%d:  %d\n"), IOEXP_RESET_NUM, PWRState(IOEXP_RESET_NUM));
		return 0;
	}
	else if(NumberOfArguments() == 2)
	{
		SubsystemNumber = argAsInt(1);
		SubsystemState = argAsInt(2);
		
		printf_P(PSTR("Set %d:  %d\n"), SubsystemNumber, SubsystemState);
		
		if(SubsystemNumber == HV_PWR_NUM)
		{
			HVPower(SubsystemState);
		}
		else if(SubsystemNumber == GPS_PWR_NUM)
		{
			GPSPower(SubsystemState);
		}
		else if(SubsystemNumber == IOEXP_RESET_NUM)
		{
			IOExpReset(SubsystemState);
		}
		return 0;
	}
	
	printf_P(PSTR("Cannot use only one argument\n"));
	return 0;
}

//Turn the relay on or off
static int _F9_Handler (void)
{
	uint8_t RTC_Cmd;
	//uint8_t RTC_Data;
	uint8_t DataByte;
	int8_t SignedDataByte;
	struct tm TheTime;
	time_t TimeVal;
	time_t TempTime;
	
	struct tm * TheTime2;
	//int32_t ZoneVal;
	
	RTC_Cmd = argAsInt(1);
	
	switch (RTC_Cmd)
	{
		case 1:
		DS3231M_GetStatus();
		break;
		
		case 2:
		printf_P(PSTR("Initialize DS3231M..."));
		DataByte = DS3231M_Init();
		if(DataByte == 0)
		{
			printf_P(PSTR("OK\n"));
		}
		else if(DataByte == 1)
		{
			printf_P(PSTR("OK, Oscillator has stopped, time is invalid\n"));
		}
		else
		{
			printf_P(PSTR("TWI Error\n"));
		}
		break;
		
		case 3:
		printf_P(PSTR("Setting the time..."));
		TheTime.tm_hour = 23;
		TheTime.tm_min = 18;
		TheTime.tm_sec = 00;
		
		TheTime.tm_mon = 1;
		TheTime.tm_mday = 26;
		TheTime.tm_year = 119;
		DS3231M_SetTime(&TheTime);
		printf_P(PSTR("Done\n"));
		break;
		
		case 4:
		printf_P(PSTR("Reading the time\n"));
		DS3231M_GetTime(&TheTime);
		printf_P(PSTR("Time: %02d:%02d:%02d\n"), TheTime.tm_hour, TheTime.tm_min, TheTime.tm_sec);
		printf_P(PSTR("Date: %02d/%02d/20%02d\n"), TheTime.tm_mon, TheTime.tm_mday, (TheTime.tm_year-100));
		if( DS3231M_TimeIsValid() == 1 )
		{
			printf_P(PSTR("RTC time is valid\n"));
		}
		else
		{
			printf_P(PSTR("RTC time is not valid\n"));
		}
		break;
		
		case 5:
		TheTime2->tm_hour = 1;
		TheTime2->tm_isdst = 0;
		TheTime2->tm_mday = 0;
		TheTime2->tm_min = 1;
		TheTime2->tm_mon = 0;
		TheTime2->tm_sec = 1;
		TheTime2->tm_wday = 0;
		TheTime2->tm_yday = 0;
		TheTime2->tm_year = 0;
		
		DS3231M_GetTime(TheTime2);
		
		printf_P(PSTR("Time: %02d:%02d:%02d\n"), TheTime2->tm_hour, TheTime2->tm_min, TheTime2->tm_sec);
		
		//DataByte = TheTime2->tm_hour;
		//printf_P(PSTR("Time: %02d:"), DataByte);
		//DataByte = TheTime2->tm_min;
		//printf_P(PSTR("%02d:"), DataByte);
		//DataByte = TheTime2->tm_sec;
		//printf_P(PSTR("%02d\n"), DataByte);
		
		
		//printf_P(PSTR("Time: %02d:%02d:%02d\n"), (*TheTime2).tm_hour, (*TheTime2).tm_min, (*TheTime2).tm_sec);
		//printf_P(PSTR("Date: %02d/%02d/20%02d\n"), (*TheTime2).tm_mon, (*TheTime2).tm_mday, (*TheTime2).tm_year-100);
		
		//set_zone(-6 * ONE_HOUR);		//Set time zone to CST
		//set_position( 43.066667*ONE_DEGREE,  -89.4*ONE_DEGREE);
		//set_dst(usa_dst);
		//ZoneVal = -6*ONE_HOUR;
		//TimeVal = mktime(TheTime2);
		//TheTime2->tm_isdst = 0;
		
		//printf_P(PSTR("Time: %02d:%02d:%02d\n"), (*TheTime2).tm_hour, (*TheTime2).tm_min, (*TheTime2).tm_sec);
		//printf_P(PSTR("Date: %02d/%02d/20%02d\n"), (*TheTime2).tm_mon, (*TheTime2).tm_mday, (*TheTime2).tm_year-100);
		
		printf_P(PSTR("Time: %02d:%02d:%02d\n"), TheTime2->tm_hour, TheTime2->tm_min, TheTime2->tm_sec);
		printf_P(PSTR("Time: %02d:%02d:%02d\n"), TheTime2->tm_hour, TheTime2->tm_min, TheTime2->tm_sec);
		//set_system_time(mktime(TheTime2));
		//SetClockStatus(CLOCK_STATUS_NORMAL);
		break;
		
		case 6:
		
		//struct tm TheTime;
		//time_t TimeVal;
		//time_t TempTime;
		
		
		TimeVal = time(NULL);
		
		gmtime_r(&TimeVal, &TheTime);
		printf_P(PSTR("GMT System Time: %s\n"), asctime(&TheTime));
		printf_P(PSTR("DST: %d\n"), TheTime.tm_isdst);
		
		localtime_r(&TimeVal, &TheTime);
		printf_P(PSTR("Local System Time: %s\n"), asctime(&TheTime));
		printf_P(PSTR("DST: %d\n"), TheTime.tm_isdst);
		//printf("%s\n", asctime(localtime(&TimeVal)));
		TempTime = sun_rise(&TimeVal);
		printf_P(PSTR("Sunrise Time: %s\n"), asctime(localtime(&TempTime)));
		TempTime = sun_set(&TimeVal);
		printf_P(PSTR("Sunset Time: %s\n"), asctime(localtime(&TempTime)));
		
		
		break;
		
		case 7:
			
			printf_P(PSTR("Saving system time to RTC..."));
			TimeVal = time(NULL);
			gmtime_r(&TimeVal, &TheTime);
			DS3231M_SetTime(&TheTime);
			printf_P(PSTR("Done\nSet system time to %s\n"), asctime(&TheTime));
		break;
		
		case 8:
			if(NumberOfArguments() == 2)
			{
				SignedDataByte = argAsInt(2);
				SetTimeZone(SignedDataByte);
				//eeprom_update_byte (&NV_TimeZone, SignedDataByte);
				printf_P(PSTR("Writing %d to EEPROM\n"), SignedDataByte);
			}
			else
			{
				SignedDataByte = 1;
				SignedDataByte = GetTimeZone();
				printf_P(PSTR("Read %d from EEPROM\n"), SignedDataByte);
			}
		break;
	}
	
	return 0;
}

//Sys
static int _F10_Handler (void)
{
	//char CommandData[10];
	uint8_t DataByte;
	
	int8_t CurrentTimeZone;
	int32_t CurrentLat;
	int32_t CurrentLong;

	CurrentTimeZone = GetTimeZone();
	CurrentLat = GetLat();
	CurrentLong = GetLong();

	
	//printf_P(PSTR("System Status: 0x%02X\n"), GetSystemStatus());
	//printf_P(PSTR("Clock Status: 0x%02X\n"), GetClockStatus());
	
	DataByte = GetSystemStatus();
	
	printf_P(PSTR("System Status:\n"));
	printf_P(PSTR("-----------------------------------\n"));
	if((DataByte & SYSTEM_STATUS_RTC_PRESENT) == SYSTEM_STATUS_RTC_PRESENT)
	{
		printf_P(PSTR("RTC Present		Yes\n"));
	}
	else
	{
		printf_P(PSTR("RTC Present		No\n"));
	}
	
	if((DataByte & SYSTEM_STATUS_RTC_VALID) == SYSTEM_STATUS_RTC_VALID)
	{
		printf_P(PSTR("RTC Valid		Yes\n"));
	}
	else
	{
		printf_P(PSTR("RTC Valid		No\n"));
	}
	
	if((DataByte & SYSTEM_STATUS_GPS_PRESENT) == SYSTEM_STATUS_GPS_PRESENT)
	{
		printf_P(PSTR("GPS Present		Yes\n"));
	}
	else
	{
		printf_P(PSTR("GPS Present		No\n"));
	}
	
	if((DataByte & SYSTEM_STATUS_BUTTON_BOARD_PRESENT) == SYSTEM_STATUS_BUTTON_BOARD_PRESENT)
	{
		printf_P(PSTR("Buttons Present		Yes\n"));
	}
	else
	{
		printf_P(PSTR("Buttons Present		No\n"));
	}
	printf_P(PSTR("Clock Mode		0x%02X\n"), GetClockStatus());
	printf_P(PSTR("-----------------------------------\n"));
	
	
	
	printf_P(PSTR("Settable Values:\n"));
	printf_P(PSTR("-----------------------------------\n"));
	printf_P(PSTR("1: Time Zone		%d\n"), CurrentTimeZone);
	printf_P(PSTR("2: Latitude		%ld\n"), CurrentLat);
	printf_P(PSTR("3: Longitude		%ld\n"), CurrentLong);
	printf_P(PSTR("-----------------------------------\n"));
	
	printf_P(PSTR("Chose a value to update (0 to exit):\n"));
	GetNewCommand();
	//TODO: Why does this keep the system from updating the time? GetNewCommand is probably blocking...
	DataByte = argAsInt(0);
	
	switch(DataByte)
	{
		case 0:
			return 0;
			
		case 1:
			printf_P(PSTR("Enter the new time zone in hours offset from UTC.\n"));
			GetNewCommand();
			CurrentTimeZone = argAsInt(0);
			printf_P(PSTR("Set time zone to %d\n"), CurrentTimeZone);
			SetTimeZone(CurrentTimeZone);
			break;

		case 2:
			printf_P(PSTR("Enter new latitude in arcseconds north\n"));
			GetNewCommand();
			CurrentLat = argAsInt(0);
			printf_P(PSTR("Set latitude to %ld\n"), CurrentLat);
			SetLat(CurrentLat);
			break;
			
		case 3:
			printf_P(PSTR("Enter new longitude in arcseconds east\n"));
			GetNewCommand();
			CurrentLong = argAsInt(0);
			printf_P(PSTR("Set longitude to %ld\n"), CurrentLong);
			SetLong(CurrentLong);
			break;
		
		
		
	}
	
	//argAsChar(0, CommandData);
	//printf("I heard %s\n", CommandData);


	
	
	return 0;
}

/** GPS Function
 *
 * Valid NEMA Strings:
 *  GxGGA: $GNGGA,032451.000,4304.405522,N,08928.339184,W,1,5,2.93,261.671,M,-34.228,M,,*73
 *  GxRMC: $GNRMC,032451.000,A,4304.405522,N,08928.339184,W,0.04,154.46,160319,,,A*61
 *
 * Invalid NEMA Strings:
 *  GxGGA: $GNGGA,170525.049,,,,,0,0,,,M,,M,,*5F
 *  GxRMC: $GNRMC,170525.049,V,,,,,0.00,0.00,160319,,,N*56
 *
 */
static int _F11_Handler (void)
{
	struct tm TimeToSet;
	char GPGGA_STRING[155];
	char GPRMC_STRING[155];
	uint16_t i;
	uint8_t StatusCode = 0;
	//uint16_t LocationInString;
	
	int32_t LatSec;
	int32_t LongSec;

	LatSec = 0;
	LongSec = 0;
	TimeToSet.tm_hour	= 0;
	TimeToSet.tm_min	= 0;
	TimeToSet.tm_sec	= 0;
	TimeToSet.tm_mday	= 0;
	TimeToSet.tm_mon	= 0;
	TimeToSet.tm_year	= 0;
		
	printf("Reading time from GPS....");
	StatusCode = GPS_GetStrings(GPGGA_STRING, GPRMC_STRING);
	printf("Done\n");
		
	if(StatusCode == 0)
	{
		StatusCode = GPS_ValidateRMC(GPRMC_STRING, &TimeToSet, &LatSec, &LongSec);
		if(StatusCode == 0)
		{
			//Set the system time based on the GPS
			printf("Setting system time....");
			SetSystemTime(&TimeToSet, GetTimeZone(), LatSec, LongSec);
			printf("Done\n");
			
			//Set RTC based on GPS
			
		}
		else if(StatusCode == 1)
		{
			printf("No GPS lock\n");
		}
		else if(StatusCode == 2)
		{
			printf("End of the string reached without finding all of the data\n");
		}
		else if(StatusCode == 3)
		{
			printf("String is longer than 150 bytes\n");
		}
		else if(StatusCode == 4)
		{
			printf("Validation Failure: commas are out of place\n");
		}
		else if(StatusCode == 5)
		{
			printf("Validation Failure: Longitude is out of range\n");
		}
		else if(StatusCode == 6)
		{
			printf("Validation Failure: Latitude is out of range\n");
		}
		else if(StatusCode == 7)
		{
			printf("Validation Failure: Time values are out of range\n");
		}
		else
		{
			printf("Unknown validation status: %d\n", StatusCode);
		}
		
		if(StatusCode != 0)
		{
			//The function did not work correctly, output debug info
			//Character map of GPS strings
			printf("GxGGA\n");
			printf("  \t 0  \t 1  \t 2  \t 3  \t 4  \t 5  \t 6  \t 7  \t 8  \t 9\n00\t");
			for(i=0;i<100;i++)
			{
				printf("0x%02X\t", GPGGA_STRING[i]);
				if(((i+1)%10 == 0) && (i != 99))
				{
					printf("\n%02d\t", (i+1)/10);
				}
			}
			printf("\n\n");
			
			printf("GxRMC\n");
			printf("  \t 0  \t 1  \t 2  \t 3  \t 4  \t 5  \t 6  \t 7  \t 8  \t 9\n00\t");
			for(i=0;i<100;i++)
			{
				printf("0x%02X\t", GPRMC_STRING[i]);
				if(((i+1)%10 == 0) && (i != 99))
				{
					printf("\n%02d\t", (i+1)/10);
				}
			}
			printf("\n\n");
			
			
			//Human readable GPS strings
			printf("GxGGA: %s\n", GPGGA_STRING);
			printf("GxRMC: %s\n", GPRMC_STRING);
			
			//Processed string data
			printf("%02d/%02d/20%02d\n", TimeToSet.tm_mon+1, TimeToSet.tm_mday, TimeToSet.tm_year-100);
			printf("%02d:%02d:%02d\n", TimeToSet.tm_hour, TimeToSet.tm_min, TimeToSet.tm_sec);
			printf("Lat: %ld\n", LatSec);
			printf("Long: %ld\n", LongSec);
		}
	}
	else if(StatusCode == 1)
	{
		printf("Buffer Flush Failed\n");
	}
	else if(StatusCode == 2)
	{
		printf("Failed to find strings\n");
	}
	else
	{
		printf("Unknown GPS string return code: %d\n", StatusCode);
	}

	return 0;
}

//Scan the TWI bus for devices
static int _F12_Handler (void)
{
	uint8_t AddressToScan;
	uint8_t RetVal;
	
	if(NumberOfArguments() == 0)
	{
		InitTWI();
		TWIScan();
		DeinitTWI();
	}
	else
	{
		InitTWI();
		AddressToScan = argAsInt(1);
		printf_P(PSTR("Scanning address 0x%02X..."), AddressToScan);
		RetVal = TWIDeviceCheck(AddressToScan);
		if(RetVal == 1)
		{
			printf_P(PSTR("Device found\n"), AddressToScan);
		}
		else
		{
			printf_P(PSTR("Device not found\n"), AddressToScan);
		}
		DeinitTWI();
		
		
	}
	return  0;
}

//Dataflash functions
static int _F13_Handler (void)
{
	//uint8_t arg1 = argAsInt(1);
	//uint8_t arg2;
	//uint8_t arg3;
	//uint8_t ReadBytes[3];
	
	/*switch (arg1)
	{
	case 1:
	arg2 = argAsInt(2);
	if(arg2 == 1)
	{
	printf_P(PSTR("Dataflash selected\n"));
	AT45DB321D_Select();
	}
	else
	{
	printf_P(PSTR("Dataflash deselected\n"));
	AT45DB321D_Deselect();
	}
	break;
	
	case 2:
	arg3 = AT45DB321D_ReadStatus();
	printf_P(PSTR("Stat: 0x%02X\n"), AT45DB321D_ReadStatus());
	break;
	
	case 3:
	printf_P(PSTR("HW Status: 0x%02X\n"), BH_GetStatus(BH_STATUS_HW));
	printf_P(PSTR("HIO Status: 0x%02X\n"), BH_GetStatus(BH_STATUS_HIO));
	printf_P(PSTR("Prog Status: 0x%02X\n"), BH_GetStatus(BH_STATUS_PROG));
	break;
	
	case 4:
	AT45DB321D_ReadID(ReadBytes);
	printf_P(PSTR("ID1: 0x%02X\n"), ReadBytes[0]);
	printf_P(PSTR("ID2: 0x%02X\n"), ReadBytes[1]);
	printf_P(PSTR("ID3: 0x%02X\n"), ReadBytes[2]);
	break;
	
	case 5:
	DS3232M_GetStatus();
	break;
	
	case 6:
	arg2 = argAsInt(2);
	if(arg2 == 1)
	{
	StartTemperatureController(0);
	printf_P(PSTR("Controller on\n"));
	}
	else
	{
	StopTemperatureController(0);
	printf_P(PSTR("Controller off\n"));
	}
	break;
	
	}*/
	
	return  0;
}

/** @} */