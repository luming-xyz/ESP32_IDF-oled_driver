#include "oled_driver.h"  
#include "biaoqing_table.h" 

expression_t expression_now;
// expression_t expression_before;

// init cmmand
static const uint8_t oled_init_cmd[] = {
    0x80,0xAE,//--turn off oled panel
    0x80,0x00,//---set low column address
    0x80,0x10,//---set high column address
    0x80,0x40,//--set start line address  Set Mapping RAM Display Start Line
    0x80,0x81,//--set contrast control register
    0x80,0xCF,// Set SEG Output Current Brightness
    0x80,0xA1,//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    0x80,0xC8,//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    0x80,0xA6,//--set normal display
    0x80,0xA8,//--set multiplex ratio(1 to 64)
    0x80,0x3F,//--1/64 duty
    0x80,0xD3,//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    0x80,0x00,//-not offset
    0x80,0xD5,//--set display clock divide ratio/oscillator frequency
    0x80,0x80,//--set divide ratio, Set Clock as 100 Frames/Sec
    0x80,0xD9,//--set pre-charge period
    0x80,0xF1,//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    0x80,0xDA,//--set com pins hardware configuration
    0x80,0x12,//
    0x80,0xDB,//--set vcomh
    0x80,0x40,//Set VCOM Deselect Level
    0x80,0x20,//-Set Page Addressing Mode (0x00/0x01/0x02)
    0x80,0x02,//
    0x80,0x8D,//--set Charge Pump enable/disable
    0x80,0x14,//--set(0x10) disable
    0x80,0xA4,// Disable Entire Display On (0xa4/0xa5)
    0x80,0xA6,// Disable Inverse Display On (0xa6/a7) 
    0x00,0xAF // 
}; 

static void disp_buf(uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

static esp_err_t i2c_master_write_slave(i2c_port_t i2c_num, uint8_t *data_wr, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_ADDR << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, data_wr, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t oled_init()
{
    esp_err_t esp_err;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = OLED_I2C_SDA_IO,         // select GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = OLED_I2C_SCL_IO,         // select GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = OLED_I2C_FREQ,  // select frequency specific to your project
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    if(i2c_param_config(OLED_I2C_POER_NUM, &conf) != ESP_OK)
        return 1;

    if(i2c_driver_install(OLED_I2C_POER_NUM, I2C_MODE_MASTER, 0, 0, 0) != ESP_OK)
        return 2;

    if(i2c_master_write_slave(OLED_I2C_POER_NUM, oled_init_cmd, 56) != ESP_OK)
        return 3; 

    return ESP_OK;
}

// 设置page 及其 显存起始地址  
esp_err_t oled_set_start_address(uint8_t page_addr, uint16_t ram_addr)
{
    esp_err_t esp_err;
    uint8_t set_addr_cmd[6] = {
        0x80,0xb0,// Set Page Start Address 
        0x80,0x00,// Set Lower Column Start Address
        0x00,0x10 // Set Higher Column Start Address
        };
    set_addr_cmd[1] = 0xb0 | page_addr;
    set_addr_cmd[3] = 0x00 | (ram_addr & 0x00ff);
    set_addr_cmd[5] = 0x10 | (ram_addr >> 8);
    // disp_buf(set_addr_cmd, 6); 
    esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, set_addr_cmd, 6);
    // printf("oled_set_start_address: %d \n", esp_err);
    return esp_err;
}  

// 刷新一个page  
esp_err_t oled_page_refresh(uint8_t page_addr, uint8_t *buf)
{
    esp_err_t esp_err;
    uint8_t refresh_page_cmd[129];
    refresh_page_cmd[0] = 0x40;
    for (size_t i = 1; i < 129; i++)
        refresh_page_cmd[i] = buf[i-1];
    oled_set_start_address(page_addr, 0);
    // disp_buf(refresh_page_cmd, 129);
    esp_err = i2c_master_write_slave(OLED_I2C_POER_NUM, refresh_page_cmd, 129);
    // printf("oled_page_refresh: %d \n", esp_err);
    return esp_err;
}

// 刷新整个显存 Graphic Display Data RAM (GDDRAM)  
esp_err_t oled_gddram_refresh(uint8_t *buf)
{
    esp_err_t esp_err;
    uint8_t refresh_page_gdd[128];
    for (uint8_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 128; j++)
        {
            refresh_page_gdd[j] = buf[i*128 + j];
        }
        esp_err = oled_page_refresh(i, refresh_page_gdd);
    }
    return esp_err;
} 

