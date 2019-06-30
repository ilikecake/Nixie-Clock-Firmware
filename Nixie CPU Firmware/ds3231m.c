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
*	\brief		Hardware driver for the DS3231M RTC chip.
*	\author		Pat Satyshur
*	\version	1.0
*	\date		1/14/2013
*	\copyright	Copyright 2013, Pat Satyshur
*	\ingroup 	hardware
*
*	\defgroup	hardware Hardware Drivers
*
*	@{
*/

#include "main.h"

//TODO: Add I2C status checking

//Initalize the DS3231M
//TODO: Make this function check the osc stopped bit and return something different if the oscillator has been stopped. (needed for restart)
uint8_t DS3231M_Init( void )
{
	uint8_t RecieveData;
	uint8_t SendData[2];
	uint8_t stat;

	//Read the status register
	//If the OSF bit (bit 7) is 1, the oscillator has stopped since last initialization.
	SendData[0] = DS3231M_REG_STATUS;
	stat = TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 1, 1);
	TWI_CHECKSTAT(stat);

	//Set up control register
	// -INT/SQW pin set to interrupt on alarms, Alarm interrupts disabled
	SendData[0] = DS3231M_REG_CONTROL;
	SendData[1] = 0x04;
	stat = TWIRW(DS3231M_SLA_ADDRESS, SendData, NULL, 2, 0);
	TWI_CHECKSTAT(stat);
	
	//Disable pending interrupts
	DS3231M_DisableAlarm(1);
	DS3231M_DisableAlarm(2);
	
	if((RecieveData & 0x80) == 0x80)
	{
		//If the oscillator has been stopped since the last init
		return 0x01;
	}
	return 0x00;
}

void DS3231M_GetStatus( void )
{
	uint8_t RecieveData[2];
	uint8_t SendData = DS3231M_REG_CONTROL;
	
	TWIRW(DS3231M_SLA_ADDRESS, &SendData, RecieveData, 1, 2);
	printf("Control: 0x%02X\n", RecieveData[0]);
	printf("Status: 0x%02X\n", (RecieveData[1] & DS3231M_CONTROL_MASK) );
	
	return;
}

void DS3231M_SetTime(struct tm *TheTime)
{
	uint8_t stat;
	uint8_t RecieveData;
	uint8_t SendData[8];
	
	SendData[0] = DS3231M_REG_SEC;
	SendData[1] = ((TheTime->tm_sec % 10) | ((TheTime->tm_sec / 10) << 4));
	SendData[2] = ((TheTime->tm_min % 10) | ((TheTime->tm_min / 10) << 4));
	SendData[3] = ((((TheTime->tm_hour % 10) | ((TheTime->tm_hour / 10) << 4))) & 0x3F);		//Note: this sets the RTC in 24 hour mode
	SendData[4] = TheTime->tm_wday;
	SendData[5] = ((TheTime->tm_mday % 10) | ((TheTime->tm_mday / 10) << 4));
	SendData[6] = (((TheTime->tm_mon % 10) | ((TheTime->tm_mon / 10) << 4)))+1;						//NOTE: this probably clears the century bit. maybe look at this later. Months are stored on the RTC as 1-12, but are stored in the tm struct as 0-11
	SendData[7] = (((TheTime->tm_year - 100) % 10) | (((TheTime->tm_year - 100) / 10) << 4));

	stat = TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 8, 0);
	//TWI_CHECKSTAT(stat);
	
	//Clear the OSC stopped bit
	SendData[0] = DS3231M_REG_STATUS;
	stat = TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 1, 1);
	//TWI_CHECKSTAT(stat);
	
	SendData[0] = DS3231M_REG_STATUS;
	SendData[1] = RecieveData & 0x7F;
	stat = TWIRW(DS3231M_SLA_ADDRESS, SendData, NULL, 2, 0);
	//TWI_CHECKSTAT(stat);

	return;
}

/*
tm struct fields:
int8_t 		tm_sec
int8_t 		tm_min
int8_t 		tm_hour
int8_t 		tm_mday
int8_t 		tm_wday
int8_t 		tm_mon
int16_t 	tm_year
int16_t 	tm_yday
int16_t 	tm_isdst
*/

void DS3231M_GetTime(struct tm *TheTime)
{
	uint8_t RecieveData[7];
	uint8_t SendData;
	//uint8_t i;

	SendData = DS3231M_REG_SEC;

	//TODO: Check OSC stopped bit

	if(TWIRW(DS3231M_SLA_ADDRESS, &SendData, RecieveData, 1, 7) == 0)
	{
		//printf("Data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X ", RecieveData[0], RecieveData[1], RecieveData[2], RecieveData[3], RecieveData[4], RecieveData[5], RecieveData[6]);
		//Convert registers in BCD into the time struct
		TheTime->tm_sec = ((RecieveData[0] & 0x0F) + ((RecieveData[0] & 0x70) >> 4)*10 );
		TheTime->tm_min = ((RecieveData[1] & 0x0F) + ((RecieveData[1] & 0x70) >> 4)*10 );
		
		//Handle hours in 12 or 24 hour mode. The struct will always take 24 hour format
		if( (RecieveData[2] & 0x40) == 0x04)		//12 hour mode
		{
			if((RecieveData[2] & 0x20) == 0x20)		//PM
			{
				TheTime->tm_hour = 12 + (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
			}
			else
			{
				TheTime->tm_hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x10) >> 4)*10;
			}
		}
		else			//24 Hour Mode
		{
			TheTime->tm_hour = (RecieveData[2] & 0x0F) + ((RecieveData[2] & 0x30) >> 4)*10;
		}
		
		TheTime->tm_wday = (RecieveData[3] & 0x07);
		TheTime->tm_mday = ((RecieveData[4] & 0x0F) + ((RecieveData[4] & 0x30) >> 4)*10);
		TheTime->tm_mon = (((RecieveData[5] & 0x0F) + ((RecieveData[5] & 0x10) >> 4)*10)) - 1;	//Months are stored on the RTC as 1-12, but are stored in the tm struct as 0-11
		//TODO: Handle century bit...
		TheTime->tm_year = 100 + ((RecieveData[6] & 0x0F) + ((RecieveData[6] & 0xF0) >> 4)*10);	
	}
	else
	{
		printf_P(PSTR("I2C Error\n"));
	}
	//printf("Done\n");
	//printf("Time: %02d:%02d:%02d\n", (*TheTime).tm_hour, TheTime->tm_min, TheTime->tm_sec);
	
	
	return;
}

