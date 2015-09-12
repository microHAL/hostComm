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

    IOManager::routeSerial<1, Txd, stm32f4xx::GPIO::PortB, 6>();
    IOManager::routeSerial<1, Rxd, stm32f4xx::GPIO::PortB, 7>();

    IOManager::routeSerial<2, Txd, stm32f4xx::GPIO::PortA, 2>();
    IOManager::routeSerial<2, Rxd, stm32f4xx::GPIO::PortA, 3>();

    stm32f4xx::SerialPort::Serial1.clear();

	stm32f4xx::SerialPort::Serial1.setDataBits(stm32f4xx::SerialPort::Data8);
	stm32f4xx::SerialPort::Serial1.setStopBits(stm32f4xx::SerialPort::OneStop);
	stm32f4xx::SerialPort::Serial1.setParity(stm32f4xx::SerialPort::NoParity);
	stm32f4xx::SerialPort::Serial1.setBaudRate(stm32f4xx::SerialPort::Baud115200, stm32f4xx::SerialPort::AllDirections);


    xTaskHandle mainHandle;
    xTaskCreate(main, (const char* )"main", (10*1024), 0, tskIDLE_PRIORITY, &mainHandle);

    vTaskStartScheduler();
}

uint64_t SysTick_time = 0;
