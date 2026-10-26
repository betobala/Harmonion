# Harmonion — especificação funcional

Módulo para VCV Rack 2 que reproduz o comportamento do ACL Sinfonion (firmware 1.0),
reimplementado do zero a partir do manual público do usuário. Nenhum código ou arte
do produto original foi usado. Nome, painel e gráficos são próprios.

Fonte: *Sinfonion User Manual, firmware 1.0* (acl-synth.com/manuals-eng/sinfonion-manual-1.0c.pdf).

## Hardware reproduzido

Painel 42 HP com o mesmo arranjo de controles do original:

| Tipo | Qtd | Itens |
|---|---|---|
| Knobs | 12 | ROOT, DEGREE, TRANS, MODE, SLEW, PRESET, POT 1, POT 2, Chords PITCH, SPREAD, Arp PITCH, RANGE |
| Botões grandes | 52 | Ch1 A (ROOT 3RD 5TH 7TH 9TH 11TH 13TH FILL), Ch1 B (ROOT 5TH SCALE) + SLEW, Ch2 (ROOT 3RD 5TH 7TH 9TH 11TH 13TH FILL1..5), Ch3 (ROOT 3RD 5TH 7TH 9TH 11TH 13TH EQ-DIST), Chords (ROOT 3RD 5TH 7TH 9TH 11TH 13TH INV SLEW), Arp (ROOT 3RD 5TH 7TH 9TH 11TH 13TH PATTERN BEGIN), GATES 1 2 |
| Botões pequenos | 12 | MENU TUNE SONG OCT / SEQ ← → ▶/II / DEL − + ADD |
| Entradas CV | 11 | Ch1-3 IN, ROOT, TRANS, CV 1-4, Chords IN, Arp IN |
| Entradas gate | 9 | Ch1-3 TRIG, A/B, IN 1, IN 2, Chords TRIG, Arp CLK, Seq CLK |
| Saídas pitch | 8 | Ch1-3 OUT, Arp OUT, Chords OUT 1-4 |
| Saídas gate | 4 | OUT 1, OUT 2, GATE 1, GATE 2 |
| Display | 1 | OLED 128×64 emulado |

## Seções e funções

Todas as funções dos capítulos 4–13 do manual:

- **Harmonia global**: ROOT + DEGREE + MODE → escala de 6 a 8 notas mapeada em ROOT/9/3/11/5/13/7 e FILL1-5.
  Modos: Chords, Chords2, Slash chords, Major, Minor, Mel.Minor, Har.Minor, Har.Major, Db.H.Maj.
- **Quantizadores 1-3**: S&H por TRIG, janela de trigger, autogates, FILL, A/B do Ch1 (7 modos),
  single note do Ch2 com manual gate, EQ-DIST do Ch3, Pre Trans, Pitch Bend.
- **Página OCT**: oitava ±3, IN follows ROOT, bypass (3 estados), escala de entrada 1/2/3/5 V/oct,
  detecção de glide, matrizes de entrada (mixagem 4×4) e de saída (chaveamento 4×4).
- **Acordes**: até 4 notas da seleção, duplicação, inversão (livre/fundamental/1ª/2ª/3ª), spread,
  slash chords com baixo fixo, voicing por CV (98 voicings), shuffle, pitch bend com delay.
- **Arpejador**: 8 padrões, BEGIN (reset na troca de acorde), atalhos segurando PATTERN/BEGIN,
  divisor/multiplicador de clock (16 razões), delay de trigger, randomize.
- **Sequenciador de progressões**: 16 músicas × 4 partes × 32 passos, 1-128 beats por passo,
  divisor de clock por parte, página SONG, live mode, freeze (duplo clique em ▶/II), todas as
  funções de IN/CV/OUT do sequenciador.
- **Global**: 16 presets de botões, slew linear/exponencial, afinação temperada/justa/mesotônica,
  modo de afinação (TUNE), voltímetro, Harmonic Shift, Chaotic Note Shuffle, Chaotic Detune,
  4 CVs (28 funções), 2 pots (28 funções), IN 1/2 (20 funções), OUT 1/2 (14 funções),
  Reset To Defaults, factory reset, calibração das saídas.
- **Harmonic Link**: OUT 1 = Sync Master, IN 1 = Sync Slave, via cabo polifônico.

## Adaptações para o VCV Rack

- O link serial de 115200 baud vira um cabo polifônico de 16 canais com o estado harmônico.
- Configurações são salvas no patch (JSON) em vez da flash interna.
- O menu do display também está disponível pelo menu de contexto (clique direito).
- A calibração aplica um offset/ganho por saída. No VCV as saídas já são exatas.
- Pontos que o manual não define (lista exata das 88 escalas, opções de A/B, valores de alguns
  menus) foram reconstruídos com critério musical e estão marcados no código como `// reconstruído`.
