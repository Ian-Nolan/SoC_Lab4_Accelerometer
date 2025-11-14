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

/**
 * blink once per second for 5 times.
 * provide a sanity check for timer (based on SYS_CLK_FREQ)
 * @param led_p pointer to led instance
 */
// void timer_check(GpoCore *led_p) {
//    int i;

//    for (i = 0; i < 5; i++) {
//       led_p->write(0xffff);
//       sleep_ms(500);
//       led_p->write(0x0000);
//       sleep_ms(500);
//       debug("timer check - (loop #)/now: ", i, now_ms());
//    }
// }

/**
 * core test
 * @param led_p pointer to led instance
 * @param sw_p pointer to switch instance
 */
void show_test_id(int n, GpoCore *led_p) {
   int i, ptn;

   ptn = n; //1 << n;
   for (i = 0; i < 20; i++) {
      led_p->write(ptn);
      sleep_ms(30);
      led_p->write(0);
      sleep_ms(30);
   }
}

/**
 * check individual led
 * @param led_p pointer to led instance
 * @param n number of led
 */
void led_check(GpoCore *led_p, int n) {
   int i;

   for (i = 0; i < n; i++) {
      led_p->write(1, i);
      sleep_ms(200);
      led_p->write(0, i);
      sleep_ms(200);
   }
}


/*
void led_writes(GpoCore *led_p, int16_t LEDs) {
   for (int i = 0; i < 30; i++) {
      led_p->write(LEDs);
      sleep_ms(50);
      led_p->write(0);
      sleep_ms(50);
   }
}
*/

/**
 * uart transmits test line.
 * @note uart instance is declared as global variable in chu_io_basic.h
 */
void uart_check() {
   static int loop = 0;

   uart.disp("uart test #");
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

/**
 * Test adxl362 accelerometer using SPI
 */

void gsensor_check(SpiCore *spi_p, GpoCore *led_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t PART_ID_REG = 0x02;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;
   float x, y, z;
   int id;

   spi_p->set_freq(400000);
   spi_p->set_mode(0, 0);
   // check part id
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(PART_ID_REG);  // part id address
   id = (int) spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   uart.disp("read ADXL362 id (should be 0xf2): ");
   uart.disp(id, 16);
   uart.disp("\n\r");
   // read 8-bit x/y/z g values once
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(DATA_REG);  //
   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   x = (float) xraw / raw_max;
   y = (float) yraw / raw_max;
   z = (float) zraw / raw_max;
   uart.disp("x/y/z axis g values: ");
   uart.disp(x, 3);
   uart.disp(" / ");
   uart.disp(y, 3);
   uart.disp(" / ");
   uart.disp(z, 3);
   uart.disp("\n\r");
}

void setup_accel(SpiCore *spi) {
    const uint8_t PART_ID_REG = 0x02;
    const uint8_t RD_CMD = 0x0b;
    
    spi->assert_ss(0);    // activate
    // Set SPI frequency to 1 MHz (within recommended 1–5 MHz range)
    spi->set_freq(400000);

    // Set SPI mode 0: CPOL = 0, CPHA = 0
    spi->set_mode(0, 0);

    // Deassert all slave selects initially
    spi->write_ss_n(0xFFFFFFFF);

    // Assert the slave select for ADXL362
    spi->transfer(RD_CMD);  // part id address
    spi->transfer(PART_ID_REG);  // part id address

    // Put ADXL362 into measurement mode
    spi->transfer(0x0A);        // Write register command
    spi->transfer(0x2D);        // POWER_CTL register address
    spi->transfer(0x02);        // Set measurement mode

    spi->deassert_ss(0);    // Done with setup
}


// uint8_t read_register(SpiCore *spi_p, int ss_id, uint8_t reg_addr) {
//     spi_p->assert_ss(ss_id);           // Select ADXL362
//     spi_p->transfer(0x0B);             // Read register command
//     spi_p->transfer(reg_addr);        // Register address
//     uint8_t data = spi_p->transfer(0x00); // Dummy byte to receive data
//     spi_p->deassert_ss(ss_id);        // Deselect ADXL362
//     return data;
// }

// int16_t read_axis(SpiCore *spi_p, int ss_id, uint8_t addr_l, uint8_t addr_h) {
//     uint8_t low = read_register(spi_p, ss_id, addr_l);
//     uint8_t high = read_register(spi_p, ss_id, addr_h);
//     return (int16_t)((high << 8) | low);
// }


/**
 * adxl362 accelerometer using SPI
 */
 

float accel_sensor_x(SpiCore *spi_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw;
   float x, y, z;

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
   float x, y, z;

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
   float x, y, z;

   spi_p->assert_ss(0);    // activate
   
   spi_p->transfer(RD_CMD);  // for read operation

   spi_p->transfer(DATA_REG);  //

   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);

   spi_p->deassert_ss(0);
   
   return (float) zraw / raw_max;
}

