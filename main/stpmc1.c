#include "stpmc1.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#define STPMC1_HOST    HSPI_HOST
#define DMA_CHAN    2
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS  2
#define PIN_SYN     4

static const char *TAG = "Dulab STPM";

spi_device_handle_t spi;


static bool verifyParity(uint32_t d)
{
	bool ret = true;
	uint8_t prt = 1;
	prt ^= d & 0x000000FF;
	prt ^= (d & 0x0000FF00) >> 8;
	prt ^= (d & 0x00FF0000) >> 16;
	prt ^= (d & 0xFF000000) >> 24;
	prt ^= prt << 4;
	prt &= 0xF0;
	
	if (prt != 0xF0)
	{
		ret = false;
	}
	
	return ret;
}

static stpmc1_t stpmc1 = {
    .meas_event = false,
};

static uint32_t stpmc1_read(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);
    uint32_t ret;
    esp_err_t err;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    // t.flags=SPI_TRANS_USE_RXDATA;
    t.length=STPMC1_REG_NUM*4*8+1;
    t.user=(void*)0;
    t.rx_buffer = stpmc1.reg;

    gpio_set_level(PIN_NUM_CS, 1);
    gpio_set_level(PIN_SYN, 0);
    gpio_set_level(PIN_SYN, 0);
    gpio_set_level(PIN_SYN, 0);
    gpio_set_level(PIN_SYN, 1);

    gpio_set_level(PIN_NUM_CS, 0);
    
    gpio_set_level(PIN_SYN, 0);
    gpio_set_level(PIN_SYN, 1);


    // for (int i = 0; i < STPMC1_REG_NUM; i++) {
    //     spi_device_polling_transmit(spi, &t);
    //     stpmc1.reg[i] = (t.rx_data[3] << 24) |
    //                     (t.rx_data[2] << 16) |
    //                     (t.rx_data[1] << 8) |
    //                     (t.rx_data[0]);
    // }

    err=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(err==ESP_OK);            //Should have had no issues.
    gpio_set_level(PIN_NUM_CS, 1);

    // for (int i = 0; i<STPMC1_REG_NUM; i++) {
	// 		ESP_LOGI(TAG, "0x%X", 
	// 		stpmc1.reg[i * 4 + 3] << 24 |
	// 		stpmc1.reg[i * 4 + 2] << 26 |
	// 		stpmc1.reg[i * 4 + 1] << 8 |
	// 		stpmc1.reg[i * 4 + 0] << 0);
    // }

    // ret = (t.rx_data[3] << 24) |
    //         (t.rx_data[2] << 16) |
    //         (t.rx_data[1] << 8) |
    //         (t.rx_data[0]);
    ret = 0;
    return ret;
}

static void stpmc1_write(uint8_t data)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);
	
	gpio_set_level(PIN_NUM_CS, 0);
    gpio_set_level(PIN_SYN, 0);
	
	esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=9;                     //Command is 8 bits
    t.tx_buffer=&data;               //The data is the cmd itself
    t.user=(void*)1;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
	
    gpio_set_level(PIN_SYN, 1);
	gpio_set_level(PIN_NUM_CS, 1);
    
}

