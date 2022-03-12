#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h" 


#define OLED_I2C_SDA_IO 22          //I2C SDA GPIO Number   
#define OLED_I2C_SCL_IO 23          //I2C SCL GPIO Number    
#define OLED_I2C_POER_NUM 1         //OLED 使用的 I2C 驱动号
#define OLED_I2C_FREQ 400000        //OLED 对应 I2C 时钟  
#define OLED_ADDR 0x3c              //OLED I2C 器件地址    

#define WRITE_BIT I2C_MASTER_WRITE  //I2C master write 
#define READ_BIT I2C_MASTER_READ    //I2C master read 
#define ACK_CHECK_EN 0x1            //I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0           //I2C master will not check ack from slave 
#define ACK_VAL 0x0                 //I2C ack value 
#define NACK_VAL 0x1                //I2C nack value   

typedef enum  tag_forehead{
    forehead_none, // 正常
    arc // Raise your eyebrows 挑眉
}forehead_t;

typedef enum  tag_eyes{
    eyes_none, //正常
    blink, // 眨眼
    dull, // 呆滞
    tear, // 流泪
    wink, // 使眼色
    look_left, //向左看
    look_right, //向右看
}eyes_t;

typedef enum  tag_nose{
    nose_none, // 正常
}nose_t;

typedef enum  tag_mouth{
    mouth_none, // 正常  
    line_up, // 线上扬 
    eat, // 线波浪  
    grin, // 龇牙咧嘴
}mouth_t;

typedef enum tag_chin{
    chin_none // 正常
}chin_t;

typedef struct tag_expression
{
    forehead_t forehead_expr ; //page 0-1
    eyes_t eyes_expr ; // page 2-3
    nose_t nose_expr ; // page 4
    mouth_t mouth_expr ; // page 5-6
    chin_t chin_expr ; // page 7
    uint8_t forehead_refresh_ena;
    uint8_t eyes_refresh_ena;
    uint8_t nose_refresh_ena;
    uint8_t mouth_refresh_ena;
    uint8_t chin_refresh_ena;
    uint16_t frame_delay_ms;
}expression_t;


// 初始化OLED
//      - 初始化OLED对应的 I2C 接口  
//      - 初始化OLED工作状态  
// i2c_num : i2c端口号 0/1
esp_err_t oled_init();

// 设置显存的起始地址，包括页地址和ram地址。
//      - page_addr,页地址  
//      - ram_addr, 此页内的ram地址  
esp_err_t oled_set_start_address(uint8_t page_addr, uint16_t ram_addr);

// 刷新一个page的显存  
//      - page_addr，页地址  
//      - buf, 图像数据  
esp_err_t oled_page_refresh(uint8_t page_addr, uint8_t *buf);

// 刷新整个显存 Graphic Display Data RAM (GDDRAM)  
//      - buf，图像数据
esp_err_t oled_gddram_refresh(uint8_t *buf);

// 更新额头 forehead 表情  
esp_err_t oled_forehead_refresh(forehead_t forehead_expr, uint8_t frame_cnt);

// 更新额头 eyes 表情  
esp_err_t oled_eyes_refresh(eyes_t eyes_expr, uint8_t frame_cnt);

// 更新鼻子 nose 表情  
esp_err_t oled_nose_refresh(nose_t nose_expr, uint8_t frame_cnt);

// 更新嘴巴 mouth 表情  
esp_err_t oled_mouth_refresh(mouth_t mouth_expr, uint8_t frame_cnt);

// 更新下巴 chin 表情  
esp_err_t oled_chin_refresh(chin_t chin_expr, uint8_t frame_cnt);

// 更新表情  
//      - expression: 当前表情 
esp_err_t oled_refresh_expression(expression_t expression);