/*
void detect_orientation(SpiCore& spi, int ss_id, volatile uint32_t* led_addr) {
    int16_t x = read_axis(spi, ss_id, 0x0E, 0x0F); // XDATA
    int16_t y = read_axis(spi, ss_id, 0x10, 0x11); // YDATA

    // Clear all orientation LEDs (LEDs 6–9)
    *led_addr &= ~(0x3C0); // Clear bits 6–9

    // Threshold for ~1g (adjust if needed)
    const int16_t threshold = 200;

    if (x > threshold && abs(y) < threshold) {
        led_toggle(GpoCore *led_p, int i); // 90°
    } else if (y < -threshold && abs(x) < threshold) {
        *led_addr |= (1 << 7); // 180°
    } else if (x < -threshold && abs(y) < threshold) {
        *led_addr |= (1 << 8); // 270°
    } else if (y > threshold && abs(x) < threshold) {
        *led_addr |= (1 << 9); // 0°
    }
}
*/

int abs(int inNum) {
    int outNum;
    if (inNum < 0) {
        outNum = (-1) * inNum;
    } else {
        outNum = inNum;
    }
    return outNum;
}


// instantiate switch, led
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
SpiCore SPI(get_slot_addr(BRIDGE_BASE, S9_SPI));

int main() {
	//accel_spi(&SPI);

	//////setup_accel(&SPI);

    const float threshold = 0.8;


	//int16_t x = read_axis(&SPI, 0, 0x0E, 0x0F); // XDATA
    //int16_t y = read_axis(&SPI, 0, 0x10, 0x11); // YDATA



	// int16_t x = read_axis(&SPI, 0, 0x0E, 0x0F);
	// led_toggle(&led, 6);
   // sleep_ms(500);

	// int16_t y = read_axis(&SPI, 0, 0x10, 0x11);
	// led_toggle(&led, 7);
   // sleep_ms(500);
	
	// int16_t z = read_axis(&SPI, 0, 0x12, 0x13);
	// led_toggle(&led, 8);
   // sleep_ms(500);






   //sleep_ms(2000);
   
   while (1) {
        //led_check(&led, 16);
        
        
        //show_test_id(3, &led);
        uart_check(); 
      
        float x = accel_sensor_x(&SPI);
        float y = accel_sensor_y(&SPI);
        


        //accel_sensor_z(&SPI, &led);
        //gsensor_check(&SPI, &led);

        
        if (x > threshold && abs(y) < threshold) {
            led_toggle(&led,  6); // 90°
            uart.disp("90 degrees!");
            uart.disp("\n\r");
        } else if (y < -threshold && abs(x) < threshold) {
            led_toggle(&led,  7); // 180°
            uart.disp("90 degrees!");
            uart.disp("\n\r");
        } else if (x < -threshold && abs(y) < threshold) {
            led_toggle(&led,  8); // 270°
            uart.disp("90 degrees!");
            uart.disp("\n\r");
        } else if (y > threshold && abs(x) < threshold) {
            led_toggle(&led,  9); // 0°
            uart.disp("90 degrees!");
            uart.disp("\n\r");

        } else {
            led_toggle(&led,  4); // ELSE
            led_toggle(&led,  5); // ELSE
            led_toggle(&led,  10); // ELSE
            led_toggle(&led,  11); // ELSE
            uart.disp("ELSE!");
            uart.disp("\n\r");
        }
        
        

        uart.disp("x = ");
        uart.disp(x, 3);
        uart.disp("\n\r");

        uart.disp("y = ");
        uart.disp(y, 3);
        uart.disp("\n\r\n\r");


        sleep_ms(250);

   } //while
} //main