// 更新额头 forehead 表情  
esp_err_t oled_forehead_refresh(forehead_t forehead_expr, uint8_t frame_cnt)
{
    esp_err_t esp_err;
    oled_set_start_address(0, 0);
    oled_page_refresh(0, forehead_table_p0[forehead_expr][frame_cnt]);
    oled_set_start_address(1, 0);
    esp_err = oled_page_refresh(1, forehead_table_p1[forehead_expr][frame_cnt]);
    return esp_err;
}

// 更新眼睛 eyes 表情  
esp_err_t oled_eyes_refresh(eyes_t eyes_expr, uint8_t frame_cnt)
{
    // printf("oled_eyes_refresh:%d\n", frame_cnt);
    esp_err_t esp_err;
    oled_set_start_address(2, 0);
    oled_page_refresh(2, eyes_table_p2[eyes_expr][frame_cnt]);
    oled_set_start_address(3, 0);
    esp_err = oled_page_refresh(3, eyes_table_p3[eyes_expr][frame_cnt]);
    return esp_err;
}

// 更新鼻子 nose 表情  
esp_err_t oled_nose_refresh(nose_t nose_expr, uint8_t frame_cnt)
{
    esp_err_t esp_err;
    oled_set_start_address(4, 0);
    esp_err = oled_page_refresh(4, nose_table_p4[nose_expr][frame_cnt]);
    return esp_err;
}

// 更新嘴巴 mouth 表情  
esp_err_t oled_mouth_refresh(mouth_t mouth_expr, uint8_t frame_cnt)
{
    esp_err_t esp_err;
    oled_set_start_address(5, 0);
    oled_page_refresh(5, mouth_table_p5[mouth_expr][frame_cnt]);
    oled_set_start_address(6, 0);
    esp_err = oled_page_refresh(6, mouth_table_p6[mouth_expr][frame_cnt]);
    return esp_err;
}

// 更新下巴 chin 表情  
esp_err_t oled_chin_refresh(chin_t chin_expr, uint8_t frame_cnt)
{
    esp_err_t esp_err;
    oled_set_start_address(7, 0);
    esp_err = oled_page_refresh(7, chin_table_p7[chin_expr][frame_cnt]);
    return esp_err;
}

// 更新表情  
//      - expression: 当前表情 
esp_err_t oled_refresh_expression(expression_t expression)
{
    esp_err_t esp_err;
    for (size_t frame_cnt = 0; frame_cnt < 6; frame_cnt++)
    {
        expression_now = expression;
        if (expression_now.forehead_refresh_ena == 1)
            oled_forehead_refresh(expression_now.forehead_expr, frame_cnt);
        if (expression_now.eyes_refresh_ena == 1)
            oled_eyes_refresh(expression_now.eyes_expr, frame_cnt);
        if (expression_now.nose_refresh_ena == 1)
            oled_nose_refresh(expression_now.nose_expr, frame_cnt);
        if (expression_now.mouth_refresh_ena == 1) 
            oled_mouth_refresh(expression_now.mouth_expr, frame_cnt);
        if (expression_now.chin_refresh_ena == 1)
            esp_err = oled_chin_refresh(expression_now.chin_expr, frame_cnt);
        vTaskDelay(expression_now.frame_delay_ms / portTICK_RATE_MS);
    }
    return esp_err;
    
}

