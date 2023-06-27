
// github project:      https://github.com/nicelight/air-sensor-esp32


#include <Arduino.h>
#include <PMS.h>
#define RX1_PIN 4 //25
#define TX1_PIN 5 //26
#define PMS_SET_PIN 18
#define PMS_RST_PIN 19
PMS pms(Serial1);
PMS::DATA data;

#include <GyverBME280.h>
GyverBME280 bme;
int16_t bmeTemp = 0; //BME280 параметры 
int16_t bmeHum = 0;
int16_t bmePres = 0;
int16_t bmeAlt = 0;

#define LUM_PIN 34 // ADC6 освнещенность снаружи
#define LUM_POWER_PIN 32 // подтяжка к плюсу фоторезистора
uint16_t lum = 0; // освещенность


//********  MODEM   *********// 
#define TINY_GSM_MODEM_M590
#define SerialAT Serial2  // Серийник GSM 
#define SerialMon Serial  // Серийник отладки
#define RX2_PIN 16
#define TX2_PIN 17
#define TINY_GSM_YIELD() { delay(2); }
#define TINY_GSM_RX_BUFFER 650
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define GSM_PIN ""  // set GSM enabling  PIN, if any
#define MODEM_RESET 5
// Your GPRS credentials, if any
const char apn[] = "default"; // Мегафон TJ
const char gprsUser[] = "";
const char gprsPass[] = "";
// web Server(for GET) details
const int      port = 80;
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
#include <TinyGsmClient.h>
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);



//**********   MQTT   **********//
IPAddress MQTTserver(95, 142, 87, 232); // Фарика сервак
const char* broker = "95.142.87.232";
const char* topicLed = "GsmClientTest/led";
const char* topicInit = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";
#include <PubSubClient.h>
PubSubClient  mqtt(MQTTserver, 1883, mqttCallback, client);




/***************************************/
/*     FUNCTION     DECLARATIONS       */
/***************************************/
// put function declarations here:
void modemPingPong();
void srvGETconnection();
void mqttCallback(char* topic, byte* payload, unsigned int len);


/***************************************/
/*               SETUP                 */
/***************************************/
void setup() {
  pinMode(PMS_RST_PIN, OUTPUT); // PMS ресет и запуск
  digitalWrite(PMS_RST_PIN, 1);
  pinMode(PMS_SET_PIN, OUTPUT);
  digitalWrite(PMS_SET_PIN, 0); // 1 работаем, ноль спим
  pinMode(LUM_POWER_PIN, OUTPUT); // мерим освещенность 
  digitalWrite(LUM_POWER_PIN, 1); // подаем 3.3 вольта на светорезистор

  Serial.begin(115200);

  Serial1.begin(9600, SERIAL_8N1, RX1_PIN, TX1_PIN);  // PMS Serial port.   Rx = 4, Tx = 5 will work for ESP32, S2, S3 and C3
  // Serial.println("Waking up, wait 30 seconds for stable readings...");
  // pms.wakeUp(); // надо реализовать вручную
  // delay(30000);

  bme.begin();

  modem.setBaud(57600);
  pinMode(MODEM_RESET, OUTPUT); // для сброса модема даем ноль на 100 мс
  digitalWrite(MODEM_RESET, 1);
  Serial2.begin(57600, SERIAL_8N1, RX2_PIN, TX2_PIN); // GSM Serial port
  delay(3000); // в примере предлагают

  Serial.print("*** GSM modem Neoway M590E init(30sec): ");
  modem.restart();
  Serial2.println("AT");
  delay(500);
  modemPingPong(); // ответ получаем если есть

  // modem.init();
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) { SerialMon.println("Network connected"); }
  // GPRS connection parameters are usually set after network registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
    delay(3000);
    return;
  }
  SerialMon.println(" OK");
  if (modem.isGprsConnected()) { SerialMon.println("GPRS connected"); }



} // setup()


/***************************************/
/*               LOOP                  */
/***************************************/
void loop() {
  Serial.print(millis() / 1000);
  Serial.println("\tuptime");
  modemPingPong(); // GSM modem общение
  srvGETconnection();
  lum = analogRead(LUM_PIN);
  Serial.print("lum = ");
  Serial.println(lum);
  delay(2000);


  /*  ////////// BME280
    Serial.print("Temp: ");
    bmeTemp = bme.readTemperature();
    Serial.print(bmeTemp);        // Выводим темперутуру в [*C]
    Serial.print(" *C\t");

    Serial.print("Hum: ");
    bmeHum = bme.readHumidity();
    Serial.print(bmeHum);           // Выводим влажность в [%]
    Serial.print(" %\tPress: ");

    float pressure = bme.readPressure();        // Читаем давление в [Па]
    bmePres = pressureToMmHg(pressure);
    Serial.print(bmePres);     // Выводим давление в [мм рт. столба]
    Serial.print(" mm Hg\t");
    bmeAlt =pressureToAltitude(pressure);
    Serial.print("Altitide: ");
    Serial.print(bmeAlt); // Выводим высоту в [м над ур. моря]
    Serial.println(" m"); */


    /* ////// PMS Air sensor
      Serial.println("PMS sensor Send read request...");
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
      // pms.sleep(); реализовать надо
      */


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


void srvGETconnection() {
  ///// Connection to web server - make GET example
  SerialMon.print("Connecting to ");
  SerialMon.println(server);
  if (!client.connect(server, port)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK\r\n\t Performing HTTP GET request...");
  client.print(String("GET ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.println();
  // ожидаем ответа от сервера
  uint32_t timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      SerialMon.print(c);
      timeout = millis();
    }
  }
  SerialMon.println();
  // отключаемся от сервера
  client.stop();
  SerialMon.println(F("Server disconnected"));
  ///////////////////////////////////////////////////////// 
}//void srvGETconnection()


//подпись на топики
void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  // Only proceed if incoming message's topic matches
  if (String(topic) == topicLed) {
    bool  ledStatus = 0; 
    //ledStatus = !ledStatus;
    Serial.println("now we would toogle led");
    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
  }
}//mqttCallback()