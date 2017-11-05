/*
 * global_vars.h
 *
 *  Created on: 05.11.2017
 *      Author: jan
 */

#ifndef GLOBAL_VARS_H_
#define GLOBAL_VARS_H_

#include "WebSchalter4_1.h"
// Global variables
extern int z;                   //Aktuelle EEPROM-Adresse zum lesen
extern int i;
extern int active_low;    // "0" beim schalten mit +5V , "1" beim schalten mit 0V
extern IPAddress apIP;
#if defined DNS
#include <DNSServer.h>
extern const byte DNS_PORT;
extern DNSServer dnsServer;
#endif

extern String BenuzPass;
extern char ssid[32];
extern char passwort[64];
extern char BenuzName[32];
extern char BenuzPasswort[32];
extern ESP8266WebServer server;
extern EspClass esp;
extern int val[4];
extern String Temp;
extern boolean AP; // Acsespoint Modus aus
extern boolean WLAN_Fehlt;
extern unsigned long NTPTime, ZeitTemp;
extern int timout; //

extern size_t Timers1html_len;
extern const char Timers1html[];
extern size_t Timers2html_len;
extern const char Timers2html[];

extern size_t WEBset_len;
extern const char WEBset[];
// End global variables


#endif /* GLOBAL_VARS_H_ */
