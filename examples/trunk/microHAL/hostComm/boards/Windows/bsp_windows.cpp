/*
 * bsp_windows.cpp
 *
 *  Created on: 23 cze 2015
 *      Author: Pawel
 */



#include "serialPort_windows.h"
#include "consoleIODevice.h"

using namespace microhal;


windows::SerialPort serialPort20("\\\\.\\COM20");
//windows::SerialPort serialPort21("\\\\.\\COM21");
//windows::SerialPort serialPort22("\\\\.\\COM22");

microhal::IODevice &debugPort = windows::consoleIODev;
microhal::SerialPort &communicationPort = serialPort20;
