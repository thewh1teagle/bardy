#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include "html.h"
// SKETCH BEGIN
AsyncWebServer server(80);

#define FORMAT_SPIFFS_IF_FAILED true

void writeFile(const char *path, const char *message) {
    Serial.printf("Writing file: %s\r\n", path);

    File file = SPIFFS.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("file written");
    } else {
        Serial.println("frite failed");
    }
}

String getFiles(String path) {
    File root = SPIFFS.open("/");
    File file = root.openNextFile();

    // Create a JSON object to store the file information
    DynamicJsonDocument jsonBuffer(1024);  // Adjust the buffer size based on your requirements
    JsonArray jsonArray = jsonBuffer.to<JsonArray>();

    while (file) {
        // Create an object to store file details
        JsonObject fileObject = jsonArray.createNestedObject();
        fileObject["name"] = file.name();
        fileObject["size"] = file.size();
        fileObject["isDir"] = file.isDirectory();

        // Move to the next file
        file = root.openNextFile();
    }

    // Serialize the JSON object into a string
    String jsonData;
    serializeJson(jsonArray, jsonData);

    // Close the root directory
    root.close();

    // Return the JSON data as a string
    return jsonData;
}

int deleteFile(String path) {
    Serial.printf("Deleting file: %s\r\n", path);
    if (!path.startsWith("/")) {
        path = "/" + path;
    }
    if (SPIFFS.remove(path)) {
        return 0;
    } else {
        return -1;
    }
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
        logmessage = "Upload Start: " + String(filename);
        // open the file on first call and store the file handle in the request object
        request->_tempFile = SPIFFS.open("/" + filename, "w");
        Serial.println(logmessage);
    }

    if (len) {
        // stream the incoming chunk to the opened file
        request->_tempFile.write(data, len);
        logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
        Serial.println(logmessage);
    }

    if (final) {
        logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
        // close the file handle as the upload is now done
        request->_tempFile.close();
        Serial.println(logmessage);
        request->redirect("/");
    }
}

void startWebFs() {
    SPIFFS.begin();
    writeFile("/hola.txt", "hello world!");
    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });

    server.on(
        "/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
            request->send(200);
        },
        handleUpload);

    server.on("/dir", HTTP_GET, [](AsyncWebServerRequest *request) {
        String filesJson = getFiles("/");
        request->send(200, "application/json", filesJson);
    });

    server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
        String filesJson = getFiles("/");
        request->send(200, "application/json", filesJson);
    });

    server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
        String name = request->getParam("name")->value();
        deleteFile(name);
        request->send(200, "text/plain", "");
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", HTML);
    });
    server.serveStatic("/", SPIFFS, "/");

    server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index)
            Serial.printf("UploadStart: %s\n", filename.c_str());
        Serial.printf("%s", (const char *)data);
        if (final)
            Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
    });

    server.begin();
}