/*
 * XA1110.h
 *
 * Created: 2/17/2019 10:41:04 PM
 *  Author: pat
 */ 


#ifndef XA1110_H_
#define XA1110_H_

#define XA1110_SLA_ADDR		0x10

//TODO: Make a function that only gets the RMC string?
uint8_t GPS_GetPacket( char *DataString);
uint8_t GPS_GetStrings(char *GPGGA_STRING, char *GPRMC_STRING);

uint8_t GPS_ValidateRMC(char *GPRMC_STRING, struct tm *RMC_Time, int32_t *LatInSeconds, int32_t *LongInSeconds);



#endif /* XA1110_H_ */