#include "Jogo.h"
#include "Fase2.h"
#include "Fase3.h"

namespace {
bool fase2Inicializada = false;

int tentativasFase2 = 0;
int acertosFase2 = 0;

// Verde -> Amarelo -> Vermelho -> Amarelo
const int sequenciaFase2[4] = {
  0, 1, 2, 1
};

int posicaoSequenciaFase2 = 0;

unsigned long ultimoMovimentoFase2 = 0;
unsigned long inicioErroFase2 = 0;

bool avisoErroFase2 = false;
bool aguardandoSoltarB2 = false;

} // namespace

void reiniciarFase2() {
  fase2Inicializada = false;
  tentativasFase2 = 0;
  acertosFase2 = 0;
  avisoErroFase2 = false;
  aguardandoSoltarB2 = false;
}

// Reiniciar movimento pelo LED verde
void reiniciarMovimentoFase2(unsigned long agora) {

  posicaoSequenciaFase2 = 0;

  acenderSomenteLed(
    sequenciaFase2[posicaoSequenciaFase2]
  );

  ultimoMovimentoFase2 = agora;
}

// ---------------------------
// INICIALIZAR FASE 2
// ---------------------------

void iniciarFase2() {

  tentativasFase2 = 0;
  acertosFase2 = 0;

  avisoErroFase2 = false;
  // Exigir B2 solto antes da primeira tentativa
  aguardandoSoltarB2 = true;

  digitalWrite(BUZZER_PIN, LOW);

  reiniciarMovimentoFase2(millis());

  Serial.println();
  Serial.println("======================");
  Serial.println("===== FASE 2 =====");
  Serial.println("======================");

  Serial.println("INTERROMPER O SINAL");
  Serial.println("Pressione o botao AMARELO");
  Serial.println("quando o LED AMARELO estiver aceso.");

  Serial.println("Objetivo: 3 acertos.");
  Serial.println("Limite: 5 tentativas.");

  fase2Inicializada = true;
}

// ---------------------------
// EXECUTAR FASE 2
// ---------------------------

void executarFase2() {

  if (!fase2Inicializada) {
    iniciarFase2();
    return;
  }

  unsigned long agora = millis();

  // ---------------------------
  // AGUARDAR SOLTAR B2
  // ---------------------------

  if (
    aguardandoSoltarB2 &&
    botoes[1].estadoEstavel == HIGH
  ) {
    aguardandoSoltarB2 = false;
  }

  // ---------------------------
  // AVISO SONORO DE ERRO
  // ---------------------------

  if (avisoErroFase2) {

    // Enquanto o buzzer toca,
    // os LEDs nao se movimentam.

    if (
      agora - inicioErroFase2 >= DURACAO_ERRO_FASE2
    ) {

      digitalWrite(BUZZER_PIN, LOW);

      avisoErroFase2 = false;

      // Retomar pelo LED verde
      reiniciarMovimentoFase2(agora);
    }

    return;
  }

  // ---------------------------
  // VERIFICAR BOTAO CENTRAL
  // ---------------------------

  if (
    botoes[1].pressionado &&
    !aguardandoSoltarB2
  ) {

    // Cada acionamento consome uma tentativa.
    tentativasFase2++;

    // Exigir soltar antes da proxima tentativa.
    aguardandoSoltarB2 = true;

    int ledAtual =
      sequenciaFase2[posicaoSequenciaFase2];

    bool acertou = ledAtual == 1;

    Serial.println();
    Serial.print("Tentativa ");
    Serial.print(tentativasFase2);
    Serial.print("/");
    Serial.println(MAX_TENTATIVAS);

    // ---------------------------
    // ACERTO
    // ---------------------------

    if (acertou) {

      acertosFase2++;

      Serial.println("ACERTO!");
      Serial.print("Acertos: ");
      Serial.print(acertosFase2);
      Serial.print("/");
      Serial.println(ACERTOS_NECESSARIOS_FASE2);

    }

    // ---------------------------
    // ERRO
    // ---------------------------

    else {

      Serial.println("ERRO!");
      Serial.println("O LED central nao estava aceso.");

      Serial.print("Acertos acumulados: ");
      Serial.println(acertosFase2);
    }

    // ---------------------------
    // VITORIA
    // ---------------------------

    if (acertosFase2 >= ACERTOS_NECESSARIOS_FASE2) {

      faseAtual = FASE_3;
      reiniciarFase3();

      apagarLeds();

      Serial.println();
      Serial.println("======================");
      Serial.println("FASE 2 CONCLUIDA!");
      Serial.println("======================");

      Serial.println("Voce conseguiu 3 acertos!");
      Serial.println("Proxima etapa: FASE 3.");

      tocarBuzzer(500);

      return;
    }

    // ---------------------------
    // DERROTA
    // ---------------------------

    if (tentativasFase2 >= MAX_TENTATIVAS) {

      faseAtual = JOGO_PERDIDO;

      apagarLeds();

      Serial.println();
      Serial.println("======================");
      Serial.println("FIM DE JOGO!");
      Serial.println("======================");

      Serial.print("Total de acertos: ");
      Serial.println(acertosFase2);

      Serial.println("Tentativas esgotadas!");

      tocarBuzzer(1000);

      Serial.println("Pressione START para reiniciar.");

      return;
    }

    // ---------------------------
    // CONTINUAR A FASE
    // ---------------------------

    if (acertou) {

      // Reiniciar imediatamente pelo LED verde
      reiniciarMovimentoFase2(agora);

    } else {

      // Ativar aviso sonoro sem usar delay().
      // A movimentacao fica pausada.
      avisoErroFase2 = true;
      inicioErroFase2 = agora;

      digitalWrite(BUZZER_PIN, HIGH);
    }

    return;
  }

  // ---------------------------
  // MOVIMENTAR OS LEDS
  // ---------------------------

  if (
    agora - ultimoMovimentoFase2 >=
    INTERVALO_LED_FASE2
  ) {

    ultimoMovimentoFase2 = agora;

    posicaoSequenciaFase2++;

    if (posicaoSequenciaFase2 >= 4) {
      posicaoSequenciaFase2 = 0;
    }

    int proximoLed =
      sequenciaFase2[posicaoSequenciaFase2];

    acenderSomenteLed(proximoLed);
  }
}
