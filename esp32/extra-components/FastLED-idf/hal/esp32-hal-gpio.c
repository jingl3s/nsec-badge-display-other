// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esp32-hal-gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp32/rom/ets_sys.h"
#include "esp_attr.h"
#include "esp_intr_alloc.h"
#include "esp32/rom/gpio.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_struct.h"
#include "soc/rtc_io_reg.h"
#include "soc/rtc_io_periph.h"

#include "driver/rtc_io.h"

// 
// NOTE. There was a structure rtc_gpio_desc which described the rtc
// pins in terms of GPIO pin numbers. In 4.1 this was depricated - although it
// could be included through menuconfig - in favor of rtc_io_desc, which is 
// indexed by the RTC_IO_PIN, which is looked up from gpio pin through rtc_io_num_map.
// as of 9/2020, have recoded for the new API . This is touched really only in pinMode.
// The obvious way to cover both 4.0 and 4.1++ is to have two versions of that function,
// see below.

#include "esp_idf_version.h"
// #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4,1,0) --- this is how it's done


const int8_t esp32_adc2gpio[20] = {36, 37, 38, 39, 32, 33, 34, 35, -1, -1, 4, 0, 2, 15, 13, 12, 14, 27, 25, 26};

// BB - this is defined in everything except 4.1
#ifndef GPIO_PIN_COUNT
#define GPIO_PIN_COUNT 40
#endif

const DRAM_ATTR esp32_gpioMux_t esp32_gpioMux[GPIO_PIN_COUNT]={
    {0x44, 11, 11, 1},  /* 00 */
    {0x88, -1, -1, -1}, /* 01 */
    {0x40, 12, 12, 2},  /* 02 */
    {0x84, -1, -1, -1}, /* 03 */
    {0x48, 10, 10, 0},  /* 04 */
    {0x6c, -1, -1, -1}, /* 05 */
    {0x60, -1, -1, -1}, /* 06 */
    {0x64, -1, -1, -1}, /* 07 */
    {0x68, -1, -1, -1}, /* 08 */
    {0x54, -1, -1, -1}, /* 09 */
    {0x58, -1, -1, -1}, /* 10 */
    {0x5c, -1, -1, -1}, /* 11 */
    {0x34, 15, 15, 5},  /* 12 */
    {0x38, 14, 14, 4},  /* 13 */
    {0x30, 16, 16, 6},  /* 14 */
    {0x3c, 13, 13, 3},  /* 15 */
    {0x4c, -1, -1, -1}, /* 16 */
    {0x50, -1, -1, -1}, /* 17 */
    {0x70, -1, -1, -1}, /* 18 */
    {0x74, -1, -1, -1}, /* 19 */
    {0x78, -1, -1, -1}, /* 20 */
    {0x7c, -1, -1, -1}, /* 21 */
    {0x80, -1, -1, -1}, /* 22 */
    {0x8c, -1, -1, -1}, /* 23 */
    {0, -1, -1, -1},    /* 24 */
    {0x24, 6, 18, -1}, //DAC1
    {0x28, 7, 19, -1}, //DAC2
    {0x2c, 17, 17, 7},  /* 27 */
    {0, -1, -1, -1},    /* 28 */
    {0, -1, -1, -1},    /* 29 */
    {0, -1, -1, -1},    /* 30 */
    {0, -1, -1, -1},    /* 31 */
    {0x1c, 9, 4, 8},    /* 32 */
    {0x20, 8, 5, 9},    /* 33 */
    {0x14, 4, 6, -1},   /* 34 */
    {0x18, 5, 7, -1},   /* 35 */
    {0x04, 0, 0, -1},   /* 36 */
    {0x08, 1, 1, -1},   /* 37 */
    {0x0c, 2, 2, -1},   /* 38 */
    {0x10, 3, 3, -1}    /* 39 */
};

typedef void (*voidFuncPtr)(void);
typedef void (*voidFuncPtrArg)(void*);
typedef struct {
    voidFuncPtr fn;
    void* arg;
    bool functional;
} InterruptHandle_t;
static InterruptHandle_t __pinInterruptHandlers[GPIO_PIN_COUNT] = {0,};

