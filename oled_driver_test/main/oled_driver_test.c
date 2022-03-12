#include <stdio.h>
#include <stdlib.h>
// #include "freertos/task.h"
// #include "freertos/queue.h"
// #include "driver/gpio.h"
#include "oled_driver.h" 

// 存储需要刷新的表情  
expression_t expression = {
    .forehead_expr       = forehead_none,
    .eyes_expr           = eyes_none,
    .nose_expr           = nose_none,
    .mouth_expr          = mouth_none,
    .chin_expr           = chin_none,
    .forehead_refresh_ena= 1,
    .eyes_refresh_ena    = 1,
    .nose_refresh_ena    = 1,
    .mouth_refresh_ena   = 1,
    .chin_refresh_ena    = 1,
    .frame_delay_ms = 10
};

void oled_task(void* arg)
{
    for(;;) {
        oled_refresh_expression(expression);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void app_main(void)
{
    oled_init();
    xTaskCreate(oled_task, "gpio_task", 2048, NULL, 10, NULL); 
    int cnt = 0;
    while(1)
    {
        printf("app_main cnt:%d s\n", cnt++);
        if (cnt % 2 == 0)
            expression.eyes_expr = wink;
        else   
            expression.eyes_expr = eyes_none;
        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

}
