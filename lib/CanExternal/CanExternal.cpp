/**
 * @file CanExternal.cpp
 * @author Oscar (oscarckh0818@gmail.com),
 *         Chiho (nganchihon@gmail.com),
 *         Planeson (carson.cpk@proton.me)
 * @brief Implementation for the CanExternal class for handling external CAN communication.
 * @version 0.2
 * @date 2026-11-10
 * 
 * @copyright Copyright (c) 2026 Red Bird Racing
 * 
 */

#include "CanExternal.hpp"

CanExternal::CanExternal(MCP2515 &can_external_, AmsState &ams_state_, Ams &ams_)
    : can_external(can_external_), ams_state(ams_state_), ams(ams_)
{
}

void CanExternal::waitHost()
{
    can_frame read_frame;

    while (!(can_external.readMessage(&read_frame) == MCP2515::ERROR_OK && read_frame.can_id == CAN_EXTERNAL_DETERMINE_HOST_ADDRESS))
    {
        // Process the response from the slave
        // For example, you can check the data in read_frame.data to determine if the host is VCU or CHARGER
        ams_state.host_vcu = (read_frame.data[0]);     // Expected: 0x01 for VCU, 0x00 for CHARGER
    }
}

void CanExternal::sendVCUData()
{
    // Create a CAN frame to send the host data to the slaves
    if (ams_state.fault_flags)
    {
        // If SDC fault is active, send a panic message to the slaves
        can_frame send_panic_frame = {
            CAN_EXTERNAL_PANIC_ADDRESS, // CAN ID for the panic message
            1,                          // Data length code (DLC)
            ams_state.fault_flags             // Panic data (can be customized as needed)
        };

        can_external.sendMessage(&send_panic_frame); // Send the CAN frame to the slave
    }
    
    can_frame send_vcu_frame = {
        CAN_EXTERNAL_VCU_ADDRESS, // CAN ID for the VCU data
        8,                        // Data length code (DLC)
        ams.scaleSOCToVCU(ams_state.battery_soc), // Scaled SOC
        ams.scaleACTSVoltageToVCU(ams_state.raw_ac_voltage), // Scaled AC voltage
        ams.scaleACTSVoltageToVCU(ams_state.raw_tc_voltage), // Scaled TC voltage
        static_cast<uint8_t>(ams.scaleHallSensorToVCU(ams_state.coulomb_count)), // Scaled Hall sensor value
        ams.scaleMaxMinVoltageToVCU(ams_state.voltage_max), // Scaled maximum voltage
        ams.scaleMaxMinVoltageToVCU(ams_state.voltage_min), // Scaled minimum voltage
        ams.scaleMaxMinTemperatureToVCU(ams_state.temperature_max), // Scaled maximum temperature
        ams.scaleMaxMinTemperatureToVCU(ams_state.temperature_min)  // Scaled minimum temperature
    };

    can_external.sendMessage(&send_vcu_frame); // Send the CAN frame to the slave
}

void CanExternal::sendChargerData()
{
    // Create a CAN frame to send the host data to the slaves
    if (ams_state.fault_flags)
    {
        // If SDC fault is active, send a panic message to the slaves
        can_frame send_panic_frame = {
            CAN_EXTERNAL_PANIC_ADDRESS, // CAN ID for the panic message
            1,                          // Data length code (DLC)
            ams_state.fault_flags             // Panic data (can be customized as needed)
        };

        can_external.sendMessage(&send_panic_frame); // Send the CAN frame to the slave
    }

    can_frame send_battery_frame = {
        CAN_EXTERNAL_CHARGER_BATTERY_STAT_ADDRESS, // CAN ID for the battery status
        5,                              // Data length code (DLC)
        ams_state.battery_soc,                // Battery SOC
        static_cast<uint8_t>(ams_state.voltage_min & 0xFF),          // Minimum voltage (low byte)
        static_cast<uint8_t>((ams_state.voltage_min >> 8) & 0xFF),   // Minimum voltage (high byte)
        static_cast<uint8_t>(ams_state.voltage_max & 0xFF),          // Maximum voltage (low byte)
        static_cast<uint8_t>((ams_state.voltage_max >> 8) & 0xFF)   // Maximum voltage (high byte)
    };

    can_external.sendMessage(&send_battery_frame); // Send the CAN frame to the slave

    can_frame send_temperature_frame = {
        CAN_EXTERNAL_CHARGER_TEMPERATURE_STAT_ADDRESS, // CAN ID for the temperature status
        4,                              // Data length code (DLC)
        static_cast<uint8_t>(ams_state.temperature_min & 0xFF),      // Minimum temperature (low byte)
        static_cast<uint8_t>((ams_state.temperature_min >> 8) & 0xFF), // Minimum temperature (high byte)
        static_cast<uint8_t>(ams_state.temperature_max & 0xFF),      // Maximum temperature (low byte)
        static_cast<uint8_t>((ams_state.temperature_max >> 8) & 0xFF) // Maximum temperature (high byte)
    };

    can_external.sendMessage(&send_temperature_frame); // Send the CAN frame to the slave
}

void CanExternal::sendWaitingTsAct()
{
    can_frame send_waiting_frame = {
        CAN_EXTERNAL_WAITING_TS_ACT_ADDRESS, 
        1,                                   // Data length code (DLC)
        0x67                                 // placeholder
    };
    can_external.sendMessage(&send_waiting_frame);
}

void CanExternal::sendState()
{
    can_frame send_state_frame = {
        CAN_EXTERNAL_STATE_ADDRESS,             
        1,                                      // Data length code (DLC)
        static_cast<uint8_t>(ams_state.air_state)     // AIR
    };
    can_external.sendMessage(&send_state_frame);
}

void CanExternal::sendDebug()
{
    can_frame send_debug_frame = {
        CAN_EXTERNAL_DEBUG_ADDRESS,                 // CAN ID
        7,                                          // Data length code (DLC)
        static_cast<uint8_t>(ams_state.raw_ac_voltage & 0xFF),
        static_cast<uint8_t>((ams_state.raw_ac_voltage >> 8) & 0xFF),
        static_cast<uint8_t>(ams_state.raw_tc_voltage & 0xFF),
        static_cast<uint8_t>((ams_state.raw_tc_voltage >> 8) & 0xFF),
        ams_state.fault_flags,
        static_cast<uint8_t>(digitalRead(PIN_SDC) == SDC_CLOSED),
        static_cast<uint8_t>(ams_state.air_state)
    };
    can_external.sendMessage(&send_debug_frame);
}
