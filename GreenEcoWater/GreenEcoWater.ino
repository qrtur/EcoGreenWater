#include <EEPROM.h>

/*
 * Bibliotecas
 */
#include <Arduino.h>  

// Configuração ESP / Servidor web
#include <ESP8266WiFi.h>            
#include <ESP8266WebServer.h>

// Ler arquivos do arduino (servidor web)
#include <FS.h>

// Data e hora via internet
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

/*
 * Variaveis e instancias de objetos
 */
//Web Server
ESP8266WebServer server(80);    //Webserver Object

/*
 * WIFI
 */
//Configuração do WiFi Client - Navegação
String wifiClientSsid     = "XXXXXXXXX";
String wifiClientPassword = "XXXXXXXXX";

//Configuracao de WIFI - AP - WiFi para conexao direta
String wifiServerSsid     = "GreenEcoWater";
String wifiServerPassword = "XXXXXXXXX";


/*
 * TELEGRAM
 * 
Comandos do telegram
hello - Bot vai dar oi
solo_umidade - Umidade do solo atual
irrigar - Irrigar a terra
*/
#include "CTBot.h"
CTBot telegramBot;

//Token bot Telegram (GreenEcoWater_bot)
String tokenTelegram   = "XXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXX";
//Para quem enviar no telegram - 59732087
int64_t adminTelegram = 1654897945;
//Mensagem telegram
String stringTelegram;
TBMessage msgTelegram;

/*
 * Data e hora
 */
WiFiUDP ntpUDP;
long fusoHorario = -10800;
NTPClient timeClient(ntpUDP, "br.pool.ntp.org", fusoHorario);


/*
 * Sensores 
 */
// pino do sensor de Umidade de solo
const int pinoSensor       = A0;

// pino do rele da bomba d'agua
const int pinoBombaDagua   = D6;

/*
 * Variaveis de calibração do sensor de umidade
 */
// se estamos em um processo de irrigacao
int emProcessoDeIrrigacao = 0;

// verificar a leitura analógica do sensor quando o sensor estiver no ar seco
const int sensorSeco      = 1024;
// verificar a leitura analógica do sensor quando o sensor estiver na agua
const int sensorMolhado   = 770;

// variaveis de calculo do sensor de solo
int leituraUmidadeDoSolo  = 0;
int leituraPercentualUmidadeDoSolo = 0;

// Variaveis de configuração vindas da página web
// /api/saveConfig?&autoIrrigar=0&umidadeInicial=0&umidadeFinal=50&tempoIrrigacao=100&tempoEntreIrrigacao=1000
//0 desligado e 1 ligado
int autoIrrigar         = 0;
//percentual de umidade para iniciar irrigacao
int umidadeInicial      = 10;
//percentual de umidade para finalizar irrigacao
int umidadeFinal        = 50;
//umidade mínima registada
int umidadeMinima       = -1;
int umidadeMaxima       = -1;

//tempo para deixar irrigando
int tempoIrrigacao      = 100;
//tempo entre uma irrigacao e outra (para esperar o arduino ler)
int tempoEntreIrrigacao = 30000;

//variavel de controle para ver quando a bomba foi ativada pela ultima vez
int tempoUltimaAtivacaoBomba   = 0;

// controle de interacoes
int interacoes = 0;

int irrigacoesEfetuadas    = 0;
time_t dthrUltimaIrrigacao;

/*
   variaveis de calculo de litragem
*/
// Calculo litragem
float mililitroPorMilisegundo = 0.0188;

// Tempo para iniciar bomba (1200 ms)
int tempoDeInicioBomba = 1200;

// Total de miliLitrosIrrigacao
float totalMililitragemIrrigacao = 0;
// Total de ml da ultima irrigacao
float totalMililitroUltimaIrrigacao = 0;



void ligarBomba(int tempo){
   int tempoLigacaoTotal = tempoDeInicioBomba + tempo;
   totalMililitroUltimaIrrigacao = tempo * mililitroPorMilisegundo;

   
  //Liga bomba dagua 
  digitalWrite(pinoBombaDagua, LOW);
  
  //Esperar tempo de ligacao
  delay(tempoLigacaoTotal);
  
  //Desligar bomba dagua 
  digitalWrite(pinoBombaDagua, HIGH);
  
  // Atualizamos o total de água já irrigado
  totalMililitragemIrrigacao = totalMililitragemIrrigacao + totalMililitroUltimaIrrigacao;

  //Manda mensagem pro telegram
  stringTelegram = "Irrigação de "+String(totalMililitroUltimaIrrigacao)+" ml efetuada";
  telegramMensagemAdmin(stringTelegram);

  //Configura quando foi a ultima vez da ativacao da bomba
  tempoUltimaAtivacaoBomba = millis();

  //Soma o total de irrigações efetuadas
  irrigacoesEfetuadas++;

  //Configura data e hora da ultima irrigacao
  dthrUltimaIrrigacao = now();
}


