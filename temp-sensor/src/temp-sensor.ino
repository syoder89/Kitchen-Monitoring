/*
 * Project Kitchen-Monitoring
 * Description: Test project for 
 * Author: Joshua Snyder
 * Date: 5-9-2019
 */

#include <Particle.h>
#include <RH_RF95.h>
#include <Adafruit_SHT31.h>
#include <RHReliableDatagram.h>

#define MY_ADDRESS        77
#define SERVER_ADDRESS    2
#define TXPWR             13
// In milliseconds
//#define SENSORS_PROBE_TIME 1*60*1000
//#define PUBLISH_TIME 1*60*1000
#define SENSORS_PROBE_TIME 30*1000
#define PUBLISH_TIME 30*1000

SYSTEM_MODE(MANUAL);

typedef struct METRICS
{
  float temperature;
  float humidity;
  boolean alert_temperature;
  boolean alert_humidity;
  float bat_voltage;
  int seq;
};

const float FREQ = 915.0;
const float SAFE_COLD_TEMP = 4.44;
RH_RF95 rf95(D6,D2);
RHDatagram manager(rf95, MY_ADDRESS);
METRICS data = { }; 
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Timer sensorsTimer(SENSORS_PROBE_TIME, readSensors);
Timer publishTimer(PUBLISH_TIME, schedulePublish);
float temperature, humidity;
int uptime;
boolean doPublish = false;

// setup() runs once, when the device is first turned on.
void setup() {
	// Put initialization like pinMode and begin functions here.
	Serial.begin(115200);
	waitFor(Serial.isConnected, 15000);
	Serial.println("Init manager...");
	if (manager.init()) {
		if (!rf95.setFrequency(FREQ))
			Serial.println("Unable to set RF95 Frequency");
                rf95.setSignalBandwidth(125000);
                rf95.setCodingRate4(5);
                rf95.setSpreadingFactor(10);
                rf95.setPayloadCRC(true);
		rf95.setTxPower(TXPWR);
		Serial.println("RF95 Radio Init: Successful");  
	}
	else {
		Serial.println("Rf95 Radio Init: Failed");
	}
	Serial.printlnf("RF95 Max message Mtu %d", rf95.maxMessageLength());
	sht31.begin(0x44);
	readSensorsInit();
	publishTimer.start();
//	sensorsTimer.start();
	uptime = millis()/1000;
	doPublish = true;
	RGB.control(true);
	RGB.color(0,255,0);
	RGB.brightness(0);
}

#define EWMA_LEVEL              96
#define EWMA_DIV                128
double ewma_add(double old_val, double new_val)
{
	return (new_val * (EWMA_DIV - EWMA_LEVEL) + old_val * EWMA_LEVEL) / EWMA_DIV;
}

void readSensorsInit() {
	temperature = sht31.readTemperature();
	humidity = sht31.readHumidity();
}

void readSensors() {
	temperature = sht31.readTemperature();
	humidity = sht31.readHumidity();
//	temperature = ewma_add(temperature, sht31.temp);
//	humidity = ewma_add(humidity, sht31.humidity);
//	Serial.print("Temperature: "); Serial.println(temperature);
//	Serial.print("Humidity: "); Serial.println(humidity);
}

void publishSensors() {
	RGB.brightness(50);
	readSensors();
	data.temperature = temperature;
	data.humidity = humidity;
	data.alert_temperature = false;
	data.alert_humidity = false;
	data.bat_voltage = analogRead(BATT) * 0.0011224;
	data.seq++;
  
	// Send Data over the Radio
//	Serial.println("Sending Packet Data");
	if (manager.sendto((uint8_t *) &data, sizeof(data), SERVER_ADDRESS)) {
//		Serial.println("Sendto succeeded");
	}
//	else
//		Serial.println("Sendto failed");
	rf95.sleep();
	RGB.brightness(0);
}

void schedulePublish() {
	doPublish = true;
}

void loop() {
//	if (Particle.connected() == false) {
//		Particle.connect();
//	}
	uptime = millis()/1000;
	if (doPublish) {
		doPublish = false;
		publishSensors();
	}
	System.sleep(PUBLISH_TIME);
}

