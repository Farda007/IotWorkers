#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>

#define SERVER_IP "192.168.0.100:5000"

#define STASSID "mkuo"
#define STAPSK  "roman47heslo"

class Machine 
{
  public:
  Machine(String line, String name, int count)
  {
    Line = line;
    Name = name;
    Count = count;
  };

  String Line;
  String Name;
  int Count;
};

Machine machs[8] = {
                    Machine("ABS", "Welder#1", 10),
                    Machine("ABS", "Welder#2", 20),
                    Machine("ABS", "Welder#3", 30),
                    Machine("ABS", "Welder#4", 40),
                    Machine("ABS", "Welder#5", 50),
                    Machine("ABS", "Welder#6", 60),
                    Machine("ABS", "Welder#7", 70),
                    Machine("ABS", "Welder#8", 80)
                    };

void setup() {
  Serial.begin(115200);
}

void SendCounters()
{
  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  if ((WiFi.status() == WL_CONNECTED)) {

    char postString[512];
    int l = 0;

    postString[0] = '[';
    postString[1] = '\0';

    for(int n = 0; n < (sizeof(machs) / sizeof(machs[0])); n++)
    {
      l = strlen(postString);
      sprintf(postString + l, "{\"Line\":\"%s\", \"Name\":\"%s\",\"Count\":%d},", &machs[n].Line, &machs[n].Name, machs[n].Count );
    }

    l = strlen(postString);
    if (l > 0)
    {
      postString[l - 1] = ']';
      postString[l] = '\0';
    }

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" SERVER_IP "/Welders/Counters"); //HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(postString);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("Return status: ");
        Serial.println(payload);
        Serial.println("\n");
      if (payload == "OK")
      {
        for(int n = 0; n < (sizeof(machs) / sizeof(machs[0])); n++)
        {
          machs[n].Count = 0;
        }
      }

      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();

    //Update check
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, "192.168.0.100", 5000, "/Welders/UpdateIot");
    switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] Update no new version.");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update ok."); // may not called we reboot the ESP
        break;
    }

    WiFi.disconnect();
  }
}

void loop() {

  for(int n = 0; n < (sizeof(machs) / sizeof(machs[0])); n++)
  {
    machs[n].Count += 10;
  }
  
  SendCounters();

  delay(20000);
}