void setup() {
  
  // Configura o pino do sensor como leitura
  pinMode(pinoSensor , INPUT);

  // Configura o pino da bomba d'agua
  pinMode(pinoBombaDagua, OUTPUT);
  digitalWrite(pinoBombaDagua, HIGH);

  //Configura ultima ativacao quando liga arduino
  tempoUltimaAtivacaoBomba = millis();
  
  //Gerencia de arquivos
  SPIFFS.begin();  
  
  //Inicia eeprom - salvar variaveis arduino
  EEPROM.begin(512);

  
  // Saída serial arduino x computador
  Serial.begin(115200);
  delay(2000);
  Serial.println("Arduino Iniciado");

  
  //Dump eeprom
  eepromDumpSerial();

  //Busca variáveis da EEPROM
  //autoIrrigar         = readIntFromEEPROM(0);
  tempoDeInicioBomba  = readIntFromEEPROM(2);
  umidadeInicial      = readIntFromEEPROM(4);
  umidadeFinal        = readIntFromEEPROM(6);
  tempoIrrigacao      = readIntFromEEPROM(8);
  tempoEntreIrrigacao = readIntFromEEPROM(10);
  
  //Configura WiFi como access point e cliente ao mesmo tempo
  WiFi.mode(WIFI_AP_STA);
  
  // Configura Access Point (wifi server)
  WiFi.softAP(wifiServerSsid, wifiServerPassword);
 
  // Configura WiFi Client (navegação internet)
  WiFi.begin(wifiClientSsid, wifiClientPassword);

  // Espera conexão WiFi
  Serial.print("Conectando WiFi ");
  Serial.print(wifiClientSsid);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }
  
  // Quando conectar no WiFi, diz ip local
  Serial.println();
  Serial.print("Conectado ao WiFi! IP address: ");
  Serial.println(WiFi.localIP());
  
  //Define o token do telegram
  telegramBot.setTelegramToken(tokenTelegram);

  //Verifica a conexao
  if (telegramBot.testConnection()){
    Serial.println("Conexao telegram OK");
    //Avisa IP no telegram
    IPAddress ip = WiFi.localIP();
    stringTelegram = "GreenEcoWater disponível em: " + String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    telegramMensagemAdmin(stringTelegram);
  } else {
    Serial.println("ERRO Conexão telegram");
  }

  //Atualiza data e horario
  Serial.println("Configurando data e hora via NTP");
  timeClient.begin();
  timeClient.update();
  //Configura horário no arduino
  setTime(timeClient.getEpochTime());
  Serial.println(getDataHora());


  //Funcoes do servidor webx
  server.on("/api/data",                    webApiData);
  server.on("/api/grafico",                 webApiGrafico);
  server.on("/api/teste",                   webApiTeste);
  server.on("/api/sairProccessoIrrigacao",  webSairProccessoIrrigacao);

  //carrega as configuracoes do ARDUINO para pagina WEB
  server.on("/api/loadConfig",  webApiLoadConfig);
  //salva as configuracoes do WEB para pagina ARDUINO
  server.on("/api/saveConfig",  webApiSaveConfig);

  //Configuração do servidor Web
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin(); //Start the server
}

void loop() {

  
  //Trata conexão WEB
  server.handleClient();

  //Atualiza sensor do solo
  Serial.println("====================================");
  leituraUmidadeDoSolo = analogRead(pinoSensor);
  Serial.print("Valor sensor de solo: ");
  Serial.println(leituraUmidadeDoSolo);

  //Calcula percentual de umidade do solo entre sensorSeco x sensorMolhado
  leituraPercentualUmidadeDoSolo = map(leituraUmidadeDoSolo, sensorSeco, sensorMolhado, 0, 100);

  if(leituraPercentualUmidadeDoSolo > 100){
    leituraPercentualUmidadeDoSolo = 100;
  }

  //Registra minima se for menor ou ainda não estiver sido preenchida
  if(umidadeMinima == -1 || leituraPercentualUmidadeDoSolo < umidadeMinima){
    umidadeMinima = leituraPercentualUmidadeDoSolo;
  }
  
  //Registra maxima se for maior ou ainda não estiver sido preenchida
  if(umidadeMaxima == -1 || leituraPercentualUmidadeDoSolo > umidadeMaxima){
    umidadeMaxima = leituraPercentualUmidadeDoSolo;
  }

  Serial.print("percentual de umidade do solo: ");
  Serial.print(leituraPercentualUmidadeDoSolo);
  Serial.println(" %");

  //Caso esteja ligado para auto irrigar
  if(autoIrrigar == 1){
    Serial.println("Auto irrigar ligado");
    //Caso esteja menor ou igual umidade inicial
    if(emProcessoDeIrrigacao == 0 and leituraPercentualUmidadeDoSolo <= umidadeInicial){
      stringTelegram = "Iniciando processo de irrigação";
      telegramMensagemAdmin(stringTelegram);
      Serial.println(stringTelegram);
      
      emProcessoDeIrrigacao = 1;
    } 
    //Caso esteja em processo de irrigacao 
    if(emProcessoDeIrrigacao == 1){
      //E chegou no percetual selecionado, não precisa mais irrigar
      if(leituraPercentualUmidadeDoSolo >= umidadeFinal){
        stringTelegram = "Chegamos no percentual desejado de umidade do solo, parando processo de irrigação\nUmidade do solo atual: "+String(leituraPercentualUmidadeDoSolo)+"%";
        telegramMensagemAdmin(stringTelegram);
        Serial.println(stringTelegram);

        emProcessoDeIrrigacao = 0;
      } else {
        //Vamos verificar se o tempo de agora e a ultima ativacao da bomba, ~é maior que 
        if((millis() - tempoUltimaAtivacaoBomba) >= tempoEntreIrrigacao){
          Serial.println("Ja aguardou tempo de irrigacao, vamos IRRIGAR");
          ligarBomba(tempoIrrigacao);
        } else {
          Serial.println("Aguardando tempo entre irrigacoes para ligar bomba");
        }
      }
    } 

    
  }

  //A cada 10 interacoes, verifica mensagens do telegram
  if((interacoes % 20) == 0){
    telegramCheckMessage();
  }

  
  interacoes++;
  delay(500);
}