static void stpcm1_load_regMap(void)
{
	// for (int i = 0; i<STPMC1_REG_NUM; i++) 
	// {
	// 	if (verifyParity(stpmc1.reg[i]) == false)
	// 	{
	// 		ESP_LOGI(TAG, "[APP] Parity failed. %s", __func__);
	// 		return;
	// 	}
	// }
	
	// load register to register map
	for (int i = 0; i<STPMC1_REG_NUM; i++) 
	{
        *((uint32_t*)&(stpmc1.regMap.DAP) + i) =
			(stpmc1.reg[i * 4 + 3] << 24) |
			(stpmc1.reg[i * 4 + 2] << 16) |
			(stpmc1.reg[i * 4 + 1] << 8) |
			(stpmc1.reg[i * 4 + 0] << 0);
    }
	
	stpmc1.phase3.energy_active_wideband = ENERGY(stpmc1.regMap.DAP);
	stpmc1.phase3.energy_reactive = ENERGY(stpmc1.regMap.DRP);
	stpmc1.phase3.energy_active_fundamental = ENERGY(stpmc1.regMap.DFP);
	stpmc1.phase3.status =  (stpmc1.regMap.DAP & 0x000000FF) | (stpmc1.regMap.DRP & 0x000000F0 << 4);
	
	stpmc1.R.uMOM = uMOM(stpmc1.regMap.DMR);
	stpmc1.R.iMOM = iMOM(stpmc1.regMap.DMR);
	stpmc1.R.uRMS = uRMS(stpmc1.regMap.DER);
	stpmc1.R.iRMS = iRMS(stpmc1.regMap.DER);
	stpmc1.R.energy_active_wideband = ENERGY(stpmc1.regMap.DAR);
	stpmc1.R.energy_active_wideband_status = STATUS(stpmc1.regMap.DAR);
	stpmc1.R.energy_reactive = ENERGY(stpmc1.regMap.DRR);	
	stpmc1.R.energy_reactive_status = STATUS(stpmc1.regMap.DRR);
	stpmc1.R.energy_active_fundamental = ENERGY(stpmc1.regMap.DFR);
	stpmc1.R.energy_active_fundamental_status = STATUS(stpmc1.regMap.DFR);
	
	stpmc1.S.uMOM = uMOM(stpmc1.regMap.DMS);
	stpmc1.S.iMOM = iMOM(stpmc1.regMap.DMS);
	stpmc1.S.uRMS = uRMS(stpmc1.regMap.DES);
	stpmc1.S.iRMS = iRMS(stpmc1.regMap.DES);
	stpmc1.S.energy_active_wideband = ENERGY(stpmc1.regMap.DAS);
	stpmc1.S.energy_active_wideband_status = STATUS(stpmc1.regMap.DAS);
	stpmc1.S.energy_reactive = ENERGY(stpmc1.regMap.DRS);
	stpmc1.S.energy_reactive_status = STATUS(stpmc1.regMap.DRS);
	stpmc1.S.energy_active_fundamental = ENERGY(stpmc1.regMap.DFS);
	stpmc1.S.energy_active_fundamental_status = STATUS(stpmc1.regMap.DFS);
	
	stpmc1.T.uMOM = uMOM(stpmc1.regMap.DMT);
	stpmc1.T.iMOM = iMOM(stpmc1.regMap.DMT);
	stpmc1.T.uRMS = uRMS(stpmc1.regMap.DET);
	stpmc1.T.iRMS = iRMS(stpmc1.regMap.DET);
	stpmc1.T.energy_active_wideband = ENERGY(stpmc1.regMap.DAT);
	stpmc1.T.energy_active_wideband_status = STATUS(stpmc1.regMap.DAT);
	stpmc1.T.energy_reactive = ENERGY(stpmc1.regMap.DRT);
	stpmc1.T.energy_reactive_status = STATUS(stpmc1.regMap.DRT);
	stpmc1.T.energy_active_fundamental = ENERGY(stpmc1.regMap.DFT);
	stpmc1.T.energy_active_fundamental_status = STATUS(stpmc1.regMap.DFT);
	
	stpmc1.N.uMOM = uMOM(stpmc1.regMap.DMN);
	stpmc1.N.iMOM = iMOM(stpmc1.regMap.DMN);
	stpmc1.N.uRMS = uRMS(stpmc1.regMap.DEN);
	stpmc1.N.iRMS = iRMS(stpmc1.regMap.DEN);
	
	stpmc1.period = (stpmc1.regMap.PRD & 0x0FFF0000) >> 16;
	stpmc1.DC = stpmc1.regMap.PRD & 0x0000FFFF;
	
	
	stpmc1.TSG = stpmc1.regMap.DRP & 0x0000000F;
	
	// config
	stpmc1.config._0_15 = (stpmc1.regMap.CF0 & 0x0000FFFF) >> 0;
	stpmc1.config._16_31 = ((stpmc1.regMap.CF0 & 0x0FFF0000) >> 16) |
						   ((stpmc1.regMap.CF1 & 0x0000000F) << 16);
	stpmc1.config._32_47 = (stpmc1.regMap.CF1 & 0x000FFFF0) << 4;
	stpmc1.config._48_63 = ((stpmc1.regMap.CF1 & 0x0FF00000) >> 20) |
						   ((stpmc1.regMap.CF2 & 0x000000FF) << 8);
	stpmc1.config._64_79 = ((stpmc1.regMap.CF2 & 0x00FFFF00) >> 8);
	stpmc1.config._80_95 = ((stpmc1.regMap.CF2 & 0x0F000000) >> 24) |
						   ((stpmc1.regMap.CF3 & 0x00000FFF) >> 0);
	stpmc1.config._96_111 = (stpmc1.regMap.CF3 & 0x0FFFF000) >> 12;
	
}

