/*
 * ntp.h
 *
 *  Created on: 05.11.2017
 *      Author: jan
 */

#ifndef NTP_H_
#define NTP_H_


String PrintDate (unsigned long epoch);
unsigned long GetNTP(void);
String PrintTime (unsigned long epoch);
boolean summertime(int year, byte month, byte day, byte hour, byte tzHours);


#endif /* NTP_H_ */
