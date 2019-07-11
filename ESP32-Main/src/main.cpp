/*
  LoRa Duplex communication Mesh - Sending multiple sensor Values to SINK
  Note: while sending, LoRa radio is not listening for incoming messages.
  IPG - MCM 2019
  SENDING NODE
  0xFF como BroadCast
*/
#include <SPI.h>      // include libraries
#include <LoRa.h>    
#include "SSD1306.h"
#include <ArduinoJson.h>
//SENSORES
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// BME280
//#define SEALEVELPRESSURE_HPA (1032.00)
Adafruit_BME280 bme; // I2C
// 811
//#include "Adafruit_CCS811.h"
//Adafruit_CCS811 ccs;

#define PIN_SDA 4
#define PIN_SCL 15

//#include <AsyncDelay.h>
//#include <SoftWire.h>
//SoftWire sw(PIN_SDA, PIN_SCL);


//Pinout! Customized for TTGO LoRa32 V2.0 Oled Board!
#define SX1278_SCK  5    // GPIO5  -- SX1278's SCK
#define SX1278_MISO 19   // GPIO19 -- SX1278's MISO
#define SX1278_MOSI 27   // GPIO27 -- SX1278's MOSI
#define SX1278_CS   18   // GPIO18 -- SX1278's CS
#define SX1278_RST  14   // GPIO14 -- SX1278's RESET
#define SX1278_DI0  26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

#define OLED_ADDR   0x3c  // OLED's ADDRESS
#define OLED_SDA  4
#define OLED_SCL  15
#define OLED_RST  16

#define LORA_BAND   868.1E6 // LoRa Band (Europe)
#define PABOOST true

// LoRaWAN Parameters
#define TXPOWER 14
#define SPREADING_FACTOR 12
#define BANDWIDTH 125000
#define CODING_RATE 5
#define PREAMBLE_LENGTH 8
#define SYNC_WORD 0x34
SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);


#define LEDPIN 2
uint64_t chipid;
String MAC;


//////////////////////CONFIG 1///////////////////////////

// Este node e servidor
// 0 = Servidor internet
// 1 = Vizinho de servidor internet
// 2 = Vizinho com um Vizinho de um servidor internet 
byte isServer = 0;
String nodeFunction[4] = {"SINK","ESTRADA","CAMINHO","SOLTEIRO"};

byte const maxTableArrayVizinhos = 32; // quantidade de vizinhos pode ser aumentada conform memoria dispo
byte myNeighbours[maxTableArrayVizinhos] = {}; // address of vizinhos directos

byte const maxTableArrayServers = 4; // quantidade de servidores ao qual tenho acesso pode ser aumentada
byte myServers[maxTableArrayServers]     = {}; // address dos servidores que encontrei

byte localAddress = 101;     // Este sou eu!!!!! 
byte destination = 0xFF;     // Destino original broadcast (0xFF broad)

int interval = 4000;       // interval between sends
String message = "Hello";    // send a message
///////////////////////////////////////////////////////

byte msgCount     = 0;        // count of outgoing messages
long lastSendTime = 0;        // last send time

/*
//////////////////////CONFIG 2///////////////////////////
byte localAddress = 18;    // address of this device
byte destination = 8;      // destination to send to
int interval = 2000;       // interval between sends
String message = "Pong!"; // send a message
///////////////////////////////////////////////////////
*/

// generate random message simulate Sensor
String rndMsg(){
  String randomTemp = String(random(20,860)/ 10.0);
  String randomHum = String(random(200,900));
  String JSON = "{\"T\":"+randomTemp+",\"H\":"+randomHum+"}";
  return String(JSON);
}
String otherValues= "";

String otherNodesMsg(String value){
 otherValues = value;
 // String JSON = "{\"T\":"+randomTemp+",\"H\":"+randomHum+"}";
  return String(value);
}



 boolean arrayIncludeElement(byte array[], byte element, byte max) {
 for (int i = 0; i < max; i++) {
      if (array[i] == element) {
          return true;
      }
    }
  return false;
 }

 
 void arrayAddElement(byte array[], byte element, byte max) {
 for (int i = 0; i < max; i++) {
      if (array[i] == 0) {
          array[i] = element;
          return;
      }
    }
 }


void printVizinhos(){
  Serial.print("Vizinhos: {");
  for (int i = 0; i < sizeof(myNeighbours); i++) { 
     Serial.print(String(myNeighbours[i]));  Serial.print(" ");
    }  Serial.println("}"); 

  Serial.print("Sink: {");
  for (int i = 0; i < sizeof(myServers); i++) { 
     Serial.print(String(myServers[i]));     Serial.print(" ");
    }  Serial.println("}"); 
}




