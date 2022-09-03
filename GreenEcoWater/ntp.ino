/*
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// Configura client NTP
WiFiUDP ntpUDP;
// GMT -3 = 3*3600 = 10800 (negativo)
long fusoHorario = -10800;
NTPClient timeClient(ntpUDP, "br.pool.ntp.org");


//No setup()
  // Initialize a NTPClient to get time
  timeClient.begin();
  

*/

String getDataHora(time_t dataHora){
  String retorno = "";
  retorno += getDigits(day(dataHora))     +"/";
  retorno += getDigits(month(dataHora))   +"/";
  retorno += String(year(dataHora))       +" ";
  retorno += getDigits(hour(dataHora))    +":";
  retorno += getDigits(minute(dataHora))  +":";
  retorno += getDigits(second(dataHora));
  return retorno;
}

String getDataHora(){
  String retorno = "";
  time_t dataHora = now();
  retorno += getDigits(day(dataHora))     +"/";
  retorno += getDigits(month(dataHora))   +"/";
  retorno += String(year(dataHora))       +" ";
  retorno += getDigits(hour(dataHora))    +":";
  retorno += getDigits(minute(dataHora))  +":";
  retorno += getDigits(second(dataHora));
  return retorno;
}

void serialDataHora(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
  
  Serial.println("Forrmato brasileiro"); 
  getDataHora(now());
}

String getDigits(int digits){
  if(digits < 10){
    return String("0"+String(digits));
  } else {
    return String(digits);
  }
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
