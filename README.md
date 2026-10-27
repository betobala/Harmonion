# Harmonion

Plugin para VCV Rack 2 inspirado no ACL Sinfonion: três quantizadores, gerador de acordes de
quatro vozes, arpejador e sequenciador de progressões, todos compartilhando a mesma harmonia.
Reimplementado do zero a partir do manual público; veja [SPEC.md](SPEC.md).

![painel](docs/preview.png)

## Compilar

Requer o Rack SDK 2.6 da sua plataforma em `dep/Rack-SDK`. No Windows, MSYS2 (MINGW64) com
`mingw-w64-x86_64-gcc`, `make`, `zstd` e `jq`; no Linux, `gcc`, `make`, `zstd` e `jq`.

```bash
make -j8 dist
```

No Windows, copie `dist/Harmonion-2.1.2-win-x64.vcvplugin` para `%LOCALAPPDATA%\Rack2\plugins-win-x64\`
e reinicie o Rack.

## Testes

Windows:

```bash
g++ -std=c++11 -O1 -DARCH_WIN -D_USE_MATH_DEFINES -Idep/Rack-SDK/include -Idep/Rack-SDK/dep/include src/Harmony.cpp src/Harmonion.cpp src/Buttons.cpp src/Screen.cpp test/test.cpp -Ldep/Rack-SDK -lRack -o build/test.exe
```

Rode `build/test.exe` com a pasta do Rack (`libRack.dll`) no `PATH`.

Linux:

```bash
g++ -std=c++11 -O1 -DARCH_LIN -Idep/Rack-SDK/include -Idep/Rack-SDK/dep/include src/Harmony.cpp src/Harmonion.cpp src/Buttons.cpp src/Screen.cpp test/test.cpp -Ldep/Rack-SDK -lRack -o build/test
LD_LIBRARY_PATH=dep/Rack-SDK build/test
```

### Prévia do display

O conteúdo do display é montado sem NanoVG (`src/Screen.cpp`), então dá para conferir as telas sem
abrir o Rack. O teste grava as telas principais em JSON e o script as renderiza em PNG (requer Pillow):

```bash
build/test --dump-screens build/screens
python3 tools/preview_display.py build/screens
```

## Diferenças em relação ao hardware

- **Atalhos de dois dedos**: segurar PATTERN, BEGIN ou INV e apertar outro botão funciona, mas com
  mouse não dá para segurar dois botões ao mesmo tempo. Por isso o menu de contexto (clique direito)
  também escolhe padrão, início e inversão. O display mostra o valor novo ao mudar.
- **Factory reset e calibração**: as combinações do hardware funcionam (Ch 3 3RD + 7TH + 11TH + EQ-DIST +
  Arp BEGIN; TUNE + ← + Ch 1 3RD), mas exigem segurar vários botões, então também ficam no menu de contexto.
- **Harmonic Link**: OUT 1 = "Sync Master" envia um cabo polifônico de 16 canais. IN 1 = "Sync Slave" recebe.
- **Gates e triggers** saem com 10 V (padrão do VCV). O menu de contexto troca para os 5 V do hardware.
- **Display**: mesmo conteúdo e layout do OLED do original (conferidos com as fotos do manual), com a
  fonte e a cor do Harmonion. Além das telas do original, avisos curtos confirmam o que no hardware só
  muda uma luz: opções da página OCT, parte da música (A–D), play/pause/freeze, valor dos knobs SLEW,
  POT 1/2, PITCH, SPREAD e RANGE, parte cheia no ADD e SONG/▶ sem o SEQ ligado.
- Partes que o manual não detalha foram reconstruídas e estão marcadas com `// reconstruído` no código:
  lista completa das escalas dos modos, a ordem de parte das funções de CV/IN/OUT e do A/B do canal 1
  (as posições visíveis nas fotos foram respeitadas) e os valores intermediários do Arpeggio Clock.

## Licença

GPL-3.0-or-later. Veja [LICENSE](LICENSE).
