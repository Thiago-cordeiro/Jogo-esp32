#include "Jogo.h"
#include "Fase1.h"
#include "Fase2.h"

namespace {
bool fase1Inicializada = false;

int senha[TOTAL_CORES];
int resposta[TOTAL_CORES];

int posicaoFase1 = 0;
int tentativasFase1 = 0;

} // namespace

void reiniciarFase1() {
  fase1Inicializada = false;
  posicaoFase1 = 0;
  tentativasFase1 = 0;
}

void executarFase1() {

  // ---------------------------
  // INICIALIZACAO
  // ---------------------------

  if (!fase1Inicializada) {

    randomSeed(micros());

    for (int i = 0; i < TOTAL_CORES; i++) {
      senha[i] = i;
    }

    // Embaralhar senha
    for (int i = TOTAL_CORES - 1; i > 0; i--) {

      int j = random(i + 1);

      int temp = senha[i];
      senha[i] = senha[j];
      senha[j] = temp;
    }

    apagarLeds();

    if (DEBUG_SENHA) {

      Serial.print("Senha: ");

      for (int i = 0; i < TOTAL_CORES; i++) {
        Serial.print(cores[senha[i]]);
        Serial.print(" ");
      }

      Serial.println();
    }

    Serial.println();
    Serial.println("===== FASE 1 =====");
    Serial.println("Descubra a senha!");
    Serial.println("Voce possui 5 tentativas.");
    Serial.println("Informe 3 cores e pressione START.");

    fase1Inicializada = true;
    return;
  }

  // ---------------------------
  // BOTOES DAS CORES
  // ---------------------------

  for (int i = 0; i < TOTAL_CORES; i++) {

    if (!botoes[i].pressionado) {
      continue;
    }

    if (posicaoFase1 >= TOTAL_CORES) {
      continue;
    }

    resposta[posicaoFase1] = i;
    posicaoFase1++;

    Serial.print("Posicao ");
    Serial.print(posicaoFase1);
    Serial.print(": ");
    Serial.println(cores[i]);

    if (posicaoFase1 == TOTAL_CORES) {
      Serial.println("Combinacao completa! Pressione START.");
    }
  }

  // ---------------------------
  // BOTAO START
  // ---------------------------

  if (!botoes[3].pressionado) {
    return;
  }

  if (posicaoFase1 < TOTAL_CORES) {

    Serial.println("Informe as 3 cores antes de confirmar!");
    return;
  }

  // Consumir tentativa
  tentativasFase1++;

  bool acertou = true;

  Serial.println();
  Serial.print("Tentativa ");
  Serial.print(tentativasFase1);
  Serial.print("/");
  Serial.println(MAX_TENTATIVAS);

  // ---------------------------
  // VERIFICAR SENHA
  // ---------------------------

  for (int p = 0; p < TOTAL_CORES; p++) {

    bool correta = resposta[p] == senha[p];

    digitalWrite(
      ledPins[p],
      correta ? HIGH : LOW
    );

    if (!correta) {
      acertou = false;
    }

    Serial.print("Posicao ");
    Serial.print(p + 1);
    Serial.print(": ");

    Serial.println(
      correta ? "CORRETA" : "INCORRETA"
    );
  }

  posicaoFase1 = 0;

  // ---------------------------
  // VITORIA
  // ---------------------------

  if (acertou) {

    Serial.println();
    Serial.println("FASE 1 CONCLUIDA!");
    Serial.println("Avancando para FASE 2!");

    tocarBuzzer(500);

    faseAtual = FASE_2;
    reiniciarFase2();

    return;
  }

  // ---------------------------
  // DERROTA
  // ---------------------------

  if (tentativasFase1 >= MAX_TENTATIVAS) {

    faseAtual = JOGO_PERDIDO;

    Serial.println();
    Serial.println("FIM DE JOGO!");
    Serial.println("Tentativas esgotadas.");

    tocarBuzzer(1000);
    apagarLeds();

    Serial.println("Pressione START para reiniciar.");
    return;
  }

  // ---------------------------
  // CONTINUAR TENTANDO
  // ---------------------------

  Serial.println();
  Serial.println("Senha incorreta!");
  Serial.println("Informe uma nova combinacao.");

  tocarBuzzer(100);
}
