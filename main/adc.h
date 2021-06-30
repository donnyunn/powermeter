#ifndef __ADC_H
#define __ADC_H

#include "main.h"

typedef struct {
    int reading[2];
} adc_t;

adc_t * adc_init(void);

#endif /* __ADC_H */