//
// we need two versions of this function, because the API has changed
// between 4.0 and 4.1++. You can get the depricated API through a menuconfig
// setting, or you just code it like this.

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4,1,0) 

static bool IRAM_ATTR __pinModeLockRTC(uint8_t pin, uint8_t mode) {

    uint32_t rtc_reg = rtc_gpio_desc[pin].reg;
    if(mode == ANALOG) {
        if(!rtc_reg) {
            return(false);//not rtc pin
        }
        //lock rtc
        uint32_t reg_val = ESP_REG(rtc_reg);
        if(reg_val & rtc_gpio_desc[pin].mux){
            return(false);//already in adc mode
        }
        reg_val &= ~(
                (RTC_IO_TOUCH_PAD1_FUN_SEL_V << rtc_gpio_desc[pin].func)
                |rtc_gpio_desc[pin].ie
                |rtc_gpio_desc[pin].pullup
                |rtc_gpio_desc[pin].pulldown);
        ESP_REG(RTC_GPIO_ENABLE_W1TC_REG) = (1 << (rtc_gpio_desc[pin].rtc_num + RTC_GPIO_ENABLE_W1TC_S));
        ESP_REG(rtc_reg) = reg_val | rtc_gpio_desc[pin].mux;
        //unlock rtc
        ESP_REG(DR_REG_IO_MUX_BASE + esp32_gpioMux[pin].reg) = ((uint32_t)2 << MCU_SEL_S) | ((uint32_t)2 << FUN_DRV_S) | FUN_IE;
        return(false);
    }

    //RTC pins PULL settings
    if(rtc_reg) {
        //lock rtc
        ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_gpio_desc[pin].mux);
        if(mode & PULLUP) {
            ESP_REG(rtc_reg) = (ESP_REG(rtc_reg) | rtc_gpio_desc[pin].pullup) & ~(rtc_gpio_desc[pin].pulldown);
        } else if(mode & PULLDOWN) {
            ESP_REG(rtc_reg) = (ESP_REG(rtc_reg) | rtc_gpio_desc[pin].pulldown) & ~(rtc_gpio_desc[pin].pullup);
        } else {
            ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_gpio_desc[pin].pullup | rtc_gpio_desc[pin].pulldown);
        }
        //unlock rtc
    }
    return(true);

}

#else // ESP_IDF_VERSION >= 4.1

static bool IRAM_ATTR __pinModeLockRTC(uint8_t pin, uint8_t mode) {

    // Find out if GPIO pin is RTC pin
    int rtc_pin = rtc_io_number_get(pin);

    if(mode == ANALOG) {
        if(rtc_pin == -1) {
            return(false);//not rtc pin
        }
        uint32_t rtc_reg = rtc_io_desc[rtc_pin].reg;
        //lock rtc
        uint32_t reg_val = ESP_REG(rtc_reg);
        if(reg_val & rtc_io_desc[rtc_pin].mux){
            return(false);//already in adc mode
        }
        reg_val &= ~(
                (RTC_IO_TOUCH_PAD1_FUN_SEL_V << rtc_io_desc[rtc_pin].func)
                |rtc_io_desc[rtc_pin].ie
                |rtc_io_desc[rtc_pin].pullup
                |rtc_io_desc[rtc_pin].pulldown);
        ESP_REG(RTC_GPIO_ENABLE_W1TC_REG) = (1 << (rtc_io_desc[rtc_pin].rtc_num + RTC_GPIO_ENABLE_W1TC_S));
        ESP_REG(rtc_reg) = reg_val | rtc_io_desc[rtc_pin].mux;
        //unlock rtc
        ESP_REG(DR_REG_IO_MUX_BASE + esp32_gpioMux[rtc_pin].reg) = ((uint32_t)2 << MCU_SEL_S) | ((uint32_t)2 << FUN_DRV_S) | FUN_IE;
        return(false);
    }

    //RTC pins PULL settings
    if(rtc_pin != -1) {
        //lock rtc
        uint32_t rtc_reg = rtc_io_desc[rtc_pin].reg;
        ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_io_desc[rtc_pin].mux);
        if(mode & PULLUP) {
            ESP_REG(rtc_reg) = (ESP_REG(rtc_reg) | rtc_io_desc[rtc_pin].pullup) & ~(rtc_io_desc[rtc_pin].pulldown);
        } else if(mode & PULLDOWN) {
            ESP_REG(rtc_reg) = (ESP_REG(rtc_reg) | rtc_io_desc[rtc_pin].pulldown) & ~(rtc_io_desc[rtc_pin].pullup);
        } else {
            ESP_REG(rtc_reg) = ESP_REG(rtc_reg) & ~(rtc_io_desc[rtc_pin].pullup | rtc_io_desc[rtc_pin].pulldown);
        }
        //unlock rtc
    }
    return(true);

}

