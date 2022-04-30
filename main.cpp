/******************************************************************************
 * Project: Robocar
 * Application: Wemos_Wifi_SPI
 * Platform: Wemos D1 mini lite
 * @file main.c
 * @version v1.0
 * @brief Robocar_wemos main program. This file is responsible of communicating 
 * the main car board (Nucleo-F767ZI) to a control app (Python) via Wifi.
 *  - This program runs on a Wemos D1 mini board
 *  - Wemos D1 mini and car board communicate via SPI, being Wemos the master.
 * Data packages are interpreted as uint16_t.
 *  - Wemos D1 mini and the control app communicate via Wifi. Data packages are
 * interpreted as array of 5 chars (representing a uint16_t number)
 ******************************************************************************/

/*******************************************************************************
 * PINOUT
 ******************************************************************************/

// WEMOS_D0_IN      --> WEMOS_RST_OUT
// WEMOS_D5_OUT     --> SPI_SCLK
// WEMOS_D6_IN      --> SPI_MISO
// WEMOS_D7_OUT     --> SPI_MOSI
// WEMOS_D8_OUT     --> SPI_CS

/*******************************************************************************
 * INCLUSIONS
 ******************************************************************************/

#include <Arduino.h>
#include <limits.h>
#include "spi_wrapper.h"
#include "wifi_wrapper.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

//#define DEBUG   // Uncomment for debugging (serial output enabled)
//#define DEBUG2  // Uncomment for debugging (printing out from this file)
//#define DEBUG3  // Uncomment for debugging (emulation of spi data as all zeroes)

#define NB_PARAMS_2CAR   5
#define NB_PARAMS_2TLMT  10
#define WIFI_CHAR_PER_PARAM 5
#define WIFI_IN_LENGTH_CH   (NB_PARAMS_2CAR*WIFI_CHAR_PER_PARAM)
#define WIFI_OUT_LENGTH_CH  (NB_PARAMS_2TLMT*WIFI_CHAR_PER_PARAM)

#define max(a,b) \
   ({   __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b;      })

/*******************************************************************************
 * GLOBAL DATA
 ******************************************************************************/



/*******************************************************************************
 * FUNCTIONS PROTOTYPES
 ******************************************************************************/

static void decode_spi2wifi(uint16_t *mess_spi_in, char *mess_wifi_out);
static void decode_wifi2spi(char *mess_wifi_in, uint16_t *mess_spi_out);

/*******************************************************************************
 * MAIN FUNCTION
 * @brief  The application entry point.
 * @retval int
 ******************************************************************************/

/*****************************************
 * @brief initialises Wifi and SPI peripherals
 * @param None
 * @retval void
 ****************************************/
void setup() {
    // Serial port
    #if defined (DEBUG)
        Serial.begin(9600);   // Serial comm (for debug)
        Serial.println("\nWemos alive");
    #endif
    // Wifi
    myWifi_config();
    // SPI
    mySpi_config();
}

/*****************************************
 * @brief runs endlessly, sending data from SPI to Wifi and data from Wifi to SPI
 * @param None
 * @retval void
 ****************************************/
