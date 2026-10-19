#include "Jogo.h"
#include "Fase3.h"

namespace {
bool fase3Inicializada = false;
bool aguardandoSoltarFase3[TOTAL_CORES];
int movimentosFase3 = 0;

// Bits: verde = 1, amarelo = 2, vermelho = 4.
const int PAINEL_COMPLETO = 7;
const int efeitosBotoesFase3[TOTAL_CORES] = {3, 6, 2};
int painelFase3 = 0;

enum EtapaFase3 {
  JOGANDO,
  AVISO_MOVIMENTO,
  SUCESSO,
  VITORIA,
  DERROTA
};

EtapaFase3 etapaFase3 = JOGANDO;

struct PassoSinal {
  unsigned int frequencia;
  unsigned long duracao;
  int leds; // -1 preserva o painel.
};

PassoSinal avisoMovimento[] = {
  {1200, DURACAO_TOQUE_FASE3, -1},
  {0, 0, -1}, // Intervalo calculado pelos movimentos restantes.
  {1200, DURACAO_TOQUE_FASE3, -1}
};

const PassoSinal sinalSucesso[] = {
  {1600, DURACAO_TOQUE_FASE3, -1}, {0, 80, -1},
  {1600, DURACAO_TOQUE_FASE3, -1}, {0, 80, -1},
  {1600, DURACAO_TOQUE_FASE3, -1}, {0, 160, -1}
};

// Melodia e animacao executadas juntas, sem bloquear a leitura dos botoes.
const PassoSinal sinalVitoria[] = {
  {523, 180, 1}, {659, 180, 2}, {784, 180, 4},
  {1047, 300, 7}, {0, 120, 0},
  {784, 180, 4}, {1047, 400, 7}, {0, 150, 0},
  {0, 150, 7}, {0, 150, 0}, {0, 150, 7}
};

const PassoSinal sinalDerrota[] = {
  {220, 300, 4}, {0, 150, 0},
  {147, 600, 4}, {0, 150, 0}
};

const PassoSinal* sinalAtualFase3 = nullptr;
int totalPassosFase3 = 0;
int passoAtualFase3 = 0;
unsigned long inicioPassoFase3 = 0;

void mostrarPainelFase3(int painel) {
  for (int i = 0; i < TOTAL_CORES; i++) {
    digitalWrite(ledPins[i], (painel & (1 << i)) ? HIGH : LOW);
  }
}

void executarPassoSinalFase3(unsigned long agora) {
  const PassoSinal &passo = sinalAtualFase3[passoAtualFase3];

  noTone(BUZZER_PIN);
  if (passo.frequencia > 0) {
    tone(BUZZER_PIN, passo.frequencia);
  }

  if (passo.leds >= 0) {
    mostrarPainelFase3(passo.leds);
  }

  inicioPassoFase3 = agora;
}

void iniciarSinalFase3(
  EtapaFase3 etapa, const PassoSinal* sinal, int total
) {
  etapaFase3 = etapa;
  sinalAtualFase3 = sinal;
  totalPassosFase3 = total;
  passoAtualFase3 = 0;
  executarPassoSinalFase3(millis());
}

// Retorna true somente depois do ultimo passo.
bool atualizarSinalFase3(unsigned long agora) {
  if (agora - inicioPassoFase3 <
      sinalAtualFase3[passoAtualFase3].duracao) {
    return false;
  }

  passoAtualFase3++;

  if (passoAtualFase3 >= totalPassosFase3) {
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
    return true;
  }

  executarPassoSinalFase3(agora);
  return false;
}

void informarPainelFase3() {
  Serial.print("Painel: ");
  for (int i = 0; i < TOTAL_CORES; i++) {
    Serial.print(cores[i]);
    Serial.print((painelFase3 & (1 << i)) ? " ACESO  " : " APAGADO  ");
  }
  Serial.println();
}

void iniciarFase3() {
  movimentosFase3 = 0;
  etapaFase3 = JOGANDO;

  // random(7) sorteia de 0 a 6: exclui somente os tres LEDs acesos.
  randomSeed(micros());
  painelFase3 = random(PAINEL_COMPLETO);
  mostrarPainelFase3(painelFase3);

  for (int i = 0; i < TOTAL_CORES; i++) {
    aguardandoSoltarFase3[i] = true;
  }

  Serial.println();
  Serial.println("======================");
  Serial.println("===== FASE 3 =====");
  Serial.println("======================");
  Serial.println("RESTAURAR A ENERGIA");
  Serial.println("Deixe todos os LEDs acesos.");
  Serial.println("B1 (VERDE): inverte verde e amarelo.");
  Serial.println("B2 (AMARELO): inverte amarelo e vermelho.");
  Serial.println("B3 (VERMELHO): inverte somente amarelo.");
  Serial.println("Limite: 5 movimentos. START nao interfere.");
  informarPainelFase3();

  fase3Inicializada = true;
}
} // namespace

