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
*	\brief		XA1110 GPS functions
*	\author		Pat Satyshur
*	\version	1.0
*	\date		2/17/2019
*	\copyright	Copyright 2019, Pat Satyshur
*	\ingroup 	hardware
*
*	@{
*/

#include "main.h"


//uint8_t GPS_String[300];



//TODO: Abstract the TWI function using this, and check for bus errors?
uint8_t GPS_GetPacket(char *DataString)
{
	//uint8_t DataToSend;
	uint8_t ReadData[255];
	uint8_t TimeoutCount = 0;
	uint16_t i;
	uint16_t StringLocation = 0;
	uint8_t EndLoop = 0;
	
	do 
	{
		TimeoutCount++;
		
		TWIRW(XA1110_SLA_ADDR, NULL, ReadData, 0, 255);
		for(i=0; i<255; i++)
		{
			if(ReadData[i] != 0x0A)
			{
				DataString[StringLocation] = ReadData[i];
				StringLocation++;
			}
			else if(StringLocation > 0)
			{
				DataString[StringLocation] = '\0';
				EndLoop = 1;
			}
		}
		
		if(TimeoutCount > 10)
		{
			EndLoop = 1;
		}
	} while (EndLoop == 0);
	
	
	return 0;
}

/** @brief Flush old data from the GPS internal buffer and get new data.
 *
 *  This function attempts to flush the GPS internal buffer and get up-to-date
 *  NEMA strings for GxGGA and GxRMC. This function is tested with the [XA1110 
 *  module from Sparkfun Electronics.](https://www.sparkfun.com/products/14414)
 *  
 *  @param GPGGA_STRING The GxGGA string. This string is assumed to be XA1110_NEMA_STRING_MAX_SIZE(150) bytes long.
 *  @param GPRMC_STRING The GxRMC string. This string is assumed to be XA1110_NEMA_STRING_MAX_SIZE(150) bytes long.
 *  @return Returns the result of the function. Possible values are:
 *  - 0: The function completed successfully. The NEMA strings should be readable.
 *  - 1: The buffer did not flush in 100 tries.
 *  - 2: The buffer flushed properly, but the function did not find both the GxGGA and
         GxRMC strings.
 */
uint8_t GPS_GetStrings(char *GPGGA_STRING, char *GPRMC_STRING)
{
	#define XA1110_STATUS_CODE_START		0
	#define XA1110_STATUS_CODE_NEMA			8
	#define XA1110_STATUS_CODE_GGA			1
	#define XA1110_STATUS_CODE_RMC			3
	#define XA1110_NEMA_STRING_MAX_SIZE		150		//TODO: make this a variable???
	
	uint8_t GPS_Packet[255];		//Packet buffer
	uint8_t NEMA_CODE[8];			//Buffer for NEMA saving NEMA codes
	uint8_t LocationInString = 0;	//String location variable for parsing strings
	uint16_t i = 0;
	uint16_t j = 0;
	uint8_t EmptyPacketCount = 0;
	uint8_t StatusFlag = 0;
			
	for(j=0;j<100;j++)
	{
		StatusFlag = 0;
				
		TWIRW(XA1110_SLA_ADDR, NULL, GPS_Packet, 0, 255);
		for(i=0;i<255;i++)
		{
			if(GPS_Packet[i] != 0x0A)
			{
				//Valid data found in packet
				EmptyPacketCount = 0;			
				break;
			}
		}
		
		if(i >= 255)
		{
			//No data in packet.
			EmptyPacketCount++;
			if(EmptyPacketCount > 2)
			{
				//Several empty packets in a row indicate the buffer is empty
				break;
			}
		}
	}
	
	if(j >= 100)
	{
		//Did not flush the buffer in 100 loops
		return 1;
	}

	//Buffer is flushed, look for good data
	for(j=0;j<200;j++)
	{
		TWIRW(XA1110_SLA_ADDR, NULL, GPS_Packet, 0, 255);
		for(i=0;i<255;i++)
		{
			if(GPS_Packet[i] != 0x0A)
			{
				if(GPS_Packet[i] == '$')	//TODO: check the status code here also??
				{
					//found a '$'
					LocationInString = 0;
					NEMA_CODE[LocationInString] = GPS_Packet[i];
					LocationInString++;
					StatusFlag = XA1110_STATUS_CODE_NEMA;
				}
				else if(StatusFlag == XA1110_STATUS_CODE_NEMA)
				{
					NEMA_CODE[LocationInString] = GPS_Packet[i];
					LocationInString++;
					if(LocationInString >= 6)
					{
						//Entire NEMA code captured
						if((NEMA_CODE[3] == 'G') && (NEMA_CODE[4] == 'G') && (NEMA_CODE[5] == 'A'))
						{
							StatusFlag = XA1110_STATUS_CODE_GGA;
							LocationInString = 0;
						}
						else if ((NEMA_CODE[3] == 'R') && (NEMA_CODE[4] == 'M') && (NEMA_CODE[5] == 'C'))
						{
							StatusFlag = XA1110_STATUS_CODE_RMC;
							LocationInString = 0;
						}
						else
						{
							//NEMA string is not GGA or RMC, ignore and wait for the next one
							StatusFlag = XA1110_STATUS_CODE_START;
						}
					
					}
				}
				else if(StatusFlag == XA1110_STATUS_CODE_GGA)	//Writing to $GxGGA string
				{
					if((GPS_Packet[i] == 0x0D))		//Carriage Return indicate end of string
					{
						//End of string
						GPGGA_STRING[LocationInString] = '\0';
						StatusFlag = XA1110_STATUS_CODE_START;
					}
					else
					{
						//keep writing to string
						GPGGA_STRING[LocationInString] = (char)GPS_Packet[i];
						LocationInString++;
					}
				}
				else if(StatusFlag == XA1110_STATUS_CODE_RMC)	//Writing to $GxRMC string
				{
					if((GPS_Packet[i] == 0x0D))	//Carriage Return indicates end of string
					{
						//End of string
						GPRMC_STRING[LocationInString] = '\0';
						//Found both strings
						return 0;
					}
					else
					{
						//keep writing to string
						GPRMC_STRING[LocationInString] = (char)GPS_Packet[i];
						LocationInString++;
					}
				}
			
				if(LocationInString >= XA1110_NEMA_STRING_MAX_SIZE)
				{
					//Keep the buffers from overflowing
					return 6;
				}
			}			
		}
	}
			
	if(j>=200)
	{
		//Strings not found
		return 2;
	}
	
	//Should never get here...
	return 5;
}



/** @brief Separate and validate the RMC string
 *
 *  This function will separate the RMC string into the time,
 *  date, latitude and longitude. The function will also attempt
 *  to verify that the string is valid and the captured values
 *  correct. This function is tested with the [XA1110 module from
 *  Sparkfun Electronics.](https://www.sparkfun.com/products/14414)
 *
 *  NOTE: If the RMC string is longer than ~255 bytes, a bunch of
 *  uint8_t will need to change to uint16_t. This should not be necessary.
 *  
 *  @param GPRMC_STRING The GPRMC string. This string is assumed to be 150 bytes long.
 *  @param RMC_Time The time structure to save the time into.
 *  @param LatInSeconds The latitude from the GPS fix, expressed in arcseconds.
 *  @param LongInSeconds The longitude from the GPS fix, expressed in arcseconds.
 *  @return Returns a status based on the validation result. Possible values are:
 *  - 0: Data is valid
 *  - 1: No GPS Lock
 *  - 2: End of the string reached without finding all of the data
 *  - 3: String is longer than max allowable length (150 bytes)
 *  - 4: Validation Failure: commas are out of place
 *  - 5: Validation Failure: Longitude is out of range
 *  - 6: Validation Failure: Latitude is out of range
 *  - 7: Validation Failure: Time values are out of range
 */
uint8_t GPS_ValidateRMC(char *GPRMC_STRING, struct tm *RMC_Time, int32_t *LatInSeconds, int32_t *LongInSeconds)
{
	uint8_t i = 0;
	uint8_t ReturnFlag = 3;
	uint8_t LocationInString = 0;
	
	for(i=0;i<150;i++)
	{
		if(GPRMC_STRING[i] == '\0')
		{
			ReturnFlag = 2;
			break;
		}
		else if(GPRMC_STRING[i] == ',')
		{
			LocationInString++;
			if(LocationInString == 1)	//latitude, next comma is in location +11
			{
				if(GPRMC_STRING[i+11] != ',')
				{
					//Ending comma is missing
					ReturnFlag = 4;
					break;
				}
				//Get the time string
				RMC_Time->tm_hour = (GPRMC_STRING[i+1]-48)*10  + (GPRMC_STRING[i+2]-48);
				RMC_Time->tm_min =  (GPRMC_STRING[i+3]-48)*10  + (GPRMC_STRING[i+4]-48);
				RMC_Time->tm_sec =  (GPRMC_STRING[i+5]-48)*10  + (GPRMC_STRING[i+6]-48);
			}
			else if((LocationInString == 2) && (GPRMC_STRING[i+1] == 'V'))	//Lock status, commas not validated
			{
				//A 'V' here indicates that the GPS is not locked.
				ReturnFlag = 1;
				break;
			}
			else if(LocationInString == 3) //latitude, next comma is in location +12
			{
				if(GPRMC_STRING[i+12] != ',')
				{
					//Ending comma is missing
					ReturnFlag = 4;
					break;
				}
				*LatInSeconds = ((int32_t)(GPRMC_STRING[i+1]-48)*10  + (int32_t)(GPRMC_STRING[i+2]-48))*3600l + (((int32_t)(GPRMC_STRING[i+3]-48)*1000l  + (int32_t)(GPRMC_STRING[i+4]-48)*100l + (int32_t)(GPRMC_STRING[i+6]-48)*10l + (int32_t)(GPRMC_STRING[i+7]-48))*60l)/100l;
			}
			else if(LocationInString == 4)	//N or S latitude, next comma is in location +2
			{
				if(GPRMC_STRING[i+2] != ',')
				{
					//Ending comma is missing
					ReturnFlag = 4;
					break;
				}
				if(GPRMC_STRING[i+1] == 'S')
				{
					//Latitude is in seconds of north latitude
					*LatInSeconds = -1 * (*LatInSeconds);
				}
			}
			else if(LocationInString == 5)	//Longitude, next comma is in location +13
			{
				if(GPRMC_STRING[i+13] != ',')
				{
					//Ending comma is missing
					ReturnFlag = 4;
					break;
				}
				*LongInSeconds = ((int32_t)(GPRMC_STRING[i+1]-48)*100l  + (int32_t)(GPRMC_STRING[i+2]-48)*10l + (int32_t)(GPRMC_STRING[i+3]-48))*3600l + (((int32_t)(GPRMC_STRING[i+4]-48)*1000l  + (int32_t)(GPRMC_STRING[i+5]-48)*100l + (int32_t)(GPRMC_STRING[i+7]-48)*10l + (int32_t)(GPRMC_STRING[i+8]-48))*60l)/100l;
			}
			else if(LocationInString == 6)	//E or W longitude, next comma is in location +2
			{
				if(GPRMC_STRING[i+2] != ',')
				{
					//Ending comma is missing
					ReturnFlag = 4;
					break;
				}
				if(GPRMC_STRING[i+1] == 'W')
				{
					//Longitude is in seconds of east longitude
					*LongInSeconds = -1 * (*LongInSeconds);
				}
				//Gets E or W of longitude
			}
			else if(LocationInString == 9)	//Date, next comma is in location +7
			{
				if(GPRMC_STRING[i+7] != ',')
				{
					//Ending comma is missing
					ReturnFlag = 4;
					break;
				}
				RMC_Time->tm_mday	= (GPRMC_STRING[i+1]-48)*10 + (GPRMC_STRING[i+2]-48);
				RMC_Time->tm_mon	= ((GPRMC_STRING[i+3]-48)*10 + (GPRMC_STRING[i+4]-48)) - 1;		//Months start at zero
				RMC_Time->tm_year	= ((GPRMC_STRING[i+5]-48)*10 + (GPRMC_STRING[i+6]-48)) + 100;	//Years from 1900
				ReturnFlag = 0;		//Indicate that we found what we wanted.
				break;				//This is the last bit of the NEMA string I care about
			}
		}
	}
	
	if(ReturnFlag == 0)
	{
		//Make sure the data is valid
		if( ((*LongInSeconds) > 1296000) || ((*LongInSeconds) < -1296000) )
		{
			//Longitude is out of range
			ReturnFlag = 5;
		}
		else if( ((*LatInSeconds) > 324000) || ((*LatInSeconds) < -324000) )
		{
			//Latitude is out of range
			ReturnFlag = 6;
		}
		else if( (RMC_Time->tm_hour > 23) || (RMC_Time->tm_hour < 0)  || (RMC_Time->tm_min > 59) || (RMC_Time->tm_min < 0) || (RMC_Time->tm_sec > 59) || (RMC_Time->tm_sec < 0) || (RMC_Time->tm_mday > 31) || (RMC_Time->tm_mday < 1) || (RMC_Time->tm_mon > 11) || (RMC_Time->tm_mon < 0)  || (RMC_Time->tm_year < 100) || (RMC_Time->tm_year > 500) )
		{
			//Time value is out of range
			ReturnFlag = 7;
		}
	}
	
	return ReturnFlag;
}