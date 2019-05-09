/*
 * Project Kitchen-Monitoring
 * Description: Test project for 
 * Author: Joshua Snyder
 * Date: 5-9-2019
 */

#include <Particle.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>

#define MY_ADDRESS        77
#define SERVER_ADDRESS    2
#define TXPWR             10

typedef struct METRICS
{
  float temp;
  float hum;
  char alert[1];
};

const float FREQ = 915.0;
const float SAFE_COLD_TEMP = 4.44;
RH_RF95 rf95;
RHDatagram manager(rf95, MY_ADDRESS);
METRICS data; 

// setup() runs once, when the device is first turned on.
void setup() {
  pinMode(D7, OUTPUT);
  digitalWrite(D7, HIGH); // Set the LED On while we are in init.
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  if (manager.init()) {
    if (!rf95.setFrequency(FREQ))
      Serial.println("Unable to set RF95 Frequency");
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128))
      Serial.println("Unable to set Modem Config or Invalid Config");
    rf95.setTxPower(TXPWR);
    Serial.println("RF95 Radio Init: Successful");  
  }
  else
  {
    Serial.println("Rf95 Radio Init: Failed");
  }
  Serial.printlnf("RF95 Max message Mtu %d", rf95.maxMessageLength());
  digitalWrite(D7, LOW);
}

void loop() {
  // Going to fake temp & Humdity for now
  data.hum = random(2,100);
  data.temp = random(-20,45);
  if (data.temp < SAFE_COLD_TEMP ) {
    strcpy(data.alert, "0"); /* Check that we are below the safe cold storage temp */
  }
  else
  {
  strcpy(data.alert, "1");  /* Alert if we are above 4.4c  */
  }
  
  // Send Data over the Radio
  if (!manager.sendto((uint8_t *) &data, sizeof(data), SERVER_ADDRESS)) {
    Serial.println("Sending Packet Data");
    for (int x=1; x=10; x++) {
      digitalWrite(D7, HIGH);
      delay(50);
      digitalWrite(D7,LOW);
    }
  }
  rf95.waitPacketSent(250);   
  delay(30000);
}