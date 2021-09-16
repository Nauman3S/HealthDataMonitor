//Main File

#include "consts.h"
#include "SoftwareStack.h"
#include "SensorsHandle.h"
SoftwareStack ss; //SS instance
#include "GPRSHandler.h"

void setup()
{

  Serial.begin(CONSOLE_BAUD_R);
  MQTTsetup();
  setupSensors();
  Serial.print("Device ID: ");
  Serial.println(ss.getMACAddress());

  DebugPrint("VitalsMonitor");
}

void loop()
{

  MQTTloop();

  long now = millis();
  if (now - lastMsg > delayV)
  {
    lastMsg = now;
    publish(ss.getTopicWithMAC("BPM"), getBPM());
    publish(ss.getTopicWithMAC("Temperature"), getTemperature());

    //client.publish("outTopic/BPM", ss.StrToCharArray(getBPM()));
    //client.publish("outTopic/Temperature", ss.StrToCharArray(getTemperature()));
  }
}
