
/*
 * 
 * Funcoes de integração com telegram
 * 
Command configure botfather:
hello - Informações do sistema
status - Status da irrigação
irrigar - Forçar irrigação
ligarAutoIrrigacao - Ativa irrigação automatica
desligarAutoIrrigacao - Destiva irrigação automatica
 */

void telegramMensagemAdmin(String mensagem){
  telegramBot.sendMessage(adminTelegram, mensagem);
}

void telegramCheckMessage(){
    Serial.println("Verificando telegram");
    if (telegramBot.getNewMessage(msgTelegram)){
      Serial.println("===================================");
      if (msgTelegram.text.equalsIgnoreCase("/hello")){ 
        telegramHello(msgTelegram.sender.id);
      }
      if (msgTelegram.text.equalsIgnoreCase("/start")){
        telegramStart(msgTelegram.sender.id);
      }
      if (msgTelegram.text.equalsIgnoreCase("/status")){
        telegramStatus(msgTelegram.sender.id);
      }
      if (msgTelegram.text.equalsIgnoreCase("/irrigar")){ 
        telegramIrrigar(msgTelegram.sender.id);
      }
      if (msgTelegram.text.equalsIgnoreCase("/auto_on")){ 
        telegramConfiguraIrrigacao(msgTelegram.sender.id, 1);
      }
      if (msgTelegram.text.equalsIgnoreCase("/auto_off")){ 
        telegramConfiguraIrrigacao(msgTelegram.sender.id, 0);
      }
      Serial.println("=> Mensagem do telegram:");
      Serial.println(msgTelegram.text);
      Serial.println("===================================");
    }
}

void telegramConfiguraIrrigacao(int64_t idTelegram, int novoStatus){
  //caso seja o administrador
  if(idTelegram == adminTelegram){
    //Configura novo valor e envia stattus para o telegram
    autoIrrigar = novoStatus;
    telegramStatus(idTelegram);
  } else {
    //Avisa administrador que usuário tentou modificar  
    stringTelegram = "Você não é administrador para modificar a irrigação";
    telegramBot.sendMessage(msgTelegram.sender.id, stringTelegram);
    
    stringTelegram = "Usuário não é administrador e pediu mudar irrigação: "+String(msgTelegram.sender.id);
    telegramBot.sendMessage(adminTelegram, stringTelegram);
  }
}

void telegramHello(int64_t idTelegram){
  IPAddress ip = WiFi.localIP();
  stringTelegram  = "Olá, \n\n";
  stringTelegram += "disponível em: http://" + String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3])+"/,\nna rede wifi: "+wifiClientSsid+"\n\n";
  stringTelegram += "ou\n\n";
  stringTelegram += "disponível em: http://192.168.4.1/, \nna rede wifi: "+wifiServerSsid+", \ncom senha: "+wifiServerPassword+"\n";
  telegramBot.sendMessage(msgTelegram.sender.id, stringTelegram);
}

void telegramStart(int64_t idTelegram){
  IPAddress ip = WiFi.localIP();
  stringTelegram  = "Olá, \n\n";
  stringTelegram += "eu sou o bot do projeto GreenEcoWater, use o menu para enviar comandos. Exemplo:\n";
  stringTelegram += "/status\n";
  telegramBot.sendMessage(msgTelegram.sender.id, stringTelegram);
}

void telegramStatus(int64_t idTelegram){
  stringTelegram  = "STATUS\n";
  if(autoIrrigar == 1){
    stringTelegram  += "Auto-irrigar: Ligado\n";
  } else {
    stringTelegram  += "Auto-irrigar: Desligado\n";
  }
  if(irrigacoesEfetuadas > 0){
    stringTelegram  += "Irrigações efetuadas: "+String(irrigacoesEfetuadas)+"\n";
    stringTelegram  += "Data e hora última irrigação: "+String(getDataHora(dthrUltimaIrrigacao))+"\n";
  } else {
    stringTelegram  += "Irrigações efetuadas: nenhuma\n";
  }
  stringTelegram  += "Umidade início irrigação: "+String(umidadeInicial)+" %\n";
  stringTelegram  += "Umidade termino irrigação: "+String(umidadeFinal)+" %\n";
  stringTelegram  += "\n";
  stringTelegram  += "Umidade do solo atual: "+String(leituraPercentualUmidadeDoSolo)+"%\n";
  stringTelegram  += "Umidade mínima registrada: "+String(umidadeMinima)+" %\n";
  stringTelegram  += "Umidade máxima registrada: "+String(umidadeMaxima)+" %\n";
  stringTelegram  += "\n";
  stringTelegram  += "Total de água utilizado: "+String(totalMililitragemIrrigacao)+" ml\n";
  stringTelegram  += "Total de água última irrigação: "+String(totalMililitroUltimaIrrigacao)+" ml\n";
  
  telegramBot.sendMessage(msgTelegram.sender.id, stringTelegram);
}

void telegramIrrigar(int64_t idTelegram){
  if(idTelegram == adminTelegram){
    ligarBomba(100);
  } else {
    stringTelegram = "Você não é administrador para enviar irrigação";
    telegramBot.sendMessage(msgTelegram.sender.id, stringTelegram);
    
    stringTelegram = "Usuário não é administrador e pediu para irrrigar: "+String(msgTelegram.sender.id);
    telegramBot.sendMessage(adminTelegram, stringTelegram);
  }
}
