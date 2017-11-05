#include "WebSchalter4_1.h"

#include "global_vars.h"

struct Timer_stuct {
  byte art;
  unsigned long zeit;
  byte relais;
  boolean ein;
};

Timer_stuct timer[50];


void Ereignis_Timers_Zeigen()
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  Temp = "";
  int Balken = 0;
  for (i = 0; i < 50; i++)        // Deaktivirte Timer Suchen
  {
    if (timer[i].art)
    {
      Balken++;
      Temp += "<tr ";
      if (Balken % 2 == 0) Temp += "class=\"alt\" ";
      Temp += ">\r\n";
      Temp += "<td>";
      if (timer[i].art == 1) Temp += "Einmalig";
      if (timer[i].art == 2) Temp += "Täglich";
      if (timer[i].art == 4) Temp += "Montags";
      if (timer[i].art == 5) Temp += "Dienstags";
      if (timer[i].art == 6) Temp += "Mittwochs";
      if (timer[i].art == 7) Temp += "Donerstags";
      if (timer[i].art == 8) Temp += "Freitags";
      if (timer[i].art == 9) Temp += "Samstags";
      if (timer[i].art == 3) Temp += "Sontags";
      Temp += "</td>\r\n<td>";
      if (timer[i].zeit > 1000000000) Temp += PrintDate(timer[i].zeit);
      Temp += "</td>\r\n<td>" + PrintTime (timer[i].zeit);
      Temp += "</td>\r\n<td> Relais Nr.: " + (String)timer[i].relais + "</td>\r\n<td>";
      Temp += String((timer[i].ein) ? "ON" : "OFF") + "</td>\r\n";
      Temp += "<td><a href=\"delete.php?Nr=" + (String)i + "\" >Löschen</a>" + "</td>\r\n";
      Temp += "</tr>\r\n";
    }
  }
  server.sendHeader("Cache-Control", "no-cache");
  //server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.setContentLength(Timers2html_len + Timers1html_len + Temp.length() );
  server.send(200, "text/html", ""); // Antwort an Internet Browser
  server.sendContent(Timers1html);
  server.sendContent(Temp);
  server.sendContent(Timers2html);
  //server.client().stop();
}

/*

  void Timers_speichern()
  {
  File DataFile = SPIFFS.open("/timern.dat", "w");
  DataFile.write(reinterpret_cast<uint8_t*>(&timer), sizeof(timer));
  DataFile.close();
  Serial.println("Timers_speichern");
  }
*/

void TimerNr_speichern(int datensatz)
{
  File DataFile = SPIFFS.open("/timern.dat", "r+");
  DataFile.seek(sizeof(timer[0])*datensatz, SeekSet);
  DataFile.write(reinterpret_cast<uint8_t*>(&timer[datensatz]), sizeof(timer[0]));
  DataFile.close();
  Serial.println("Speichere Timer Nr.: " + String(datensatz));
}



void TimerSchalte(int k)
{
  Temp = PrintDate(now()) + "   " + PrintTime (now()) + "   Relais: " + String(timer[k].relais);
  if (timer[k].ein)
  {
    val[timer[k].relais - 1] = 1; // Relais setzen
    Temp += " ON ";
    Serial.println(Temp);
  }
  else
  {
    val[timer[k].relais - 1] = 0;// Relais zurück setzen
    Temp += " OFF";
    Serial.println(Temp);
  }

  Temp += "     ";
  Temp += "Timer";
  Temp += "\r\n";
  LogSchreiben(Temp);

  digitalWrite(Relay1, active_low ^ val[0]);
  digitalWrite(Relay2, active_low ^ val[1]);
  digitalWrite(Relay3, active_low ^ val[2]);
  digitalWrite(Relay4, active_low ^ val[3]);
}

void Timers_pruefen(unsigned long* Zeit)
{
  for (i = 0; i < 50; i++)        // alle timer-einträge durchgehen
  {
    if (timer[i].art)//-------------------------- Timer aktiv --------------------------------
    {
      if (timer[i].zeit == *Zeit &  timer[i].art == 1) //Timer mit der aktueller zeit stimmt überein
      {
        TimerSchalte(i);
        timer[i].art = 0; // Timer Deaktivieren
        TimerNr_speichern(i);
      } //--------------------Ende Timerfunktion Einmalig ----------------------------------------------------

      if (timer[i].art > 1)  //-------------------- Timerfunktion Täglich ----------------------------------------------------
        if ((timer[i].zeit  % 86400L) == (*Zeit  % 86400L)) //Tageszeit Überinstimmung
        {
          Serial.println("Tageszeit Überinstimmung ");
          if  (timer[i].art == 2)      // Wenn Tägliche Timer
          {
            TimerSchalte(i);
          }
          if  (((timer[i].art > 2) & timer[i].art) < 10)  // Wochentage Timer
          {
            if (weekday() == timer[i].art - 2 ) TimerSchalte(i); // Wochentag übereinsimmung
          }
        } // ende Tageszeit Überinstimmung
    }   // ende Timer aktiv
  }     // ende alle timer-einträge durchgehen
}

void Ereignis_NeueTimer()
{
  if (BenuzPasswort[0]) // Wenn Benutzerpasswort angegeben wurde
  {
    if (!server.authenticate(&BenuzName[0], &BenuzPasswort[0]))
      return server.requestAuthentication();
  }
  for (i = 0; i < 50; i++)        // Deaktivirte Timer Suchen
  {
    if (!timer[i].art)  break;        // Wenn Timer nich aktiv
  }
  //art=2&zeit=1480790953&relais=1&ein=0
  if (server.arg("art") != "")
  {
    String StrArt = server.arg("art");         // Relais Nr. empfangen
    timer[i].art = StrArt.toInt();
    String StrZeit = server.arg("zeit");         // Relais Nr. empfangen
    timer[i].zeit = StrZeit.toInt();
    String StrRelais = server.arg("relais");         // Relais Nr. empfangen
    timer[i].relais = StrRelais.toInt();
    String StrEin = server.arg("ein");
    timer[i].ein = (boolean)StrEin.toInt();
    TimerNr_speichern(i);
  }
  Ereignis_Timers_Zeigen();
}
