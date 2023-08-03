#include "BardAPI.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#define JSON_BUFFER_SIZE 1024

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
    getSnime();
    if (snime == "") {
        Serial.println("Error: Snime value not found!");
    } else {
    }
    reqId = 0;
    conversationId = "";
    responseId = "";
    choiceId = "";
}

void BardAPI::setHeaders() {
    client.addHeader("Host", "bard.google.com");
    client.addHeader("X-Same-Domain", "1");
    client.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36");
    client.addHeader("Content-Type", "application/x-www-form-urlencoded;charset=UTF-8");
    client.addHeader("Origin", "https://bard.google.com");
    client.addHeader("Referer", "https://bard.google.com/");
    String cookieString = "__Secure-1PSID=" + token + ";";
    client.addHeader("Cookie", cookieString);
}

void BardAPI::getSnime() {
    client.begin("https://bard.google.com/");
    setHeaders();
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
    char buffer[512];
    String payload = R"===(f.req=[null,+"[[\"%s\"],+null,+[\"%s\",+\"%s\",+\"%s\"]]"]&at=%s)===";
    sprintf(buffer, payload.c_str(), question.c_str(), "", "", "", snime.c_str());

    payload = urlencode(buffer);
    char urlWithParams[256];
    snprintf(urlWithParams, sizeof(urlWithParams), "%s?bl=boq_assistant-bard-web-server_20230713.13_p0&_reqid=%d&rt=c", "https://bard.google.com/_/BardChatUi/data/assistant.lamda.BardFrontendService/StreamGenerate", reqId);
    client.begin(urlWithParams);
    setHeaders();
    client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = client.POST(buffer);
    client.end();
    String resStr = client.getString();
    String respDict = lineSlice(resStr, 3);
    DynamicJsonDocument response(1024 * 20);
    JsonArray respArr = response.to<JsonArray>();
    deserializeJson(response, respDict);
    // serializeJsonPretty(response, Serial);
    DynamicJsonDocument bardAnswer(1024 * 5);
    deserializeJson(bardAnswer, respArr[0][2].as<String>());
    return bardAnswer[4][0][1][0].as<String>();
}
