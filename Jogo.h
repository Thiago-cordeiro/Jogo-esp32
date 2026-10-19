#pragma once
#include "Configuracao.h"

enum EstadoJogo {
  FASE_1,
  FASE_2,
  FASE_3,
  JOGO_VENCIDO,
  JOGO_PERDIDO
};

extern EstadoJogo faseAtual;

// ---------------------------
// CONTROLE DOS BOTOES
// ---------------------------

struct EstadoBotao {
  int ultimaLeitura;
  int estadoEstavel;
  unsigned long ultimaMudanca;
  bool pressionado;
};

extern EstadoBotao botoes[4];


void apagarLeds();
void acenderSomenteLed(int indice);
void tocarBuzzer(unsigned long duracao);
void atualizarBotoes();
void iniciarNovoJogo();
