#include "CanInternalHelper.hpp"

CanInternalHelper::CanInternalHelper(MCP2515 &can_internal_, AmsState &ams_, AmsHelper &ams_helper_)
    : can_internal(can_internal_), ams(ams_), ams_helper(ams_helper_)
{
    // Constructor implementation
}

void CanInternalHelper::packingMaskCellBalState(uint16_t &flag)
{
    if (flag & CELLBAL_STATE_BIT)
    {
        ams.cellbal_active = true;

        if (ams.cellbal_odd)
        {
            flag &= CELLBAL_ODD_MASK; // Clear even bits, keep odd bits
        }
        else
        {
            flag &= CELLBAL_EVEN_MASK; // Clear odd bits, keep even bits
        }
    }
}

void CanInternalHelper::requestSlaveData(uint8_t frame_index)
{
    // Implementation of the requestSlaveData function
    // This function will send a request to the slave to send its data
    uint16_t address = CAN_INTERNAL_MASTER_REQUEST_ADDRESS + frame_index * 0x10; // Calculate the CAN ID for the specific slave

    can_frame request_frame = {
        address, // CAN ID for the slave
        1,       // Data length code (DLC)
        0x00     // Command byte to request data from the slave
    };

    can_internal.sendMessage(&request_frame); // Send the CAN frame to the slave
}

void CanInternalHelper::drainCanBuffer()
{
    // Implementation of the drainCanBuffer function
    // This function will clear the CAN buffer and reset the timestamps

    uint8_t expected_slave_index = 0;           // Expected slave index for the first frame
    can_frame buffer[NUM_SLAVE] = {0}; // Temporary buffer to store received frames

    uint32_t start = millis();
    while (digitalRead(PIN_INT_0) == HIGH)
    {
        if (millis() - start > CAN_TIMEOUT_MAX)
        {
            ams.fault_flags |= CAN_TIMEOUT_FAULT_BIT; // No slave replied
            return;
        }
    }

    while (digitalRead(PIN_INT_0) == LOW)
    {
        can_frame frame;
        if (can_internal.readMessage(&frame) == MCP2515::ERROR_OK)
        {
            // Read and discard messages until the buffer is empty
            if (frame.can_id >= CAN_INTERNAL_SLAVE_ADDRESS)
            {
                uint8_t buffer_index = frame.can_id - CAN_INTERNAL_SLAVE_ADDRESS; // Calculate the buffer index based on the CAN ID
                uint8_t slave_frame = buffer_index / 0x10;                   // Extract the slave frame index from the CAN ID
                uint8_t slave_index = buffer_index % 0x10;     // Extract the slave index from the CAN ID

                if (slave_index != expected_slave_index)    //  Check if the received frame is in the expected order, false means frame dropped
                {
                    ams.fault_flags |= CAN_TIMEOUT_FAULT_BIT;
                    continue;
                }

                buffer[slave_index] = frame; // Store the received frame in the temporary buffer
                ++expected_slave_index;

                if (expected_slave_index == NUM_SLAVE)
                {
                    for (uint8_t i = 0; i < NUM_SLAVE; ++i)
                    {
                        packSlaveData(buffer[i]); // Pack the received frames into the appropriate places
                    }
                }
            }
        }
    }
}

