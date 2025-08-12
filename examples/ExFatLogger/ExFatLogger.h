// Avoid IDE problems by defining struct in septate .h file.
// Pad record so size is a power of two for best write performance.
#pragma once
const size_t ADC_COUNT = 4;
struct data_t {
  uint16_t adc[ADC_COUNT];
};