void loop() {
    // Data
    static uint16_t mess_spi_out[NB_PARAMS_2CAR];
    static uint16_t mess_spi_in[NB_PARAMS_2TLMT];
    static char mess_wifi_out[WIFI_OUT_LENGTH_CH];
    static char mess_wifi_in[WIFI_IN_LENGTH_CH];
    bool wifi_mess_in_received = false;
    bool spi_mess_in_received = false;
    unsigned int start_time_ms;
    unsigned int end_time_ms;
    const unsigned int minimum_time_ms = 50;
    const unsigned int minimum_time_ack_ms = 500;

    // Each iteration will take at least a specific threshold time to avoid
    // saturating spi
    start_time_ms = millis();

    // Reconnect Wifi and server if disconnected (blocking)
    myWifi_config();
    // Wifi in + decoding
    wifi_mess_in_received = myWifi_recv(mess_wifi_in, WIFI_IN_LENGTH_CH);
    if (wifi_mess_in_received == true) {
        decode_wifi2spi(mess_wifi_in, mess_spi_out);
    }
    // SPI out and in + decoding of spi in
    if (wifi_mess_in_received == true) {
        spi_mess_in_received = mySpi_transmit_uint16(
            mess_spi_out, NB_PARAMS_2CAR, mess_spi_in, NB_PARAMS_2TLMT);
    } else {
        spi_mess_in_received = mySpi_transmit_uint16(
            NULL, 0, mess_spi_in, NB_PARAMS_2TLMT);
    }

    #if defined(DEBUG3)
    spi_mess_in_received = true;
    memset(mess_spi_in, 0, NB_PARAMS_2TLMT*sizeof(uint16_t));
    #endif

    // Wifi out
    if (spi_mess_in_received == true) {
        decode_spi2wifi(mess_spi_in, mess_wifi_out);
        myWifi_send(mess_wifi_out, WIFI_OUT_LENGTH_CH);
    }

    // Assure minimum time duration of the iteration
    end_time_ms = millis();
    while ((end_time_ms - start_time_ms) <= minimum_time_ms) {
        end_time_ms = millis();
    }
    #if defined (DEBUG2)
    Serial.println(end_time_ms-start_time_ms);
    #endif
}

/*******************************************************************************
 * FUNCTIONS DEFINITIONS
 ******************************************************************************/

/*****************************************
 * @brief takes data received via spi (uint16) and converts it into the format
 * used by wifi out (array of 5 chars representing the uint16 value)
 * @param[in] mess_spi_in   Pointer to the beginning of the uint16 array
 * containing data received by spi
 * @param[out] mess_wifi_out Pointer to the char array where the decoded data 
 * will be stored for wifi
 * @retval None
 ****************************************/
static void decode_spi2wifi(uint16_t *mess_spi_in, char *mess_wifi_out) {
    #if defined (DEBUG2)
        Serial.print("\nSPI in decoded: ");
    #endif
    char format[5] = "%.5u";
    format[2] = (char)WIFI_CHAR_PER_PARAM + '0';
    for (unsigned int iter=0; iter<NB_PARAMS_2TLMT; iter++) {
        char *mess_ptr = mess_wifi_out + iter*WIFI_CHAR_PER_PARAM;
        snprintf(mess_ptr, WIFI_CHAR_PER_PARAM+1, format, mess_spi_in[iter]);
        #if defined (DEBUG2)
            char mess_debug[WIFI_CHAR_PER_PARAM+1];
            strncpy(mess_debug, mess_ptr, WIFI_CHAR_PER_PARAM);
            Serial.print(mess_debug);
            Serial.print(", ");
        #endif
    }
}

/*****************************************
 * @brief takes data received via wifi (array of 5 chars representing a uint16
 * value) and converts it into the format used by spi (uint16)
 * @param[in] mess_wifi_in   Pointer to the beginning of the char array where 
 * the wifi in data is stored
 * @param[out] mess_spi_out Pointer to the beginning of the uint16 array
 * where the decoded data will be stored for spi
 * @retval None
 ****************************************/
static void decode_wifi2spi(char *mess_wifi_in, uint16_t *mess_spi_out) {
    #if defined (DEBUG2)
        Serial.print("\nWifi in decoded: ");
        char format[5] = "%.5u";
        format[2] = (char)WIFI_CHAR_PER_PARAM + '0';
    #endif
    for (unsigned int iter=0; iter<NB_PARAMS_2CAR; iter++) {
        // Store next 5 (+ NULL) char digits in temp_ch
        char temp_ch[WIFI_CHAR_PER_PARAM+1];
        snprintf(temp_ch, WIFI_CHAR_PER_PARAM+1, "%s", 
            &(mess_wifi_in[iter*WIFI_CHAR_PER_PARAM]));
        // Convert value them into uint16_t type
        unsigned int temp_uint;
        sscanf(temp_ch, "%u", &temp_uint);
        mess_spi_out[iter] = (uint16_t) temp_uint;
        #if defined (DEBUG2)
            char mess_debug[WIFI_CHAR_PER_PARAM+1];
            snprintf(mess_debug, WIFI_CHAR_PER_PARAM+1, format, mess_spi_out[iter]);
            Serial.print(mess_debug);
            Serial.print(", ");
        #endif
    }
}
