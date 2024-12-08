/* 08.12.2024
*/

#define ADR_Slave    0x08           // Адрес подчиненного устройства с которым будем работать
#define KONTR           2           //  (D2) Пин для сигнализации о низком напряжениии
#define T               A2          //  (A2) Пин для измерения температуры
#define U               A1          //  (A1) Пин для измерения напряжения
#define I               A0          //  (A0) Пин для измерения силы тока

#include <Wire.h>

int analogValue, analogCurrent, analogTemp, tmr = 0;
float voltage, current, temp; 
String str;
size_t msg_size = 0;

void setup() {
  pinMode(KONTR, OUTPUT);
  pinMode(T, INPUT);        // Конфигурация для концевика
  pinMode(U, INPUT);        // Конфигурация для концевика
  pinMode(I, INPUT);        // Конфигурация для концевика

  Wire.begin(8);                  // задаем на шине i2c 8 адрес 
  Wire.onReceive(receiveEvent);   // регистрируем полученное событие 
  Wire.onRequest(requestEvent);   // регистрируем запрошенное событие 
  Serial.begin(9600);             // открываем серийный порт для дебаггинга 
  delay(100);
}

int Scan_Pin(){
  analogValue = analogRead(U);                      // Чтение значения с пина 
  voltage = analogValue * (5.0 / 1023.0);           // Преобразование в напряжение (0-5 В)
  analogCurrent = analogRead(I);                    // Чтение значения с пина 
  current = analogCurrent * (5.0 / 1023.0) * 100;;  // Преобразование силы тока
  analogTemp = analogRead(T);                       // Чтение значения с пина 
  temp = analogTemp * (5.0 / 1023.0) * 100;         // Преобразование температуры
  return 0;
}

void loop() {
  Scan_Pin();
  str = String(voltage, 2) + ", " + 
        String(current, 2) + ", " +
        String(temp, 2);                    // Формируем строку

  if (str.length() > 32) {                  // Ограничиваем длину строки для передачи
    str = str.substring(0, 32);
  }

  Serial.println(str); // Для отладки
  delay(100);          // Период измерений
}

void receiveEvent(int howMany) {                        // Функция для извлечения любых принимаемых данных от мастера на шину
  int i=0;
  while (0 <Wire.available()) {                         // Если нам мастер что либо прислал то
    char c = Wire.read();                               // получаем байт как символ
 }
}
 
void requestEvent() {                                   // Функция для отправки строки мастеру 
  msg_size = str.length();                              // Определяем длину строки 
  if(tmr == 0){                                         // Переменная служит для определения в какую функцию входить, если она 0 то
    Wire.write((uint8_t*)&msg_size, sizeof(msg_size));  // отправляем по запросу от мастера длинну строки которую собираемся передать 
    if(msg_size == 0){                                  // если размер строки равен 0 то
      return;                      // выходим из функции
    }                              // если данные для передачи есть
    tmr++;                         // устанавливаем таймер на 1
    return;                        // и выходим из функции
  }                                // при следующем запросе от мастера мы начнем выполнять код с этого места так как таймер равен 1
  Wire.write(str.c_str());         // отправляем и саму строку которая записанна в строке str
  tmr=0;                           // сбрасываем таймер в 0
  str = "";                        // отчищаем строку от данных
}