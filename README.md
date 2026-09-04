# G3X Compressor

G3X Compressor é um compressor/expander musical VST3 com aplicativo Standalone,
desenvolvido em C++20, JUCE 8.0.8 e CMake. O projeto parte dos requisitos do
[PRD](PRD.md), com DSP, interface, presets e identidade próprios.

## Recursos

- Compressor e expansor contínuos (`0.5:1` a `50:1`) com soft knee de 12 dB.
- Detector híbrido peak/RMS, link estéreo e envelopes sem alocação no áudio.
- Release Manual ou Auto program-dependent.
- Comportamentos Modern e Vintage e caráter Clean ou Warm.
- Mix dry/wet alinhado, ganho de saída, limiter e Trim pós-limiter.
- Medidores estéreo de entrada/saída, gain change bipolar e atividade do limiter.
- Automação completa, estado versionado e oito presets de fábrica no host.
- Interface redimensionável, HiDPI e controles acessíveis pelo teclado.

## Compilar

Pré-requisitos: CMake 3.22+, compilador C++20 e Git. O CMake baixa a versão
fixada do JUCE automaticamente.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

No Linux, ALSA, FreeType, Fontconfig e OpenGL precisam estar instalados. O
artefato fica em `build/G3XCompressor_artefacts/Release/`. No Windows com Visual
Studio, use um gerador multi-configuração e `--config Release`.

## Parâmetros

| Parâmetro | Faixa | Padrão |
|---|---:|---:|
| Threshold | -60–0 dB | 0 dB |
| Ratio | 0.5:1–50:1 | 1:1 |
| Attack | 0.5–500 ms | 16 ms |
| Release | 5–5000 ms | 160 ms |
| Release Mode | Manual / Auto | Manual |
| Behavior | Modern / Vintage | Modern |
| Character | Clean / Warm | Clean |
| Output | -30–+30 dB | 0 dB |
| Mix | 0–100% | 100% |
| Trim | -18–+18 dB | 0 dB |

## Estado do projeto

Alpha funcional. Build Linux VST3/Standalone e testes DSP são validados em CI;
o build Windows produz VST3 com MSVC. Validação auditiva, pluginval, VST3
Validator e testes no FL Studio continuam sendo requisitos para o beta.

## Referência e independência

O Waves Renaissance Compressor foi estudado apenas como referência de fluxo e
categoria. Marca, código, interface, componentes, textos e presets do G3X Compressor
são originais. Veja [as fontes e limites de uso](docs/references/README.md).

## Licença

Distribuído sob a [licença MIT](LICENSE).

## Download e instalação — Windows x64

1. Abra [Actions](https://github.com/6uilhermeTeixeira/plugin-g3x-compressor/actions) e selecione uma execução bem-sucedida da branch `main`.
2. Em **Artifacts**, baixe `G3X-Compressor-Windows-x64-<commit>`. O download fica disponível por 30 dias; **Run workflow** permite gerar um novo build.
3. Extraia o ZIP. A raiz contém somente `SHA256SUMS.txt` e a pasta `G3X Compressor.vst3`, com todos os arquivos internos do plugin.
4. Na pasta extraída, abra o PowerShell e verifique o binário:

```powershell
$expected, $relativePath = (Get-Content -LiteralPath .\SHA256SUMS.txt -Raw).Trim() -split '  ', 2
$actual = (Get-FileHash -LiteralPath $relativePath -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actual -ne $expected) { throw "SHA-256 divergente; baixe o artifact novamente." }
"SHA-256 confirmado."
```

5. Copie a pasta **`G3X Compressor.vst3` inteira** para `C:\Program Files\Common Files\VST3` e atualize a busca de plugins da DAW. A cópia pode solicitar permissão de administrador.

O SHA-256 verifica o binário Windows x64 dentro do bundle; não é o hash do ZIP ou dos recursos. O artifact contém o VST3 Release; o aplicativo Standalone continua disponível como alvo de compilação, mas não é incluído no download.
