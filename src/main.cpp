#include <ArduinoJson.h>
#include <BardAPI.h>
#include <WiFiClientSecure.h>

#include "env.h"

WiFiClientSecure client;
BardAPI bard(BARD_TOKEN);

void setup_netowrk() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    client.setInsecure();
}

void setup() {
    Serial.begin(115200);
    setup_netowrk();
    String answer = bard.getAnswer("hi how are you?");
    Serial.print("Bard say: ");
    Serial.println(answer);
}

void loop() {}