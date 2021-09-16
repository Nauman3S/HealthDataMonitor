//Main File

#include "consts.h"
#include "SoftwareStack.h"
#include "SensorsHandle.h"
#include "GPRSHandler.h"


SoftwareStack ss;//SS instance
void setup()
{

  Serial.begin(CONSOLE_BAUD_R);
  MQTTsetup();
  setupSensors();
  
  DebugPrint("VitalsMonitor");
}

void loop()
{


    MQTTloop();

    long now = millis();
    if (now - lastMsg > delayV) {
      lastMsg = now;
      //publish();
      //client.publish("outTopic/BPM", ss.StrToCharArray(getBPM()));
      //client.publish("outTopic/Temperature", ss.StrToCharArray(getTemperature()));
      
      
  }
}
