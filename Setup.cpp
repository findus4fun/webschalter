#include "WebSchalter4_1.h"

#include "global_vars.h"

String Mitteilung = "";


unsigned int hexToDec(String hexString) {

  unsigned int decValue = 0;
  int nextInt;

  for (int i = 0; i < hexString.length(); i++) {

    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return decValue;
}

void LeseEeprom(char *daten, int lenge)
{
  char a;
  do {
    a = EEPROM.read(z);
    *daten = a;
    daten++;
    z++;
    lenge--;
    if (!lenge)break;
  }
  while (a);
}

void LeseEepromStr(String *daten, int lenge)
{
  char a;
  *daten = "";
  do {
    a = EEPROM.read(z);
    *daten += String(a);
    z++;
    lenge--;
    if (!lenge)break;
  } while (a);
}



void SchreibeEeprom (String k)
{
  int i = 0, blja = 64;
  int  pos, wert;
  char Buchstabe;
  String HexZahl, ersatz = "f";
  k.replace("+", " ");

  while (blja--)
  {
    pos = k.indexOf('%');   // Kodierte zeichen enthalten
    if (pos < 0)break;
    HexZahl = k.substring(pos + 1, pos + 3); // Position finden
    wert = hexToDec(HexZahl);
    ersatz[0] = wert;               //Int to Chahr im String
    HexZahl = "%" + HexZahl;        //Suchmuster finden
    k.replace(HexZahl, ersatz);
  }

  while (k[i])
  {
    EEPROM.write(z, k[i]);
    z++;
    i++;
  }
  EEPROM.write(z, '\0');
  z++;
}

void Ereignis_Setup()			// Wird ausgeuehrt wenn "http://<ip address>/" aufgerufen wurde
{
  Mitteilung = "";
  server.send(200, "text/html", WEBset);
}

void Ereignis_daten()			// Wird ausgeuehrt wenn "http://<ip address>/" aufgerufen wurde
{
  int i;
  if (ssid[0] == 255) ssid[0] = 0;            // Wenn speicher leer alles auf 0
  if (passwort[0] == 255) passwort[0] = 0;
  if (BenuzPass[0] == 255) BenuzPass = "";
  //if(nachricht[0]==255) nachricht[0]=0;
  String temp = "";              // Sternhen einfпїЅgen beim Passwortanzeige
  String pass = String(passwort);
  if (pass.length() > 2) for (i = 1; i < (pass.length() - 1); i++) pass[i] = '*';
  temp += "var ssid1=\"" + String(ssid) + "\"\r\n";
  temp += "var pass1=\"" + pass + "\"\r\n";
  temp += "var pass2=\"" + BenuzPass + "\"\r\n";
  //temp+="var nachricht=\""+nachricht+"\"\r\n";
  temp += "var Mitteilung=\"" + Mitteilung + "\"\r\n";
  temp += "var Firmware=\"4.1\"\r\n";
  temp += "var ChipID1=\"" + String(esp.getChipId()) + "\"\r\n";
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/plain", temp);
}

void formular()			// Wird ausgeuehrt wenn "http://<ip address>/setup.php"
{
  z = 0;
  SchreibeEeprom (server.arg("ssid"));
  String temp1 = server.arg("passwort");
  if (temp1[2] != '*') SchreibeEeprom (server.arg("passwort")); // wenn mit *
  else SchreibeEeprom (passwort);                   // dann geladene pass abspeichern
  SchreibeEeprom (server.arg("BenuzPass"));
  //SchreibeEeprom (server.arg("nachricht"));
  EEPROM.commit();

  z = 0;                        // EEPROM Star den Datensatzes
  LeseEeprom(ssid, sizeof(ssid));
  LeseEeprom(passwort, sizeof(passwort));
  LeseEepromStr(&BenuzPass, 50);
  //LeseEepromStr(&nachricht,100);
  Mitteilung = "Gespeichert!";
  server.send(200, "text/html", WEBset);
}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}


void Einstellen()
{
  Serial.println("Einstellen");
  EEPROM.begin(250);                                 // EEPROM initialisieren mit 200 Byts
  z = 0;                        // EEPROM Star den Datensatzes

  LeseEeprom(ssid, sizeof(ssid));
  LeseEeprom(passwort, sizeof(passwort));
  LeseEepromStr(&BenuzPass, 50);
  //LeseEepromStr(&nachricht,100);
  /*
  Serial.println(ssid);
  Serial.println(passwort);
  Serial.println(url);
  Serial.println(nachricht);
  */
  boolean endlos = 1;
  WiFi.mode(WIFI_AP);			// access point modus
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Web Schalter Setup", "tiramisu");	// Name des Wi-Fi netzes
#if defined DNS
  dnsServer.start(DNS_PORT, "*", apIP);
  #endif
  delay(500);					// Abwarten 0,5s
  server.onNotFound ( handleNotFound );
  server.on("/", Ereignis_Setup);

  server.on("/daten.js", Ereignis_daten);
  server.on("/setup.php", formular);
  server.begin();				// Starte den Server
  while (endlos)
  {
    #if defined DNS
    dnsServer.processNextRequest();
    #endif
    server.handleClient();

  }
}


