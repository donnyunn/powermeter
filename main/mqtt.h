#ifndef __MQTT_H
#define __MQTT_H

#include "main.h"

typedef struct {
    void (*publish)(const char * topic, const char * msg);
} mqtt_t;

mqtt_t * mqtt_init(void);

#endif /* __MQTT_H */