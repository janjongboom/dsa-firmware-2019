#ifndef _DSA_PERIPHERALS_H_
#define _DSA_PERIPHERALS_H_

#include "mbed.h"
#include "HTS221Sensor.h"
#include "LPS22HBSensor.h"
#include "TSL2572Sensor.h"
#include "SPIFBlockDevice.h"
#include "DavisAnemometer.h"
#include "SX1276_LoRaRadio.h"

#define LED_ON  0
#define LED_OFF 1

// Peripherals
static DigitalOut led1(PA_5, LED_OFF);
static DigitalOut led2(PA_6, LED_OFF);

static InterruptIn btn1(PD_8);
static InterruptIn btn2(PD_9);

static DevI2C dev_i2c(PB_9, PB_8);
static HTS221Sensor hts221(&dev_i2c);
static LPS22HBSensor lps22hb(&dev_i2c, LPS22HB_ADDRESS_LOW);
static TSL2572Sensor tsl2572(PB_9, PB_8);

static AnalogIn grove12_7(PA_2);   // marked as ADC12.7
static AnalogIn grove12_8(PA_3);   // marked as ADC12.8

static DavisAnemometer anemometer(PA_1, PD_4);

// Pull these down
static DigitalIn pm25_tx(PA_2);
static DigitalIn pm25_rx(PA_3);

// Block device
static SPIFBlockDevice bd(PB_5, PB_4, PB_3, PE_12);

// Unused pins
static DigitalIn unused[] = { PA_4, PA_7, PA_8, PA_11, PA_12, PB_6, PB_7, PC_0, PC_1, PC_2, PC_3, PC_4, PC_5, PC_6, PC_7, PC_8, PC_9, PC_13, PD_0, PD_2, PD_3, PD_5, PD_6, PD_7/*, PD_10 */, PD_11, PD_12, PD_13, PD_14, PD_15, PE_7, PE_8, PE_9, PE_10, PE_11, PE_13, PE_14, PE_15 };

static SX1276_LoRaRadio radio(PB_15,
                              PB_14,
                              PB_13,
                              PB_12,
                              PE_6,
                              PE_5,
                              PE_4,
                              PE_3,
                              PE_2,
                              PE_1,
                              PE_0,
                              NC,
                              NC,
                              NC,
                              NC,
                              NC,
                              NC,
                              NC);

#endif // _DSA_PERIPHERALS_H_
