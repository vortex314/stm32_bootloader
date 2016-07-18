#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Services/ArduinoJson/include/ArduinoJson.h>
#include <Services/WebHelpers/base64.h>
#include <Log.h>
#include <Stm32.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// Forward declarations
void startMqttClient();
void onMessageReceived(String topic, String message);

#define LED_PIN 16
Timer ledTimer;
bool state = true;

Timer procTimer;

// MQTT client
// For quickly check you can use: http://www.hivemq.com/demos/websocket-client/ (Connection= test.mosquitto.org:8080)
MqttClient mqtt("test.mosquitto.org", 1883, onMessageReceived);

// Publish our message
uint32_t id = 0;

void blink() {
	digitalWrite(LED_PIN, state);
	state = !state;
}

void mqttLog(char* s, uint32_t length) {
	if (mqtt.getConnectionState() == eTCS_Connected)
		mqtt.publish("stm32/out/log", s);
}

Bytes bytesIn(300);
Bytes bytesOut(300);
char dataOut[400];

void handle(JsonObject& resp, JsonObject& req) {
	String dataIn = req["data"];
	int len = base64_decode(dataIn.length(),dataIn.c_str(),300,bytesIn.data());	//
	bytesIn.length(len);
	resp["error"] = Stm32::engine(bytesOut,bytesIn);
	len = base64_encode(bytesOut.length(),bytesOut.data(),400,dataOut);
	dataOut[len]='\0';
	resp["data"] = dataOut;
	resp["id"] = req["id"];
	resp["cmd"] = req["cmd"];
	resp["time"] = millis();
}

void publishMessage() {
	if (mqtt.getConnectionState() != eTCS_Connected)
		startMqttClient(); // Auto reconnect
	mqtt.setKeepAlive(5);
	String str(millis());
	mqtt.publish("stm32/out/clock", str); // or publishWithQoS

}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message) {
	LOGF(" recv %s : %s \n", topic.c_str(), message.c_str());

	if (topic == "stm32/in/cmd") {
		StaticJsonBuffer<200> request;
		StaticJsonBuffer<200> reply;
		JsonObject& req = request.parseObject(message);
		JsonObject& repl = request.createObject();
		handle(repl, req);
		String str;
		repl.printTo(str);
		mqtt.publish("stm32/out/cmd", str);
		Serial.printf(" out stm32/out/cmd : %s\n", str.c_str());
	} else {
		LOGF(" unknown topic received %s ", topic.c_str());
	}

}

// Run MQTT client
void startMqttClient() {
	if (!mqtt.setWill("stm32/alive", "false", 1, true)) {
		Serial.println(
				"Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}

	mqtt.connect("esp8266");
	mqtt.subscribe("stm32/in/#");
}

// Will be called when WiFi station was connected to AP
void onWifiConnected() {
	Serial.println("Wifi CONNECTED.");

	// Run MQTT client
	startMqttClient();

	// Start publishing loop
	procTimer.initializeMs(1000, publishMessage).start(); // every 20 seconds
}

// Will be called when WiFi station timeout was reached
void wifiDisconnected() {
	Serial.println("Wifi NOT CONNECTED.");
}

void init() {
	pinMode(LED_PIN, OUTPUT);
	ledTimer.initializeMs(1000, blink).start();

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(false); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(onWifiConnected, 20, wifiDisconnected); // We recommend 20+ seconds for connection timeout at start
}