static void stpmc1_latch_measures(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);

    gpio_set_level(PIN_NUM_CS, 1);
    gpio_set_level(PIN_SYN, 0);
    gpio_set_level(PIN_SYN, 1);

}

void stpmc1_get_measures(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);

    uint32_t val = stpmc1_read();
}

void stpmc1_update_measures(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);
	
	stpcm1_load_regMap();
}

static void stpmc1_remote_reset(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);

    gpio_set_level(PIN_NUM_CS, 0);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_SYN, 0);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_CLK, 0);
    // vTaskDelay(1 / portTICK_PERIOD_MS);

    gpio_set_level(PIN_NUM_MOSI, 0);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_MOSI, 1);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_MOSI, 0);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_MOSI, 1);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_CLK, 1);

    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_SYN, 1);
    // vTaskDelay(1 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_CS, 1);

    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void stpmc1_set_config(uint8_t configAddr)
{
	stpmc1_write(configAddr | 0x80);
}
void stpmc1_clr_config(uint8_t configAddr)
{
	stpmc1_write(configAddr & ~0x80);
}

static void stpmc1_debug_msg()
{
    ESP_LOGI(TAG, "eaf: 0x%X", stpmc1.phase3.energy_active_fundamental);
    ESP_LOGI(TAG, "eaw: 0x%X", stpmc1.phase3.energy_active_wideband);
    ESP_LOGI(TAG, "er : 0x%X", stpmc1.phase3.energy_reactive);
    ESP_LOGI(TAG, "sts: 0x%X", stpmc1.phase3.status);
}


static void stpmc1_task(void* arg)
{
    stpmc1_clr_config(4);
    
    for (;;) {
         stpmc1_latch_measures();
        stpmc1_get_measures();
        stpmc1_update_measures();

        stpmc1.meas_event = true;
        vTaskDelay(1500 / portTICK_PERIOD_MS);
    }
}

void stpmc1_remoteLatch(void)
{
    gpio_set_level(PIN_NUM_CS, 1);
    gpio_set_level(PIN_SYN, 1);
    gpio_set_level(PIN_SYN, 0);
    gpio_set_level(PIN_SYN, 1);
}
void stpmc1_remoteReset(void)
{
    stpmc1_remote_reset();
}

stpmc1_t* stpmc1_init(void)
{
    ESP_LOGI(TAG, "[APP] %s", __func__);

    esp_err_t ret;
    
    gpio_set_direction(PIN_SYN, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_SYN, 1);

    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CS, 1);

    gpio_set_direction(PIN_NUM_CLK, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_CLK, 1);

    gpio_set_direction(PIN_NUM_MOSI, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_NUM_MOSI, 1);

    stpmc1_remote_reset();

    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=16
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=1*1000*1000,
        .flags=SPI_DEVICE_3WIRE,
        .mode=3, 
        .spics_io_num=-1,
        .queue_size=7,
    };

    ret=spi_bus_initialize(STPMC1_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);
    ret=spi_bus_add_device(STPMC1_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);

    xTaskCreate(stpmc1_task, "stpmc1_task", 2048, NULL, 10, NULL);

    return &stpmc1;
}
