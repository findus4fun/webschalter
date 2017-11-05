// Do not remove the include below
#include "WebSchalter4_1.h"

extern const byte DNS_PORT;

#include "Index.h"
#include "Timers1.h"
#include "Timers2.h"
#include "TimersJs.h"
#include "WEBset.h"

// Global variables
int z = 0;                   //Aktuelle EEPROM-Adresse zum lesen
int i = 0;
int active_low = 1;    // "0" beim schalten mit +5V , "1" beim schalten mit 0V
IPAddress apIP(192, 168, 168, 30);
#if defined DNS
#include <DNSServer.h>
const byte DNS_PORT = 53;
DNSServer dnsServer;
#endif

String BenuzPass = "";
char ssid[32] = "fhgdhfghd\0";
char passwort[64] = "gghdfgh\0";
char BenuzName[32] = "admin\0";
char BenuzPasswort[32] = "gghdfgh\0";
ESP8266WebServer server;
EspClass esp;
int val[4] = {0, 0, 0, 0};
String Temp = "";
boolean AP = 0; // Acsespoint Modus aus
boolean WLAN_Fehlt = 0;
unsigned long NTPTime = 0, ZeitTemp = 0;
int timout = 0; //



struct Timer_stuct {
  byte art;
  unsigned long zeit;
  byte relais;
  boolean ein;
};

extern Timer_stuct timer[50];

// End global variables

void Zeit_Einstellen()
{
  NTPTime = GetNTP();
  timout = 0;
  while (NTPTime == 0 )
  {
    timout++;
    if  (timout > 10)  break;
    NTPTime = GetNTP();
  }
  setTime(NTPTime);
  Serial.print("NTP Time: ");
  Serial.println(PrintTime(NTPTime));
  if (!summertime( year(), month(), day(),  hour(), 0))  adjustTime(-60 * 60);
  Serial.print("Zeit nach Sommer- Winterzeitanpassung--: ");
  Serial.println( PrintTime(now()) );
}

void Ereignis_Zustand()
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  String Antwort = String(val[0]) + ";" + String(val[1]) + ";" + String(val[2]) + ";" + String(val[3]) + ";";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/plain", Antwort); // Antwort an Internet Browser senden
}

void Ereignis_Schalte()
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  String RelayNr = server.arg("Relay");          // Relais Nr. empfangen
  int RelayNrInt = RelayNr.toInt();
  String ArgumentOn = server.arg("On");
  int RelayOn = ArgumentOn.toInt();
  Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Relais: " + RelayNr;
  if (RelayOn == 1) Temp += " ON ";
  if (RelayOn == 0) Temp += " OFF";
  Temp += "     ";
  Temp += server.client().remoteIP().toString();
  Temp += "\r\n";
  if (RelayOn == 1) val[RelayNrInt - 1] = 1;
  if (RelayOn == 0) val[RelayNrInt - 1] = 0;
  digitalWrite(Relay1, active_low ^ val[0]);
  digitalWrite(Relay2, active_low ^ val[1]);
  digitalWrite(Relay3, active_low ^ val[2]);
  digitalWrite(Relay4, active_low ^ val[3]);
  Ereignis_Zustand();
  LogSchreiben(Temp);
}



void Ereignis_Index()    // Wird ausgeuehrt wenn "http://<ip address>/" aufgerufen wurde
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", indexHTML);
}

void Ereignis_js()    // Wird ausgeuehrt wenn "http://<ip address>/" aufgerufen wurde
{
  server.setContentLength(TimersJs_len);
  server.send(200, "text/plain", "");
  server.sendContent(TimersJs);
}


void Ereignis_log()    //
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  if (!SPIFFS.begin()) Serial.println("Failed to mount file system");
  File DataFile = SPIFFS.open("/log.txt", "r");
  if (!DataFile)Serial.println("Failed to open log.txt");
  server.sendHeader("Cache-Control", "no-cache");
size_t sent = server.streamFile(DataFile, "text/plain");
  DataFile.close();
}

void Ereignis_DeleteTimer()
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  if (server.arg("Nr") != "")
  {
    String StrNr = server.arg("Nr");         // Relais Nr. empfangen
    int Nr = StrNr.toInt();
    timer[Nr].art = 0;
    TimerNr_speichern(Nr);
  }
  Ereignis_Timers_Zeigen();
}

