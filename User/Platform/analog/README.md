# Analog contracts

This directory is reserved for service-oriented ADC and DAC contracts:

- `platform_adc.c/.h` will expose raw conversion codes and channel instances;
- `platform_dac.c/.h` will expose raw output codes and channel instances.

They are intentionally not scaffolded as empty APIs. A contract should be
added only when its required operations, optional capabilities and quantized
data model are known.
