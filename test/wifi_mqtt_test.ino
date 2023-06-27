/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <WiFi.h>
const char* ssid = "kuso4ek_raya";
const char* password = "1234567812345678";

#include <PubSubClient.h>
#include <GyverBME280.h>
GyverBME280 bme;
int16_t bmeTemp = 0; //BME280 параметры
int16_t bmeHum = 0;
int16_t bmePres = 0;
int16_t bmeAlt = 0;

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "95.142.87.232";

WiFiClient espClient;
PubSubClient wifiMQTTclient(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// LED Pin
const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  setup_wifi();
  wifiMQTTclient.setServer(mqtt_server, 1883);
  wifiMQTTclient.setCallback(wifiMqttCallback);

  pinMode(ledPin, OUTPUT);
}

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
      digitalWrite(ledPin, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
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
}
void loop() {
  if (!wifiMQTTclient.connected()) {
    reconnectMQTTwifi();
  }
  wifiMQTTclient.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Temperature in Celsius
    bmeTemp = bme.readTemperature();
    // Uncomment the next line to set temperature in Fahrenheit
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit

    // Convert the value to a char array
    char tempString[8];
    //dtostrf(bmeTemp, 1, 2, tempString);
    itoa(bmeTemp, tempString, 10);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    wifiMQTTclient.publish("esp32/temperature", tempString);

    bmeHum = bme.readHumidity();
    
    // Convert the value to a char array
    char humString[8];
    //dtostrf(bmeHum, 1, 2, humString);
    itoa(bmeTemp, humString, 10);
    Serial.print("Humidity: ");
    Serial.println(humString);
    wifiMQTTclient.publish("esp32/humidity", humString);
  }
}