void reiniciarFase3() {
  fase3Inicializada = false;
  movimentosFase3 = 0;
  painelFase3 = 0;
  etapaFase3 = JOGANDO;
  sinalAtualFase3 = nullptr;
  totalPassosFase3 = 0;
  passoAtualFase3 = 0;
  inicioPassoFase3 = 0;

  for (int i = 0; i < TOTAL_CORES; i++) {
    aguardandoSoltarFase3[i] = true;
  }

  noTone(BUZZER_PIN);
  digitalWrite(BUZZER_PIN, LOW);
}

void executarFase3() {
  if (!fase3Inicializada) {
    iniciarFase3();
    return;
  }

  // ---------------------------
  // SINALIZACOES SEM DELAY
  // ---------------------------

  if (etapaFase3 != JOGANDO) {
    // Descartar entradas durante os avisos e exigir soltar cada botao.
    for (int i = 0; i < TOTAL_CORES; i++) {
      aguardandoSoltarFase3[i] = true;
    }

    if (!atualizarSinalFase3(millis())) {
      return;
    }

    switch (etapaFase3) {
      case AVISO_MOVIMENTO:
        etapaFase3 = JOGANDO;
        break;

      case SUCESSO:
        iniciarSinalFase3(VITORIA, sinalVitoria,
          sizeof(sinalVitoria) / sizeof(sinalVitoria[0]));
        break;

      case VITORIA:
        faseAtual = JOGO_VENCIDO;
        Serial.println("VOCE ESCAPOU! JOGO VENCIDO!");
        Serial.println("Pressione START para novo jogo.");
        break;

      case DERROTA:
        faseAtual = JOGO_PERDIDO;
        Serial.println("Pressione START para reiniciar.");
        break;

      case JOGANDO:
        break;
    }

    return;
  }

  // ---------------------------
  // MOVIMENTOS DO PAINEL
  // ---------------------------

  for (int i = 0; i < TOTAL_CORES; i++) {
    if (aguardandoSoltarFase3[i]) {
      if (botoes[i].estadoEstavel == HIGH &&
          botoes[i].ultimaLeitura == HIGH) {
        aguardandoSoltarFase3[i] = false;
      }
      continue;
    }

    if (!botoes[i].pressionado) {
      continue;
    }

    aguardandoSoltarFase3[i] = true;
    painelFase3 ^= efeitosBotoesFase3[i];
    movimentosFase3++;
    mostrarPainelFase3(painelFase3);

    Serial.println();
    Serial.print("Movimento ");
    Serial.print(movimentosFase3);
    Serial.print("/");
    Serial.println(MAX_MOVIMENTOS_FASE3);
    informarPainelFase3();

    // Verificar sucesso antes do limite: o quinto movimento pode vencer.
    if (painelFase3 == PAINEL_COMPLETO) {
      Serial.println("FASE 3 CONCLUIDA! ENERGIA RESTAURADA!");
      iniciarSinalFase3(SUCESSO, sinalSucesso,
        sizeof(sinalSucesso) / sizeof(sinalSucesso[0]));
      return;
    }

    if (movimentosFase3 >= MAX_MOVIMENTOS_FASE3) {
      Serial.println("FIM DE JOGO! Movimentos esgotados.");
      iniciarSinalFase3(DERROTA, sinalDerrota,
        sizeof(sinalDerrota) / sizeof(sinalDerrota[0]));
      return;
    }

    int restantes = MAX_MOVIMENTOS_FASE3 - movimentosFase3;
    Serial.print("Movimentos restantes: ");
    Serial.println(restantes);
    avisoMovimento[1].duracao = restantes * INTERVALO_AVISO_FASE3;
    iniciarSinalFase3(AVISO_MOVIMENTO, avisoMovimento,
      sizeof(avisoMovimento) / sizeof(avisoMovimento[0]));

    // Acionamentos simultaneos sao descartados durante a sinalizacao.
    return;
  }
}
