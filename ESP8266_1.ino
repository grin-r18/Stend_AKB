/* 
Модификация программы A1_Telemetry. Данная программа является сервером, к которой подключается клиент (ESP8266_2) Данные с этой программы посылаются оп COM порту 
и отображаются в таблице EXEL.
08.12.2024
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

int pin = 2;                    // Пин на котором находится синий светодиод на плате
int msg_sizeG=0;
static uint32_t tmr;            // Переменная хранящая значение таймера
const char* ssid = "W";
const char* password = "";
const int serverPort = 1666;

IPAddress staticIP(192,168,5,115);
IPAddress gateway(192,168,5,1);
IPAddress subnet(255,255,255,0);

WiFiServer server(serverPort);
WiFiClient client;

void setup() {
  pinMode(pin, OUTPUT);                                      // Переключаем контакт GPIO2 в режим вывода данных (OUTPUT):
  digitalWrite(pin, LOW);                                    // включаем светодиод, переключая напряжение на «LOW»
  delay(1000);
  Serial.begin(9600);                                    
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(staticIP, gateway, subnet);
  WiFi.softAP(ssid, password);
  IPAddress IP=WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();
  Serial.println("\nWaiting for connection to access points...");
  do{
    /*--------Светодиод будет выключен пока мы не подключимся к точке доступа----------------------------*/
    digitalWrite(pin, HIGH);            // выключаем светодиод («HIGH» - это высокий уровень напряжения)
    delay(500);
  } while(!WiFi.softAPgetStationNum());//Пока к станции не подключен ни один клиент выполняем код в цикле
}

void loop() {
  client = server.available();
  if (client) {
    Serial.print("Client ");
    Serial.print(client.remoteIP());
    Serial.println(" connected");
    digitalWrite(pin, LOW);              // включаем светодиод, переключая напряжение на «LOW»
    while(client.connected()) {
      /*----------------Получение данных от сервера--------------------------------------------------------------------------------------*/
      if (client.available()) {
        int msg_size = 0;
        if (client.read((uint8_t*)&msg_size, sizeof(int)) == sizeof(int)) {
          char* msg = new char[msg_size + 1];
          if (client.readBytes(msg, msg_size) == msg_size) {
            msg[msg_size] = '\0';
            Serial.println(msg);         // передача сообщения в COM порт
          }
          delete[] msg;
        }
       }
     } 
     digitalWrite(pin, HIGH);            // выключаем светодиод («HIGH» - это высокий уровень напряжения)
     Serial.println("Client disconnected");
     client.stop();
     delay(2000);
     setup();
   }
}


  