void Ereignis_DeleteLog()
{
if (SPIFFS.remove("/log.txt") ) Serial.println("Datei log.txt gelöscht");
 server.send(200, "text/plain", "log geloescht");
}


void setup()                // Wird 1 Mal beim Start ausgefuehrt
{
  char inser;               // Serielle daten ablegen
  String nachricht = "";    //  Setup Formular
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  digitalWrite(Relay1, active_low);
  digitalWrite(Relay2, active_low);
  digitalWrite(Relay3, active_low);
  digitalWrite(Relay4, active_low);
  pinMode(Taster1, INPUT_PULLUP);
  pinMode(Taster2, INPUT_PULLUP);
  pinMode(Taster3, INPUT_PULLUP);
  pinMode(Taster4, INPUT_PULLUP);


  EEPROM.begin(250);                                 // EEPROM initialisieren mit 200 Byts
  Serial.begin(115200);

  while (Serial.available())inser = Serial.read();  // Setup Weiche, Erkennen ob TXD/RXD gebrückt sind
  Serial.println("Start");
  delay(10);
  inser = Serial.read();
  if (inser != 'S') goto weiter;
  inser = Serial.read();
  if (inser != 't') goto weiter;
  Einstellen();

weiter:
  z = 0;
  LeseEeprom(ssid, sizeof(ssid));        // EEPROM lesen
  if (ssid[0] == 255) // Wenn EEPROM leer ist dann:
  {
    for (i = 0; i < 6; i++) EEPROM.write(i, '\0');
    EEPROM.commit();
    Einstellen();
  }

  LeseEeprom(passwort, sizeof(passwort));
  LeseEeprom(BenuzPasswort, sizeof(BenuzPasswort));

Serial.println("Freie RAM = "+String(system_get_free_heap_size()));

  if (!SPIFFS.begin()) Serial.println("Failed to mount file system");

  if (ssid[0] == '\0')  //Wenn ssid leer dann Access Point Modus starten
  {
    Serial.println("Starte in Access Point modus" );
    Serial.println("IP http://192.168.168.30");
    Serial.print("SSID: WebSchalter, Passwort: ");
    Serial.println(passwort);
    WiFi.mode(WIFI_AP);      // access point modus
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    strncpy(ssid, "WebSchalter\0", 13);
    WiFi.softAP(&ssid[0], &passwort[0]);  // Name des Wi-Fi netzes
#if defined DNS
    dnsServer.start(DNS_PORT, "*", apIP);
#endif
    AP = 1;
  }
  else // Wenn ssid angegeben dann in Stationmodus mit Router verbinden
  {
    Serial.print("Verbinde mit ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passwort);
    timout = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print("O");
      timout++;
      if  (timout > 20) // Wenn Anmeldung nicht möglich
      {
        Serial.println("");
        Serial.println("Wlan verbindung fehlt");
        WLAN_Fehlt = 1;
        break;

      }
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.begin(115200);
      Serial.println("");
      Serial.println("Mit Wlan verbunden");
      Serial.print("IP Adresse: ");
      Serial.println(WiFi.localIP());
      Zeit_Einstellen();
    }
  } // ENDE Stationmodus / Access Point modus Auswahl

  if (!BenuzPasswort[0])Serial.println("Serverauthentifizierung ausgeschaltet");
  else
  {
    Serial.println("Serverauthentifizierung eingeschaltet");
    Serial.print("Benutzername : ");
    Serial.println(BenuzName);
    Serial.print("Benutzerpasswort : ");
    Serial.println(BenuzPasswort);
  }


  Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Server gestartet";
  Temp += "\r\n";
  LogSchreiben(Temp);

  //  Bechandlung der Ereignissen anschlissen
  server.on("/", Ereignis_Index);
  server.on("/schalte.php", Ereignis_Schalte);
  server.on("/zustand.php", Ereignis_Zustand);
  server.on("/log.txt", Ereignis_log);
  server.on("/timer.html", Ereignis_Timers_Zeigen);
  server.on("/settimer.php", Ereignis_NeueTimer);
  server.on("/delete.php", Ereignis_DeleteTimer);
  server.on("/Timers.js", Ereignis_js);
  server.on("/deletelog.php", Ereignis_DeleteLog);

  server.onNotFound ( handleNotFound );
  server.begin();  // Starte den Server
  Serial.println("HTTP Server gestartet");

  if (SPIFFS.exists("/timern.dat")) // Timern aus Datei laden
  {
    File DataFile = SPIFFS.open("/timern.dat", "r");
    DataFile.read(reinterpret_cast<uint8_t*>(&timer), sizeof(timer));
    DataFile.close();
  }  else
  {
  Serial.println("Datei timern.dat fehlt");
  File DataFile = SPIFFS.open("/timern.dat", "w");
  DataFile.write(reinterpret_cast<uint8_t*>(&timer), sizeof(timer));
  DataFile.close();
  Serial.println("Datei timern.dat erzeugt");
  }
}



