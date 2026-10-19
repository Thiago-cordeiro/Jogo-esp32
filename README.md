# Escape Room Eletrônico — Jogo por fases com ESP32

O jogador precisa superar três desafios para escapar: descobrir uma senha de cores,
interromper um sinal no momento certo e restaurar a energia de um painel de LEDs.
O projeto utiliza três LEDs, três botões de controle, um botão START/Enter e um buzzer.
As instruções, jogadas e resultados aparecem no Monitor Serial.

Este documento descreve o comportamento da implementação atual.

## Como preparar e executar

1. Mantenha os arquivos `.ino`, `.cpp` e `.h` juntos na pasta `JogoPorFases`.
2. Monte os componentes conforme a tabela de pinagens abaixo.
3. Abra `JogoPorFases.ino` na Arduino IDE.
4. Para a NodeMCU-32S, selecione **NodeMCU-32S** no pacote **esp32** e a porta utilizada na montagem.
5. Compile e envie o programa. Os arquivos `.cpp` da pasta são compilados junto com o sketch.
6. Abra o Monitor Serial em **115200 baud** para acompanhar o jogo.

Na versão atual, a partida começa automaticamente na fase 1 quando a ESP32 é ligada
ou reiniciada. Depois de uma vitória ou derrota, pressione START para começar novamente.

### Erro de compilação: `multiple definition of cores`

O vetor de nomes das cores é definido em `Configuracao.h`, incluído pelo sketch e
pelos três arquivos `.cpp`. A declaração deve ser `const char* const cores[...]`:
o segundo `const` torna o próprio vetor constante e permite sua definição nesse
cabeçalho sem conflito na ligação. Apenas `const char* cores[...]` deixa o vetor
mutável e gera múltiplas definições. Essa correção já está aplicada no projeto.

## Visão geral da partida

| Fase | Desafio | Limite | Condição de sucesso |
| --- | --- | --- | --- |
| 1 — Descobrir a senha | Informar uma sequência de três cores | 5 tentativas completas | Acertar as três posições da senha |
| 2 — Interromper o sinal | Pressionar B2 quando o LED amarelo estiver aceso | 5 acionamentos de B2 | Acumular 3 acertos |
| 3 — Restaurar a energia | Inverter combinações de LEDs | 5 movimentos | Deixar os três LEDs acesos |

As fases acontecem nessa ordem e a passagem entre elas é automática após o sucesso.
Cada fase tem seu próprio contador; o saldo de uma fase não é transferido para outra.
Não existe limite de tempo para a partida.

## Componentes, pinagens e controles

Organize os LEDs da esquerda para a direita: **verde, amarelo e vermelho**.

| Componente | Identificação | GPIO |
| --- | --- | ---: |
| LED verde | Posição 1, esquerda | 12 |
| LED amarelo | Posição 2, centro | 14 |
| LED vermelho | Posição 3, direita | 27 |
| Botão verde | B1 | 33 |
| Botão amarelo | B2 | 25 |
| Botão vermelho | B3 | 26 |
| Botão START | START/Enter | 13 |
| Buzzer | Sinalização sonora | 32 |

Os botões são configurados como `INPUT_PULLUP`: soltos são lidos como `HIGH` e,
quando acionados, como `LOW`. Na montagem correspondente, cada botão conecta seu
GPIO ao GND quando pressionado. Os LEDs são controlados como saídas digitais:
`HIGH` acende e `LOW` apaga.

### Leitura dos botões e debounce

`atualizarBotoes()`, no arquivo principal, lê os quatro botões a cada passagem pelo
`loop()`. A estrutura `EstadoBotao` guarda a última leitura, o estado estável,
o instante da última mudança e o evento `pressionado`.

Uma mudança só é aceita depois de permanecer estável por **50 ms**, conforme
`DEBOUNCE_TIME`. Esse filtro reduz os acionamentos falsos causados pelo contato
mecânico. O evento `pressionado` aparece apenas na transição estável para `LOW`.
Por isso, manter um botão pressionado não gera novas jogadas: é necessário soltá-lo
antes de pressionar novamente.

## Fase 1 — Descobrir a senha

### Como jogar

O programa prepara uma senha com verde, amarelo e vermelho, cada cor utilizada
exatamente uma vez. A ordem é embaralhada e não aparece ao jogador.
Os três LEDs começam apagados.

1. Pressione os botões das cores na ordem que deseja testar.
2. Após informar três cores, pressione START para confirmar.
3. Observe os LEDs e tente outra combinação, se necessário.