void printScreen() {
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.setColor(BLACK);
  display.fillRect(0, 0, 127, 30);
  display.display();
  display.setColor(WHITE);
 // display.drawString(0, 00, String(LORA_BAND/1000000)+" LoRa sender " + String(localAddress));
  display.drawString(0, 00, String(LORA_BAND/1000000)+" LoRa " + nodeFunction[isServer]);
  display.drawString(0, 10,"Me: " + String(localAddress)
                          + " To: " + String(destination)
                          + " N: " + String(msgCount));
  display.drawString(0, 20, "Tx: " + message);
  display.display();
}

void sendMessage(String outgoing, byte destination) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(isServer);                 // add server ID
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  printScreen();
  
  Serial.println("Enviar Mensagem " + String(msgCount) + " para Node: "+ String(destination));
  Serial.println("Mensagem: " + message); 
  Serial.println();
  delay(1000);
  msgCount++;                           // increment message ID
}





void onReceive(int packetSize) {
 // Serial.println("error");
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgHand = LoRa.read();     // incoming msg ID
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  if (incomingLength != incoming.length()) {   // check length for error
    Serial.println("error: message length does not match length");
    incoming = "message length error";
    return;                             // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("Esta mensagem não é para mim.");
    incoming = "message is not for me";
    message= incoming;
    printScreen();
    delay(150);
    return;                             // skip rest of function
  }

  display.setColor(BLACK);
  display.fillRect(0, 32, 127, 61);
  display.display();

  display.setColor(WHITE);
  display.drawLine(0,31,127,31);
//  display.drawString(0, 32, "Rx: " + incoming);

  //Novo vizinho
  if(!arrayIncludeElement(myNeighbours,sender,maxTableArrayVizinhos)){
    arrayAddElement(myNeighbours,sender,maxTableArrayVizinhos);
    display.drawString(0, 32, "NOVO: " + String(sender)); 
  }
  display.drawString(0, 42, "FR:"  + String(sender)
                          + " TO:" + String(recipient)
                          + " LG:" + String(incomingLength)
                          + " ID:" + String(incomingMsgId));
  display.drawString(0, 52, "RSSI: " + String(LoRa.packetRssi())
                          + " SNR: " + String(LoRa.packetSnr()));

  display.display();
  printVizinhos();

  // if message is for this device, or broadcast, print details:
  Serial.println("Handshake: "+String(incomingMsgHand));

  Serial.println("Recebido de: 0x" + String(sender, HEX));
  Serial.println("Enviar Para: 0x" + String(recipient, HEX));
  Serial.println("Mensagem ID: " + String(incomingMsgId));
  Serial.println("Mensagem length: " + String(incomingLength));
  Serial.println("Msg: " + incoming);
  //--->> Guardar Valores--->>
  otherValues += incoming;
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
  delay(1000);
 

  // Posicionamento dos servidores na mesh
   switch (incomingMsgHand) {
   case 0:
       // statements
        if(!arrayIncludeElement(myServers,sender,maxTableArrayServers)){
           Serial.println("Encontrei um SINK! "+sender);
           arrayAddElement(myServers,sender,maxTableArrayServers);
           display.drawString(0, 32, "NOVO: " + String(sender)); 
        }
      destination = sender;
      break;
   case 1:
       // statements
        if(!arrayIncludeElement(myNeighbours,sender,maxTableArrayVizinhos)){
           Serial.println("Encontrei AUTOESTRADA para SINK! "+sender);
           arrayAddElement(myNeighbours,sender,maxTableArrayVizinhos);
           display.drawString(0, 32, "NOVO: " + String(sender)); 
        }
        if(isServer!=0){
          destination = sender;
        }
       break;
   case 2:
       // statements
       Serial.println("Encontrei CAMINHO para SINK!");
       break;       
   default:
       // statements
       break;
   }

}


void configForLoRaWAN()
{
  LoRa.setTxPower(TXPOWER);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE);
  LoRa.setPreambleLength(PREAMBLE_LENGTH);
  LoRa.setSyncWord(SYNC_WORD);
  LoRa.crc();
}


const size_t CAPACITY = JSON_ARRAY_SIZE(12);
StaticJsonDocument<CAPACITY> doc;
JsonArray array = doc.to<JsonArray>();
String Values;

