/*
https://github.com/GyverLibs/GyverFilters


    GFilterRA - компактная альтернатива фильтра экспоненциальное бегущее среднее (Running Average)
    GMedian3 - быстрый медианный фильтр 3-го порядка (отсекает выбросы)
    GMedian - медианный фильтр N-го порядка. Порядок настраивается в GyverFilters.h - MEDIAN_FILTER_SIZE
    GABfilter - альфа-бета фильтр (разновидность Калмана для одномерного случая)
    GKalman - упрощённый Калман для одномерного случая (на мой взгляд лучший из фильтров)
    GLinear - линейная аппроксимация методом наименьших квадратов для двух массивов
    FastFilter - быстрый целочисленный экспоненциальный фильтр
    RingAverage - бегущее среднее с кольцевым буфером (не работает для float!)


*/




#define RX1_PIN 4 //25
#define TX1_PIN 5 //26
#define RX2_PIN 16
#define TX2_PIN 17
#define PMS_SET_PIN 18
#define PMS_RST_PIN 19
#define LUM_PIN 34 // ADC6
#define LUM_POWER_PIN 35

#define MODEM_RESET 5

#include <PMS.h>

PMS pms(Serial1);
PMS::DATA data;
//датчик
uint8_t r_pm1[20], r_pm2[20], r_pm10[20];  // массивы выборок для усреднения
uint16_t pm1 = 0, pm2 = 0, pm10 = 0; // усредненные значения

/***************************************/
/*               SETUP                 */
/***************************************/
void setup() {
  pinMode(MODEM_RESET, OUTPUT); // для сброса модема даем ноль на 100 мс
  digitalWrite(MODEM_RESET, 1);
  pinMode(PMS_RST_PIN, OUTPUT); // PMS ресет и запуск
  digitalWrite(PMS_RST_PIN, 1);
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, 1); // 1 работаем, 0 спим
  pinMode(LUM_POWER_PIN, OUTPUT); // мерим освещенность
  digitalWrite(LUM_POWER_PIN, 1); // подаем 3.3 вольта на светорезистор

  Serial.begin(115200);
  Serial.flush();
  Serial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);  // PMS Serial port.   Rx = 4, Tx = 5 will work for ESP32, S2, S3 and C3

  Serial.print("PM1, PM2.5, PM10");
  delay(500);
  //  Serial.println("Waking up, wait 30 seconds for stable readings...");
  // pms.wakeUp(); // надо реализовать вручную
  delay(20000);

} // setup()


/***************************************/
/*               LOOP                  */
/***************************************/
void loop() {


  memset(r_pm1, 0, 20); // обнуляем массивы
  memset(r_pm2, 0, 20);
  memset(r_pm10, 0, 20);
  // выборка из 20 значений,  примерно на 20 секунд
  for (int i = 0; i < 20; i++) {
    pms.requestRead();
    //  Serial.println("Wait max. 1 second for read...");
    if (pms.readUntil(data)) {
      //  Serial.print("PM 1.0 (ug/m3): ");
      r_pm1[i] = data.PM_AE_UG_1_0;
      Serial.print(r_pm1[i]);
      Serial.print(",");
      //  Serial.print("PM 2.5 (ug/m3): ");
      r_pm2[i] = data.PM_AE_UG_2_5;
      Serial.print(r_pm2[i]);
      Serial.print(",");
      //  Serial.print("PM 10.0 (ug/m3): ");
      r_pm10[i] = data.PM_AE_UG_10_0;
      Serial.print(r_pm10[i]);
      //печатаем отфильтрованные 
      Serial.print(",");
      Serial.print(pm1);
      Serial.print(",");
      Serial.print(pm2);
      Serial.print(",");
      Serial.print(pm10);
      Serial.println();
    } else {
      Serial.println("No data.");
    }
  }//for
  //фильтрация значений массивов
  filteringPm1();
  filteringPm2();
  filteringPm10();

} // loop()

//пузырьковая сортировка pm1
void filteringPm1() {
  // сортируем массив от малого к большему
  for (int j = 0; j + 1 < 20; ++j) {
    for (int i = 0; i + 1 < 20 - 1; ++i) {
      if ( r_pm1[i] > r_pm1[i + 1]) {
        int temp = r_pm1[i];
        r_pm1[i] = r_pm1[i + 1];
        r_pm1[i + 1] = temp;
      }
    }
  }
  int sum = 0;
  //берем 16 центральных значений, отсекая по 2 с краев
  for (int k = 2; k < 18; k++) {
    sum += r_pm1[k];
  }//for k
  pm1 = sum / 16;
}//filteringPm1()

//пузырьковая сортировка pm1
void filteringPm2() {
  for (int j = 0; j + 1 < 20; ++j) {
    for (int i = 0; i + 1 < 20 - 1; ++i) {
      if ( r_pm2[i] > r_pm2[i + 1]) {
        int temp = r_pm2[i];
        r_pm2[i] = r_pm2[i + 1];
        r_pm2[i + 1] = temp;
      }
    }
  }
  int sum = 0;
  //берем 16 центральных значений, отсекая по 2 с краев
  for (int k = 2; k < 18; k++) {
    sum += r_pm2[k];
  }//for k
  pm2 = sum / 16;
}//filteringPm2

//пузырьковая сортировка pm1
void filteringPm10() {
  for (int j = 0; j + 1 < 20; ++j) {
    for (int i = 0; i + 1 < 20 - 1; ++i) {
      if ( r_pm10[i] > r_pm10[i + 1]) {
        int temp = r_pm10[i];
        r_pm10[i] = r_pm10[i + 1];
        r_pm10[i + 1] = temp;
      }
    }
  }
  int sum = 0;
  //берем 16 центральных значений, отсекая по 2 с краев
  for (int k = 2; k < 18; k++) {
    sum += r_pm10[k];
  }//for k
  pm10 = sum /16;
}//filteringPm10





