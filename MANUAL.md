# HARMONION — manual

**betobullet** · 42 HP · processador harmônico de CV

O Harmonion põe uma harmonia global (tom, grau e modo) no centro do sistema, e tudo o que passa
por ele obedece a essa harmonia:
- três quantizadores;
- um gerador de acordes de quatro vozes;
- um arpejador;
- um sequenciador de progressões.

Ele reproduz o comportamento do ACL Sinfonion (firmware 1.0), reimplementado a partir do manual
público. Os detalhes de cada função seguem o manual do Sinfonion, capítulos 4 a 13, e a
especificação está em [SPEC.md](SPEC.md).

---

## 1. Visão geral do painel

| Seção | Conteúdo |
|---|---|
| **Patchbay** (coluna esquerda) | IN, OUT e TRIG dos canais 1–3, A/B, ROOT, TRANS, CV 1–4, IN 1/2, OUT 1/2, acordes (IN, TRIG, OUT 1–4), GATE 1/2, arpejador (IN, OUT, CLK) e CLK do sequenciador |
| **Centro** | ROOT, DEGREE, display, TRANS, MODE, SLEW, PRESET, POT 1/2 e os botões pequenos MENU, TUNE, SONG, OCT, SEQ, ←, →, ▶/II, DEL, −, +, ADD |
| **Canais 1–3** | Botões das notas permitidas: ROOT, 3RD, 5TH, 7TH, 9TH, 11TH, 13TH, além de FILL, A/B (canal 1) e EQ-DIST (canal 3) |
| **Acordes** | Botões das notas, INV, SLEW, PITCH e SPREAD |
| **Arpejador** | Botões das notas, PATTERN, BEGIN, PITCH e RANGE |

## 2. A harmonia

- **ROOT**: a tônica.
- **DEGREE**: o grau, ou seja, o acorde dentro do tom.
- **MODE**: a coleção de escalas. As opções são Chords, Chords2, Slash chords, Major, Minor,
  Mel.Minor, Har.Minor, Har.Major e Db.H.Maj.

O resultado é uma escala de 6 a 8 notas, mapeada nos botões **ROOT, 3RD, 5TH, 7TH, 9TH, 11TH e
13TH**. As notas que ficam fora da escala estão em **FILL 1–5**.

- **TRANS** transpõe tudo em semitons, de −12 a +12.
- **PRESET** escolhe um de 16 conjuntos de botões.

## 3. Quantizadores (canais 1–3)

Cada canal quantiza o seu **IN** para as notas cujos botões estão acesos e entrega o resultado em
**OUT**.

- **TRIG**: com cabo, o canal faz sample & hold no trigger. Sem cabo, quantiza continuamente.
- **Canal 1, A/B**: duas seleções de notas (A e B), alternadas pelo jack A/B ou pelo ritmo
  (menu *Ch 1 A/B*).
- **Canal 2, single note**: cada botão toca a sua nota diretamente, com gate manual.
- **Canal 3, EQ-DIST**: quantização equidistante, que mapeia a entrada só nas notas acesas, em
  passos iguais.
- **Página OCT** (botão OCT):
  - oitava de −3 a +3 por seção;
  - *IN follows ROOT*;
  - bypass em 3 estados;
  - escala da entrada em 1, 2, 3 ou 5 V/oct;
  - detecção de glide;
  - matrizes de entrada (mixagem 4×4) e de saída (chaveamento 4×4).

## 4. Acordes

**OUT 1–4** tocam até quatro notas da seleção.

- **INV**: inversão. As opções são Free, Root position, 1st, 2nd e 3rd inversion.
- **SPREAD**: abre o voicing.
- **PITCH**: transpõe o acorde.
- **SLEW**: liga o portamento.
- **IN** e **TRIG** fazem a transposição e o sample & hold do acorde.
- **Voicing por CV**: 98 voicings, escolhidos por CV.
- **Slash chords**: com baixo fixo.

## 5. Arpejador

O arpejador toca as notas selecionadas a cada pulso em **CLK** e entrega o resultado em **OUT**.

