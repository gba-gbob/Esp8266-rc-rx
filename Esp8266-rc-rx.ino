//#define DEBUG_WIRELESS

#include "wifi.h"

Packet wifiPacket;
long oneSecondUpdates = 0;

void setup() {
  Serial.begin(115200);
  wifiInitBind();
  while(binding){
    delay(3);
  }
  wifiPacket = wifiInitData(wifiPacket);
}

void loop() {
  if (millis() - oneSecondUpdates > 1000) {
    if(isBound()){
      wifiPacket.data[0] = getRssi();
  	  wifiSendPacket(wifiPacket);
  	}
  	#ifdef DEBUG_WIRELESS
        Serial.print("Received: "); Serial.print(getPacketCountAndReset()); Serial.println(" packets per second");
    #endif
    oneSecondUpdates = millis();
  }
  delay(3);
}
