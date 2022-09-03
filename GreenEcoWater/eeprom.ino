/*
gravacao de memoria de eeprom - TABELA EEPROM
POS   Tamanho   Variavel            Descrição
0     2         AutoIrrigar         int 0=nao, 1=sim
2     2         tempoDeInicioBomba  int 
4     2         umidadeInicial      int 
6     2         umidadeFinal        int 
8     2         tempoIrrigacao      int 
10    2         tempoEntreIrrigacao int 
*/

void eepromDumpSerial(){
  Serial.println("------  EEPROM DUMP ------");
  for(int i=0; i<EEPROM.length(); i++){
    Serial.println("i = "+EEPROM.read(i));
  }
  Serial.println("------ /EEPROM DUMP ------");
  Serial.println("");
  
  Serial.println("------  Variaveis personalizadas EEPROM ------");
  Serial.println("");
  Serial.println("autoIrrigar");
  Serial.println(readIntFromEEPROM(0));
  
  Serial.println("");
  Serial.println("tempoDeInicioBomba");
  Serial.println(readIntFromEEPROM(2));
  
  Serial.println("");
  Serial.println("umidadeInicial");
  Serial.println(readIntFromEEPROM(4));
  
  Serial.println("");
  Serial.println("umidadeFinal");
  Serial.println(readIntFromEEPROM(6));
  
  Serial.println("");
  Serial.println("tempoIrrigacao");
  Serial.println(readIntFromEEPROM(8));
  
  Serial.println("");
  Serial.println("tempoEntreIrrigacao");
  Serial.println(readIntFromEEPROM(10));
  Serial.println("------ /Variaveis personalizadas EEPROM ------");
}

void writeLongIntoEEPROM(int address, long number){ 
  EEPROM.write(address, (number >> 24) & 0xFF);
  EEPROM.write(address + 1, (number >> 16) & 0xFF);
  EEPROM.write(address + 2, (number >> 8) & 0xFF);
  EEPROM.write(address + 3, number & 0xFF);
}

long readLongFromEEPROM(int address){
  return ((long)EEPROM.read(address) << 24) +
         ((long)EEPROM.read(address + 1) << 16) +
         ((long)EEPROM.read(address + 2) << 8) +
         (long)EEPROM.read(address + 3);
}

void writeIntIntoEEPROM(int address, int number){ 
  EEPROM.write(address,     number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
  EEPROM.commit();
}

int readIntFromEEPROM(int address){
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void saveStringIntoEEPROM(String mensagem, int inicio, int fim){
    for(int i=0; i<(fim-inicio); i++){
      EEPROM.write(i+inicio, mensagem[i]);
    }
    EEPROM.commit();
}

String readStringFromEEPROM(int inicio, int fim){
  String retorno = "";
  for (int i = 0; i < (fim-inicio); ++i){
    retorno += char(EEPROM.read(i+inicio));
  }
  return retorno;
}
