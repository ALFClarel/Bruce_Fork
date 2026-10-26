#pragma once
// Forced into ALL translation units (including libraries and the JS ROM generator).
#ifdef BRUCE_TEMBED_BASE
#define BRUCE_WIFI_24GHZ_ONLY
#define BRUCE_NO_LORA
#ifndef BRUCE_TEMBED_PLUS
#define BRUCE_NO_NRF24
#endif
#define BRUCE_NO_GPS
#define BRUCE_NO_FM
#define BRUCE_NO_LF_RFID
#define BRUCE_NO_IBUTTON
#define BRUCE_NO_ETHERNET
#define BRUCE_PN532_ONLY
#define BRUCE_NO_EXTERNAL_NFC
#define BRUCE_NO_I2C_DISCOVERY
#endif