void makeData(){
// add some values
array.add(MAC); //<- Lora MAC
array.add(1556969160); //<-- Timestamp
array.add(bme.readTemperature());
array.add(bme.readHumidity());
array.add(bme.readPressure() / 100.0F);
array.add(0);
array.add(0);
array.add(0);
array.add(0);
array.add(0);
array.add(0);

 // serialize the array and send the result to Serial
// serialize the array and send the result to Serial
 serializeJson(doc, Values);
 serializeJson(doc, Serial);
 Serial.println("");
}


void setup() {
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);  // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH); // while OLED running, must set GPIO16 in high
  delay(1000);

  //makeData();
  // Use the Blue pin to signal transmission.
  pinMode(LEDPIN,OUTPUT);


  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.clear();

  Serial.begin(115200);                   // initialize serial
  while (!Serial);
  Serial.println("IPG SFarm LoRa V0.5");
  display.drawString(0, 00, "IPG SFarm MeshLoRa V0.5");
  display.display();

  LoRa.setPins(SX1278_CS, SX1278_RST, SX1278_DI0);// set CS, reset, IRQ pin]52qw2ws

    // should be done before LoRa.begin
  configForLoRaWAN();

  if (!LoRa.begin(LORA_BAND))
  {             // initialize ratio at 868 MHz
    Serial.println("LoRa init failed. Check your connections.");
    display.drawString(0, 10, "LoRa init failed");
    display.drawString(0, 20, "Check connections");
    display.display();
    
    while (true);                       // if failed, do nothing
  }

  //LoRa.onReceive(onReceive);
  LoRa.receive();
 
  if (!bme.begin(0x76))
  {
    display.clear();
    display.drawString(0, 0, "Sensor não encontrado");
    display.display();
    while(1);
  }
/*
  if (!ccs.begin())
  {
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  Serial.println("Failed to start sensor! Please check your wiring.");
  display.clear();
  display.drawString(0, 0, "Failed to start sensor CO2!");
  display.display();
  while(1);
  }
  */
  Serial.println("LoRa init succeeded.");
  display.drawString(0, 10, "LoRa init succeeded.");
  display.drawString(0, 20, "LoRa Mesh init...");
  display.display();
  delay(1500);
//  display.clear();
//  display.display();

// MAC do LoRa
chipid=ESP.getEfuseMac();
//Serial.printf("ESP32 Chip ID = %04X",(uint16_t)(chipid>>32));//print High 2 bytes
//Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.

MAC = String((uint16_t)(chipid>>32), HEX); 
MAC += String((uint32_t)chipid, HEX);
Serial.println(MAC);

}

String sendTable(){
  const size_t CAPACITY = JSON_ARRAY_SIZE(4);
      StaticJsonDocument<CAPACITY> doc;
      JsonArray array = doc.to<JsonArray>();
        for (int i = 0; i < sizeof(myServers); i++) { 
            array.add(myServers[i]);
       }
      String Values;
      serializeJson(doc, Values);
      return Values;
}


void printSensor(){
  
  Serial.print("Temperature = ");
	Serial.print(bme.readTemperature());
	Serial.println("*C");

	Serial.print("Pressure = ");
	Serial.print(bme.readPressure() / 100.0F);
	Serial.println("hPa");

	Serial.print("Humidity = ");
	Serial.print(bme.readHumidity());
	Serial.println("%");
 
/* if(ccs.available()){
    if(!ccs.readData()){
    float temp = ccs.calculateTemperature();
      int co2 = ccs.geteCO2();
      int tvoc = ccs.getTVOC();
      Serial.println(co2);
      Serial.println(tvoc);
      Serial.println(temp);
    }
  }
   */
}

void loop() {
  if (millis() - lastSendTime > interval) {


  //  Serial.print("Destino = ");
  //  Serial.println(destination);
  
    Serial.println(msgCount);
    if(msgCount>10)
    {
      message = sendTable();
      sendMessage(message, 255);
      //<<<--- enviar Todos os valores de todos os Nodes
      //SendValues(otherValues);
      otherValues="";
      msgCount = 0;    // increment message ID

    }else{
      message = Values;
        if (isServer==0){ 
           // enviar para TTN
           digitalWrite(LEDPIN, HIGH);
          // printSensor(); 
          otherValues += Values;
          Serial.print(otherValues);

        }else{
           // enviar para mais próximo do TTN em random
            digitalWrite(LEDPIN, LOW);
           destination = myNeighbours[0];
           sendMessage(message, destination);
           printSensor();
        }

    }
    msgCount++;                           // increment message ID

    lastSendTime = millis();            // timestamp the message
    interval = random(interval) + 20000;     // 20-30 seconds
    LoRa.receive();                     // go back into receive mode
    makeData();
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize) { onReceive(packetSize);  }
}