START antes de três entradas não consome tentativa. Depois de três entradas,
novos acionamentos das cores são ignorados até a confirmação. A entrada aceita
cores repetidas, mas uma resposta assim não pode coincidir com a senha completa.

Os LEDs indicam **posições corretas**, conforme a tabela:

| LED aceso | Significado |
| --- | --- |
| Verde | A primeira cor informada está correta |
| Amarelo | A segunda cor informada está correta |
| Vermelho | A terceira cor informada está correta |

Exemplo: para a senha **verde, amarelo, vermelho**, a resposta **verde, vermelho,
amarelo** acende somente o LED verde. O resultado permanece visível enquanto o
jogador informa a próxima combinação. Todas as três posições devem ser digitadas
novamente, inclusive as que já estavam corretas.

Acertar a senha, inclusive na quinta tentativa, leva à fase 2.
Confirmar a quinta combinação incorreta encerra a partida com derrota.

### Como foi implementada

`Fase1.cpp` mantém os vetores `senha` e `resposta`, o índice `posicaoFase1` e o
contador `tentativasFase1`. Essas variáveis ficam no `namespace` anônimo do módulo.

Na primeira chamada de `executarFase1()`, a senha recebe os índices `0`, `1` e `2`.
Um embaralhamento Fisher–Yates troca posições usando `random()`, com a semente
inicializada por `micros()`. A flag `fase1Inicializada` evita repetir essa preparação
a cada passagem pelo `loop()`.

Cada botão preenche uma posição de `resposta`. START confirma somente um vetor
completo e incrementa o contador. Um laço compara `resposta[p]` com `senha[p]` e
atualiza o LED da posição correspondente. A verificação de sucesso ocorre antes
da verificação do limite de tentativas.

`DEBUG_SENHA`, em `Configuracao.h`, está definido como `false`. Se for ativado,
a senha passa a ser impressa no Monitor Serial para depuração.

## Fase 2 — Interromper o sinal

### Como jogar

Apenas um LED fica aceso por vez. A luz percorre continuamente o trajeto:

**Verde → amarelo → vermelho → amarelo → verde → ...**

Pressione **B2, o botão amarelo**, quando o LED amarelo estiver aceso.
Cada acionamento de B2 consome uma tentativa:

- LED amarelo aceso: registra um acerto.
- LED verde ou vermelho aceso: registra um erro.

Os acertos são acumulados; um erro não apaga os anteriores. Deixar a luz passar
sem pressionar B2 não consome tentativa. B1, B3 e START são ignorados nesta fase.
Após uma tentativa, se o desafio continuar, o movimento recomeça pelo LED verde.
É necessário soltar B2 antes de tentar novamente.

São necessários **3 acertos em até 5 tentativas** para avançar à fase 3.
O terceiro acerto na quinta tentativa também é válido. Esgotar as cinco tentativas
com menos de três acertos encerra a partida.

### Como foi implementada

`Fase2.cpp` utiliza `sequenciaFase2 = {0, 1, 2, 1}` para representar o trajeto.
`posicaoSequenciaFase2` indica qual LED deve estar aceso. A função compartilhada
`acenderSomenteLed()` garante que os outros LEDs fiquem apagados.

O deslocamento usa a diferença entre `millis()` e `ultimoMovimentoFase2`.
O intervalo é de **400 ms**, definido em `INTERVALO_LED_FASE2`.
O movimento normal não utiliza `delay()`.

Ao receber B2, o código incrementa `tentativasFase2` e verifica o LED atual.
Se for amarelo, incrementa `acertosFase2`. `aguardandoSoltarB2` exige a liberação
do botão, inclusive na entrada da fase.

Após um erro que não encerre a partida, `avisoErroFase2` mantém o buzzer ligado
por **500 ms**. Durante esse período, os LEDs ficam parados e novas tentativas
não são processadas. Ao terminar, o movimento recomeça pelo verde.

Quando os três acertos são atingidos, o código muda `faseAtual` para `FASE_3`
e chama `reiniciarFase3()` para preparar o próximo desafio.

## Fase 3 — Restaurar a energia

### Como jogar

O painel começa com LEDs acesos ou apagados em uma configuração sorteada.
A configuração com os três LEDs acesos é excluída, pois já representaria a solução.
O sorteio acontece somente na entrada da fase.

O objetivo é deixar todos os LEDs acesos em até **5 movimentos**.
Cada botão inverte uma combinação diferente: um LED aceso apaga e um apagado acende.

Use esta tabela como cartão de pistas junto ao painel:

| Botão | Efeito |
| --- | --- |
| B1 — verde | Inverte os LEDs verde e amarelo |
| B2 — amarelo | Inverte os LEDs amarelo e vermelho |
| B3 — vermelho | Inverte somente o LED amarelo |

