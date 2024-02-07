#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#include "pico/binary_info.h"
#include "LCDops.h"
#include "generalOps.h"
#include "presetMessages.h"
#include "presetChars.h"

#define LDR_PIN 26 
#define LED_PIN2 18
#define LED_PIN 19
#define OPTICAL_PIN 13
#define BUZZER_PIN 14
#define COUNTS_PER_REVOLUTION 20

#define EN 15
#define RS 16
#define RW 17

#define D0 0
#define D1 1
#define D2 2
#define D3 3

#define D4 4
#define D5 5
#define D6 6
#define D7 7

volatile int wheel = 0;
void optical_isr() {
    // This function is called when the optical sensor reading changes from 0 to 1
    wheel++;
    printf("Wheel: %d\n", wheel);
}

float calculate_and_display_speed() {
    float speed = ((float)wheel / (20) )*0.2;
    char speed_message[80];
    sprintf(speed_message, "Speed: %.2f m/s", speed);
    LCDgoto("00");
    LCDwriteRawMessage(speed_message);
    wheel=0;
    return speed;
}



int LCDpins[14]={D0,D1,D2,D3,D4,D5,D6,D7,EN,RS,RW,16,2};

int main() {
    
    stdio_init_all();
    for(int gpio = 0; gpio < 11; gpio++){
        gpio_init(LCDpins[gpio]);
        gpio_set_dir(LCDpins[gpio], true);
        gpio_put(LCDpins[gpio], false);
    }
    char message[80];
    adc_init();
    adc_gpio_init(LDR_PIN);
    adc_select_input(0);

    gpio_init(OPTICAL_PIN);
    gpio_set_dir(OPTICAL_PIN, GPIO_IN);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(LED_PIN2);
    gpio_set_dir(LED_PIN2, GPIO_OUT);
    


    // Setup interrupt on optical sensor pin change (0 to 1)
    gpio_set_irq_enabled_with_callback(OPTICAL_PIN, GPIO_IRQ_EDGE_RISE, true, &optical_isr);


    LCDinit();
    LCDgoto("00");
    LCDsendRawInstruction(0, 0, "00001100");
    // LCDwriteRawMessage(message);
    int count = 0;
    float initialSpeed = 0.94;
    float speed_percentage = 1;
    float maxSpeed = initialSpeed * speed_percentage;


    while (1) {
        uint16_t ldr_value = adc_read();
        // uint16_t optical_value = gpio_get(OPTICAL_PIN);
        printf("Raw ADC Value: %d\n", ldr_value);

        

        if (ldr_value <= 3100) {
        speed_percentage = 0.5;  // 50% of the maximum speed
         } else if (ldr_value >= 3600) {
        speed_percentage = 1.0;  // 100% of the maximum speed
        } else {
        speed_percentage = 0.75;
        }    



        char maxSpeed_message[80];
        sprintf(maxSpeed_message, "Max: %.2f m/s", maxSpeed);
        LCDgoto("40");  // Assuming bottom row starts at address 40
        LCDwriteRawMessage(maxSpeed_message);
        
        if (ldr_value <= 3100) {
            gpio_put(LED_PIN, 1);
            gpio_put(LED_PIN2, 1);
           
        } else {
            gpio_put(LED_PIN, 0);
            gpio_put(LED_PIN2, 0);
        }

        float speed=calculate_and_display_speed();
        maxSpeed = initialSpeed * speed_percentage;


        // if(count < 1){
        //     sleep_ms(3000);
        //     count++;
        // }     

        if (maxSpeed < speed) {
            gpio_put(BUZZER_PIN, 1);
        } else {
            gpio_put(BUZZER_PIN, 0);
        } 
       sleep_ms(1000);
    }   
    return 0;
}
