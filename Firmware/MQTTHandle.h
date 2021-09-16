
#include <SoftwareSerial.h>

void mqttCallback(char *topic, byte *payload, unsigned int len);
void MQTTsetup();
void publish(String topic, String data);
void MQTTloop();
uint8_t mqttStatus();
boolean mqttConnect();
uint8_t MQTTstatus = 0;
uint8_t mqttStatus()
{
  return MQTTstatus;
}
void vTaskDelay(int v)
{
  delay(v);
}
//#define TINY_GSM_MODEM_SIM808
SoftwareSerial GPRSSerial(12, 14); // RX, TX

#define TINY_GSM_MODEM_SIM800

// See all AT commands, if wanted
//#define DUMP_AT_COMMANDS
#define SUPRESS_LOGS 1

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 115200
#define GSM_AUTOBAUD_MAX 115200

// Add a reception delay, if needed
#define TINY_GSM_YIELD() \
  {                      \
    vTaskDelay(2);       \
  }

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT GPRSSerial

// Your GPRS credentials
// Leave empty, if missing user or pass

// MQTT details

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else

TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

#define LED_PIN 13
int ledStatus = LOW;

long lastReconnectAttempt = 0;

void MQTTsetup()
{

  // Set console baud rate
  //SerialMon.begin(38400);
  vTaskDelay(10);

  // Set your reset, enable, power pins here

  //SerialMon.println("Wait...");

  // Set GSM module baud rate
  SerialAT.begin(115200);
  vTaskDelay(1000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  //SerialMon.println("Initializing modem...");
  //modem.restart();
  modem.init();

  //String modemInfo = modem.getModemInfo();

  //SerialMon.print("Modem: ");
  //SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

#if TINY_GSM_USE_WIFI
  //SerialMon.print(F("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass))
  {
    //SerialMon.println(" fail");
    vTaskDelay(5000);
    return;
  }
  //SerialMon.println(" OK");
#endif

#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the gprsConnect function BEFORE waiting for network!
  modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

  //SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork(240000L))
  {
    //SerialMon.println(" fail");
    vTaskDelay(5000);
    return;
  }
  //SerialMon.println(" OK");

  if (modem.isNetworkConnected())
  {
    //SerialMon.println("Network connected");
  }

#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_HAS_GPRS
  //SerialMon.print(F("Connecting to "));
  //SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass))
  {
    //SerialMon.println(" fail");
    vTaskDelay(5000);
    return;
  }
  //SerialMon.println(" OK");
#endif
  //modem.gprsConnect(apn, user, pass);
  if (!modem.gprsConnect(apn, user, pass))
  {
    //SerialMon.println(" fail");
    MQTTstatus = 0;
    vTaskDelay(5000);
  }
  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);

  /*String name = modem.getModemName();
  DBG("Modem Name:", name);

//  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);
String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);
*/

  /* String cop = modem.getOperator();
  DBG("Operator:", cop);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);
*/

  // This is only supported on SIMxxx series
  // String gsmLoc = modem.getGsmLocation();
  // DBG("GSM location:", gsmLoc);

  // This is only supported on SIMxxx series
  // String gsmTime = modem.getGSMDateTime(DATE_TIME);
  // DBG("GSM Time:", gsmTime);
  // String gsmDate = modem.getGSMDateTime(DATE_DATE);
  // DBG("GSM Date:", gsmDate);

  /*String ussd_balance = modem.sendUSSD("*222#");
  DBG("Balance (USSD):", ussd_balance);
  String ussd_phone_num = modem.sendUSSD("*161#");
  DBG("Phone number (USSD):", ussd_phone_num);
  String gsmLoc = modem.getGsmLocation();
  DBG("GSM location:", gsmLoc);

//  This is only supported on SIMxxx series
  String gsmTime = modem.getGSMDateTime(DATE_TIME);
   DBG("GSM Time:", gsmTime);
   String gsmDate = modem.getGSMDateTime(DATE_DATE);
   DBG("GSM Date:", gsmDate);
*/
}

boolean mqttConnect()
{
  //SerialMon.print("Connecting to ");
  //SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect(randomString("pn1").c_str(), "dev", "smartmonitor");

  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false)
  {
    //SerialMon.println(" fail");
    MQTTstatus = 0;
    return false;
  }

  //SerialMon.println(" OK");

  mqtt.subscribe(ss.getTopicWithMAC("configs").c_str());

  return mqtt.connected();
}
void publish(String topic, String data)
{
  mqtt.publish(topic.c_str(), data.c_str());
}
void MQTTloop()
{

  if (!mqtt.connected())
  {
    MQTTstatus = 0;
    ////SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    unsigned long t = millis();
    if (t - lastReconnectAttempt > 5000L)
    {
      lastReconnectAttempt = t;
      if (mqttConnect())
      {
        lastReconnectAttempt = 0;
      }
    }
    vTaskDelay(50);
    return;
  }

  mqtt.loop();
}

void mqttCallback(char *topic, byte *payload, unsigned int len)
{

  //  //SerialMon.print("Message arrived [");
  //  //SerialMon.print(topic);
  //  //SerialMon.print("]: ");
  //
  char dest[1500];

  for (int i = 0; i < len; i++)
  {
    dest[i] = (char)payload[i];
  }
  dest[len + 0] = '\0';
  //  Serial.println("--------------");
  //  Serial.println(dest);
  //  Serial.println("--------------");
  //
  //  SerialMon.write(payload, len);
  //  //SerialMon.println();
  if (String(topic) == String(ss.getTopicWithMAC("configs")))
  {
    Serial.println("Configs Received.");
  }
}