B3 altera o **amarelo**, embora o botão seja identificado pela cor vermelha.
Cada acionamento de B1, B2 ou B3 consome um movimento. START não interfere.
Um movimento que não conclui o painel pode fazer parte da solução; ele não é
tratado como uma jogada errada.

Exemplo com todos os LEDs inicialmente apagados:

| Ação | Verde | Amarelo | Vermelho |
| --- | --- | --- | --- |
| Estado inicial | Apagado | Apagado | Apagado |
| Pressionar B1 | Aceso | Aceso | Apagado |
| Pressionar B2 | Aceso | Apagado | Aceso |
| Pressionar B3 | Aceso | Aceso | Aceso |

Todas as sete configurações iniciais permitidas possuem solução em até três
movimentos. O jogador pode usar os cinco movimentos disponíveis.
O sucesso é reconhecido automaticamente, inclusive no quinto movimento,
sem confirmação por START. O quinto movimento sem completar o painel causa derrota.

### Como foi implementada

`Fase3.cpp` representa o painel por três bits de um número inteiro:

| LED | Valor do bit |
| --- | ---: |
| Verde | 1 |
| Amarelo | 2 |
| Vermelho | 4 |

Assim, `0` representa todos apagados e `7` representa todos acesos.
`random(7)` sorteia valores de `0` a `6`, excluindo a configuração completa.
A flag `fase3Inicializada` impede um novo sorteio durante a partida.

Os efeitos dos botões ficam em `efeitosBotoesFase3 = {3, 6, 2}`.
A operação `painelFase3 ^= efeitosBotoesFase3[i]` aplica XOR para inverter
somente os bits dos LEDs afetados. `mostrarPainelFase3()` traduz os bits para
os níveis digitais dos três pinos.

Após cada movimento, o código verifica primeiro se `painelFase3 == 7`.
Somente depois verifica se `movimentosFase3` chegou ao limite.
Essa ordem permite vencer no último movimento disponível.

A fase também possui etapas internas: `JOGANDO`, `AVISO_MOVIMENTO`, `SUCESSO`,
`VITORIA` e `DERROTA`. Elas organizam a interação, os sons e a animação sem
bloquear o `loop()` com `delay()`.

## Sinalizações sonoras e visuais

A implementação atual utiliza as seguintes sinalizações:

| Situação | Sinalização implementada |
| --- | --- |
| Fase 1: tentativa incorreta com saldo | Bipe de 100 ms |
| Fase 1 ou 2: conclusão | Bipe de 500 ms e passagem automática |
| Fase 1 ou 2: derrota | Bipe de 1.000 ms e LEDs apagados |
| Fase 2: erro com saldo | Bipe de 500 ms, com movimento dos LEDs pausado |
| Fase 3: movimento não conclusivo com saldo | Dois toques de 80 ms, com intervalo conforme os movimentos restantes |
| Fase 3: conclusão | Três toques rápidos, seguidos da sequência de vitória |
| Fase 3: vitória | Animação dos LEDs acompanhada de uma sequência de notas |
| Fase 3: derrota | Dois sons mais graves e longos, com flashes do LED vermelho |

Na fase 3, o intervalo entre os dois toques diminui com o saldo:

| Movimentos restantes | Intervalo entre os toques |
| ---: | ---: |
| 4 | 400 ms |
| 3 | 300 ms |
| 2 | 200 ms |
| 1 | 100 ms |

`PassoSinal` descreve cada trecho da sinalização com frequência, duração e estado
dos LEDs. As funções `iniciarSinalFase3()`, `executarPassoSinalFase3()` e
`atualizarSinalFase3()` percorrem esses trechos usando `millis()`.
Durante a sinalização da fase 3, as entradas são descartadas e é necessário soltar
os botões antes de uma nova jogada. Acionamentos simultâneos são processados na
ordem B1, B2 e B3; depois que um movimento inicia o aviso, os demais são descartados.

Nas fases 1 e 2, a função compartilhada `tocarBuzzer()` usa `HIGH`, `delay()` e
`LOW` para os bipes de conclusão e derrota, além do erro da fase 1.
A temporização do erro da fase 2 e todas as sinalizações da fase 3 usam `millis()`.

### Observação sobre o buzzer ativo da montagem

A montagem informada utiliza **buzzer ativo**. As fases 1 e 2 o acionam ligando
e desligando a saída digital. Na fase 3, o código atual utiliza `tone()` e
`noTone()`, incluindo frequências diferentes na vitória.

