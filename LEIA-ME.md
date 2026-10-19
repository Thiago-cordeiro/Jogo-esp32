# Jogo por fases — ESP32

Projeto separado a partir de fase01.ino. O arquivo original foi preservado.
As pinagens, regras, tempos e mensagens das duas fases existentes foram mantidos.
A fase 3 implementa o painel lógico de restauração da energia, com vitória e reinício.

## Abrir e enviar

1. Mantenha todos os arquivos desta pasta juntos. A pasta deve se chamar `JogoPorFases`.
2. Abra `JogoPorFases.ino` na Arduino IDE.
3. Selecione a mesma placa ESP32 e a mesma porta utilizadas no projeto original.
4. Compile e envie para a placa. Os arquivos `.cpp` da pasta são compilados automaticamente.
5. Abra o Monitor Serial em **115200 baud**.

## Organização

| Arquivo | Responsabilidade |
| --- | --- |
| `JogoPorFases.ino` | setup, loop, botões, LEDs, buzzer e reinício do jogo |
| `Configuracao.h` | Pinagens, constantes, listas de LEDs, botões e cores |
| `Jogo.h` | Estado do jogo, estrutura dos botões e funções compartilhadas |
| `Fase1.cpp` / `Fase1.h` | Descobrir a senha em até cinco tentativas |
| `Fase2.cpp` / `Fase2.h` | Acertar o LED amarelo três vezes em até cinco tentativas |
| `Fase3.cpp` / `Fase3.h` | Restaurar a energia em até cinco movimentos, avisos e animação de vitória |

Cada fase guarda suas próprias variáveis no respectivo `.cpp`.
As funções `reiniciarFase1()`, `reiniciarFase2()` e `reiniciarFase3()` limpam o estado
ao iniciar um jogo. A fase 2 avança automaticamente para `FASE_3`.

## Fase 3 — Restaurar a energia

O painel começa com uma das sete configurações que não possuem todos os LEDs acesos.
O sorteio acontece somente ao entrar na fase. Cada acionamento consome um dos cinco
movimentos; manter um botão pressionado não repete o movimento. START é ignorado.

Use esta tabela como cartão de pistas junto ao painel:

| Botão | Efeito |
| --- | --- |
| B1 — verde | Inverte os LEDs verde e amarelo |
| B2 — amarelo | Inverte os LEDs amarelo e vermelho |
| B3 — vermelho | Inverte somente o LED amarelo |

Ao deixar todos os LEDs acesos, inclusive no quinto movimento, a vitória é automática.
São emitidos três toques rápidos, seguidos de uma melodia e animação dos LEDs.
Se o quinto movimento não concluir o painel, há sinal sonoro de derrota e flashes
do LED vermelho. Após o encerramento, START inicia um novo jogo desde a fase 1.

Cada movimento não conclusivo, enquanto ainda houver movimentos, emite dois toques
com intervalo de 400, 300, 200 ou 100 ms conforme o saldo restante. Esses avisos
representam consumo de movimentos. Durante a sinalização, entradas são descartadas
e os botões precisam ser soltos antes de novo acionamento. Os avisos, a melodia e
a animação da fase 3 usam `millis()`, sem `delay()`.

## Pinagens preservadas

| Componente | GPIO |
| --- | ---: |
| LED verde | 12 |
| LED amarelo | 14 |
| LED vermelho | 27 |
| Botão verde | 33 |
| Botão amarelo | 25 |
| Botão vermelho | 26 |
| Botão START | 13 |
| Buzzer | 32 |

Os botões continuam configurados como `INPUT_PULLUP`, com acionamento em `LOW`.
O debounce permanece em 50 ms; o movimento da fase 2, em 400 ms;
o aviso de erro da fase 2, em 500 ms.

## Verificação

Os testes em `tests/Fase3Teste.cpp` executam o código do projeto com entradas,
saídas e relógio simulados. Foram verificados os 1.701 caminhos de cinco botões
dos sete painéis iniciais, encerrando cada caminho assim que houver vitória.
Isso inclui vitória no quinto movimento, derrota, reinício por START,
debounce, botões mantidos, descarte durante avisos e transição da fase 2.

Os quatro módulos foram compilados em objetos com o compilador Xtensa e os
cabeçalhos reais do Arduino ESP32 3.3.11. Essa verificação não gera um firmware
completo. Compile e envie pela Arduino IDE para validar na placa os LEDs, os
botões, o volume e a melodia do buzzer. A melodia usa `tone()`/`noTone()`;
um buzzer passivo permite distinguir as notas.
