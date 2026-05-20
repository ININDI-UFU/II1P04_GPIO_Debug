#include <Arduino.h>
#include "services/lasecNet.h"
#include "services/wserial.h"

const char *hostName = KIT_HOSTNAME;

void receivedFunc(std::string str) {
  wserial.println(str.c_str());
}

void setup() {
  wserial.begin();
  wserial.onInputReceived(receivedFunc);
  
  net.onOtaStart([]() { wserial.println("[OTA] Start"); });
  net.onOtaEnd([]() { wserial.println("[OTA] End"); });
  net.onOtaProgress([](uint32_t p, uint32_t t) {
    wserial.println("[OTA] " + String((p * 100) / t));
  });
  net.onOtaError([](ota_error_t e) {
    wserial.println("[OTA] Error " + String(e));
  });

  if (!net.begin(hostName)) wserial.println("[mDNS] begin failed");
  else {
      wserial.println("[IP] is " + net.localIP().toString());
      wserial.println("[mDNS] begin in " + String(net.hostname()));
  }
}

float t_reta = 0.0f;            // variavel de tempo para a reta

void loop() {
  net.update();
  wserial.update();

  uint32_t ts = millis();
  static uint32_t lastRetry0 = 0;
  if (millis() - lastRetry0 > 200) {
    lastRetry0 = millis();
    wserial.plot("reta", 10 * t_reta);
    t_reta += 1.0f;                      // incrementa o tempo (ajuste a velocidade)
  }

  static float t_seno = 0.0f;            // variavel de tempo para o seno
  static uint32_t lastRetry1 = 0;
  if (millis() - lastRetry1 > 100) {
    lastRetry1 = millis();
    wserial.plot("seno", sin(t_seno));   // envia para o grafico
    t_seno += 0.2f;                      // incrementa o tempo (ajuste a velocidade)
    if (t_seno > 2 * M_PI) t_seno = 0;   // reinicia o ciclo a cada 2*pi
  }
}
