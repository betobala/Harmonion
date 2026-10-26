# Harmonion — guia de estilo "Brass & Ivory" (v1)

Conceito: **console de harmônio / órgão de tubos**. Madeira escura aquecida, latão polido,
teclas de marfim e filetes finos gravados. Cada grau do acorde tem uma cor própria, derivada do
círculo das quintas, que aparece no anel de cada botão. Elegante, quente, "instrumento", em oposição
ao visual técnico e frio dos módulos de laboratório.

Referências visuais geradas: `docs/art/source/style_v1/` (concepts do painel e folha de componentes).
O nome, o painel e os gráficos são próprios (ver SPEC.md); nada daqui copia o produto que inspirou o módulo.

## 1. Paleta

| Token | Hex | Uso |
|---|---|---|
| `ebony` | `#1B1613` | fundo do painel |
| `walnut` | `#2A1F1A` | placas e seções rebaixadas |
| `oxblood` | `#4A1D2A` | texto sobre latão, faixas de título, seleção do display |
| `brass` | `#C8A24A` | filetes, cartelas, anéis de jack, knobs |
| `brass_hi` | `#EBD08A` | realce de latão, texto de título |
| `brass_lo` | `#8C6D2B` | sombra de latão, aros |
| `ivory` | `#F2E8D0` | botões, texto principal |
| `parchment` | `#D9CBA8` | texto secundário |
| `verdigris` | `#3F9C8F` | gate ativo, LEDs de gate |
| `amber` | `#FFB347` | tela do display (OLED âmbar) |
| `void` | `#120D0A` | fundo do display, furos de jack |

**Cores dos graus** (anel do botão, nota destacada no display, LED do canal):

| Grau | Hex | Nome |
|---|---|---|
| ROOT | `#E8B84A` | ouro |
| 3RD | `#D4607A` | rosa |
| 5TH | `#3F9C8F` | verdigris |
| 7TH | `#8A6FC4` | violeta |
| 9TH | `#5B9BD5` | céu |
| 11TH | `#8DB255` | musgo |
| 13TH | `#E98A3C` | laranja |
| FILL / INV / extras | `#CFC6B2` | pérola |

Regras: sem azul no fundo, sem neon, sem cinza frio. O contraste vem do latão e do marfim contra a madeira.
No painel atual (cinza-azulado e dourado) basta trocar `#2B2F38`→`walnut`, os dourados → `brass`, os botões amarelos → `ivory` com anel do grau.

## 2. Tipografia

Todas sob licença SIL OFL (podem ir em `res/fonts/`).

| Papel | Fonte | Peso | Regras |
|---|---|---|---|
| Título do módulo | **Cormorant SC** | SemiBold | tracking +12%, `brass_hi`, 12 pt |
| Cartelas de seção | **Cormorant SC** | Bold | `oxblood` sobre latão, 7 pt |
| Rótulos dos controles | **Jost** | Medium | MAIÚSCULAS, tracking +10%, 6 pt, `parchment` |
| Display | **IBM Plex Mono** | Medium | `amber` sobre `void`, 10 pt notas, 8 pt rodapé |

Como no Rack o texto do SVG não é renderizado, os rótulos são desenhados em runtime ou convertidos em curvas.

## 3. Formas e proporções

- **Bordas de seção:** filete duplo, 0,3 mm de latão + 0,3 mm de vão + 0,15 mm de latão, canto chanfrado 2 mm.
- **Cartela de título:** topo em arco (raio 2,5 mm), preenchimento latão (`brass_hi` → `brass` → `brass_lo`), texto `oxblood`.
- **Pauta decorativa:** 5 linhas de 0,12 mm, espaçamento 0,5 mm, `brass` a 40%, entre seções; usar como divisor, nunca atrás de controles.
- **Botões:** tampa `ivory` redonda, aro de latão 0,5 mm, anel fino na cor do grau; pressionado escurece 12%.
- **Knobs:** tampa de latão com serrilhado fino (24 dentes) e ponteiro `ivory`; sombra suave só no knob.
- **Jacks:** furo `void`, porca de latão sextavada; LED âmbar ou verdigris a 1,4 mm ao lado.
- **Display:** moldura latão 0,4 mm, fundo `void`, texto `amber`; nota selecionada em fundo `brass` com texto `oxblood`.
- **Ícones:** traço 0,3 mm; clave de sol e colcheia só em ornamentos, nunca como rótulo de função.

