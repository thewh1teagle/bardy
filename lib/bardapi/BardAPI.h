#ifndef BardAPI_h
#define BardAPI_h

#include <Arduino.h>
#include <HTTPClient.h>

class BardAPI {
    HTTPClient client;
    String token;
    String snime;

    String conversationId;
    String responseId;
    String choiceId;
    int reqId;

   public:
    BardAPI(String token);
    String getAnswer(String question);

   private:
    void setHeaders();
    void getSnime();
};

#endif