
// github project:      https://github.com/nicelight/air-sensor-esp32


#include <Arduino.h>
#include <WiFi.h>
const char* ssid = "kuso4ek_raya";
const char* password = "1234567812345678";

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
uint32_t lum = 0; // освещенность
#define LED_PIN 2 

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



//**********   MQTT for gsm   **********//
IPAddress MQTTserver(95, 142, 87, 232); // Фарика сервак
const char* broker = "95.142.87.232";
const char* topicLed = "GsmClientTest/led";
const char* topicInit = "GsmClientTest/init";
const char* topicLedStatus = "GsmClientTest/ledStatus";
#include <PubSubClient.h>
PubSubClient  mqtt(client);

//**********   MQTT for wifi   **********//
const char* mqtt_server = "95.142.87.232";
WiFiClient espClient;
PubSubClient wifiMQTTclient(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


/***************************************/
/*     FUNCTION     DECLARATIONS       */
/***************************************/
// put function declarations here:
void modemPingPong();
void srvGETconnection();
void mqttCallback(char* topic, byte* payload, unsigned int len);
void setup_wifi();
void wifiMqttCallback(char* topic, byte* message, unsigned int length);
void reconnectMQTTwifi();



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

  setup_wifi();

  wifiMQTTclient.setServer(mqtt_server, 1883);
  wifiMQTTclient.setCallback(wifiMqttCallback);

  pinMode(LED_PIN, OUTPUT);


  /* // инициализация GSM модема
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

   modem.init();
    //mqtt via GSM
    mqtt.setServer(MQTTserver, 1883);
  mqtt.setCallback(mqttCallback);

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
  */

} // setup()


/***************************************/
/*               LOOP                  */
/***************************************/
void loop() {
  //modemPingPong(); // GSM modem общение
  //srvGETconnection();
  // Serial.print(millis() / 1000);
  // Serial.println("\tuptime");
  for (int i = 0; i < 5; i++) {
    lum += analogRead(LUM_PIN);
    delay(20);
  }
  lum = lum / 5;
  Serial.print("lum = ");
  Serial.println(lum);

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

  if (!wifiMQTTclient.connected()) {
    reconnectMQTTwifi();
  }
  wifiMQTTclient.loop();

  long now = millis();
  if (now - lastMsg > 10000L) {
    lastMsg = now;

    // Temperature in Celsius
    bmeTemp = bme.readTemperature();
    // Uncomment the next line to set temperature in Fahrenheit
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit

    // Convert the value to a char[]
    char msgArray[8];
    //dtostrf(bmeTemp, 1, 2, tempString);
    itoa(bmeTemp, msgArray, 10);
    Serial.print("Temperature: ");
    Serial.print(msgArray);
    wifiMQTTclient.publish("SOGD01/temp", msgArray);

    bmeHum = bme.readHumidity();

    // Convert the value to a char[]
    //dtostrf(bmeHum, 1, 2, humString);
    itoa(bmeHum, msgArray, 10);
    Serial.print("\tHumidity: ");
    Serial.print(msgArray);
    wifiMQTTclient.publish("SOGD01/hum", msgArray);

    // converting lumenus to char[]
    itoa(lum, msgArray, 10);
    Serial.print("\tLumens: ");
    Serial.print(msgArray);
    wifiMQTTclient.publish("SOGD01/lum", msgArray);

      // converting pressure to char[]
    float pressure = bme.readPressure();        // Читаем давление в [Па]
    bmePres = pressureToMmHg(pressure);
    itoa(bmePres, msgArray, 10);
    Serial.print("\tPressure: ");
    Serial.print(msgArray);
    wifiMQTTclient.publish("SOGD01/pres", msgArray);
  }//sending mqtt via wifi
  Serial.println();

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


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}//setup_wifi()


void wifiMqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(LED_PIN, HIGH);
    } else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(LED_PIN, LOW);
    }
  }
}//wifiMqttCallback()


void reconnectMQTTwifi() {
  // Loop until we're reconnected MQTTwifi
  while (!wifiMQTTclient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (wifiMQTTclient.connect("ESP8266Client", "mqtt-user", "1234567890")) {
      Serial.println("connected");
      // Subscribe
      wifiMQTTclient.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(wifiMQTTclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}//reconnectMQTTwifi()
