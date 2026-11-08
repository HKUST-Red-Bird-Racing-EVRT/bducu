#include "CanExternalHelper.hpp"

CanExternalHelper::CanExternalHelper(MCP2515 &can_external_, AmsState &ams_, AmsHelper &ams_helper_)
    : can_external(can_external_), ams(ams_), ams_helper(ams_helper_)
{
}

void CanExternalHelper::waitHost()
{
    can_frame read_frame;

    while (!(can_external.readMessage(&read_frame) == MCP2515::ERROR_OK && read_frame.can_id == CAN_EXTERNAL_DETERMINE_HOST_ADDRESS))
    {
        // Process the response from the slave
        // For example, you can check the data in read_frame.data to determine if the host is VCU or CHARGER
        uint8_t host_status = read_frame.data[0]; // Assuming the first byte indicates the host status
        ams.host_vcu = (host_status == 0x01);     // Expected: 0x01 for VCU, 0x00 for CHARGER
    }
}

void CanExternalHelper::sendVCUData()
{
    // Create a CAN frame to send the host data to the slaves
    if (ams.fault_flags)
    {
        // If SDC fault is active, send a panic message to the slaves
        can_frame send_panic_frame = {
            CAN_EXTERNAL_PANIC_ADDRESS, // CAN ID for the panic message
            1,                          // Data length code (DLC)
            ams.fault_flags             // Panic data (can be customized as needed)
        };

        can_external.sendMessage(&send_panic_frame); // Send the CAN frame to the slave
    }
    
    can_frame send_vcu_frame = {
        CAN_EXTERNAL_VCU_ADDRESS, // CAN ID for the VCU data
        8,                        // Data length code (DLC)
        ams_helper.scaleSOCToVCU(ams.battery_soc), // Scaled SOC
        ams_helper.scaleACTSVoltageToVCU(ams.raw_ac_voltage), // Scaled AC voltage
        ams_helper.scaleACTSVoltageToVCU(ams.raw_tc_voltage), // Scaled TC voltage
        ams_helper.scaleHallSensorToVCU(ams.coulomb_count), // Scaled Hall sensor value
        ams_helper.scaleMaxMinVoltageToVCU(ams.voltage_max), // Scaled maximum voltage
        ams_helper.scaleMaxMinVoltageToVCU(ams.voltage_min), // Scaled minimum voltage
        ams_helper.scaleMaxMinTemperatureToVCU(ams.temperature_max), // Scaled maximum temperature
        ams_helper.scaleMaxMinTemperatureToVCU(ams.temperature_min)  // Scaled minimum temperature
    };

    can_external.sendMessage(&send_vcu_frame); // Send the CAN frame to the slave
}

void CanExternalHelper::sendChargerData()
{
    // Create a CAN frame to send the host data to the slaves
    if (ams.fault_flags)
    {
        // If SDC fault is active, send a panic message to the slaves
        can_frame send_panic_frame = {
            CAN_EXTERNAL_PANIC_ADDRESS, // CAN ID for the panic message
            1,                          // Data length code (DLC)
            ams.fault_flags             // Panic data (can be customized as needed)
        };

        can_external.sendMessage(&send_panic_frame); // Send the CAN frame to the slave
    }

    can_frame send_battery_frame = {
        CAN_EXTERNAL_CHARGER_BATTERY_STAT_ADDRESS, // CAN ID for the battery status
        5,                              // Data length code (DLC)
        ams.battery_soc,                // Battery SOC
        ams.voltage_min & 0xFF,          // Minimum voltage (low byte)
        (ams.voltage_min >> 8) & 0xFF,   // Minimum voltage (high byte)
        ams.voltage_max & 0xFF,          // Maximum voltage (low byte)
        (ams.voltage_max >> 8) & 0xFF   // Maximum voltage (high byte)
    };

    can_external.sendMessage(&send_battery_frame); // Send the CAN frame to the slave

    can_frame send_temperature_frame = {
        CAN_EXTERNAL_CHARGER_TEMPERATURE_STAT_ADDRESS, // CAN ID for the temperature status
        4,                              // Data length code (DLC)
        ams.temperature_min & 0xFF,      // Minimum temperature (low byte)
        (ams.temperature_min >> 8) & 0xFF, // Minimum temperature (high byte)
        ams.temperature_max & 0xFF,      // Maximum temperature (low byte)
        (ams.temperature_max >> 8) & 0xFF // Maximum temperature (high byte)
    };

    can_external.sendMessage(&send_temperature_frame); // Send the CAN frame to the slave
}

void CanExternalHelper::sendWaitingTsAct()
{
    can_frame send_waiting_frame = {
        CAN_EXTERNAL_WAITING_TS_ACT_ADDRESS, 
        1,                                   // Data length code (DLC)
        0x67                                 // placeholder
    };
    can_external.sendMessage(&send_waiting_frame);
}

void CanExternalHelper::sendState()
{
    can_frame send_state_frame = {
        CAN_EXTERNAL_STATE_ADDRESS,             
        1,                                      // Data length code (DLC)
        static_cast<uint8_t>(ams.air_state)     // AIR
    };
    can_external.sendMessage(&send_state_frame);
}

void CanExternalHelper::sendDebug()
{
    can_frame send_debug_frame = {
        CAN_EXTERNAL_DEBUG_ADDRESS,                 // CAN ID
        7,                                          // Data length code (DLC)
        ams.raw_ac_voltage & 0xFF,
        (ams.raw_ac_voltage >> 8) & 0xFF,
        ams.raw_tc_voltage & 0xFF,
        (ams.raw_tc_voltage >> 8) & 0xFF,
        ams.fault_flags,
        static_cast<uint8_t>(digitalRead(PIN_SDC) == SDC_CLOSED),
        static_cast<uint8_t>(ams.air_state)
    };
    can_external.sendMessage(&send_debug_frame);
}
