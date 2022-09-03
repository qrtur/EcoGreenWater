
/*
 * 
 * Funcoes do Servidor WEB
 * 
 */
String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

void webApiData() { 
  String message = "{\n";
  message += "\"umidadeSolo\":"+String(leituraPercentualUmidadeDoSolo)+",\n";
  message += "\"valorLeitorUmidadeSolo\":"+String(leituraUmidadeDoSolo)+",\n";
  message += "\"totalMililitros\":"+String(totalMililitragemIrrigacao)+",\n";
  message += "\"autoIrrigar\":"+String(autoIrrigar)+",\n";
  message += "\"totalMl\":"+String(totalMililitroUltimaIrrigacao)+",\n";
  message += "\"umidadeInicial\":"+String(umidadeInicial)+",\n";
  message += "\"umidadeFinal\":"+String(umidadeFinal)+",\n";
  if(totalMililitroUltimaIrrigacao > 0){
    message += "\"totalMlUltimaIrrigacao\":"+String(totalMililitroUltimaIrrigacao)+",\n";
    message += "\"dataHoraUltimaIrrigacao\":\""+getDataHora(dthrUltimaIrrigacao)+"\",\n";
    
  }
  message += "\"umidadeMinima\":"+String(umidadeMinima)+",\n";
  message += "\"umidadeMaxima\":"+String(umidadeMaxima)+"\n";
  message += "}";
  server.send(200, "application/json", message);          //Returns the HTTP response
}

void webApiGrafico() { 
  String message = "adicionaDadosGrafico("+String(leituraPercentualUmidadeDoSolo)+");";

  //Carrega informacoes de umidade do solo
  message += "document.getElementById('textoUmidade').innerHTML = '"+String(leituraPercentualUmidadeDoSolo)+"%';";
  
  //Litros utilizados
  message += "document.getElementById('textoLitros').innerHTML = '"+String(totalMililitragemIrrigacao)+" ml';";
  
  //Status
  if(autoIrrigar == 1){
    if(emProcessoDeIrrigacao == 1){
      message += "document.getElementById('textoStatus').innerHTML = 'Irrigação automatica LIGADA e em processo de irrigação';";
    } else {
      message += "document.getElementById('textoStatus').innerHTML = 'Irrigação automatica LIGADA e fora de processo de irrigação, aguardando chegar em "+String(umidadeInicial)+"% para iniciar nova irrigação';";
    }
  } else {
    message += "document.getElementById('textoStatus').innerHTML = 'Irrigação automatica desligada';";
  }

  //Se houver uma irrigação adiciona status
  if(totalMililitroUltimaIrrigacao > 0){
    message += "document.getElementById('textoStatus').innerHTML += '<br><br><b>Total ml última irrigação</b>: "+String(totalMililitroUltimaIrrigacao)+" ml';";
    message += "document.getElementById('textoStatus').innerHTML += '<br><b>Data e hora última irrigação</b>: "+getDataHora(dthrUltimaIrrigacao)+"';";
  }

  //Umidade mínima e máxima registrada
  message += "document.getElementById('textoStatus').innerHTML += '<br><br><b>Umidade mínima registada</b>: "+String(umidadeMinima)+" %';";
  message += "document.getElementById('textoStatus').innerHTML += '<br><b>Umidade máxima registada</b>: "+String(umidadeMaxima)+" %';";

  
  server.send(200, "text/plain", message);          //Returns the HTTP response
}


