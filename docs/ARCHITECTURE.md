# Arquitetura

## Fluxo de áudio

```text
entrada finita
  → detector peak/RMS estéreo
  → curva compressor/expander soft-knee
  → envelope Manual/Auto + Behavior
  → ganho dinâmico + Character
  → mix dry/wet
  → Output
  → limiter 0 dBFS
  → Trim pós-limiter
  → saída
```

`CompressorEngine` não aloca memória, bloqueia threads, registra logs nem chama
a UI durante `process`. Mudanças de Output, Mix e Trim usam smoothing; Attack e
Release recalculam apenas coeficientes escalares.

## Curva estática

Fora do knee, o ganho em dB é `(1 / ratio - 1) × (input - threshold)`. Dentro
do knee de 12 dB é usada uma interpolação quadrática, contínua em valor e
primeira derivada. Ratios menores que 1 produzem expansão upward.

## Envelope

O ataque reage quando a magnitude do gain change se afasta de zero. A
recuperação Manual altera a trajetória perto de zero e em reduções profundas
conforme o Behavior. Auto combina componentes rápida e lenta ponderadas pela
profundidade da atuação.

## Threading e estado

Parâmetros são lidos dos atomics do `AudioProcessorValueTreeState`. Medições são
publicadas em atomics e consumidas pela UI a 45 Hz. O estado usa ValueTree/XML,
inclui `stateVersion` e preserva os IDs públicos definidos no PRD.

## Limitações conhecidas da alpha

- O caráter Warm usa waveshaping zero-latency sem oversampling.
- O limiter é sample-peak, não true-peak/lookahead.
- Sidechain externo, multibanda, mid/side, AAX e iOS não fazem parte da alpha.
