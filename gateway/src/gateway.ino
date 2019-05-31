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
#include "HttpClient.h"

#define MY_ADDRESS        77
#define SERVER_ADDRESS    2
#define TXPWR             14
#define INFLUX_UPDATE_INTERVAL 30

#define SENSOR_ID "a313"
#define INFLUXDB_HOST "205.159.243.4"
#define INFLUXDB_PORT 18964
#define INFLUXDB_DB "iot"
HttpClient http;

SYSTEM_MODE(SEMI_AUTOMATIC);

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
RHDatagram manager(rf95, SERVER_ADDRESS);
METRICS data; 
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Timer influxTimer(INFLUX_UPDATE_INTERVAL*1000, scheduleInflux);
float temperature, humidity;
int uptime;
boolean doPublish = false, doInflux = false;
int last_seq = 0;

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
	uptime = millis()/1000;
//	doPublish = true;
	doInflux = true;
	influxTimer.start();
}

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
	{ "Connection", "close" },
	{ "Accept" , "application/json" },
	{ NULL, NULL } // NOTE: Always terminate headers will NULL
};

bool sendInflux(String payload) {
	http_request_t     request;
	http_response_t    response;

	request.hostname = INFLUXDB_HOST;
	request.port     = INFLUXDB_PORT;
	request.path     = "/write?db=" + String(INFLUXDB_DB);
	request.body     = payload;

	http.post(request, response, headers);

	if (response.status >= 200 && response.status < 300) {
		return true;
	} else {
		return false;
	}
}

void publishInflux() {
	char temperature[10], humidity[10], bat_voltage[10], seqno[10], uptime[10];
        sprintf(uptime, "%d", uptime);
	sprintf(seqno, "%d", data.seq);
        sprintf(temperature, "%.2f", data.temperature);
	sprintf(humidity, "%.2f", data.humidity);
	sprintf(bat_voltage, "%.2f", data.bat_voltage);
	String influxpayload = "temperature,sensor=" + String(SENSOR_ID) + " value=" + temperature +
            "\nhumidity,sensor=" + String(SENSOR_ID) + " value=" + humidity +
            "\nuptime,sensor=" + String(SENSOR_ID) + " value=" + uptime +
            "\nseqno,sensor=" + String(SENSOR_ID) + " value=" + seqno +
            "\nbattery_voltage,sensor=" + String(SENSOR_ID) + " value=" + bat_voltage;
        if (!(sendInflux(influxpayload)))
		influxTimer.changePeriod(1000);
}

void scheduleInflux() {
	doInflux = true;
	influxTimer.changePeriod(INFLUX_UPDATE_INTERVAL*1000);
}

void publishSensors() {
}

void schedulePublish() {
	doPublish = true;
}

void loop() {
	uint8_t bufLen = sizeof(data);
	if (Particle.connected() == false) {
		Particle.connect();
	}
	uptime = millis()/1000;
	if (manager.available()) {
		if (manager.recvfrom((uint8_t *) &data, &bufLen)) {
			Serial.print("Got ");Serial.print(bufLen);Serial.println(" byte message!!!!");
			Serial.print("RSSI: ");
			Serial.println(rf95.lastRssi());
			Serial.print("SNR: ");
			Serial.println(rf95.lastSNR());
			if (bufLen == sizeof(data)) {
				if (data.seq < last_seq)
					Serial.println("<!!!!!!! Sequence number reset!!! ");
				else if (data.seq != last_seq + 1)
					Serial.println("<!!!!!!! Dropped " + String(data.seq - last_seq - 1) + " packets!!!");
				last_seq = data.seq;
				Serial.println("<======= Received sequence    = " + String(data.seq));
				Serial.println("<======= Received temperature = " + String(data.temperature,1));
				Serial.println("<======= Received humidity    = " + String(data.humidity,1));
				Serial.println("<======= Received bat_voltage = " + String(data.bat_voltage,3));
//				Serial.println("<======= Received alarm_temperature = " + String(data.alert_temperature,1));
//				Serial.println("<======= Received alarm_humidity = " + String(data.alert_humidity,1));
			}
			else
				Serial.println("Incorrect response size");
		}
	}
//	else
//		Serial.println("Timed out waiting for response");
//	Serial.println();
/*
	if (doPublish) {
		doPublish = false;
		publishSensors();
	}
*/
	if (doInflux) {
		doInflux = false;
		publishInflux();
	}
}

