#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

const char *ssid = "InovaIndustria";
const char *password = "industria50";
const char *hostName = KIT_HOSTNAME;

void receivedFunc(std::string str){
  Serial.println(str.c_str());  
}

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(100);
  WiFi.setHostname(hostName);

  // Tenta listen até conseguir
  Serial.begin(115200);
//   Serial.onInputReceived(receivedFunc);
  Serial.println("[IP] is " + String(WiFi.localIP().toString()));

  if (!MDNS.begin(hostName)) Serial.println("[mDNS] begin failed");
  else Serial.println("[mDNS] begin in " + String(hostName));

  ArduinoOTA
      .onStart([]() {Serial.println("[OTA] Start");})
      .onEnd([]() {Serial.println("[OTA] End"); })
      .onProgress([](unsigned int p, unsigned int t) {Serial.println("[OTA] " + String((p*100)/t));})
      .onError([](ota_error_t e) { Serial.println("[OTA] Error " + String(e)); })
      .setHostname(hostName)
      .begin();
}

float t_reta = 0.0f;            // variável de tempo para a reta

void loop() {
  ArduinoOTA.handle();
//   Serial.update();

  uint32_t ts = millis();
  static uint32_t lastRetry0 = 0;
  if (millis() - lastRetry0 > 200) {
    lastRetry0 = millis();
    Serial.printf(">reta:%lu:%f|g\r\n",      ts,10*t_reta);
    t_reta += 1.0f;                      // incrementa o tempo (ajuste a velocidade)
  }

  static float t_seno = 0.0f;            // variável de tempo para o seno
  static uint32_t lastRetry1 = 0;
  if (millis() - lastRetry1 > 100) {
    lastRetry1 = millis();
    Serial.printf(">seno:%lu:%f|g\r\n", ts, sin(t_seno));   // envia para o gráfico
    t_seno += 0.2f;                      // incrementa o tempo (ajuste a velocidade)
    if (t_seno > 2 * M_PI) t_seno = 0;   // reinicia o ciclo a cada 2π
  }
}
