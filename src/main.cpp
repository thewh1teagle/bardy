#include <ArduinoJson.h>
#include <BardAPI.h>
#include <WiFiClientSecure.h>
#include <webfs.h>

#include <base64.hpp>

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
    startWebFs();  // start web file manager in background
    bard.setDebug(true);
    bard.setup();
    // String answer = bard.getAnswer("hi how are you?");
    // Serial.print("Bard say: ");
    // Serial.println(answer);
    // bard.speech("how are you?", "iw-IL");

    // unsigned char imageb64[] = "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";
    // int size = strlen((char *)imageb64);
    // unsigned char imageBuf[200];
    unsigned char imageBuf[2] = {0x1, 0x2};
    // size = decode_base64(imageb64, 2, imageBuf);
    bard.ask_about_image("what is in the image?", imageBuf, 2, "iw-IL");
}

void loop() {}