- **PATTERN**: Up, Down, Up/Down, Down/Up, Octaves, Baroque, Random e Brownian.
- **BEGIN**: a nota onde o arpejo recomeça na troca de acorde. As opções são Next Note, Root, 3rd,
  5th, 7th, 9th, 11th e 13th.
- **PITCH**: a nota mais grave do arpejo.
- **RANGE**: a extensão do arpejo.
- **Arpeggio Clock** (menu): 16 razões de clock, de 1/4 a 4/1.
- **Arp Trigger Delay** (menu): atrasa o disparo do arpejo.

## 6. Sequenciador de progressões

São 16 músicas, cada uma com 4 partes (A–D) de até 32 passos. Cada passo guarda a harmonia, os
botões e as oitavas, e dura de 1 a 128 beats.

- **SEQ**: liga o sequenciador.
- **▶/II**: play/pause. Um duplo clique entra em *freeze*.
- **← / →**: navega pelos passos.
- **ADD / DEL**: acrescenta ou apaga passos.
- **− / +**: muda os beats do passo.
- **SONG**: abre a página da música, que dá acesso às partes, aos saltos e ao *live mode*.
- **Clock Divider**: cada parte tem o seu divisor de clock (menu *Clock Divider*).

## 7. Menu (botão MENU)

| Item | Opções |
|---|---|
| **CV 1–4** | 28 funções: Transpose, Chaotic Note Shuffle, Chaotic detune, Harmonic Shift, Button Preset, Slew Time, Voltmeter, Root, Degree, Mode, Pre Trans e Pitch Bend por canal, Chords Spread/Voicing, Arpeggio Clock/Randomize/Range, Song, Song Part, Song + Part, Sequence Step |
| **POT 1 / POT 2** | As mesmas funções dos CVs |
| **IN 1 / IN 2** | 20 funções: Sync Slave, Slew por canal, Quantize por canal, Chords Shuffle, Arpeggio Reset, resets e passos do sequenciador, Tuning Mode, Play/Pause |
| **OUT 1 / OUT 2** | 14 funções: Sync Master, Autogates, Ch 2 Manual Gate, Arpeggio Beat, Sequencer Clock/Beat, Song Step/Reset, Tuning Mode Active, Ch 1 A/B State, Chord Change |
| **Clock Divider** | 1 a 64 |
| **Arpeggio Clock** | 1/4 … 4/1 |
| **Intonation** | Equal, Just, Quarter-comma Meantone |
| **Slew Curve** | linear ou exponencial |
| **Trigger Time Window** | janela de trigger dos quantizadores |
| **Arp Trigger Delay** | atraso do arpejador |
| **Pitch Bend Delay** | None, Short, Medium, Long |
| **Ch 1 A/B** | Off, via Jack, B on Beat 1, Each Odd/Even Beat, B on 1st Beat of Step, B on Clock 1, Each Odd/Even Clock |
| **Ch 2 Buttons** | modo single note |
| **Display** | Scale Tones, Scale Structure, Chord Structure, Octave Switches |
| **Reset To Defaults** | volta o menu e as matrizes ao padrão, sem apagar músicas e presets |

## 8. Display

- **Topo**: tom e modo à esquerda; acorde à direita.
- **Base**: transposição à esquerda; preset ou música e parte à direita. Com o sequenciador
  ligado, no centro aparecem os beats (círculos) e os passos (quadrados).
- **Linha central**: notas da escala, estrutura da escala, estrutura do acorde ou chaves de
  oitava, conforme o item *Display* do menu.

## 9. Harmonic Link (vários Harmonions)

1. Ligue o **OUT 1** de um Harmonion, configurado como *Sync Master*, no **IN 1** dos outros,
   configurados como *Sync Slave*.
2. O cabo polifônico de 16 canais leva a harmonia do mestre para os escravos.

## 10. Diferenças em relação ao hardware

- **Combinações de botões**: as que exigem segurar dois botões também ficam no menu de contexto
  (clique direito): padrão, início, inversão, factory reset e calibração.
- **Gates**: saem com 10 V. O menu de contexto troca para os 5 V do hardware.
- **Salvamento**: tudo fica salvo no patch.
- **Theme** (menu de contexto): Black ou White.
