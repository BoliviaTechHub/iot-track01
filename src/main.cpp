#include <Arduino.h>

// cayenne debug
#define CAYENNE_PRINT Serial
#define CAYENNE_DEBUG

// gsm debug
// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MODEM_SIM900
#include <CayenneMQTTGSM.h>
#define GSM_SERIAL Serial2
#define GSM_POWER 9

#include <TinyGPS++.h>
#define GPS_SERIAL Serial1

// GSM connection info.
const char apn[] = "int.movil.com.bo"; // Access point name. Leave empty if it is not needed.
const char gprsLogin[] = ""; // GPRS username. Leave empty if it is not needed.
const char gprsPassword[] = ""; // GPRS password. Leave empty if it is not needed.
const char pin[] = ""; // SIM pin number. Leave empty if it is not needed.

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
const char username[] = "username";
const char password[] = "password";
const char clientID[] = "clientID";

// The TinyGPS++ object
TinyGPSPlus gps;

float x = -16.7;
float y = -64.6;
float z = 200.0;

void sendGpsData() {
	char buffer[32];
	buffer[0] = '[';
	size_t offset = 1;
	dtostrf(x, 1, 6, &buffer[offset]);
	offset += strlen(&buffer[offset]);
	buffer[offset++] = ',';
	dtostrf(y, 1, 6, &buffer[offset]);
	offset += strlen(&buffer[offset]);
	buffer[offset++] = ',';
	dtostrf(z, 1, 1, &buffer[offset]);
	offset += strlen(&buffer[offset]);
	buffer[offset++] = ']';
	buffer[offset] = 0;

	Cayenne.virtualWrite(20, buffer, "gps", "m");
}

void setup() {
    // put your setup code here, to run once:
	Serial.begin(115200);

	Serial.println("Powering on SIM900...");
	pinMode(GSM_POWER, OUTPUT);
	digitalWrite(GSM_POWER, LOW);
	delay(1000);
	digitalWrite(GSM_POWER, HIGH);
	delay(1000);
	digitalWrite(GSM_POWER, LOW);
	delay(5000);

	Serial.println("Connecting to cayenne...");
	GPS_SERIAL.begin(9600);
	GSM_SERIAL.begin(9600);
	Cayenne.begin(username, password, clientID, GSM_SERIAL, apn, gprsLogin, gprsPassword, pin);
}

void loop() {
    // put your main code here, to run repeatedly:
	if (GPS_SERIAL.available() > 0) {
		if (gps.encode(GPS_SERIAL.read()) && gps.location.isValid() && gps.location.isUpdated()) {
			x = gps.location.lat();
			y = gps.location.lng();
			Cayenne.loop(0);
		}
	}
}

// Default function for sending sensor data at intervals to Cayenne.
// You can also use functions for specific channels, e.g CAYENNE_OUT(1) for sending channel 1 data.
CAYENNE_OUT_DEFAULT() {
	sendGpsData();
}

// Default function for processing actuator commands from the Cayenne Dashboard.
// You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
CAYENNE_IN_DEFAULT() {
	CAYENNE_LOG("Channel %u, value %s", request.channel, getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
