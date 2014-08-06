#ifndef UserDataType_h
#define UserDataType_h
const uint8_t ADC_DIM = 4;
struct data_t {
  unsigned long time;
  unsigned short adc[ADC_DIM];
};
#endif  // UserDataType_h
