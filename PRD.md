# G3X RComp — Product Requirements Document

**Versão:** 0.1.0  
**Status:** proposta para confirmação  
**Target:** C++20, JUCE fixado e CMake  
**Entrega inicial:** VST3 64-bit para Windows; Standalone para desenvolvimento

## 1. Visão do produto

G3X RComp é um compressor/expander musical de uso geral que une controles
convencionais a escolhas simples de comportamento e caráter. Deve funcionar em
vocais, baixo, bateria, instrumentos e buses, cobrindo nivelamento transparente,
controle de picos, compressão paralela e efeitos de pumping.

A ergonomia é informada pelo Waves Renaissance Compressor: threshold e ratio
integrados aos medidores, release manual/adaptativo, duas respostas de envelope,
dois caracteres tonais, mix e proteção de picos. A implementação G3X será
independente e não buscará equivalência sample-a-sample.

## 2. Objetivos

- Oferecer compressão suave e previsível com poucos controles.
- Permitir compressão downward e expansão upward numa curva contínua soft-knee.
- Alternar entre resposta moderna e comportamento program-dependent vintage.
- Disponibilizar caráter limpo ou harmônico sem esconder ganho excessivo.
- Suportar compressão paralela, automação e restauração de sessão.
- Preservar imagem estéreo e operar com segurança em tempo real.

## 3. Parâmetros públicos

### Threshold (`thresholdDb`)

- Faixa proposta: -60 a 0 dBFS; padrão 0 dBFS.
- Define a região em que compressão ou expansão ganha atuação significativa.
- Integrado visualmente ao medidor de entrada.

### Ratio (`ratio`)

- Faixa proposta: 0.5:1 a 50:1; padrão 1:1.
- 0.5–0.99 executa expansão; 1 é neutro; acima de 1 executa compressão.
- Mapeamento bipolar com maior resolução perceptual ao redor de 1:1.

### Attack (`attackMs`)

- Faixa: 0.5–500 ms; padrão inicial 16 ms; escala logarítmica.

### Release (`releaseMs`)

- Faixa: 5–5000 ms; padrão inicial 160 ms; escala logarítmica.
- Em Auto, atua como fator global para o cálculo program-dependent.

### Release Mode (`releaseMode`)

- `Manual`: constante determinada diretamente por Release.
- `Auto`: múltiplos envelopes de recuperação combinados conforme o programa.

### Behavior (`behavior`)

- `Modern`: abaixo de 3 dB de redução, acelera perto de zero; com redução maior,
  recupera mais lentamente para nivelamento e loudness.
- `Vintage`: abaixo de 3 dB, desacelera perto de zero; em reduções maiores,
  recupera mais rápido para resposta expressiva.
- Os nomes e as curvas são próprios; não usar Electro/Opto como marca na UI.

### Character (`character`)

- `Clean`: caminho transparente.
- `Warm`: adiciona harmônicos graves suaves conforme a compressão aumenta.
- A loudness e o ganho deverão ser controlados para comparação honesta.

### Output (`outputGainDb`)

- Faixa proposta: -30 a +30 dB; padrão 0 dB.
- Aplicado antes da proteção de picos.

### Mix (`mixPercent`)

- Faixa: 0–100%; padrão 100%; caminho dry alinhado em fase.

### Trim (`trimDb`)

- Faixa: -18 a +18 dB; padrão 0 dB; aplicado após a proteção de picos.

Os IDs tornam-se contratos de compatibilidade após o primeiro beta.

## 4. Arquitetura DSP proposta

```text
Input
  -> segurança DC/denormal/NaN
  -> detector peak + RMS estéreo linkado
  -> curva soft-knee compressor/expander
  -> envelope Manual ou Auto + Behavior
  -> Character Clean/Warm
  -> dry/wet Mix
  -> Output Gain
  -> peak limiter
  -> post-limiter Trim
  -> Output
```

### 4.1 Curva dinâmica

- Curva estática contínua para ratios de expansão e compressão.
- Knee inicial de 12 dB de largura total, começando aproximadamente 6 dB antes
  do threshold, a confirmar em testes.
- Sem descontinuidade de valor ou derivada nos limites do knee.
- Ganho calculado em domínio logarítmico e suavizado antes da aplicação.

### 4.2 Detector e envelope

- Detector híbrido peak/RMS com coeficientes derivados do sample rate.
- Link estéreo pelo máximo ou energia combinada, selecionado por teste auditivo.
- Auto release próprio usando componentes rápida e lenta ponderadas pela
  profundidade e duração da redução.
- Behavior modifica somente a trajetória do envelope, não os valores públicos.
- Zero alocações e estabilidade para mudanças rápidas de Attack/Release.

### 4.3 Caráter Warm

