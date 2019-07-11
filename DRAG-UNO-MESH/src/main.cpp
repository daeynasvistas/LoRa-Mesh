// Comunicação LoRa com Arduino 
// Definicao das bibliotecas a serem utilizadas no projeto
#include <SPI.h>             
#include <LoRa.h>
#include <ArduinoJson.h>

// Definicacao de constantes
//const int csPin = 10;         // Chip Select (Slave Select do protocolo SPI) do modulo Lora
//const int resetPin = 0;       // Reset do modulo LoRa
//const int irqPin = 4;         // Pino DI0
 
String outgoing;              // outgoing message
byte localAddress = 0xAA;     // Endereco deste dispositivo LoRa
byte msgCount = 0;            // Contador de mensagens enviadas
byte msgHandshakeCount = 0;   // Contador de mensagens enviadas
byte destination = 0xFF;      // Endereco do dispositivo para enviar a mensagem (0xFF envia para todos devices)

long lastSendTime = 0;        // TimeStamp da ultima mensagem enviada
int interval = 15000;         // Intervalo em ms no envio das mensagens (inicial 5s)
 
boolean foundServer = false;

// Funcao que envia uma mensagem LoRa
void sendMessage(String outgoing) 
{
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(destination);              // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
    
  LoRa.write(1);

  LoRa.write(msgCount);                 // Contador da mensagem
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem 
  LoRa.endPacket();                     // Finaliza o pacote e envia
  msgCount++;                           // Contador do numero de mensagnes enviadas
}

// Funcao que envia uma mensagem LoRa
void sendHandshake(String outgoing, byte sender, byte isServer) 
{
  LoRa.beginPacket();                   // Inicia o pacote da mensagem
  LoRa.write(sender);                   // Adiciona o endereco de destino
  LoRa.write(localAddress);             // Adiciona o endereco do remetente
  LoRa.write(isServer);                 // isServer Id
  LoRa.write(msgHandshakeCount);        // Contador da mensagem handshake
  LoRa.write(outgoing.length());        // Tamanho da mensagem em bytes
  LoRa.print(outgoing);                 // Vetor da mensagem 
  LoRa.endPacket();                     // Finaliza o pacote e envia
  msgHandshakeCount++;                  // Contador do numero de mensagnes enviadas
  Serial.println("Enviei "+outgoing+" para: 0x" + String(sender, HEX));
  Serial.println("");  
}
 
// Funcao para receber mensagem 
void onReceive(int packetSize) 
{
  if (packetSize == 0) return;          // Se nenhuma mesnagem foi recebida, retorna nada
 
  // Leu um pacote, vamos decodificar? 
  int recipient = LoRa.read();          // Endereco de quem ta recebendo
  byte sender = LoRa.read();            // Endereco do remetente
  byte handShake = LoRa.read();         // Mensagem
  
  byte incomingMsgId = LoRa.read();     // Mensagem
  byte incomingLength = LoRa.read();    // Tamanho da mensagem
 
  String incoming = "";
 
  while (LoRa.available())
  {
    incoming += (char)LoRa.read();
  }
 
  if (incomingLength != incoming.length()) 
  {   
    // check length for error
    Serial.println("erro!: o tamanho da mensagem nao condiz com o conteudo!");
    return;                        
  }
 
  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF)
  {
    Serial.println("This message is not for me.");
    return;                      // skip rest of function
  }

  // Caso a mensagem seja para este dispositivo, imprime os detalhes
  Serial.println("Recebido do dispositivo: 0x" + String(sender, HEX));
  Serial.println("Enviado para: 0x" + String(recipient, HEX));
  Serial.println("ID da mensagem: " + String(incomingMsgId));
  Serial.println("Tamanho da mensagem: " + String(incomingLength));
  Serial.println("Mensagem: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();


   // Posicionamento dos servidores na mesh
    switch (handShake) {
    case 0:
        // statements
        Serial.println("Encontrei um SERVIDOR!");
        destination = sender;
        break;
    case 1:
        // statements
        Serial.println("Encontrei AUTOESTRADA para servidor!");
        destination = sender;
        break;
    case 2:
        // statements
        Serial.println("Encontrei CAMINHO para servidor!");
        destination = sender;
        break;        
    default:
        // statements
        break;
    }
 
 
}

//StaticJsonDocument<200> doc;

const size_t CAPACITY = JSON_ARRAY_SIZE(12);
StaticJsonDocument<CAPACITY> doc;
JsonArray array = doc.to<JsonArray>();
String Values;
void makeData(){
/*
// add some values
  doc["id"] = "1A0000000";
  doc["date_of_value"] = 1556969160;
  doc["air_temp"] = 23.1;
  doc["air_humidity"] = 28.7;
  doc["air_pressure"] = 982;
  doc["air_CO2"] = 1351824120;
  doc["air_TVOC"] = 468;
  doc["lux"] = 540;
  doc["flame"] = 10;
  doc["soil_humidity"] = 230;
  doc["sound"] = 40;
 */


// add some values
array.add("1F0000000");
array.add(1756969160);
array.add(29.3);
array.add(32.1);
array.add(232);
array.add(0);
array.add(0);
array.add(0);
array.add(0);
array.add(0);
array.add(0);

// serialize the array and send the result to Serial
  serializeJson(doc, Values);
  serializeJson(doc, Serial);
  Serial.println("");
}


// Setup do Microcontrolador
void setup() 
{
  makeData();
  // inicializacao da serial 
  Serial.begin(9600);                   
  while (!Serial);
 
  Serial.println(" Comunicacao LoRa Duplex - Mesh IPG ");
  // Inicializa o radio LoRa em 868MHz e cheka se esta ok!
  if (!LoRa.begin(868.1E6)) 
  {             
    Serial.println(" Erro ao iniciar modulo LoRa. Verifique a coenxao dos seus pinos!! ");
    while (true);                      
  }
  Serial.println(" Modulo LoRa iniciado com sucesso!!!  ");
}
 

// Loop do microcontrolador - Operacoes de comunicacao LoRa
void loop()
{
  if (millis() - lastSendTime > interval)
  { 
    Serial.println(Values);

    sendMessage(Values);
    lastSendTime = millis();           // timestamp the message
    interval = random(10000) + 10000;  // 10 seconds
    LoRa.receive();                    // go back into receive mode
  }
 
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    onReceive(packetSize);
  }

}