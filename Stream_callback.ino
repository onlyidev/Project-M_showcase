#include <WiFi.h>
#include <FirebaseESP32.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

#define FIREBASE_HOST "project-m-3164c.firebaseio.com" //Without http:// or https:// schemes
#define FIREBASE_AUTH "7cqUHT5soqFzCevaOq4TCLuwcuAK2LjH9Njk3uaS"
FirebaseData firebaseData;

bool g[12];
int forWrite;

int data = 19;
int clk = 18;

void setup()
{

  Serial.begin(115200);
  Serial.println("Booting");
  pinMode(data, OUTPUT);
  pinMode(clk, OUTPUT);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(2000);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(firebaseData, "/Lockers"))
  {
    Serial.println("ERROR IN SETUP");
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else Serial.println("Connected to Firebase");
  for(auto &i : g) i = false;
  forWrite = 4094;
  digitalWrite(clk, LOW);
  write();
}

void loop()
{
  if (firebaseData.streamAvailable()) {
    Serial.println("------------------------------------");
    Serial.println("Stream Data available...");
    Serial.println("STREAM PATH: " + firebaseData.streamPath());
    Serial.println("EVENT PATH: " + firebaseData.dataPath());
    Serial.println("DATA TYPE: " + firebaseData.dataType());
    Serial.println("EVENT TYPE: " + firebaseData.eventType());
    Serial.println(firebaseData.dataType());
    if(firebaseData.dataType() == "boolean") {
      Serial.println(firebaseData.dataPath());
      Serial.println(firebaseData.boolData());
      String tmp = firebaseData.dataPath();
      tmp.remove(0, 1);
      int del = tmp.toInt();
      if(g[del-1] != firebaseData.boolData()) {
        g[del-1] = firebaseData.boolData();
        forWrite += (firebaseData.boolData() ? -1 : 1) * (int)(1<<(del-1));
        write();
      }
    }
  }

  if (!Firebase.readStream(firebaseData))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't read stream data...");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  if (firebaseData.streamTimeout())
  {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }

}

void write() {
  shiftOut(data, clk, MSBFIRST, ~(forWrite>>8));
  shiftOut(data, clk, MSBFIRST, ~(forWrite));
  Serial.println(forWrite);
}
