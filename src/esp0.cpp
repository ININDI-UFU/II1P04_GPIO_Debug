#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "services/wserial.h"

const char *ssid = "InovaIndustria";
const char *password = "industria50";
const char *hostName = KIT_HOSTNAME;

void receivedFunc(std::string str){
  wserial.println(str.c_str());  
}

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(100);
  WiFi.setHostname(hostName);

  // Tenta listen até conseguir
  wserial.begin();
  wserial.onInputReceived(receivedFunc);
  wserial.println("[IP] is " + String(WiFi.localIP().toString()));

  if (!MDNS.begin(hostName)) wserial.println("[mDNS] begin failed");
  else wserial.println("[mDNS] begin in " + String(hostName));

  ArduinoOTA
      .onStart([]() {wserial.println("[OTA] Start");})
      .onEnd([]() {wserial.println("[OTA] End"); })
      .onProgress([](unsigned int p, unsigned int t) {wserial.println("[OTA] " + String((p*100)/t));})
      .onError([](ota_error_t e) { wserial.println("[OTA] Error " + String(e)); })
      .setHostname(hostName)
      .begin();
}

float t_reta = 0.0f;            // variável de tempo para a reta

void loop() {
  ArduinoOTA.handle();
  wserial.update();

  uint32_t ts = millis();
  static uint32_t lastRetry0 = 0;
  if (millis() - lastRetry0 > 200) {
    lastRetry0 = millis();
    wserial.plot("reta",10*t_reta);
    t_reta += 1.0f;                      // incrementa o tempo (ajuste a velocidade)
  }

  static float t_seno = 0.0f;            // variável de tempo para o seno
  static uint32_t lastRetry1 = 0;
  if (millis() - lastRetry1 > 100) {
    lastRetry1 = millis();
    wserial.plot("seno",sin(t_seno));   // envia para o gráfico
    t_seno += 0.2f;                      // incrementa o tempo (ajuste a velocidade)
    if (t_seno > 2 * M_PI) t_seno = 0;   // reinicia o ciclo a cada 2π
  }
}
