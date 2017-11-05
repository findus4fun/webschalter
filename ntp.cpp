
#include "WebSchalter4_1.h"
#include <WiFiUdp.h>
#define TIMEZONE 2
IPAddress timeServerIP; 
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; 
byte packetBuffer[ NTP_PACKET_SIZE]; 
WiFiUDP udp;

/**
 * Посылаем запрос NTP серверу на заданный адрес
 */
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
// Очистка буфера в 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
// Формируем строку зыпроса NTP сервера
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
// Посылаем запрос на NTP сервер (123 порт)
  udp.beginPacket(address, 123); 
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

/**
 * Посылаем и парсим запрос к NTP серверу
 */
unsigned long GetNTP(void) {
unsigned long ntp_time = 0;
   udp.begin(2390); 
  WiFi.hostByName(ntpServerName, timeServerIP); 
  sendNTPpacket(timeServerIP); 
  int cb = udp.parsePacket();
  delay(100);
  int timout1 = 0;
  while (cb  < NTP_PACKET_SIZE) 
     {
     timout1++;
     if  (timout1 > 50) return 0;                  // 3s Timout
     cb = udp.parsePacket();
     delay(100);
     } 
    Serial.print("packet received, length=");
    Serial.println(cb);
// Читаем пакет в буфер    
    udp.read(packetBuffer, NTP_PACKET_SIZE); 
// 4 байта начиная с 40-го сождержат таймстамп времени - число секунд 
// от 01.01.1900   
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
// Конвертируем два слова в переменную long
    unsigned long secsSince1900 = highWord << 16 | lowWord;
// Конвертируем в UNIX-таймстамп (число секунд от 01.01.1970
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;
// Делаем поправку на местную тайм-зону
    ntp_time = epoch + TIMEZONE*3600;    
    Serial.print("Unix time = ");
    Serial.println(ntp_time);
  
  return ntp_time;
}

String PrintTime (unsigned long epoch)
{
  String TimeString = "";
  if ( (epoch  % 86400L) / 3600 < 10 )TimeString += "0";
  TimeString += String((epoch  % 86400L) / 3600); // Stunden
  TimeString += ":";
  if ( ((epoch % 3600) / 60) < 10 )TimeString += "0";
  TimeString += String((epoch  % 3600) / 60); // Minuten
  TimeString += ":";
  if ( (epoch % 60) < 10 )TimeString += "0";
  TimeString += String(epoch % 60); // Sekunden
  return  TimeString;
}

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // API starts months from 1, this array starts from 0

String PrintDate (unsigned long epoch)
{
  String DateString = "";
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;
  time = (uint32_t)epoch;
  time /= 60; // now it is minutes
  time /= 60; // now it is hours
  time /= 24; // now it is days
  year = 0;
  days = 0;
  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) year++;
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  days = 0;
  month = 0;
  monthLength = 0;
  for (month = 0; month < 12; month++)
  {
    if (month == 1)
    { // february
      if (LEAP_YEAR(year)) monthLength = 29;
      else  monthLength = 28;
    } else  monthLength = monthDays[month];


    if (time >= monthLength)time -= monthLength;
    else break;
    
  }
  if((time + 1)<10)DateString += "0"; 
  DateString += String(time + 1); // day of month
  DateString += ".";
  if((month + 1)<10)DateString += "0"; 
  DateString += String(month + 1);  // jan is month 1
  DateString += ".";
  //DateString += String(year + 1970); // year is offset from 1970
  DateString += String(year -30); // year is offset from 1970
  return DateString;
}



boolean summertime(int year, byte month, byte day, byte hour, byte tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
 if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
 if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
 if ((month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7))) || (month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7))))
   return true;
 else
   return false;
}


