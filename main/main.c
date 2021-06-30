#include "main.h"

#include "mqtt.h"
#include "stpmc1.h"
#include "adc.h"
#include "i2c_master.h"
#include "uart_rasp.h"
#include "miscellaneous.h"

static const char * TAG = "Dulab Main";

typedef struct {
    adc_t * adc;
    i2c_master_t * i2c;
    misc_t * misc;
    stpmc1_t * stpmc1;
    mqtt_t * mqtt;
} dulab_app_t;
static dulab_app_t app;

static int test = 0;

static void dulab_app_start(void)
{
    uint8_t i2c_test_buf[8] = {0x00,};
    char mqtt_tx[0x100];

    ESP_LOGI(TAG, "[APP] %s", __func__);
    app.adc = adc_init();
    app.i2c = i2c_init();
    uart_rasp_init();
    app.misc = misc_init();
    app.mqtt = mqtt_init();
    app.stpmc1 = stpmc1_init();

    while (1) {
        if (app.misc->btn_event) {
            ESP_LOGI(TAG, "[APP] button");
            // MQTT Sample Code
            app.mqtt->publish("dulab/app", "button pressed");

            // LED Sample Code
            if (app.misc->stled) {
                app.misc->stled_ctrl(false);
            } else {
                app.misc->stled_ctrl(true);
            }

            // ADC Sample Code
            ESP_LOGI(TAG, "[ADC] ch6: %d", app.adc->reading[0]);
            ESP_LOGI(TAG, "[ADC] ch7: %d", app.adc->reading[1]);

            // I2C Sample Code
            i2c_test_buf[0] = 0x00;
            i2c_test_buf[1] = 0x01;
            app.i2c->write(0x42, i2c_test_buf, 2);
            app.i2c->read(0x08, i2c_test_buf, 6);
            ESP_LOGI(TAG, "[I2C] I2C: %s", i2c_test_buf);

            vTaskDelay(100 / portTICK_PERIOD_MS);
            app.misc->btn_event = false;

            // stpmc1 reset 
            //stpmc1_remoteLatch();
            stpmc1_set_config(test++);

            //stpmc1_get_measures();
            //stpmc1_update_measures();
           
        }

        if (app.stpmc1->meas_event) {
            ESP_LOGI(TAG, "[PM]");
            app.stpmc1->meas_event = false;

            sprintf(mqtt_tx, "%08X", app.stpmc1->N.uMOM);
            app.mqtt->publish("dulab/N/uMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->N.iMOM);
            app.mqtt->publish("dulab/N/iMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->N.uRMS);
            app.mqtt->publish("dulab/N/uRMS", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->N.iRMS);
            app.mqtt->publish("dulab/N/iRMS", mqtt_tx);

            sprintf(mqtt_tx, "%08X", app.stpmc1->R.uMOM);
            app.mqtt->publish("dulab/R/uMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->R.iMOM);
            app.mqtt->publish("dulab/R/iMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->R.uRMS);
            app.mqtt->publish("dulab/R/uRMS", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->R.iRMS);
            app.mqtt->publish("dulab/R/iRMS", mqtt_tx);

            sprintf(mqtt_tx, "%08X", app.stpmc1->S.uMOM);
            app.mqtt->publish("dulab/S/uMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->S.iMOM);
            app.mqtt->publish("dulab/S/iMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->S.uRMS);
            app.mqtt->publish("dulab/S/uRMS", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->S.iRMS);
            app.mqtt->publish("dulab/S/iRMS", mqtt_tx);

            sprintf(mqtt_tx, "%08X", app.stpmc1->T.uMOM);
            app.mqtt->publish("dulab/T/uMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->T.iMOM);
            app.mqtt->publish("dulab/T/iMOM", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->T.uRMS);
            app.mqtt->publish("dulab/T/uRMS", mqtt_tx);
            sprintf(mqtt_tx, "%08X", app.stpmc1->T.iRMS);
            app.mqtt->publish("dulab/T/iRMS", mqtt_tx);

            sprintf(mqtt_tx, "%04X", app.stpmc1->config._0_15);
            app.mqtt->publish("dulab/CFG/1", mqtt_tx);
            sprintf(mqtt_tx, "%04X", app.stpmc1->config._16_31);
            app.mqtt->publish("dulab/CFG/2", mqtt_tx);
            sprintf(mqtt_tx, "%04X", app.stpmc1->config._32_47);
            app.mqtt->publish("dulab/CFG/3", mqtt_tx);
            sprintf(mqtt_tx, "%04X", app.stpmc1->config._48_63);
            app.mqtt->publish("dulab/CFG/4", mqtt_tx);
            sprintf(mqtt_tx, "%04X", app.stpmc1->config._64_79);
            app.mqtt->publish("dulab/CFG/5", mqtt_tx);
            sprintf(mqtt_tx, "%04X", app.stpmc1->config._80_95);
            app.mqtt->publish("dulab/CFG/6", mqtt_tx);
            sprintf(mqtt_tx, "%04X", app.stpmc1->config._96_111);
            app.mqtt->publish("dulab/CFG/7", mqtt_tx);

            sprintf(mqtt_tx, "%08X", app.stpmc1->regMap.CF0);
            app.mqtt->publish("dulab/CFG/CF0", mqtt_tx);

        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    dulab_app_start();
}
