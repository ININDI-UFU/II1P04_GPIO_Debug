#include <Arduino.h>
#include "services/ads1115.h"         // ads1115  — ADS1115 ADC externo
#include "services/display_ssd1306.h" // disp     — display OLED SSD1306

// --------------------------------------------------------------------------
// CONSTANTES DE CONFIGURAÇÃO
// --------------------------------------------------------------------------

// Pinos do kit (mesmos valores de lasecGPIOKit.h)
static constexpr uint8_t PIN_LED1  = 23;  ///< def_pin_D1   — LED / saída digital 1
static constexpr uint8_t PIN_RELE  = 27;  ///< def_pin_RELE — relé
static constexpr uint8_t PIN_SDA   = 21;  ///< def_pin_SDA  — I2C SDA (display)
static constexpr uint8_t PIN_SCL   = 22;  ///< def_pin_SCL  — I2C SCL (display)

// --------------------------------------------------------------------------
// VARIÁVEIS GLOBAIS — visíveis na aba "Variables" do debugger
// --------------------------------------------------------------------------

/** Conta quantas vezes updateAll() foi chamada.
 *  Use "Hit Count" nesta variável para parar na iteração N. */
static uint32_t loopCount = 0;

/** Leitura do Potenciômetro 1 (0 – 32767).
 *  Use "Expression: pot1Value >= 16384" para parar ao ultrapassar 50%. */
static uint16_t pot1Value = 0;

/** Leitura do Potenciômetro 2 (0 – 32767). */
static uint16_t pot2Value = 0;

/** Estado atual do LED D1 (true = aceso). */
static bool ledD1State = false;

/** Estado atual do Relé (true = ativado). */
static bool releState = false;

// --------------------------------------------------------------------------
// FUNÇÕES AUXILIARES
// --------------------------------------------------------------------------

void readSensors() {
    pot1Value = ads1115.analogReadPot1();  // inspecione pot1Value após esta linha
    pot2Value = ads1115.analogReadPot2();  // inspecione pot2Value após esta linha
}

void processOutputs() {
    // LED alterna a cada 5 chamadas (5 * 100 ms)
    if ((loopCount % 5) == 0) {
        ledD1State = !ledD1State;
        digitalWrite(PIN_LED1, ledD1State ? HIGH : LOW);
    }

    // Relé ativa quando pot1 ultrapassa 50% da escala (0–32767)
    releState = (pot1Value >= 16384); // ← breakpoint condicional por valor aqui
    digitalWrite(PIN_RELE, releState ? HIGH : LOW);
}

void updateUI() {
    // Display OLED — linha 2 e 3
    disp.setText(2, ("P1:" + String(pot1Value)).c_str());
    disp.setText(3, ("P2:" + String(pot2Value)).c_str());
    disp.update();

    // Saída serial no formato ">var:ts:val" (compatível com Serial Plotter)
    uint32_t ts = millis();
    Serial.printf(">pot1:%lu:%u|g\r\n",      ts, pot1Value);
    Serial.printf(">pot2:%lu:%u|g\r\n",      ts, pot2Value);
    Serial.printf(">loopCount:%lu:%lu|g\r\n", ts, loopCount);
    Serial.printf(">rele:%lu:%u|g\r\n",      ts, (uint8_t)releState);
}

void updateAll() {
    loopCount++;          // ← (a) breakpoint simples  (b) breakpoint por hit count
    readSensors();        // ← F11 Step Into  → inspeciona pot1Value / pot2Value
    processOutputs();     // ← F11 Step Into  → acompanha lógica LED e relé
    updateUI();           // ← F10 Step Over  → pula detalhes de display e Serial
}

void setup() {
    Serial.begin(115200);

    // Display OLED
    if (!disp.begin(PIN_SDA, PIN_SCL)) {
        Serial.println("Erro: display nao inicializado!");
    }
    disp.setText(1, "Debug Demo");
    disp.update();

    // ADC externo ADS1115
    if (!ads1115.begin()) {
        Serial.println("Erro: ADS1115 nao encontrado!");
    }

    pinMode(PIN_LED1, OUTPUT);  digitalWrite(PIN_LED1, LOW);
    pinMode(PIN_RELE, OUTPUT);  digitalWrite(PIN_RELE, LOW);
}

static uint32_t lastMs = 0;

void loop() {
    const uint32_t now = millis();
    if ((now - lastMs) >= 100) {
        lastMs = now;
        updateAll();  // ← ponto de entrada principal para breakpoints
    }
}