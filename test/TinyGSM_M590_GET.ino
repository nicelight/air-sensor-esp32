

#define TINY_GSM_MODEM_M590
// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
#define SerialAT Serial2
#define TINY_GSM_DEBUG Serial

#define TINY_GSM_YIELD() { delay(2); }
#define TINY_GSM_RX_BUFFER 650
#define TINY_GSM_USE_GPRS true
#define GSM_PIN ""  // set GSM PIN, if any

// Your GPRS credentials, if any
const char apn[]      = "default";
const char gprsUser[] = "";
const char gprsPass[] = "";


// Server details
const int      port = 80;
const char server[]   = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
int c1 = 0, c2 = 0;

#include <TinyGsmClient.h>
TinyGsm        modem(SerialAT);
TinyGsmClient client(modem);


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


void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);
  modem.setBaud(57600);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println("Wait...");
  SerialAT.begin(57600);
  delay(6000);

  // Restart takes quite some time  To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  //modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }
#endif
}//setup

bool gsmGPRSconnect() {
  if (modem.isNetworkConnected() && modem.isGprsConnected()) return 1;
  else {
    delay (500);
    if (modem.isNetworkConnected() && modem.isGprsConnected()) return 1;
    else {
      if (!modem.isNetworkConnected()) Serial.print("modem.isNetworkConnected() = 0");
      if (!modem.isGprsConnected()) Serial.print("\tmodem.isGprsConnected() = 0");
      Serial.println();
      Serial.println("\r\n\t\tmodem RECONNECTing...\r\n");
      if (!modem.isNetworkConnected()) {
        SerialMon.print("Waiting for network...");
        if (!modem.waitForNetwork()) {
          SerialMon.println(" fail");
          delay(10000);
          return 0;
        }
        SerialMon.println(" success");

        if (modem.isNetworkConnected()) {
          SerialMon.println("Network connected");
        }
      }
      // GPRS connection parameters are usually set after network registration
      if (!modem.isGprsConnected()) {
        SerialMon.print(F("Connecting to APN: "));
        SerialMon.print(apn);
        if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
          SerialMon.println(" fail");
          delay(10000);
          return 0;
        }
        SerialMon.println(" success");
        if (modem.isGprsConnected()) {
          SerialMon.println("GPRS connected\r\n");
          return 1;
        }
      }//gprs
    }// if GSM && GPRS not conn 2nd time
  }// if GSM && GPRS not conn
}//gsmGPRSconnect()


void loop() {
  if (gsmGPRSconnect()) {

    modemPingPong();
    SerialMon.print("Connecting to ");
    SerialMon.println(server);
    if (!client.connect(server, port)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
    }
    SerialMon.println(" success");

    // Make a HTTP GET request:
    SerialMon.println("Performing HTTP GET request...");
    client.print(String("GET ") + resource + " HTTP/1.1\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.print("Connection: close\r\n\r\n");
    client.println();

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

    // Shutdown

    client.stop();
    SerialMon.println(F("Server disconnected"));
    delay(1000);
  }

}//loop

//  /// conn to http server
//  SerialMon.print("Connecting to ");
//  SerialMon.println(server);
//  if (!client.connect(server, port)) {
//    SerialMon.println(" fail");
//    delay(10000);
//    return;
//  }
//  SerialMon.println(" success");
//
//  // Make a HTTP GET request:
//  SerialMon.println("Performing HTTP GET request...");
//  client.print(String("GET ") + resource + " HTTP/1.1\r\n");
//  client.print(String("Host: ") + server + "\r\n");
//  client.print("Connection: close\r\n\r\n");
//  client.println();
//
//  uint32_t timeout = millis();
//  while (client.connected() && millis() - timeout < 10000L) {
//    // Print available data
//    while (client.available()) {
//      char c = client.read();
//      SerialMon.print(c);
//      timeout = millis();
//    }
//  }
//  SerialMon.println();
//
//  // Shutdown
//
//  client.stop();
//  SerialMon.println(F("Server disconnected"));
//
//  modem.gprsDisconnect();
//  SerialMon.println(F("GPRS disconnected"));
//
//
//  // Do nothing forevermore
//  while (true) {
//    delay(1000);
//  }
// }//loop