void CanInternalHelper::packSlaveData(can_frame &rx_frame)
{
    // Implementation of the packSlaveData function
    // This function will handle the packing of slave data into CAN frames
    // It will receive a CAN frame and store it in the appropriate buffer

    if (rx_frame.can_id >= CAN_INTERNAL_SLAVE_ADDRESS)
    {

        uint8_t buffer_index = rx_frame.can_id - CAN_INTERNAL_SLAVE_ADDRESS; // Calculate the buffer index based on the CAN ID
        uint8_t slave_frame = buffer_index / 0x10;                   // Extract the slave frame index from the CAN ID
        uint8_t slave_index = buffer_index % 0x10;                          // Extract the slave index from the CAN ID

        switch (slave_frame)
        {
        case 0x00:
            ams.command_flags[slave_index] = rx_frame.data[0];                                         // Assuming command_flags is a 8-bit value
            ams.cellbal_states[slave_index] = rx_frame.data[1] | (rx_frame.data[2] << 8); // Assuming cellbal_states is a 16-bit value
            break;

        case 0x01:
            ams.cell_voltages[slave_index][0] = rx_frame.data[0] | (rx_frame.data[1] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][1] = rx_frame.data[2] | (rx_frame.data[3] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][2] = rx_frame.data[4] | (rx_frame.data[5] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][3] = rx_frame.data[6] | (rx_frame.data[7] << 8); // Assuming cell_voltages is a 16-bit value
            break;

        case 0x02:
            ams.cell_voltages[slave_index][4] = rx_frame.data[0] | (rx_frame.data[1] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][5] = rx_frame.data[2] | (rx_frame.data[3] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][6] = rx_frame.data[4] | (rx_frame.data[5] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][7] = rx_frame.data[6] | (rx_frame.data[7] << 8); // Assuming cell_voltages is a 16-bit value
            break;

        case 0x03:
            ams.cell_voltages[slave_index][8] = rx_frame.data[0] | (rx_frame.data[1] << 8);  // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][9] = rx_frame.data[2] | (rx_frame.data[3] << 8);  // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][10] = rx_frame.data[4] | (rx_frame.data[5] << 8); // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][11] = rx_frame.data[6] | (rx_frame.data[7] << 8); // Assuming cell_voltages is a 16-bit value
            break;

        case 0x04:
            ams.cell_voltages[slave_index][12] = rx_frame.data[0] | (rx_frame.data[1] << 8);   // Assuming cell_voltages is a 16-bit value
            ams.cell_voltages[slave_index][13] = rx_frame.data[2] | (rx_frame.data[3] << 8);   // Assuming cell_voltages is a 16-bit value
            ams.ntc_temperatures[slave_index][0] = rx_frame.data[4] | (rx_frame.data[5] << 8); // Assuming ntc_temperatures is a 16-bit value
            ams.ntc_temperatures[slave_index][1] = rx_frame.data[6] | (rx_frame.data[7] << 8); // Assuming ntc_temperatures is a 16-bit valuE
            break;

        case 0x05:
            ams.ntc_temperatures[slave_index][2] = rx_frame.data[0] | (rx_frame.data[1] << 8); // Assuming ntc_temperatures is a 16-bit value
            ams.ntc_temperatures[slave_index][3] = rx_frame.data[2] | (rx_frame.data[3] << 8); // Assuming ntc_temperatures is a 16-bit value
            ams.ntc_temperatures[slave_index][4] = rx_frame.data[4] | (rx_frame.data[5] << 8); // Assuming ntc_temperatures is a 16-bit value
            break;

        default:
            break;
        }
    }

    // Handle panic messages from slaves
    else if (rx_frame.can_id < CAN_INTERNAL_MASTER_COMMAND_ADDRESS && rx_frame.can_id >= CAN_INTERNAL_PANIC_ADDRESS)
    {
        uint8_t buffer_index = rx_frame.can_id - CAN_INTERNAL_PANIC_ADDRESS; // Calculate the buffer index based on the CAN ID
        uint8_t slave_index = buffer_index % 0x10;     // Extract the slave index from the CAN ID
        ams.fault_active = true;
        ams.fault_slave = slave_index;
        ams.fault_flags |= rx_frame.data[0]; // Assuming fault_flags is a 8-bit value
    }
}

/*
bool CanHelper::isCommunicationTimeoutOld()
{
    uint32_t current_time = millis();
    for (uint8_t buffer_index = 0; buffer_index < NUM_SLAVE * NUM_SLAVE_FRAME; ++buffer_index)
    {
        if (current_time - rx_slave_buffer[buffer_index].timestamp > CAN_TIMEOUT_MAX)
        {
            uint8_t slave_index = buffer_index / NUM_SLAVE_FRAME; // Extract the slave index from the CAN ID
            ams.fault_active = true;
            ams.fault_slave = slave_index;
            ams.fault_flags |= CAN_TIMEOUT_FAULT_BIT;
            return true; // Communication timeout detected
        }
    }
    return false; // No communication timeout
}
*/

bool CanInternalHelper::isDelayedFrame(can_frame &frame)
{
    uint8_t buffer_index = frame.can_id - CAN_INTERNAL_SLAVE_ADDRESS;            // Calculate the buffer index based on the CAN ID
    uint8_t slave_frame = buffer_index / 0x10;                              // Extract the slave frame index from the CAN ID
    uint8_t slave_index = buffer_index % 0x10;                // Extract the slave index from the CAN ID
    uint8_t frame_masked = frame.data[0] & SEQUENCE_TOGGLE_BIT;             // Mask the command byte to check the sequence toggle bit
    uint8_t command_flags_masked = ams.command_flags[slave_index] & SEQUENCE_TOGGLE_BIT; // Mask the command byte to check the sequence toggle bit
    return frame_masked != command_flags_masked;                            // Check if the sequence toggle bit matches
}

void CanInternalHelper::sendSlaveRequest(uint8_t index)
{
    // Implementation of the sendSlaveData function
    // This function will handle sending data to the slaves via CAN

    uint16_t address = CAN_INTERNAL_MASTER_REQUEST_ADDRESS + index; // Calculate the CAN ID for the specific slave

    can_frame send_frame = {
        address, // CAN ID for the slave
        1,       // Data length code (DLC)
        0        // Reserved bytes
    };

    can_internal.sendMessage(&send_frame); // Send the CAN frame to the slave
}

void CanInternalHelper::sendSlaveCommand(uint8_t index)
{
    // Implementation of the sendSlaveData function
    // This function will handle sending data to the slaves via CAN

    uint16_t address = CAN_INTERNAL_MASTER_COMMAND_ADDRESS + index; // Calculate the CAN ID for the specific slave

    can_frame send_frame = {
        address, // CAN ID for the slave
        3,       // Data length code (DLC)
        ams.command_flags[index], // Command flags for the specific slave
        ams.voltage_min & 0xFF, // Lower byte of voltage_min
        (ams.voltage_min >> 8) & 0xFF, // Upper byte of voltage_min
    };

    can_internal.sendMessage(&send_frame); // Send the CAN frame to the slave
}