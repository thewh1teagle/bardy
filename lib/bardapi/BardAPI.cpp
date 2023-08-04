#include "BardAPI.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include <map>

#include "UUID.h"
#define JSON_BUFFER_SIZE 1024

UUID uuid;

String lineSlice(String str, int index) {
    int currentLine = 0;
    int startPos = 0;
    int endPos = -1;

    while (currentLine <= index) {
        endPos = str.indexOf('\n', startPos);  // Find the next newline character
        if (endPos == -1) {
            // Reached the end of the string, break the loop
            break;
        }
        if (currentLine == index) {
            // Extract and print the line
            String line = str.substring(startPos, endPos);
            return line;
        }
        currentLine++;
        startPos = endPos + 1;  // Move the start position to the character after the newline
    }

    // If the desired line index is out of range, print an error message
    if (currentLine <= index) {
        return "";
    }
}

String urlencode(String str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encodedString += '+';
        } else if (isalnum(c)) {
            encodedString += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            code2 = '\0';
            encodedString += '%';
            encodedString += code0;
            encodedString += code1;
            // encodedString+=code2;
        }
        yield();
    }
    return encodedString;
}

BardAPI::BardAPI(String token) : token(token) {
    reqId = 0;
    conversationId = "";
    responseId = "";
    choiceId = "";
}

void BardAPI::setDebug(bool status) {
    debug = status;
}

void BardAPI::setup() {
    if (debug) {
        Serial.println("Getting snime...");
    }
    getSnime();
    if (snime == "") {
        Serial.println("Error: Snime value not found!");
    } else {
        if (debug) {
            Serial.println("Got snime value: " + snime);
        }
    }
}

void BardAPI::setSessionHeaders() {
    const String cookieString = "__Secure-1PSID=" + token + ";";
    const std::map<String, String> headers = {
        {"Host", "bard.google.com"},
        {"X-Same-Domain", "1"},
        {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36"},
        {"Content-Type", "application/x-www-form-urlencoded;charset=UTF-8"},
        {"Origin", "https://bard.google.com"},
        {"Referer", "https://bard.google.com/"},
        {"Cookie", cookieString}};
    setHeadersMap(headers);
}

void BardAPI::getSnime() {
    client.begin("https://bard.google.com/");
    setSessionHeaders();
    int code = client.GET();

    if (code == 200) {
        String body = client.getString();
        int startIndex = body.indexOf("SNlM0e\":\"");
        if (startIndex != -1) {
            startIndex += 9;  // Move to the end of "SNlM0e\":\""
            int endIndex = body.indexOf("\"", startIndex);
            if (endIndex != -1) {
                String extracted_string = body.substring(startIndex, endIndex);
                snime = extracted_string;
            } else {
            }
        } else {
        }
    }
}

String BardAPI::getAnswer(String question) {
    char buffer[2048];
    String payload = R"===(f.req=[null,+"[[\"%s\"],+null,+[\"%s\",+\"%s\",+\"%s\"]]"]&at=%s)===";
    sprintf(buffer, payload.c_str(), question.c_str(), conversationId, responseId, choiceId, snime.c_str());

    payload = urlencode(buffer);
    char urlWithParams[256];
    snprintf(urlWithParams, sizeof(urlWithParams), "%s?bl=boq_assistant-bard-web-server_20230713.13_p0&_reqid=%d&rt=c", "https://bard.google.com/_/BardChatUi/data/assistant.lamda.BardFrontendService/StreamGenerate", reqId);
    client.begin(urlWithParams);
    setSessionHeaders();
    client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = client.POST(buffer);
    if (debug) {
        Serial.println("http code: " + httpCode);
    }
    client.end();
    String resStr = client.getString();
    String respDict = lineSlice(resStr, 3);
    DynamicJsonDocument response(1024 * 20);
    JsonArray respArr = response.to<JsonArray>();
    deserializeJson(response, respDict);
    if (debug) {
        serializeJsonPretty(response, Serial);
    }
    DynamicJsonDocument bardAnswer(1024 * 5);
    deserializeJson(bardAnswer, respArr[0][2].as<String>());
    conversationId = bardAnswer[1][0].as<String>();
    responseId = bardAnswer[1][1].as<String>();
    choiceId = bardAnswer[4][0]["id"].as<String>();
    return bardAnswer[4][0][1][0].as<String>();
}

String BardAPI::speech(String text, String lang) {
    char buffer[2048];
    char urlWithParams[256];
    snprintf(urlWithParams, sizeof(urlWithParams), "%s?bl=boq_assistant-bard-web-server_20230713.13_p0&_reqid=%d&rt=c", "https://bard.google.com/_/BardChatUi/data/batchexecute", reqId);
    client.begin(urlWithParams);
    setSessionHeaders();
    client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String payload = R"===(f.req=[[["XqA3Ic","[null,\"%s\",\"%s\",null,2]",null,"generic"]]]&at=%s&)===";
    sprintf(buffer, payload.c_str(), text.c_str(), lang.c_str(), snime.c_str());

    int httpCode = client.POST(buffer);
    if (debug) {
        Serial.println("http code: " + httpCode);
    }
    client.end();
    String resStr = client.getString();
    String respDict = lineSlice(resStr, 3);
    DynamicJsonDocument response(1024 * 20);
    JsonArray respArr = response.to<JsonArray>();
    deserializeJson(response, respDict);
    if (debug) {
        serializeJsonPretty(response, Serial);
    }
    DynamicJsonDocument bardAnswer(1024 * 5);
    deserializeJson(bardAnswer, respArr[0][2].as<String>());
    String audioB64 = bardAnswer[0];
    if (debug) {
        Serial.println("audio base64: " + audioB64);
    }
    return audioB64;
}

