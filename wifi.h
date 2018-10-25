#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}
#define RANDOM_REG32  ESP8266_DREG(0x20E44)

uint8_t probe_packet[54] = {
0x50, 0x88,                         //Type/Subtype: Probe Response
0x00, 0x00,                         //Duration - will be overridden
0x01, 0x02, 0x03, 0x04, 0x05, 0x06, //Receiver/Destination address
0xaa, 0x91, 0x8f, 0x13, 0x3c, 0xc9, //Transmitter/Source address
0xaa, 0x91, 0x8f, 0x13, 0x3c, 0xc9, //BSS Id
0x00, 0x00,                         //Fragment/Sequence
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, //Timestamp - will be overridden
//managment frame fixed and taged params use for data
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

typedef struct {
  uint8_t type[2];
  uint8_t duration[2];
  uint8_t receiver[6];
  uint8_t transmitter[6];
  uint8_t bss[6];
  uint8_t sequence[2];
  uint8_t timestampt[8];
  uint8_t data[22];
} Packet;

typedef struct {
  signed rssi: 8;
  unsigned rate: 4;
  unsigned is_group: 1;
  unsigned: 1;
  unsigned sig_mode: 2;
  unsigned legacy_length: 12;
  unsigned damatch0: 1;
  unsigned damatch1: 1;
  unsigned bssidmatch0: 1;
  unsigned bssidmatch1: 1;
  unsigned MCS: 7;
  unsigned CWB: 1;
  unsigned HT_length: 16;
  unsigned Smoothing: 1;
  unsigned Not_Sounding: 1;
  unsigned: 1;
  unsigned Aggregation: 1;
  unsigned STBC: 2;
  unsigned FEC_CODING: 1;
  unsigned SGI: 1;
  unsigned rxend_state: 8;
  unsigned ampdu_cnt: 8;
  unsigned channel: 4;
  unsigned: 12;
 } RxControl;

struct Frame {
  RxControl rx_ctrl;
  Packet packet;
};

uint8_t rx_address[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
uint8_t tx_address[6];
uint8_t channel = 5;
uint8_t binding = 1;
int8_t rssi = -127;

void promisc_cb(uint8_t *buf, uint16_t len);
void generateAddress(uint8_t *address);

#ifdef DEBUG_WIRELESS
  uint32_t packetCount = 0;
  void printPacket(Packet packet);
#endif

void  wifiInitBind(){
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(promisc_cb);
  wifi_promiscuous_enable(1);
  system_phy_set_max_tpw(82);
  wifi_set_channel(channel);
}

Packet wifiInitData(Packet packet){
  wifi_set_channel(channel);
  memcpy( &packet, probe_packet, sizeof(probe_packet));
  memcpy( &packet.receiver, tx_address, 6);
  memcpy( &packet.transmitter, rx_address, 6);
  generateAddress(packet.bss);
  #ifdef DEBUG_WIRELESS
    Serial.println();
    Serial.print("Binding complete listening on: ");
    for (uint8_t i = 0; i < 6; i++ ) {
      Serial.print(rx_address[i], HEX);  Serial.print("_");
    }
    Serial.print(" chanel: "); Serial.print(channel);
    Serial.println();
    printPacket(packet);
  #endif
  return packet;
}

void promisc_cb(uint8_t *buf, uint16_t len)
{
  if (buf[12] == 0x50 && buf[13] == 0x88) { //discard if not Type/Subtype: Probe Response
    struct Frame *frame = (struct Frame*) buf;
    for (uint8_t i = 0; i < 6; i++ ) { //discard if does not match expected address
      if ( rx_address[i] != frame->packet.receiver[i] ) {
        return;
      }
    }
    #ifdef DEBUG_WIRELESS
       packetCount++;
    #endif
    if (binding) {
       memcpy(&rx_address, frame->packet.data,  6);
       memcpy(&tx_address, frame->packet.transmitter,  6);
       channel = frame->packet.data[6];
       binding = 0;
    }
    else {
      //do not messup console if debuging packets will not be forwarded to serial
      #if !defined(DEBUG_WIRELESS)
          for(uint8_t i = 0; i < 22; i++){
            Serial.write(frame -> packet.data[i]);
          }
      #endif
      rssi = frame->rx_ctrl.rssi;
    }
  }
}

void wifiSendPacket(Packet packet){
    wifi_send_pkt_freedom((uint8_t*) &packet, 54 , 0);
}

int8_t getRssi(){
    return rssi;
}

uint8_t isBound(){
    return !binding;
}

void generateAddress(uint8_t *address){
  for (uint8_t i = 0; i < 6; i++) {
    address[i] = RANDOM_REG32 % 256;
  }
}

#ifdef DEBUG_WIRELESS
uint32_t getPacketCountAndReset(){
    uint32_t count =  packetCount;
    packetCount = 0;
    return count;
}

void printPacket(Packet packet){
    uint8_t i = 0;
    for(i=0; i < 2; i++){
        Serial.print(packet.type[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 2; i++){
        Serial.print(packet.duration[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 6; i++){
        Serial.print(packet.receiver[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 6; i++){
        Serial.print(packet.transmitter[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 6; i++){
        Serial.print(packet.bss[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 2; i++){
        Serial.print(packet.sequence[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 8; i++){
        Serial.print(packet.timestampt[i], HEX);  Serial.print("_");
    }
    Serial.println();
    for(i=0; i < 22; i++){
        Serial.print(packet.data[i], HEX);  Serial.print("_");
    }
    Serial.println();
}
#endif