#include "GPRSAccessPoint.h"
#define CONSOLE_BAUD_R 115200

const char *broker = "broker.hivemq.com";

long lastMsg = 0;
char msg[50];
int value = 0;

int delayV = 1000;

#define ONE_WIRE_BUS D3

///SENSORS
//  Variables
const int PulseWire = A0; // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LEDB = D2;      // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;      // Determine which Signal to "count as a beat" and which to ignore.
                          // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                          // Otherwise leave the default "550" value.