void BardAPI::setHeadersMap(std::map<String, String> headers) {
    for (const auto& entry : headers) {
        client.addHeader(entry.first.c_str(), entry.second.c_str());
    }
}

std::map<String, String> BardAPI::getHeaders() {
    std::map<String, String> headers;

    int numHeaders = client.headers();
    for (int i = 0; i < numHeaders; i++) {
        String headerName = client.headerName(i);
        String headerValue = client.header(i);
        headers[headerName] = headerValue;
    }

    return headers;
}

String BardAPI::ask_about_image(String text, uint8_t* image, int imageSize, String lang) {
    auto imageUrl = uploadImage(image, imageSize);
    Serial.println("Image uploaded successuly to " + imageUrl);
    char buffer[2048];
    // String payload = R"===(f.req=[null,"[[\"%text\",0,null,[[[\"%url\",1],\"%imagename\"]]],[\"%lang\"],[\"%cid\",\"%rid\",\"%rcid\",null,null,[]],\"\",\"%uuid\",null,[1],0,[],[],1]"]&at=%at&)===";
    String payload = R"===(f.req=[null,"[[\"%s\",0,null,[[[\"%s\",1],\"%s\"]]],[\"%s\"],[\"%s\",\"%s\",\"%s\",null,null,[]],\"\",\"%s\",null,[1],0,[],[],1]"]&at=%s&)===";

    char* randomUUID = uuid.toCharArray();
    if (debug) {
        Serial.println("uuid: " + String(randomUUID));
    }
    sprintf(buffer, payload.c_str(), text.c_str(), imageUrl.c_str(), "image.jpg", lang.c_str(), choiceId.c_str(), responseId.c_str(), randomUUID, snime.c_str());
    payload = urlencode(buffer);
    char urlWithParams[256];
    snprintf(urlWithParams, sizeof(urlWithParams), "%s?bl=boq_assistant-bard-web-server_20230713.13_p0&_reqid=%d&rt=c", "https://bard.google.com/_/BardChatUi/data/assistant.lamda.BardFrontendService/StreamGenerate", reqId);
    client.begin(urlWithParams);
    setSessionHeaders();
    client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = client.POST(buffer);
    if (debug) {
        Serial.println("http code: " + httpCode);
    }
    client.end();
    String resStr = client.getString();
    String respDict = lineSlice(resStr, 3);
    DynamicJsonDocument response(1024 * 20);
    JsonArray respArr = response.to<JsonArray>();
    deserializeJson(response, respDict);
    if (debug) {
        serializeJsonPretty(response, Serial);
    }
    DynamicJsonDocument bardAnswer(1024 * 5);
    deserializeJson(bardAnswer, respArr[0][2].as<String>());
    conversationId = bardAnswer[1][0].as<String>();
    responseId = bardAnswer[1][1].as<String>();
    choiceId = bardAnswer[4][0]["id"].as<String>();
    return bardAnswer[4][0][1][0].as<String>();
}

String BardAPI::uploadImage(uint8_t* data, size_t size) {
    client.begin("https://content-push.googleapis.com/upload/");

    std::map<String, String> headers = {
        {"authority", "content-push.googleapis.com"},
        {"accept", "*/*"},
        {"accept-language", "en-US,en;q=0.7"},
        {"authorization", "Basic c2F2ZXM6cyNMdGhlNmxzd2F2b0RsN3J1d1U="},
        {"content-type", "application/x-www-form-urlencoded;charset=UTF-8"},
        {"origin", "https://bard.google.com"},
        {"push-id", "feeds/mcudyrk2a4khkz"},
        {"referer", "https://bard.google.com/"},
        {"x-goog-upload-command", "start"},
        {"x-goog-upload-header-content-length", ""},
        {"x-goog-upload-protocol", "resumable"},
        {"x-tenant-id", "bard-storage"},
        {"size", String(size)}};
    setHeadersMap(headers);
    setSessionHeaders();

    int status = client.sendRequest("OPTIONS");
    client.end();
    if (status != 200) {
        Serial.println("Cant upload file [OPTIONS]");
        return "";
    } else {
        client.begin("https://content-push.googleapis.com/upload/");
        setHeadersMap(headers);
        setSessionHeaders();
        String payload = "File name: Photo.jpg";
        status = client.POST(payload);
        if (status != 200) {
            Serial.println("Cant upload image [POST]");
            return "";
        } else {
            auto resHeaders = getHeaders();
            client.end();
            const auto uploadUrl = resHeaders["X-Goog-Upload-Url"];
            headers["x-goog-upload-command"] = "upload, finalize";
            headers["X-Goog-Upload-Offset"] = "0";
            Serial.println("Uploading image to " + uploadUrl);
            client.begin(uploadUrl);
            setSessionHeaders();
            setHeadersMap(headers);
            status = client.POST(data, size);
            if (status != 200) {
                client.end();
                Serial.println("Cant upload image [Resumable]");
                return "";
            } else {
                client.end();
                return client.getString();
            }
        }
    }
}