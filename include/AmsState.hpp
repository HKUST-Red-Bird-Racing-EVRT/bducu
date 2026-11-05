#ifndef AMSSTATE_HPP
#define AMSSTATE_HPP

#include <stdint.h>

#define NUM_SLAVE 1
#define NUM_VCELL 14
#define NUM_NTC 5

enum class AIRState
{
    INIT = 0X00,
    PRECHARGE = 0X01,
    ACTIVE = 0X02,
    DISCHARGE = 0X03
};

struct AmsState
{
    bool cellbal_active = false;    // CellBal ON/OFF
    bool cellbal_odd = false;       // CellBal Odd/Even (true = odd, false = even)
    bool discharge_active = false;  // Discharge ON/OFF
    bool fault_active = false;      // Fault TRUE/FALSE
    bool sdc_active = true;          // SDC TRUE/FALSE
    bool host_vcu;                  // Host VCU/CHARGER
    AIRState air_state = AIRState::INIT; // AIR State (INIT, ACTIVE, SHUTDOWN)
    uint32_t timestamp; // Timestamp of the last update in milliseconds
    uint16_t cell_voltages[NUM_SLAVE][NUM_VCELL];
    uint16_t voltage_min; // Minimum voltage in millivolts
    uint16_t voltage_max; // Maximum voltage in millivolts
    uint8_t battery_soc; // State of Charge (SOC) in percentage (0-100)
    int8_t coulomb_count; // Coulomb count in milliampere-hours (mAh)
    uint16_t ntc_temperatures[NUM_SLAVE][NUM_NTC];
    uint16_t temperature_min; 
    uint16_t temperature_max;
    uint16_t cellbal_states[NUM_SLAVE];
    uint8_t fault_slave;
    uint8_t fault_flags;
    uint8_t command_flags[NUM_SLAVE];

};

/*
Command Flag Bits
Bit 0: discharge_state
Bit 1: cellbal_state
Bit 2 - 15: cellbal_flags for cell 0 - 13 (1 = balancing, 0 = not balancing)
*/

#define DISCHARGE_STATE_BIT         0x01
#define CELLBAL_STATE_BIT           0x02
#define CELLBAL_ODD_BIT             0x04
#define SEQUENCE_TOGGLE_BIT         0x08

//  Cell Balancing Flag Masks
#define CELLBAL_EVEN_MASK           0x1555 // 0001 0101 0101 0101
#define CELLBAL_ODD_MASK            0x4AAA // 0100 1010 1010 1010

//  Fault Flag Bits
#define OVERVOLTAGE_FAULT_BIT       0x01
#define UNDERVOLTAGE_FAULT_BIT      0x02
#define OVERTEMPERATURE_FAULT_BIT   0x04
#define UNDERTEMPERATURE_FAULT_BIT  0x08
#define I2C_TIMEOUT_FAULT_BIT       0x10
#define CAN_TIMEOUT_FAULT_BIT       0x20
#define PRECHARGE_TIMEOUT_FAULT_BIT 0x40
#define SDC_FAULT_BIT               0x80

#endif // AMSSTATE_HPP