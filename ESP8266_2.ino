/* 
Модификация программы A2_Telemetry программа работает как клиент, считывает данные по шине i2c с устройства и отправляет на сервер, и ждет ответа от сервера.
08.12.2024
*/
#define ADR_Slave    0x08                   // Адрес подчиненного устройства с которым будем работать 

#include <ESP8266WiFi.h> 
#include <WiFiClient.h>
#include <Wire.h>                         // Бмблиотека для работы с шыной i2c

byte pin = 2;                               // Пин на котором находится синий светодиод на плате
int msg_sizeG=0;
static uint32_t tmr;                        // Переменная хранящая значение таймера
static uint32_t tmrled=0;                   // Преременная для функции мигнания светодиодом от зависания
const char* ssid = "W";                     // Название вашей WiFi сети
const char* password = "";                  // Пароль от вашей WiFi сети
const IPAddress serverIP(192, 168, 5, 115); // IP адрес сервера
const int serverPort = 1666;

WiFiClient client;
IPAddress clientIP(192, 168, 5, 2);         // IP адрес клиента

void setup() {
  pinMode(pin, OUTPUT);                     // Переключаем контакт GPIO2 в режим вывода данных (OUTPUT):
  digitalWrite(pin, HIGH);                  // вылючаем светодиод («HIGH» - это высокий уровень напряжения)
  Serial.begin(9600);
  Wire.begin();                             // Инициализация шыны i2c
  delay(1000);
  
  // Подключение к Wi-Fi сети
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Подключение к серверу
  Serial.print("Connecting to server: ");
  Serial.println(serverIP);
  if(client.connect(serverIP, serverPort) > 0) {}
}

void loop() {
  if(client.connected()) {             // Если клиент подключен
    digitalWrite(pin, LOW);                                  // включаем светодиод, переключая напряжение на «LOW»
    /*--------------Считывание данных с slave устройства каждые 1000 мс-----------------------------------------------------*/
    if(millis() - tmr >= 1000) {                            // Значения обновляются каждые ... мс, согласно таймера
      Wire.requestFrom(ADR_Slave, 1);                      // запрашиваем и считываем 1 байт с адреса slave устройства который содержит информацию сколько байт мы передадим в дальнейшем
      if(Wire.available()){                                // Проверка доступность Slave устройства 
        msg_sizeG = Wire.read();
        if(msg_sizeG > 0 ) {                                // Если сообщение больше 0
          char* buf_RX= new char[msg_sizeG+1];              // выделяем память под сообщение
          if (buf_RX != nullptr) {                         // Проверка, что память успешно выделена
            int i=0;                                       // переменная для записи принятых символов в массив
            Wire.requestFrom(ADR_Slave, msg_sizeG);         // запрашиваем с адреса слейва сообщение равное колличеству байт которое мы узнали в предидущем шаге
            while(Wire.available() && i <msg_sizeG){                     // выполняем пока буфер не опустеет
              buf_RX[i]= Wire.read();                      // пишем символ в массив
              i++;                                         // увеличиваем счетчик переменных 
            }
            buf_RX[msg_sizeG] = '\0';                       // Завершаем строку нулевым символом для корректной работы функций для работы со строками
            Serial.println(buf_RX);                         // Выводим полученное сообщение в Serial
            /*-------------Отправляем данные серверу-------------------------------------------------------------------------------------------*/
            if (msg_sizeG > 0) {
              client.write((char*)&msg_sizeG, sizeof(int)); // Отправляем размер сообщения клиенту
              client.write(buf_RX, msg_sizeG);      // Отправляем само сообщение клиенту
            }
            /*---------------------------------------------------------------------------------------------------------------------------------*/
            delete[] buf_RX;                         // Отчищаем выделенную память
          } else {                                   // Если память выделить не удалось
              Serial.println("Error give Memory...");// Выводим сообщение о ошибке выделения памяти
          }
        } else {                                     // Если сообщение меньше либо равно нулю
            //Serial.print(".");                       // ожидаем сообщения
        }
      } else {                                     // Если Slave устройство недоступно
          Serial.println("Error slave devices...");// Выводим сообщение об ошибке
      }
      /*-------------Сброс таймера на повторение выполнения цикла------------------------------------------------------------------------*/
      tmr = millis();
    }
    /*----------------Получение данных от сервера--------------------------------------------------------------------------------------*/
    if (client.available()) {
      int msg_size = 0;
      if (client.read((uint8_t*)&msg_size, sizeof(int)) == sizeof(int)) {
        char* msg = new char[msg_size + 1];
        if (client.readBytes(msg, msg_size) == msg_size) {
          msg[msg_size] = '\0';
          Serial.println(msg);
          /*--------------Передача сообщения слейву-------------------------------------------------------------------------------*/
          Wire.beginTransmission(ADR_Slave);       // Начинаем передачу на адресе ADR_Slave 
          Wire.write(msg);                         // Отправляем длинну считанной строки
          Wire.endTransmission();                  // прекращаем передачу
          /*----------------------------------------------------------------------------------------------------------------------*/
        }
        delete[] msg;
      }
    } else {
    }
  } else {
      Serial.println("Disconnect...");
      digitalWrite(pin, HIGH);                                      // вылючаем светодиод («HIGH» - это высокий уровень напряжения)
      delay(2000);
      setup();
  }
}