## 4. Tokens para o código (C++)

```cpp
static const NVGcolor EBONY     = nvgRGB(0x1B, 0x16, 0x13);
static const NVGcolor WALNUT    = nvgRGB(0x2A, 0x1F, 0x1A);
static const NVGcolor OXBLOOD   = nvgRGB(0x4A, 0x1D, 0x2A);
static const NVGcolor BRASS     = nvgRGB(0xC8, 0xA2, 0x4A);
static const NVGcolor BRASS_HI  = nvgRGB(0xEB, 0xD0, 0x8A);
static const NVGcolor BRASS_LO  = nvgRGB(0x8C, 0x6D, 0x2B);
static const NVGcolor IVORY     = nvgRGB(0xF2, 0xE8, 0xD0);
static const NVGcolor PARCHMENT = nvgRGB(0xD9, 0xCB, 0xA8);
static const NVGcolor VERDIGRIS = nvgRGB(0x3F, 0x9C, 0x8F);
static const NVGcolor AMBER     = nvgRGB(0xFF, 0xB3, 0x47);
static const NVGcolor VOID_     = nvgRGB(0x12, 0x0D, 0x0A);
// ROOT, 3RD, 5TH, 7TH, 9TH, 11TH, 13TH, FILL
static const NVGcolor DEGREE[8] = {
	nvgRGB(0xE8,0xB8,0x4A), nvgRGB(0xD4,0x60,0x7A), nvgRGB(0x3F,0x9C,0x8F), nvgRGB(0x8A,0x6F,0xC4),
	nvgRGB(0x5B,0x9B,0xD5), nvgRGB(0x8D,0xB2,0x55), nvgRGB(0xE9,0x8A,0x3C), nvgRGB(0xCF,0xC6,0xB2)};
```

## 5. Fazer e evitar

| Fazer | Evitar |
|---|---|
| Latão só em estrutura (filetes, cartelas, knobs) | Latão em tudo, que cansa e perde hierarquia |
| Cor do grau sempre igual em botão, display e LED | Reusar a cor de um grau para outra função |
| Serifa só em títulos e cartelas | Serifa em rótulos pequenos (ilegível a 6 pt) |
| Madeira em tons de marrom-café quente | Cinza frio, azul, neon |

## 7. Notas dos concepts (v1)

- **Variante 1** (`harmonion_panel__v1.png`): seções verticais como "registros" de órgão, cartelas em arco,
  emblema de tubos de órgão em oxblood no centro, ornamentos de flor-de-lis. A mais teatral; usar como
  referência de **atmosfera** e do **emblema** (silhueta de tubos).
- **Variante 2** (`harmonion_panel__v2.png`): linhas horizontais por grau, cada uma com cartela de latão em arco
  à esquerda, cinco botões de marfim com anel na cor do grau e um knob à direita; campo de jacks em placa oxblood.
  A mais fácil de implementar e a mais legível; usar como referência de **layout** e de **cor por grau**.
- Aproveitar nas duas: knobs de latão com serrilhado, botões marfim com aro de latão, jacks com porca de latão,
  display âmbar com notas luminosas, filetes duplos e pautas curtas como divisores.
- Cuidado ao implementar: o excesso de ornamento (flores-de-lis, cantoneiras, parafusos) não cabe na resolução
  de um painel Rack; ficar com **filete duplo, cartela em arco, anel de grau e pauta divisória**.
- Folha de componentes: `harmonion_parts__v1.png`. Folha de contato: `source/style_v1/sheet_style_v1.png`.
- Pendente: escolher a variante e fixar em `"selected"` no lote.

## 6. Procedência

Concepts gerados para o projeto com o image_gen do Codex CLI em 25/10/2026 (prompts literais nos
sidecars `docs/art/source/style_v1/*.json`). São referência visual, não entram no plugin.
