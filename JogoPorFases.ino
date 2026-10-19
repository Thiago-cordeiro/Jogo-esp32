#include "Jogo.h"
#include "Fase1.h"
#include "Fase2.h"
#include "Fase3.h"

EstadoJogo faseAtual = FASE_1;
EstadoBotao botoes[4];

void apagarLeds() {
  for (int i = 0; i < TOTAL_CORES; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void acenderSomenteLed(int indice) {
  for (int i = 0; i < TOTAL_CORES; i++) {
    digitalWrite(
      ledPins[i],
      i == indice ? HIGH : LOW
    );
  }
}

void tocarBuzzer(unsigned long duracao) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duracao);
  digitalWrite(BUZZER_PIN, LOW);
}

// ---------------------------
// LEITURA DOS BOTOES
// ---------------------------

void atualizarBotoes() {

  unsigned long agora = millis();

  for (int i = 0; i < 4; i++) {

    EstadoBotao &botao = botoes[i];

    botao.pressionado = false;

    int leitura = digitalRead(buttonPins[i]);

    if (leitura != botao.ultimaLeitura) {
      botao.ultimaMudanca = agora;
    }

    if (agora - botao.ultimaMudanca >= DEBOUNCE_TIME) {

      if (leitura != botao.estadoEstavel) {

        botao.estadoEstavel = leitura;

        if (leitura == LOW) {
          botao.pressionado = true;
        }
      }
    }

    botao.ultimaLeitura = leitura;
  }
}

// ---------------------------
// INICIAR NOVO JOGO
// ---------------------------

void iniciarNovoJogo() {

  faseAtual = FASE_1;

  reiniciarFase1();
  reiniciarFase2();
  reiniciarFase3();

  digitalWrite(BUZZER_PIN, LOW);
  apagarLeds();

  Serial.println();
  Serial.println("======================");
  Serial.println("NOVO JOGO!");
  Serial.println("======================");
}

// ---------------------------
// SETUP
// ---------------------------

void setup() {

  Serial.begin(115200);

  for (int i = 0; i < TOTAL_CORES; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);

    digitalWrite(ledPins[i], LOW);
  }

  pinMode(BUTTON_START, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Sincronizar os estados iniciais
  for (int i = 0; i < 4; i++) {

    int leitura = digitalRead(buttonPins[i]);

    botoes[i].ultimaLeitura = leitura;
    botoes[i].estadoEstavel = leitura;
    botoes[i].ultimaMudanca = millis();
    botoes[i].pressionado = false;
  }

  Serial.println("ESP32 pronta!");

  iniciarNovoJogo();
}

// ---------------------------
// LOOP PRINCIPAL
// ---------------------------

void loop() {

  atualizarBotoes();

  switch (faseAtual) {

    case FASE_1:
      executarFase1();
      break;

    case FASE_2:
      executarFase2();
      break;

    case FASE_3:
      executarFase3();
      break;

    case JOGO_VENCIDO:
    case JOGO_PERDIDO:
      if (botoes[3].pressionado) {
        iniciarNovoJogo();
      }
      break;
  }
}
