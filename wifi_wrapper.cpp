/******************************************************************************
 * Project: Robocar
 * Application: Wemos_Wifi_SPI
 * Platform: Wemos D1 mini lite
 * @file wifi_wrapper.c
 * @version v1.0
 * @brief Robocar_wemos wrapper for wifi communication. 
 ******************************************************************************/

/*******************************************************************************
 * INCLUSIONS
 ******************************************************************************/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wifi_wrapper.h"

/*******************************************************************************
 * DEFINES
 ******************************************************************************/

#define DEBUG   // Uncomment for debugging (serial output)

/*******************************************************************************
 * GLOBAL DATA
 ******************************************************************************/

// Wifi and server
char ssid[] = "my_ssid";
char pass[] = "my_pass";
WiFiClient clientEsp;
byte pyServer_IP[] = {192,168,0,1};
uint16_t pyServer_port = 60000;

/*******************************************************************************
 * FUNCTIONS PROTOTYPES
 ******************************************************************************/


/*******************************************************************************
 * FUNCTIONS DEFINITIONS
 ******************************************************************************/

/*****************************************
 * @brief connects to Wifi network and to server. Gets blocked until successful
 * connection
 * @param None
 * @retval None
 ****************************************/
void myWifi_config(void) {
    // Connect to Wifi
    if (WiFi.status() != WL_CONNECTED) {
        // Init Wifi
        WiFi.begin(ssid, pass);

        // Check Wifi status
        #if defined (DEBUG)
            Serial.print("\nWifi. Connecting..");
        #endif
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(500);
        }
        #if defined (DEBUG)
            Serial.print("\nConnected to Wifi, IP address: ");
            Serial.println(WiFi.localIP());
        #endif
    }
    // Connect to server
    if (clientEsp.connected() == false) {
            #if defined (DEBUG)
                Serial.print("\nServer. Connecting..");
            #endif
        while (clientEsp.connect(pyServer_IP, pyServer_port) == false) {
            Serial.print(".");
            delay(500);
        }
        clientEsp.setTimeout(100);   // timeout in ms for receiving data
        #if defined (DEBUG)
            Serial.println("\nConnected to server");
        #endif
    }
}

/*****************************************
 * @brief send message via Wifi consisting of several chars
 * @param[in] message_ptr: pointer to message to be sent
 * @param[in] message_length: number of elements in message
 * @retval None
 ****************************************/
void myWifi_send(char *message_ptr, unsigned int message_length) {
    if (clientEsp.connected()) {
        clientEsp.write(message_ptr,message_length);
        #if defined (DEBUG)
            char message_debug[message_length+1];
            snprintf(message_debug, message_length+1, message_ptr);
            Serial.print("\nWifi out: ");
            Serial.print(message_debug);
        #endif
    }
}

/*****************************************
 * @brief receives a new message from Wifi
 * @param[out] message_ptr  pointer to memory where message will be stored
 * @param[in] message_length    number of char elements to read from Wifi
 * @retval mess_in_received is true if there is any new incoming message
 ****************************************/
bool myWifi_recv(char *message_ptr, unsigned int message_length) {
    char *response_str = NULL;
    bool mess_in_received = false;

    if (clientEsp.available()) {
        byte response[message_length] = {};
        size_t mess_in_length = clientEsp.readBytes(response, message_length);
        if (mess_in_length == message_length) {
            mess_in_received = true;
        }
        response_str = (char *) response;
        snprintf(message_ptr, message_length+1, "%s", response_str); 
        #if defined (DEBUG)
            Serial.print("\nWifi in: ");
            Serial.print(response_str);
        #endif
    }

    return mess_in_received;
}
