/* =========================================================================================== *//**
 @file        bsp_stm32f4-discovery.cpp
 @authors     Pawel Okas
 @version     $Id$
 @package
 @brief       hardware definitions
 @hardware    STM32F4Discovery
 @copyright   $Copyright$
 @details

 *//* ============================================================================================ */
#include "microhal.h"
#include "microhal_bsp.h"
#include "SPIDevice/SPIDevice.h"

#include "FreeRTOS.h"
#include "task.h"

using namespace microhal;
using namespace stm32f4xx;

void main(void *);

void hardwareConfig(void) {
    Core::pll_start(8000000, 168000000);
    Core::fpu_enable();

    IOManager::routeSerial<3, Txd, stm32f4xx::GPIO::PortD, 8>();
    IOManager::routeSerial<3, Rxd, stm32f4xx::GPIO::PortD, 9>();

    xTaskHandle mainHandle;
    xTaskCreate(main, (const signed char* )"main", (3*1024), 0, tskIDLE_PRIORITY, &mainHandle);

    vTaskStartScheduler();
}

uint64_t SysTick_time = 0;
