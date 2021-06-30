#ifndef __MISCELLANEOUS_H
#define __MISCELLANEOUS_H

#include "main.h"

typedef struct {
    bool btn_event;
    void (*opled_ctrl)(bool on);
    bool opled;
    void (*stled_ctrl)(bool on);
    bool stled;
} misc_t;

// void misc_init(misc_t * app_misc);
misc_t* misc_init(void);

#endif /* __MISCELLANEOUS_H */