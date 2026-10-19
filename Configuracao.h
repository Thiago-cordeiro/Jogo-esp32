#pragma once


#include <Arduino.h>

// ---------------------------
// PINOS
// ---------------------------

const int LED_PIN_VERDE = 12;
const int LED_PIN_AMARELO = 14;
const int LED_PIN_VERMELHO = 27;

const int BUTTON_PIN_VERDE = 33;
const int BUTTON_PIN_AMARELO = 25;
const int BUTTON_PIN_VERMELHO = 26;

const int BUTTON_START = 13;
const int BUZZER_PIN = 32;

// ---------------------------
// CONSTANTES
// ---------------------------

const int TOTAL_CORES = 3;
const int MAX_TENTATIVAS = 5;

const int ACERTOS_NECESSARIOS_FASE2 = 3;
const int MAX_MOVIMENTOS_FASE3 = 5;

const unsigned long DURACAO_TOQUE_FASE3 = 80;
const unsigned long INTERVALO_AVISO_FASE3 = 100;

const unsigned long DEBOUNCE_TIME = 50;
const unsigned long INTERVALO_LED_FASE2 = 400;
const unsigned long DURACAO_ERRO_FASE2 = 500;

const bool DEBUG_SENHA = false;

const int ledPins[TOTAL_CORES] = {
  LED_PIN_VERDE,
  LED_PIN_AMARELO,
  LED_PIN_VERMELHO
};

const int buttonPins[4] = {
  BUTTON_PIN_VERDE,
  BUTTON_PIN_AMARELO,
  BUTTON_PIN_VERMELHO,
  BUTTON_START
};

const char* cores[TOTAL_CORES] = {
  "Verde",
  "Amarelo",
  "Vermelho"
};