void loop()
{
#if defined DNS
  if (AP)dnsServer.processNextRequest();
#endif

  if (!digitalRead(Taster1)) //Taster betätigt
  {
    delay(50);
    if (!digitalRead(Taster1)) // Nach 100mS immer noch betätigt dann ist es keine Induktionstrom in der Leitung
    {
      val[0] = !val[0];
      digitalWrite(Relay1, active_low ^ val[0]);
      Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Relais: 1";
      if (val[0]) Temp += " ON ";
      else Temp += " OFF";
      Temp += "     ";
      Temp += "Taster";
      Temp += "\r\n";
      LogSchreiben(Temp);
      while (!digitalRead(Taster1))server.handleClient(); // Warten bis der Taster losgelassen wird
    }
  }

  if (!digitalRead(Taster2)) //Taster betätigt
  {
    delay(50);
    if (!digitalRead(Taster2)) // Nach 100mS immer noch betätigt dann ist es keine Induktionstrom in der Leitung
    {
      val[1] = !val[1];
      digitalWrite(Relay2, active_low ^ val[1]);
      Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Relais: 2";
      if (val[0]) Temp += " ON ";
      else Temp += " OFF";
      Temp += "     ";
      Temp += "Taster";
      Temp += "\r\n";
      LogSchreiben(Temp);
      while (!digitalRead(Taster2))server.handleClient(); // Warten bis der Taster losgelassen wird
    }
  }

  if (!digitalRead(Taster3)) //Taster betätigt
  {
    delay(50);
    if (!digitalRead(Taster3)) // Nach 100mS immer noch betätigt dann ist es keine Induktionstrom in der Leitung
    {
      val[2] = !val[2];
      digitalWrite(Relay3, active_low ^ val[2]);
      Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Relais: 3";
      if (val[0]) Temp += " ON ";
      else Temp += " OFF";
      Temp += "     ";
      Temp += "Taster";
      Temp += "\r\n";
      LogSchreiben(Temp);
      while (!digitalRead(Taster3))server.handleClient(); // Warten bis der Taster losgelassen wird
    }
  }

  if (!digitalRead(Taster4)) //Taster betätigt
  {
    delay(50);
    if (!digitalRead(Taster4)) // Nach 100mS immer noch betätigt dann ist es keine Induktionstrom in der Leitung
    {
      val[3] = !val[3];
      digitalWrite(Relay4, active_low ^ val[3]);
      Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Relais: 4";
      if (val[0]) Temp += " ON ";
      else Temp += " OFF";
      Temp += "     ";
      Temp += "Taster";
      Temp += "\r\n";
      LogSchreiben(Temp);
      while (!digitalRead(Taster4))server.handleClient(); // Warten bis der Taster losgelassen wird
    }
  }

  if (now() != ZeitTemp)            // Ausführug 1 mal in der Sekunde
  {
    ZeitTemp = now();

    if ( (NTPTime + 86400) < ZeitTemp )NTPTime = 0; //Zeit Update alle 24Stunden
    if ( NTPTime ) Timers_pruefen(&ZeitTemp); // nur wenn zeit angestellt ist

  }       //ende Ausführug 1 mal in der Sekunde

  if (WiFi.status() == WL_CONNECTED && !NTPTime) // Zeit nachträglich einstellen
  {
    Zeit_Einstellen();
  }

  server.handleClient();              // Server Ereignisse abarbeiten
}


