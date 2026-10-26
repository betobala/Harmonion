# Harmonion — guia de estilo "Graphite" (v2, minimalista)

Conceito: **instrumento de precisão, calmo e funcional**. Grafite neutro, discos planos que **acendem em âmbar** quando
ativos, seções definidas só por um contorno fino com uma etiqueta pequena. Uma única cor de acento (âmbar), usada
para o que está ligado. Sem latão, sem madeira, sem ornamento.

Herda do módulo que inspirou o projeto o que importa: densidade organizada, seções em contorno, botões que acendem,
display âmbar. O desenho, a tipografia e o painel são próprios (ver SPEC.md).

Variantes: **Graphite** (escuro, principal) e **Paper** (claro, alternativa). Mesmo layout e mesmos tokens.

## 1. Paleta

| Token | Graphite (escuro) | Paper (claro) | Uso |
|---|---|---|---|
| `panel` | `#26272A` | `#DAD7CF` | fundo do painel |
| `plate` | `#2E3033` | `#E6E3DC` | etiquetas de seção, campo de jacks |
| `outline` | `#565960` | `#8C8A84` | contorno das seções, filetes |
| `ink` | `#E9E7E1` | `#1C1D20` | texto principal |
| `grey` | `#9A9CA2` | `#6A6B6F` | texto secundário |
| `black` | `#0D0D0F` | `#0D0D0F` | knobs, furos de jack, tela do display |
| `off` | `#44464B` | `#F6F4EF` | botão apagado |
| `accent` | `#FFB347` | `#E8960C` | **o único acento**: botão aceso, LEDs, texto do display |
| `accent_dim` | `#7A5A24` | `#C9A25E` | brilho fraco e estado intermediário |

Regras: nada de azul, verde, vermelho ou neon. Não há cor por grau; cada botão se identifica pelo rótulo e pela
posição. Se um dia fizer falta, colorir só as notas **dentro do display**, nunca o painel.
Sem degradê, sem textura, sem sombra no painel (só sombra de contato suave sob knob e jack).

## 2. Tipografia

Escolhidas por terem personalidade própria. Todas SIL OFL (podem ir em `res/fonts/`).

| Papel | Fonte | Peso | Regras |
|---|---|---|---|
| Wordmark | **Fraunces** | SemiBold (600), SOFT 100, WONK 0 | MAIÚSCULAS, tracking +14%, 9 pt; serifa suave e quente, o único traço "de instrumento" do painel |
| Seções e rótulos | **Martian Mono** | Regular (wdth 100) | MAIÚSCULAS, tracking +4%, 5 pt; monoespaçada, de ar técnico |
| Display | **Martian Mono** | Medium | `accent` sobre `black`, 10 pt |

Arquivos no plugin (`res/fonts/`): `MartianMono-Label.ttf` (wdth 90, wght 400, rótulos) e `MartianMono-Display.ttf` (wdth 80, wght 500, display), com `OFL-MartianMono.txt`.
A Fraunces só aparece **dentro do logotipo**, já em curvas; não vai no plugin.
O display usa 0,62 em de avanço por caractere; textos longos são reduzidos por `fitSize()` para caber nos 128 px.
Rótulos curtos e sempre os mesmos do manual do módulo (ROOT, 3RD, 5TH, FILL, PATTERN).

## 3. Formas e grade

- **Seções:** retângulo de contorno 0,3 mm em `outline`, raio 2 mm; **etiqueta** pequena (retângulo `plate`, sem
  preenchimento colorido) assentada sobre o contorno superior, texto 6 pt.
- **Botões:** disco plano de 6 mm em `off`, sem aro; **aceso** = disco `accent` com halo de 20% (raio 1,6×).
- **Knobs:** pretos, 11 mm (grandes) ou 8 mm (pequenos), ponteiro branco de 0,4 mm; sem serrilhado nem capa.
- **Jacks:** furo `black`, anel prateado, porca; LED `accent` de 1,2 mm ao lado quando há gate.
- **Display:** `black` com contorno `outline` de 0,3 mm, raio 1,2 mm, texto `accent`; item selecionado invertido
  (fundo `accent`, texto `black`).
- **Divisores internos:** só filete de 0,2 mm em `outline`. Nada de pauta, flor, arco ou cantoneira.
- **Grade:** passo de 9,5 mm entre botões e entre jacks; espaço mínimo de 3 mm entre seções.

## Logo

O logo é um **logotipo com o nome** ("HARMONION"), não só um ícone. Referência de espírito: um logotipo em letras de
bloco, com o "H" trabalhado. Aqui o desenho é próprio.