- Saturação dependente da redução, concentrada em harmônicos pares e região
  grave/média-grave, com nível limitado.
- Avaliar waveshaper antiderivative ou oversampling 2x; qualquer latência deve
  ser reportada e qualquer aliasing deve ser medido.
- Clean deve permanecer linear, exceto pela dinâmica e limiter.

### 4.4 Limiter

- Ceiling interno inicial em 0 dBFS para compatibilidade conceitual, com opção
  futura de -1 dBTP após validação.
- Indicador amarelo ao atuar e vermelho acima de 6 dB de redução acumulada.
- O medidor de gain change distingue compressão, expansão e limiting.
- Não mascarar clipping produzido pelo Trim, pois ele é pós-limiter.

## 5. Interface proposta

- Threshold/entrada à esquerda, Ratio/gain change no centro e Output à direita.
- Attack e Release abaixo, seguidos por Auto, Behavior e Character.
- Mix e Trim em painel secundário claro, nunca ocultos da automação.
- Medidores de entrada, gain change e saída com peak hold resetável.
- Janela redimensionável, HiDPI, foco visível e navegação completa por teclado.
- Identidade G3X própria; tipografia, cores, proporções, skins e ornamentos da
  Waves não serão reutilizados.

## 6. Medição

- Entrada e saída por canal, peak hold e alerta de clipping.
- Gain change bipolar: redução negativa e expansão positiva.
- Indicador separado de atuação do limiter.
- Atualização da UI em 30–60 Hz via atomics/FIFO lock-free.

## 7. Requisitos de tempo real

- Nenhuma alocação, mutex, I/O, logging ou chamada de UI em `processBlock`.
- 44.1–192 kHz; buffers de 16–2048 samples.
- Saída finita para silêncio, impulsos, DC e níveis extremos.
- Bypass e parâmetros suavizados, sem zipper noise ou clicks.
- Estado serializado, versionado e compatível entre versões.
- Mono, mono-to-stereo e estéreo linkado; latência sempre informada ao host.

## 8. Testes e critérios de aceitação

- `ratio = 1`, ganhos em 0 e Mix 100% produzem identidade dentro da tolerância.
- Testes de curva para ratios 0.5, 0.75, 1, 1.5, 2, 4, 10 e 50.
- Continuidade do knee e envelopes Attack/Release em todos os sample rates.
- Auto release testado com impulsos isolados, tons sustentados e bursts.
- Behavior e Character geram diferenças mensuráveis, estáveis e documentadas.
- Mix paralelo permanece alinhado; estéreo não desloca imagem.
- Limiter respeita ceiling dentro da tolerância definida.
- Builds Linux Debug/Release e Windows VST3 Release via MSVC.
- Estado restaurado pelo host; pluginval/VST3 Validator antes do beta.
- Validação no FL Studio com vocal, baixo, bateria e mix bus.

## 9. Presets iniciais

- Vocal Leveler, Bass Vintage, Drum Punch, Parallel Smash, Mix Glue,
  Clean Peaks, Upward Detail e Pump Effect.

Presets serão criados do zero e servirão apenas como pontos de partida.

## 10. Fora do escopo inicial

- Clonagem sample-a-sample ou engenharia reversa do RComp.
- Uso de marca, código, assets, presets ou trade dress da Waves.
- Sidechain externo, multibanda, mid/side, AAX e iOS.

## 11. Marcos

1. **M0 — Fundação:** PRD, naming, licença e arquitetura.
2. **M1 — Curva:** compressor/expander, knee e testes estáticos.
3. **M2 — Envelope:** Manual/Auto, behaviors, estéreo e testes temporais.
4. **M3 — Caráter/UI:** Warm, paralelo, limiter, interface e presets.
5. **M4 — Windows Alpha:** CI MSVC, VST3 e FL Studio.
6. **M5 — Beta:** validadores, regressão, documentação e empacotamento.

## 12. Decisões para confirmação

- Nome público `G3X RComp` ou nome sem referência ao produto Waves.
- Warm linear/zero-latency ou oversampling para menor aliasing.
- Ceiling 0 dBFS por familiaridade ou -1 dBTP por segurança moderna.
- Expor Trim e Mix na tela principal ou em painel avançado.
- Suporte a sidechain externo em uma versão posterior.

## 13. Fontes

- [Página oficial do Renaissance Compressor](https://www.waves.com/plugins/renaissance-compressor)
- [Manual oficial](https://assets.wavescdn.com/pdf/plugins/renaissance-compressor.pdf)
- [Imagem oficial](https://media.wavescdn.com/images/products/plugins/600/renaissance-compressor.png)

Consulta realizada em 4 de setembro de 2026. As fontes são referências de
produto e não constituem especificação para clonagem.

