#include "miscellaneous.h"

#include "driver/gpio.h"

static const char * TAG = "Dulab Misc";

#define GPIO_BUTTON_1     5
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_BUTTON_1)
#define ESP_INTR_FLAG_DEFAULT 0

#define GPIO_LED_OPERATION   26
#define GPIO_LED_STATUS      25

static misc_t misc = {
    .btn_event = false,
    .opled_ctrl = NULL,
    .opled = false,
    .stled_ctrl = NULL,
    .stled = false,
};

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR button_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void button_task(void* arg)
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // ESP_LOGI(TAG, "GPIO[%d] intr, val: %d", io_num, gpio_get_level(io_num));
            misc.btn_event = true;
        }
    }
}

static void button_init(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 0;
    io_conf.pull_down_en = 1;
    gpio_config(&io_conf);

    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(button_task, "gpio_task_example", 1024, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    
    gpio_isr_handler_add(GPIO_BUTTON_1, button_isr_handler, (void*) GPIO_BUTTON_1); //hook isr handler for specific gpio pin
}

static void opled_ctrl(bool on) 
{
    if (on) gpio_set_level(GPIO_LED_OPERATION, 1);
    else gpio_set_level(GPIO_LED_OPERATION, 0);
    misc.opled = on;
}

static void stled_ctrl(bool on) 
{
    if (on) gpio_set_level(GPIO_LED_STATUS, 1);
    else gpio_set_level(GPIO_LED_STATUS, 0);
    misc.stled = on;
}

static void led_init(void) 
{
    ESP_LOGI(TAG, "[APP] %s", __func__);
    gpio_set_direction(GPIO_LED_OPERATION, GPIO_MODE_OUTPUT);
    opled_ctrl(false);
    gpio_set_direction(GPIO_LED_STATUS, GPIO_MODE_OUTPUT);
    stled_ctrl(false);
    misc.opled_ctrl = opled_ctrl;
    misc.stled_ctrl = stled_ctrl;
}

misc_t* misc_init(void)
{
    // misc_t * dummy;
    ESP_LOGI(TAG, "[APP] %s", __func__);

    // app_misc = &misc;
    // dummy = app_misc;

    button_init();
    led_init();

    return &misc;
}