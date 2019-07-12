<a href="http://mcm.ipg.pt"><img src="http://www.ipg.pt/website/imgs/logotipo_ipg.jpg" title="IPG(MCM)" alt="IPG MCM 2018/19"></a>


![121212](https://user-images.githubusercontent.com/2634610/61129937-74f4ab80-a4ad-11e9-88e8-814fa2e26bc2.png?s=100)
![MESH_b8](https://user-images.githubusercontent.com/2634610/61143102-379e1700-a4c9-11e9-8ffd-8e10c76f5f57.gif?s=200)

 <img src="https://user-images.githubusercontent.com/2634610/61129937-74f4ab80-a4ad-11e9-88e8-814fa2e26bc2.png" style="width: 100px;"/>

# LoRa-Mesh

Uma rede mesh é essencialmente caracterizada por uma tipologia em que todos os nós da infraestrutura de rede comunicam diretamente entre eles,  de forma dinâmica, sem ter em conta qualquer hierarquicamente. Assim, os nós cooperam uns com os outros, de forma a rotear os dados de forma eficiente. Essa falta de dependência entre nós permite que cada um participe na transmissão e receção de informação entre eles. 
As redes em mesh, podem-se organizar de forma dinâmica, através da capacidade que possuem em se autoconfiguração, permitindo a distribuição dinâmica de fluxos de informação, principalmente em situações de falhas num desses nós.


![66425186_342349106659933_6656448965128486912_n](https://user-images.githubusercontent.com/2634610/60983592-1a831000-a332-11e9-8a30-94f3efa8b5b1.png)

# Como instalar

1. Clonar este repositório:
    ``
      $ git clone https://github.com/daeynasvistas/LoRa-Mesh/
    ``
2. Utilizar VisualStudio Code (PlatformIO):

    2.1. pode abrir pasta ESP32 e pasta Arduino em projectos separados.         

## Como funciona

1. Todos os Nodes dispõem de um ID único (EEPROM)
2. Ao iniciar, é verificado se o Node é SINk (dispõe de acesso à Internet) ou não
```` C++
// Este node e servidor
// 0 = Servidor internet
// 1 = Vizinho de servidor internet
// 2 = Vizinho com um Vizinho de um servidor internet 
byte isServer = getIsServer();
String nodeFunction[4] = {"SINK","ESTRADA","CAMINHO","SOLTEIRO"};
````

3. Cada Node envia em BroadCast a sua tabela de Vizinhos a cada 10 mensagens recebidas
```` C++
     if(msgCount>10)
    {
      message = sendTable();
      sendMessage(message, 255);
```` 
4. Ao receber a Tabela é verificado se na mesma existe algum SINK (identificado no broadcast Como ID=0)
```` C++
     if(msgCount>10)
    {
      message = sendTable();
      sendMessage(message, 255);
```` 
5. É guardada a Tabela de SINK's conhecidas em cada node e conhecida por cada vizinho
```` C++
String nodeFunction[4] = {"SINK","ESTRADA","CAMINHO","SOLTEIRO"};
 ````
```` C++
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
```` 
   
6. É sempre enviado um JSON com valores dos sensores recolhidos para o vizinho mais perto de um SINK
```` C++
        }else{
           // enviar para mais próximo do SINK
           destination = myNeighbours[0];
           sendMessage(message, destination);
        }
```` 
## Configuração

Para configurar deve alterar main.cpp e incluir sensores que predende utilizar.
ESP32 inclui libraria para BME e Arduino para DHT11

#### método makeDate()
O projeto utiliza a libraria  #include <ArduinoJson.h> para construir o JSON que é enviado em cada Node, incluia ou altere os que pretender.

```` C++
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
```` 
 
#### valor máximo de Nodes e Sink
32 como máximo na tabela de vizinhos .. ou simplesmente alterar parra array dinámico

```` C++
byte const maxTableArrayVizinhos = 32; // quantidade de vizinhos pode ser aumentada conform memoria dispo
byte myNeighbours[maxTableArrayVizinhos] = {}; // address of vizinhos directos

byte const maxTableArrayServers = 4; // quantidade de servidores ao qual tenho acesso pode ser aumentada
byte myServers[maxTableArrayServers]     = {}; // address dos servidores que encontrei
```` 
    
## Resultado
Este é o resultado com Serial.Print no Node SINK, onde todos os valores dos sensores de todos os Nodes chegam. 
O node Sink é responsável por enviar o JSON... por Lora (gateway), por MQTT ou pelo método que preferir

#### Descoberta de um node (187)
![777777](https://user-images.githubusercontent.com/2634610/61075281-89836600-a411-11e9-86b7-f979d2ba8ab5.png)

#### Descoberta de um node (170)
![8888](https://user-images.githubusercontent.com/2634610/61075282-89836600-a411-11e9-9c52-c1d59b9e93ab.png)

#### Recolha dos valores de todos os Nodes
![99999](https://user-images.githubusercontent.com/2634610/61075280-89836600-a411-11e9-84ae-2b8dd8264a53.png)


## Resultado ESP32
![11111](https://user-images.githubusercontent.com/2634610/61074580-eed65780-a40f-11e9-9fa3-6fde67381412.png)
![44444](https://user-images.githubusercontent.com/2634610/61074581-eed65780-a40f-11e9-9940-dfcf29cd303c.png)
![222222](https://user-images.githubusercontent.com/2634610/61074582-eed65780-a40f-11e9-83aa-3f5635a4c156.png)
![55555](https://user-images.githubusercontent.com/2634610/61074779-673d1880-a410-11e9-966f-bb56eeb5b416.png)



