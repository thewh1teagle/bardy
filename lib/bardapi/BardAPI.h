#ifndef BardAPI_h
#define BardAPI_h

#include <Arduino.h>
#include <HTTPClient.h>

#include <map>

class BardAPI {
    HTTPClient client;
    String token;
    String snime;

    String conversationId;
    String responseId;
    String choiceId;
    int reqId;

    bool debug = false;

   public:
    BardAPI(String token);
    void setDebug(bool status);
    String getAnswer(String question);
    String speech(String text, String lang);
    String ask_about_image(String text, uint8_t *image, int imageSize, String lang);
    void setup();

   private:
    void setSessionHeaders();
    std::map<String, String> getHeaders();
    void getSnime();
    String uploadImage(uint8_t *data, size_t size);
    void setHeadersMap(std::map<String, String> headers);
};

#endif