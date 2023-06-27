


#define RX1_PIN 4 //25
#define TX1_PIN 5 //26
#define RX2_PIN 16
#define TX2_PIN 17
#define PMS_SET_PIN 18
#define PMS_RST_PIN 19
#define LUM_PIN 34 // ADC6
#define LUM_POWER_PIN 32

#define MODEM_RESET 5

#include <Arduino.h>
#include <PMS.h>

// MQTT details
const char* broker = "broker.hivemq.com";

const char* topicLed       = "GsmClientTest/led";
const char* topicInit      = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";


PMS pms(Serial1);
PMS::DATA data;

uint32_t lum = 0; // освещенность

/***************************************/
/*     FUNCTION     DECLARATIONS       */
/***************************************/
// put function declarations here:
void modemPingPong();


/***************************************/
/*               SETUP                 */
/***************************************/
void setup() {
  pinMode(MODEM_RESET, OUTPUT); // для сброса модема даем ноль на 100 мс
  digitalWrite(MODEM_RESET, 1);
  pinMode(PMS_RST_PIN, OUTPUT); // PMS ресет и запуск
  digitalWrite(PMS_RST_PIN, 1);
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, 0); // 1 работаем, ноль спим
  pinMode(LUM_POWER_PIN, OUTPUT); // мерим освещенность 
  digitalWrite(LUM_POWER_PIN, 1); // подаем 3.3 вольта на светорезистор

  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);  // PMS Serial port.   Rx = 4, Tx = 5 will work for ESP32, S2, S3 and C3
  Serial2.begin(57600, SERIAL_8N1, RX2_PIN, TX2_PIN); // GSM Serial port
  delay(3000); // в примере предлагают

  Serial.print("*** GSM modem Neoway M590E init: ");
  Serial2.println("AT");
  delay(500);
  modemPingPong(); // ответ получаем если есть
  Serial.println("Waking up, wait 30 seconds for stable readings...");
  // pms.wakeUp(); // надо реализовать вручную
  // delay(30000);

} // setup()


/***************************************/
/*               LOOP                  */
/***************************************/
void loop() {
  modemPingPong(); // GSM modem общение
  lum = analogRead(LUM_PIN);
  Serial.print("lum = ");
  Serial.println(lum);
  delay(250);
/*   Serial.println(" Send read request...");
  pms.requestRead();
  Serial.println("Wait max. 1 second for read...");
  if (pms.readUntil(data)) {
    Serial.print("PM 1.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_1_0);

    Serial.print("PM 2.5 (ug/m3): ");
    Serial.println(data.PM_AE_UG_2_5);

    Serial.print("PM 10.0 (ug/m3): ");
    Serial.println(data.PM_AE_UG_10_0);
  } else {
    Serial.println("No data.");
  }

  Serial.println(" sleep ");
  // pms.sleep(); реализовать надо  */


} // loop()



/**************************************/
/*     FUNCTION   DEFINITIONS         */
/**************************************/

void modemPingPong() {
  String str;
  if (Serial.available() > 0) {
    str = Serial.readStringUntil('\n');
    Serial2.println(str);
    //  Serial.println("ping: ");
    //  Serial.println(str);
  }
  if (Serial2.available()) {  //если GSM модуль что-то послал нам, то
    str = Serial2.readStringUntil('\n');
    // Serial.print("pong: ");
    Serial.println(str);
  }
}//modemPingPong

