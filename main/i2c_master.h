#ifndef __I2C_MASTER_H
#define __I2C_MASTER_H

#include "main.h"

typedef struct {
    esp_err_t (*read)(uint8_t addr, uint8_t * data, int len);
    esp_err_t (*write)(uint8_t addr, uint8_t * data, int len);
} i2c_master_t;

i2c_master_t * i2c_init(void);

#endif  /* __I2C_MASTER_H */