O buzzer ativo possui geração própria de som; por isso, essa sequência não deve
ser interpretada como uma melodia com notas distinguíveis nessa montagem.
O som pode sair como bipes ou ter um timbre irregular. A animação dos LEDs é
controlada separadamente. A adaptação da fase 3 para bipes por `HIGH`/`LOW`
ainda não foi feita no código.

## Organização do código

| Arquivo | Responsabilidade |
| --- | --- |
| `JogoPorFases.ino` | `setup()`, `loop()`, leitura dos botões, funções compartilhadas e novo jogo |
| `Configuracao.h` | GPIOs, limites, tempos e listas de LEDs, botões e cores |
| `Jogo.h` | Estados da partida, estrutura `EstadoBotao` e declarações compartilhadas |
| `Fase1.cpp` / `Fase1.h` | Senha, entradas, comparação por posição e controle das tentativas |
| `Fase2.cpp` / `Fase2.h` | Percurso dos LEDs, tentativas, acertos e pausa de erro |
| `Fase3.cpp` / `Fase3.h` | Painel lógico, movimentos, sinalizações e animação final |
| `tests/Arduino.h` | Funções Arduino simuladas, utilizadas somente nos testes |
| `tests/Fase3Teste.cpp` | Testes automatizados da fase 3 e da integração com o jogo |

Cada fase mantém suas variáveis internas em um `namespace` anônimo no respectivo
`.cpp`, evitando que outros módulos as alterem diretamente. Os arquivos `.h`
expõem as funções de execução e reinício.

### Fluxo principal e reinício

`setup()` configura os pinos, inicia a comunicação serial, sincroniza a leitura
inicial dos botões e chama `iniciarNovoJogo()`.

Em cada repetição, `loop()` atualiza os botões e usa um `switch (faseAtual)` para
chamar `executarFase1()`, `executarFase2()` ou `executarFase3()`.
Os estados `JOGO_VENCIDO` e `JOGO_PERDIDO` aguardam um novo acionamento de START.

`iniciarNovoJogo()` retorna a `FASE_1`, chama as três funções `reiniciarFaseX()`,
interrompe o som da fase 3 e apaga os LEDs. Os contadores e índices são restaurados;
a senha e as entradas são preparadas ou sobrescritas conforme uma nova fase é
inicializada e novas jogadas são informadas.

### Principais constantes

| Constante | Valor atual | Uso |
| --- | ---: | --- |
| `TOTAL_CORES` | 3 | Quantidade de LEDs e botões de cores |
| `MAX_TENTATIVAS` | 5 | Limite das fases 1 e 2 |
| `ACERTOS_NECESSARIOS_FASE2` | 3 | Objetivo da fase 2 |
| `MAX_MOVIMENTOS_FASE3` | 5 | Limite de movimentos da fase 3 |
| `DEBOUNCE_TIME` | 50 ms | Estabilização dos botões |
| `INTERVALO_LED_FASE2` | 400 ms | Tempo de cada posição da luz |
| `DURACAO_ERRO_FASE2` | 500 ms | Duração da pausa com aviso de erro |
| `DURACAO_TOQUE_FASE3` | 80 ms | Duração dos toques curtos da fase 3 |
| `INTERVALO_AVISO_FASE3` | 100 ms | Base multiplicada pelo saldo de movimentos |

## Verificação realizada

Os testes em `tests/Fase3Teste.cpp` executam o código do projeto com entradas,
saídas e relógio simulados. Foram verificados **1.701 caminhos**, correspondentes
às 243 sequências de cinco acionamentos para cada um dos sete painéis iniciais.
Cada caminho é encerrado assim que a partida termina.

A verificação incluiu 126 caminhos com vitória no quinto movimento e 882 derrotas,
além de soluções curtas para todos os painéis, reinício por START, debounce,
botões mantidos, descarte de entradas durante avisos e transição da fase 2.
As instruções para executar os testes estão em `tests/LEIA-ME.md`.

Em 19/10/2026, a compilação completa com Arduino CLI e Arduino ESP32 3.3.11 foi
concluída para **NodeMCU-32S** (`esp32:esp32:nodemcu-32s`), após corrigir a
definição do vetor `cores`. O firmware ocupa 286.898 bytes (21%) de armazenamento
e 22.628 bytes (6%) de memória dinâmica.

Para repetir a verificação, execute na pasta do projeto:

```sh
arduino-cli compile --fqbn esp32:esp32:nodemcu-32s .
```

A validação física ainda exige enviar o firmware e conferir na placa os botões,
os LEDs e a sinalização do buzzer ativo.
