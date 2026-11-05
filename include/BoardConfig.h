#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#define PIN_CAN0_CS PIN_PD7
#define PIN_CAN1_CS PIN_PD6

// CAN Pins
#define PIN_INT_0 PIN_PD2
#define PIN_INT_1 PIN_PD3

// Current Sensor Pins
#define PIN_HALL_LO PIN_PC2
#define PIN_HALL_HI PIN_PC3

// AIR Pins
#define PIN_AIR_NEG PIN_PD1
#define PIN_AIR_POS PIN_PD4
#define PIN_AIR_PRE PIN_PD5

// Error Pin
#define PIN_AMS_ERR PIN_PD0
#define PIN_SDC PIN_PB1

#define PIN_AC_ISO PIN_PC0
#define PIN_TC_ISO PIN_PC1

// Constant Definitions
#define SHUTDOWN_VOLTAGE_THRESHOLD 90
#define TS_AC_RATIO 19/20
#define PRECHARGE_TIMEOUT 500 // ms
#define VOLTAGE_UV 3500 // Undervoltage threshold in mV

#endif // BOARDCONFIG_H