#include "CanExternalHelper.hpp"

CanExternalHelper::CanExternalHelper(MCP2515 &can_external_, AmsState &ams_, AmsHelper &ams_helper_)
    : can_external(can_external_), ams(ams_), ams_helper(ams_helper_)
{
}

void CanExternalHelper::setHost()
{
    can_frame read_frame;

    // Create a CAN frame to send the host status to the slaves
    can_frame send_frame = {
        CAN_EXTERNAL_DETERMINE_HOST_ADDRESS, // CAN ID for the host
        1,                              // Data length code (DLC)
        0x00                            // Reserved bytes (can be used for additional data if needed)
    };

    can_external.sendMessage(&send_frame); // Send the CAN frame to the slave

    uint32_t start = millis();
    while (digitalRead(PIN_INT_1) == HIGH)
    {
        if (millis() - start > CAN_HOST_TIMEOUT)
        {
            return;
        }
    }

    if (can_external.readMessage(&read_frame) == MCP2515::ERROR_OK && read_frame.can_id == CAN_EXTERNAL_DETERMINE_HOST_ADDRESS)
    {
        // Process the response from the slave
        // For example, you can check the data in read_frame.data to determine if the host is VCU or CHARGER
        uint8_t host_status = read_frame.data[0]; // Assuming the first byte indicates the host status
        ams.host_vcu = (host_status == 0x01);     // Expected: 0x01 for VCU, 0x00 for CHARGER
    }
}

void CanExternalHelper::sendHostData()
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
        CAN_EXTERNAL_BATTERY_STAT_ADDRESS, // CAN ID for the battery status
        5,                              // Data length code (DLC)
        ams.battery_soc,                // Battery SOC
        ams.voltage_min & 0xFF,          // Minimum voltage (low byte)
        (ams.voltage_min >> 8) & 0xFF,   // Minimum voltage (high byte)
        ams.voltage_max & 0xFF,          // Maximum voltage (low byte)
        (ams.voltage_max >> 8) & 0xFF   // Maximum voltage (high byte)
    };

    can_external.sendMessage(&send_battery_frame); // Send the CAN frame to the slave

    can_frame send_temperature_frame = {
        CAN_EXTERNAL_TEMPERATURE_STAT_ADDRESS, // CAN ID for the temperature status
        4,                              // Data length code (DLC)
        ams.temperature_min & 0xFF,      // Minimum temperature (low byte)
        (ams.temperature_min >> 8) & 0xFF, // Minimum temperature (high byte)
        ams.temperature_max & 0xFF,      // Maximum temperature (low byte)
        (ams.temperature_max >> 8) & 0xFF // Maximum temperature (high byte)
    };

    can_external.sendMessage(&send_temperature_frame); // Send the CAN frame to the slave
}
