// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _WebSchalter4_1_H_
#define _WebSchalter4_1_H_
#include "Arduino.h"
//add your includes for the project WebSchalter4_1 here
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <TimeLib.h>
extern "C" {
#include "user_interface.h"
}
#include "ntp.h"
#include "log.h"
#include "Setup.h"
#include "timer.h"


#define Relay4 16
#define DNS
#define Taster1 2
#define Taster2 0
#define Taster3 4
#define Taster4 5
#define Relay1 13
#define Relay2 12
#define Relay3 14





//end of add your includes here


//add your function definitions for the project WebSchalter4_1 here




//Do not add code below this line
#endif /* _WebSchalter4_1_H_ */