//AlarmMasks format: <DY/DT(1=day, 0=date)> <AxM4> <AxM3> <AxM2> <A1M1>
//Note: A2M1 does not exist (no seconds register)
void DS3231M_SetAlarm(uint8_t AlarmNumber, uint8_t AlarmMasks, struct tm *AlarmTime)
{
	uint8_t RecieveData;
	uint8_t SendData[5];
	
	if(AlarmNumber == 1)
	{
		SendData[0] = DS3231M_REG_A1_SEC;
		SendData[1] = (((AlarmMasks & 0x01) << 7) | (AlarmTime->tm_sec % 10) | ((AlarmTime->tm_sec / 10) << 4));
		SendData[2] = (((AlarmMasks & 0x02) << 6) | (AlarmTime->tm_min % 10) | ((AlarmTime->tm_min / 10) << 4));
		SendData[3] = ((((AlarmMasks & 0x04) << 5) | (AlarmTime->tm_hour % 10) | ((AlarmTime->tm_hour / 10) << 4)) & 0xBF);
		
		if((AlarmMasks & 0x10) == 0x10)
		{
			//Alarm on day of week
			SendData[4] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_wday));
		}
		else
		{
			//Alarm on date
			SendData[4] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_mday % 10) | ((AlarmTime->tm_mday / 10) << 4));
		}
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 5, 0);
	}
	else
	{
		SendData[0] = DS3231M_REG_A2_MIN;
		SendData[1] = (((AlarmMasks & 0x02) << 6) | (AlarmTime->tm_min % 10) | ((AlarmTime->tm_min / 10) << 4));
		SendData[2] = ((((AlarmMasks & 0x04) << 5) | (AlarmTime->tm_hour % 10) | ((AlarmTime->tm_hour / 10) << 4)) & 0xBF);
		
		if((AlarmMasks & 0x10) == 0x10)
		{
			//Alarm on day of week
			SendData[3] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_wday));
		}
		else
		{
			//Alarm on date
			SendData[3] = (((AlarmMasks & 0x08) << 4) | ((AlarmMasks & 0x10) << 2) | (AlarmTime->tm_mday % 10) | ((AlarmTime->tm_mday / 10) << 4));
		}
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 4, 0);
	}
	return;
}

void DS3231M_EnableAlarm(uint8_t AlarmNumber)
{
	uint8_t RecieveData;
	uint8_t SendData[2];
	
	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		//Get the current control register
		SendData[0] = DS3231M_REG_CONTROL;
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 1, 1);
		
		//Enable the requested alarm
		SendData[1] = RecieveData | AlarmNumber;
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 2, 0);
	}
	return;
}

void DS3231M_DisableAlarm(uint8_t AlarmNumber)
{
	uint8_t RecieveData;
	uint8_t SendData[2];
	
	if((AlarmNumber == 1) || (AlarmNumber == 2))
	{
		//Get the current control register
		SendData[0] = DS3231M_REG_CONTROL;
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 1, 1);
		
		//Disable the requested alarm
		SendData[1] = (RecieveData & (~AlarmNumber));
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 2, 0);
		
		//Get the current status register
		SendData[0] = DS3231M_REG_STATUS;
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 1, 1);
		
		//Clear the interrupt flag for the requested alarm
		SendData[1] = (RecieveData & (~AlarmNumber));
		TWIRW(DS3231M_SLA_ADDRESS, SendData, &RecieveData, 2, 0);
	}
	return;
}

uint8_t DS3231M_GetTemp(int8_t *TempLHS, uint8_t *TempRHS)
{

	uint8_t RecieveData[2];
	uint8_t SendData;
	uint8_t ret;

	SendData = DS3231M_REG_TEMP_HI;
	ret = TWIRW(DS3231M_SLA_ADDRESS, &SendData, RecieveData, 1, 2);

	if(ret == 0)
	{
		*TempLHS = RecieveData[0];				//LHS is in 2's compliment form
		*TempRHS = (RecieveData[1] >> 6)*25;	//The two MSB of this byte are the decimal portion of the temperature in .25s
		return 0;
		//printf("High: 0x%02X\nLow: 0x%02X\n", RecieveData[0], RecieveData[1]);
	}
	else
	{
		printf_P(PSTR("I2C Error (0x%02X)\n"), ret);
		return ret;
	}
}

uint8_t DS3231M_TimeIsValid(void)
{
	uint8_t RecieveData;
	uint8_t SendData = DS3231M_REG_STATUS;
	
	//TODO: Add error checking to the TWI transaction??
	TWIRW(DS3231M_SLA_ADDRESS, &SendData, &RecieveData, 1, 1);

	if( (RecieveData & 0x80) == 0x00 )
	{
		//Oscillator stop flag is zero, the time should be valid
		return 1;
	}
		
	return 0;	
}


/** @} */