#endif // ESP_IDF_VERSION

extern void IRAM_ATTR __pinMode(uint8_t pin, uint8_t mode)
{

    if(!digitalPinIsValid(pin)) {
        return;
    }

    if (false == __pinModeLockRTC(pin, mode)) {
        return;
    }

    uint32_t pinFunction = 0, pinControl = 0;

    //lock gpio
    if(mode & INPUT) {
        if(pin < 32) {
            GPIO.enable_w1tc = ((uint32_t)1 << pin);
        } else {
            GPIO.enable1_w1tc.val = ((uint32_t)1 << (pin - 32));
        }
    } else if(mode & OUTPUT) {
        if(pin > 33){
            //unlock gpio
            return;//pins above 33 can be only inputs
        } else if(pin < 32) {
            GPIO.enable_w1ts = ((uint32_t)1 << pin);
        } else {
            GPIO.enable1_w1ts.val = ((uint32_t)1 << (pin - 32));
        }
    }

    if(mode & PULLUP) {
        pinFunction |= FUN_PU;
    } else if(mode & PULLDOWN) {
        pinFunction |= FUN_PD;
    }

    pinFunction |= ((uint32_t)2 << FUN_DRV_S);//what are the drivers?
    pinFunction |= FUN_IE;//input enable but required for output as well?

    if(mode & (INPUT | OUTPUT)) {
        pinFunction |= ((uint32_t)2 << MCU_SEL_S);
    } else if(mode == SPECIAL) {
        pinFunction |= ((uint32_t)(((pin)==1||(pin)==3)?0:1) << MCU_SEL_S);
    } else {
        pinFunction |= ((uint32_t)(mode >> 5) << MCU_SEL_S);
    }

    ESP_REG(DR_REG_IO_MUX_BASE + esp32_gpioMux[pin].reg) = pinFunction;

    if(mode & OPEN_DRAIN) {
        pinControl = (1 << GPIO_PIN0_PAD_DRIVER_S);
    }

    GPIO.pin[pin].val = pinControl;
    //unlock gpio
}

extern void IRAM_ATTR __digitalWrite(uint8_t pin, uint8_t val)
{
    if(val) {
        if(pin < 32) {
            GPIO.out_w1ts = ((uint32_t)1 << pin);
        } else if(pin < 34) {
            GPIO.out1_w1ts.val = ((uint32_t)1 << (pin - 32));
        }
    } else {
        if(pin < 32) {
            GPIO.out_w1tc = ((uint32_t)1 << pin);
        } else if(pin < 34) {
            GPIO.out1_w1tc.val = ((uint32_t)1 << (pin - 32));
        }
    }
}

extern int IRAM_ATTR __digitalRead(uint8_t pin)
{
    if(pin < 32) {
        return (GPIO.in >> pin) & 0x1;
    } else if(pin < 40) {
        return (GPIO.in1.val >> (pin - 32)) & 0x1;
    }
    return 0;
}

static intr_handle_t gpio_intr_handle = NULL;

