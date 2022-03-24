/******************************************************************************
 * Project: Robocar
 * Application: Wemos_Wifi_SPI
 * Platform: Wemos D1 mini lite
 * @file spi_wrapper.c
 * @version v1.0
 * @brief Robocar_wemos wrapper for spi communication. Slave master (Nucleo 
 * board) sents two extra values (uint16_t 0x0E0F) as indicator of ending message,
 * to distinguish between no message sent (all 0s in MISO) and all 0s message
 * (all 0s message); the former should be ignored. The ending parameter will
 * be 0 if message has not been sent.
 ******************************************************************************/

/*******************************************************************************
 * INCLUSIONS
 ******************************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include "spi_wrapper.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

//#define DEBUG   // Uncomment for debugging (serial output)

/*******************************************************************************
 * FUNCTIONS PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS DEFINITIONS
 ******************************************************************************/

/*****************************************
 * @brief configures spi peripheral
 * @param None
 * @retval None
 ****************************************/
void mySpi_config(void) {
    const uint32_t myspi_freq_hz = 1000000;
    SPI.begin();
    SPI.setHwCs(true);
    SPI.beginTransaction (SPISettings (myspi_freq_hz, MSBFIRST, SPI_MODE0));
}

/*****************************************
 * @brief disables spi peripheral
 * @param None
 * @retval None
 ****************************************/
void mySpi_deconfig(void) {
    SPI.endTransaction();
}

/*****************************************
 * @brief Transmits uint16_t data pointed by message_out_ptr and receive
 * data interpreted as uint16_t
 * @param[in] message_out_ptr pointer to outgoing message (array of uint16_t)
 * @param[in] message_out_len length of elements to transmit
 * @param[out] message_in_ptr pointer to store incoming message (array of 
 * uint16_t)
 * @param[in] message_in_len length of elements to receive
 * @retval mess_in_received is true if there is any new incoming message
 ****************************************/
bool mySpi_transmit_uint16( uint16_t *message_out_ptr, 
                            unsigned int message_out_len,
                            uint16_t *message_in_ptr,
                            unsigned int message_in_len) {
    const uint16_t end_of_sequence = 0x0E0F;
    unsigned int message_max_len = max(message_out_len, message_in_len) + 1;
    uint16_t temp_data_in = 0;
    uint16_t temp_data_out = 0;
    bool mess_in_received = false;
    for (unsigned int iter=0; iter < message_max_len; ++iter) {
        // Prepare data to send
        if (message_out_len > 0 && iter < message_out_len) {
            temp_data_out = message_out_ptr[iter];
        } else if (message_out_len > 0 && iter == message_out_len) {
            temp_data_out = end_of_sequence;
        } else {
            temp_data_out = 0;
        }
        // Start transmission
        temp_data_in = SPI.transfer16(temp_data_out);
        // Store incoming element if not out of range
        if (iter < message_in_len) {
            message_in_ptr[iter] = temp_data_in;
        } else if (iter == message_in_len && temp_data_in == end_of_sequence) {
            mess_in_received = true;
        }
    }
    #if defined (DEBUG)
        Serial.print("\nSPI out: ");
        for (unsigned int iter=0; iter<message_out_len; iter++) {
            Serial.print(message_out_ptr[iter]);
            Serial.print(", ");
        }
        Serial.print("\nSPI in: ");
        if (mess_in_received == true) {
            for (unsigned int iter=0; iter<message_in_len; iter++) {
                Serial.print(message_in_ptr[iter]);
                Serial.print(", ");
            }
        } else {
            Serial.print("No data");
        }

    #endif
    return mess_in_received;
}