void webApiSaveConfig(){
  String message = "";
  /*
   * VARIAVEL QUE VAI VIR POR GET
    autoIrrigar=0
    umidadeInicial=0
    umidadeFinal=50
    tempoIrrigacao=100
    tempoEntreIrrigacao=1000
    ----
    VARIAVEL DENTRO ARDUINO
    //0 desligado e 1 ligado
    int autoIrrigar         = 0;
    //percentual de umidade para iniciar irrigacao
    int umidadeInicial      = 0;
    //percentual de umidade para finalizar irrigacao
    int umidadeFinal        = 50;
    //tempo para deixar irrigando
    int tempoIrrigacao      = 100;
    //tempo entre uma irrigacao e outra (para esperar o arduino ler)
    int tempoEntreIrrigacao = 30000;
  */
  autoIrrigar         = server.arg ("autoIrrigar").toInt();
  umidadeInicial      = server.arg ("umidadeInicial").toInt();
  umidadeFinal        = server.arg ("umidadeFinal").toInt();
  tempoIrrigacao      = server.arg ("tempoIrrigacao").toInt();
  tempoEntreIrrigacao = server.arg ("tempoEntreIrrigacao").toInt();
  tempoDeInicioBomba  = server.arg ("tempoDeInicioBomba").toInt();

  if(umidadeInicial >= umidadeFinal || umidadeFinal < 10 || umidadeFinal > 100 || umidadeInicial < 0 || umidadeInicial >= 100){
    message = "alert('Erro: Valores inválidos para umidade inicial e final. Umidade final deve ser maior que umidade inicial.');";
  } else {
    writeIntIntoEEPROM(0,  autoIrrigar);
    writeIntIntoEEPROM(2,  tempoDeInicioBomba);
    writeIntIntoEEPROM(4,  umidadeInicial);
    writeIntIntoEEPROM(6,  umidadeFinal);
    writeIntIntoEEPROM(8,  tempoIrrigacao);
    writeIntIntoEEPROM(10, tempoEntreIrrigacao);
    
    message = "alert('Variaveis salvas no arduino');";
  }
  server.send(200, "text/plain", message); //Returns the HTTP response


  
  
}

void webApiLoadConfig(){
  
  String message = "";
  //Auto Irrigar
  if(autoIrrigar == 1){
    message += "document.getElementById('formAutoIrrigarON').checked = true;";
  } else {
    message += "document.getElementById('formAutoIrrigarOFF').checked = true;";
  }

  //Slider da umidade inicial
  message += "document.getElementById('sliderumidadeInicial').value = '"+String(umidadeInicial)+"';";
  message += "document.getElementById('sliderumidadeInicial').onchange();";
  
  //Slider da umidade final
  message += "document.getElementById('sliderumidadeFinal').value = '"+String(umidadeFinal)+"';";
  message += "document.getElementById('sliderumidadeFinal').onchange();";
  
  //Tempo irrigacao
  message += "document.getElementById('slidertempoIrrigacao').value = '"+String(tempoIrrigacao)+"';";
  message += "document.getElementById('slidertempoIrrigacao').onchange();";
  
  //Tempo entre as irrigacoes
  message += "document.getElementById('slidertempoEntreIrrigacao').value = '"+String(tempoEntreIrrigacao)+"';";
  message += "document.getElementById('slidertempoEntreIrrigacao').onchange();";
  
  //Tempo inicio bomba
  message += "document.getElementById('slidertempoDeInicioBomba').value = '"+String(tempoDeInicioBomba)+"';";
  message += "document.getElementById('slidertempoDeInicioBomba').onchange();";

  //Configurar a pagina para pegar informacoes do grafico
  message += "consultaGrafico();";
  
  
  server.send(200, "text/plain", message);          //Returns the HTTP response
}

void webApiTeste() { 
  int tempoTeste = 100;
  if(server.arg("tempo").toInt() > 0){
    tempoTeste = server.arg("tempo").toInt();
    
    Serial.println("Configurando tempo de teste para:");
    Serial.println(tempoTeste);
  }
  if(server.arg("totalMl").toInt() > 0){
    tempoTeste = server.arg("totalMl").toInt() / mililitroPorMilisegundo;
    
    Serial.println("Configurando tempo de teste para atender "+server.arg("totalMl")+" ml:");
    Serial.println(tempoTeste);
  }
  
  Serial.println("Efetuando teste da bomba dagua");

  ligarBomba(tempoTeste);
  
  String message = "alert('Teste da bomba agua efetuado, ml desta irrigação: "+String(totalMililitroUltimaIrrigacao)+", total já irrigado: "+String(totalMililitragemIrrigacao)+"');";
  server.send(200, "text/plain", message);
}

void webSairProccessoIrrigacao(){
  emProcessoDeIrrigacao = 0;
  
  String message = "alert('Processo de irrigacao reiniciado');";
  server.send(200, "text/plain", message);
}