static void IRAM_ATTR __onPinInterrupt()
{
    uint32_t gpio_intr_status_l=0;
    uint32_t gpio_intr_status_h=0;

    gpio_intr_status_l = GPIO.status;
    gpio_intr_status_h = GPIO.status1.val;
    GPIO.status_w1tc = gpio_intr_status_l;//Clear intr for gpio0-gpio31
    GPIO.status1_w1tc.val = gpio_intr_status_h;//Clear intr for gpio32-39

    uint8_t pin=0;
    if(gpio_intr_status_l) {
        do {
            if(gpio_intr_status_l & ((uint32_t)1 << pin)) {
                if(__pinInterruptHandlers[pin].fn) {
                    if(__pinInterruptHandlers[pin].arg){
                        ((voidFuncPtrArg)__pinInterruptHandlers[pin].fn)(__pinInterruptHandlers[pin].arg);
                    } else {
                        __pinInterruptHandlers[pin].fn();
                    }
                }
            }
        } while(++pin<32);
    }
    if(gpio_intr_status_h) {
        pin=32;
        do {
            if(gpio_intr_status_h & ((uint32_t)1 << (pin - 32))) {
                if(__pinInterruptHandlers[pin].fn) {
                    if(__pinInterruptHandlers[pin].arg){
                        ((voidFuncPtrArg)__pinInterruptHandlers[pin].fn)(__pinInterruptHandlers[pin].arg);
                    } else {
                        __pinInterruptHandlers[pin].fn();
                    }
                }
            }
        } while(++pin<GPIO_PIN_COUNT);
    }
}

extern void cleanupFunctional(void* arg);

extern void __attachInterruptFunctionalArg(uint8_t pin, voidFuncPtrArg userFunc, void * arg, int intr_type, bool functional)
{
    static bool interrupt_initialized = false;

    if(!interrupt_initialized) {
        interrupt_initialized = true;
        esp_intr_alloc(ETS_GPIO_INTR_SOURCE, (int)ESP_INTR_FLAG_IRAM, __onPinInterrupt, NULL, &gpio_intr_handle);
    }

    // if new attach without detach remove old info
    if (__pinInterruptHandlers[pin].functional && __pinInterruptHandlers[pin].arg)
    {
    	cleanupFunctional(__pinInterruptHandlers[pin].arg);
    }
    __pinInterruptHandlers[pin].fn = (voidFuncPtr)userFunc;
    __pinInterruptHandlers[pin].arg = arg;
    __pinInterruptHandlers[pin].functional = functional;

    esp_intr_disable(gpio_intr_handle);
    if(esp_intr_get_cpu(gpio_intr_handle)) { //APP_CPU
        GPIO.pin[pin].int_ena = 1;
    } else { //PRO_CPU
        GPIO.pin[pin].int_ena = 4;
    }
    GPIO.pin[pin].int_type = intr_type;
    esp_intr_enable(gpio_intr_handle);
}

extern void __attachInterruptArg(uint8_t pin, voidFuncPtrArg userFunc, void * arg, int intr_type)
{
	__attachInterruptFunctionalArg(pin, userFunc, arg, intr_type, false);
}

extern void __attachInterrupt(uint8_t pin, voidFuncPtr userFunc, int intr_type) {
    __attachInterruptFunctionalArg(pin, (voidFuncPtrArg)userFunc, NULL, intr_type, false);
}

extern void __detachInterrupt(uint8_t pin)
{
    esp_intr_disable(gpio_intr_handle);
    if (__pinInterruptHandlers[pin].functional && __pinInterruptHandlers[pin].arg)
    {
    	cleanupFunctional(__pinInterruptHandlers[pin].arg);
    }
    __pinInterruptHandlers[pin].fn = NULL;
    __pinInterruptHandlers[pin].arg = NULL;
    __pinInterruptHandlers[pin].functional = false;

    GPIO.pin[pin].int_ena = 0;
    GPIO.pin[pin].int_type = 0;
    esp_intr_enable(gpio_intr_handle);
}


extern void pinMode(uint8_t pin, uint8_t mode) __attribute__ ((weak, alias("__pinMode")));
extern void digitalWrite(uint8_t pin, uint8_t val) __attribute__ ((weak, alias("__digitalWrite")));
extern int digitalRead(uint8_t pin) __attribute__ ((weak, alias("__digitalRead")));
extern void attachInterrupt(uint8_t pin, voidFuncPtr handler, int mode) __attribute__ ((weak, alias("__attachInterrupt")));
extern void attachInterruptArg(uint8_t pin, voidFuncPtrArg handler, void * arg, int mode) __attribute__ ((weak, alias("__attachInterruptArg")));
extern void detachInterrupt(uint8_t pin) __attribute__ ((weak, alias("__detachInterrupt")));

