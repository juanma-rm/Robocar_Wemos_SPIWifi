/******************************************************************************
 * Project: Robocar
 * Application: Wemos_Wifi_SPI
 * Platform: Wemos D1 mini lite
 * @file wifi_wrapper.h
 * @version v1.0
 * @brief header of wifi_wrapper.c 
 ******************************************************************************/

#ifndef WIFI_WRAPPER_H   // Beginning of header guard WIFI_WRAPPER_H
#define WIFI_WRAPPER_H


/*******************************************************************************
 * INCLUSIONS
 ******************************************************************************/

#include <Arduino.h>
#include <stdbool.h>

/*******************************************************************************
 * FUNCTIONS PROTOTYPES
 ******************************************************************************/

void myWifi_config(void);
void myWifi_send(char *message_ptr, unsigned int message_length);
bool myWifi_recv(char *message_ptr, unsigned int message_length);

#endif  // End of header guard WIFI_WRAPPER_H