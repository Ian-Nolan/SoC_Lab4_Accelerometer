/*****************************************************************//**
 * @file main_vanilla_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "spi_core.h"


void timer_check(GpoCore *led_p) {
   int i;

   for (i = 0; i < 5; i++) {
      led_p->write(0xffff);
      sleep_ms(500);
      led_p->write(0x0000);
      sleep_ms(500);
      //debug("timer check - (loop #)/now: ", i, now_ms());
   }
}

void led_multi_blink(GpoCore *led_p) {
   int i;

   for (i = 0; i < 5; i++) {
      led_p->write(0b1111100001111111);
      sleep_ms(400);
      led_p->write(0x0000);
      sleep_ms(100);
      //debug("timer check - (loop #)/now: ", i, now_ms());
   }
}


void led_check(GpoCore *led_p, int n) {
   int i;

   for (i = 0; i < n; i++) {
      led_p->write(1, i);
      sleep_ms(200);
      led_p->write(0, i);
      sleep_ms(200);
   }
}


void uart_check() {
   static int loop = 0;

   uart.disp("Sensor test #");
   uart.disp(loop);
   uart.disp("\n\r");
   loop++;
}


void led_toggle(GpoCore *led_p, int i) {

	led_p->write(1, i);
	sleep_ms(1000);
	led_p->write(0, i);
	sleep_ms(200);
}

float accel_sensor_x(SpiCore *spi_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw;

   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(DATA_REG);  //

   xraw = spi_p->transfer(0x00);

   spi_p->deassert_ss(0);

   return (float) xraw / raw_max;
}

float accel_sensor_y(SpiCore *spi_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw;

   spi_p->assert_ss(0);    // activate
   
   spi_p->transfer(RD_CMD);  // for read operation

   spi_p->transfer(DATA_REG);  //

   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);

   spi_p->deassert_ss(0);
   
   return (float) yraw / raw_max;
}

float accel_sensor_z(SpiCore *spi_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;

   spi_p->assert_ss(0);    // activate
   
   spi_p->transfer(RD_CMD);  // for read operation

   spi_p->transfer(DATA_REG);  //

   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);

   spi_p->deassert_ss(0);
   
   return (float) zraw / raw_max;
}


int abs(int inNum) {
    int outNum;
    if (inNum < 0) {
        outNum = (-1) * inNum;
    } else {
        outNum = inNum;
    }
    return outNum;
}


// instantiate
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
SpiCore SPI(get_slot_addr(BRIDGE_BASE, S9_SPI));

int main() {

    const float threshold = 0.75;

    // led_multi_blink(&led);
    // sleep_ms(1000);
   
   while (1) {
        
        uart_check(); 
      

        float x = accel_sensor_x(&SPI);
        float y = accel_sensor_y(&SPI);
        

        if (x > threshold && abs(y) < threshold) {
            led_toggle(&led,  10); // 0°
            uart.disp("0 degrees!");
            uart.disp("\n\r");
        } else if (abs(x) < threshold && y > threshold) {
            led_toggle(&led,  9); // 90°
            uart.disp("90 degrees!");
            uart.disp("\n\r");
        } else if (x < -threshold && abs(y) < threshold) {
            led_toggle(&led,  8); // 180°
            uart.disp("180 degrees!");
            uart.disp("\n\r");
        } else if (abs(x) < threshold && y < -threshold) {
            led_toggle(&led,  7); // 270°
            uart.disp("270 degrees!");
            uart.disp("\n\r");

        } else {
            led_multi_blink(&led);
            uart.disp("ELSE!");
            uart.disp("\n\r");
        }
        

        uart.disp("x = ");
        uart.disp(x, 3);
        uart.disp("\n\r");

        uart.disp("y = ");
        uart.disp(y, 3);
        uart.disp("\n\r\n\r");


        sleep_ms(50);

   } //while
} //main

