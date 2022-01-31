/******************************************************************************
 * Project: Robocar
 * Application: Wemos_Wifi_SPI
 * Platform: Wemos D1 mini lite
 * @file spi_wrapper.h
 * @version v1.0
 * @brief header of spi_wrapper.c 
 ******************************************************************************/

#ifndef SPI_WRAPPER_H   // Beginning of header guard SPI_WRAPPER_H
#define SPI_WRAPPER_H


/*******************************************************************************
 * INCLUSIONS
 ******************************************************************************/

#include <Arduino.h>
#include <stdbool.h>

/*******************************************************************************
 * FUNCTIONS PROTOTYPES
 ******************************************************************************/

void mySpi_config(void);
void mySpi_deconfig(void);
bool mySpi_transmit_uint16  (   uint16_t *message_out_ptr, 
                                unsigned int message_out_len, 
                                uint16_t *message_in_ptr, 
                                unsigned int message_in_len
                            );

#endif  // End of header guard SPI_WRAPPER_H