- **Composição:** o nome em maiúsculas de **Fraunces SemiBold** (serifa suave), tracking aberto, em que o **"H" é
  desenhado à mão**: dois postes e uma corda esticada entre eles com três nós. É a série harmônica: a corda vibra em
  nós, e os três pontos são a tríade. O nó do meio (a fundamental) leva o âmbar, o único acento.
- **Letras:** contornos de Fraunces convertidos em curvas. O plugin **não** carrega essa fonte; só o SVG.
- **Arquivos:** `docs/art/logo/logotype_graphite.svg` (tinta off-white, para Graphite) e `logotype_paper.svg`. Cópias
  usadas pelo plugin em `res/logo_graphite.svg` e `res/logo_paper.svg`.
- **Uso no painel:** 30 mm de largura (altura 3,5 mm), centralizado na coluna do meio, no topo.
  Tamanho mínimo: 24 mm de largura; abaixo disso os nós do H se perdem.

## Controles

Os controles do painel seguem a folha de componentes em
`docs/art/components.png`. Estão em `src/Controls.hpp`, desenhados em NanoVG para mudar com o tema.

| Controle | Desenho |
|---|---|
| **Knob** (`ctl::Knob`) | disco preto fosco, contorno ondulado quase invisível, **um risco branco** do centro à borda e um **anel de 15 traços grossos** em volta (a escala). Raios: 6,2 mm (grande), 4,6 mm (médio), 3,9 mm (compacto). |
| **Jack** (`ctl::Jack`) | anéis concêntricos: borda cinza-clara, anel branco, anel escuro e furo preto. Raio 3,3 mm. |
| **Botão** (`ctl::Button`) | disco chato com contorno grosso e brilho suave; aceso, preenche com o acento e ganha um halo. Pode ligar-se a uma luz do módulo. |
| **Chave** (`ctl::Toggle`) | moldura redonda de anéis com alavanca em cápsula; valor 1 = alavanca para cima. |

As cores vêm de `ctl::colors()`, definida em cada plugin e escolhida pelo tema ativo.

## 4. Tokens para o código (C++)

```cpp
// Graphite
static const NVGcolor PANEL      = nvgRGB(0x26, 0x27, 0x2A);
static const NVGcolor PLATE      = nvgRGB(0x2E, 0x30, 0x33);
static const NVGcolor OUTLINE    = nvgRGB(0x56, 0x59, 0x60);
static const NVGcolor INK        = nvgRGB(0xE9, 0xE7, 0xE1);
static const NVGcolor GREY       = nvgRGB(0x9A, 0x9C, 0xA2);
static const NVGcolor BLACK      = nvgRGB(0x0D, 0x0D, 0x0F);
static const NVGcolor OFF        = nvgRGB(0x44, 0x46, 0x4B);
static const NVGcolor ACCENT     = nvgRGB(0xFF, 0xB3, 0x47);
static const NVGcolor ACCENT_DIM = nvgRGB(0x7A, 0x5A, 0x24);
// Paper: PANEL #DAD7CF, PLATE #E6E3DC, OUTLINE #8C8A84, INK #1C1D20, GREY #6A6B6F, OFF #F6F4EF, ACCENT #E8960C, ACCENT_DIM #C9A25E.
```

## 5. O que muda no plugin hoje

O painel atual (`res/Harmonion.svg`) já é cinza escuro com seções em contorno; o ajuste é de refino:
1. Fundo e placas para `panel`/`plate` neutros (sai o cinza-azulado).
2. Etiquetas de seção: de pílula dourada preenchida para etiqueta `plate` assentada no contorno.
3. Botões: de bolinhas amarelas sempre acesas para discos `off`, que só acendem em `accent` quando ativos.
4. Knobs: reduzir o serrilhado e o reflexo; preto liso com ponteiro branco.
5. Rótulos: `IBM Plex Sans` em maiúsculas, `grey` para os controles e `ink` para as seções.

## 6. Fazer e evitar

| Fazer | Evitar |
|---|---|
| Aceso = âmbar, apagado = cinza; só isso | Cor própria por botão |
| Contorno fino para agrupar | Faixas ou etiquetas coloridas |
| Alinhar tudo na grade, espaço generoso | Ornamentos, pautas, cantoneiras |
| Rótulos curtos em maiúsculas pequenas | Textos longos no painel |

## 7. Procedência

Concepts gerados para o projeto com o image_gen do Codex CLI em 25/10/2026 (prompts literais nos sidecars
`docs/art/source/style_v2/*.json`). São referência visual e não entram no plugin. A versão ornamental anterior está em
`STYLE_v1_ornamental.md`.
