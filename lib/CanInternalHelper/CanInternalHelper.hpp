#ifndef CAN_INTERNAL_HELPER_HPP
#define CAN_INTERNAL_HELPER_HPP

#include "AmsState.hpp"
#include "AmsHelper.hpp"
#include "BoardConfig.h"

#include <stdint.h>
#include <Arduino.h>

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

//  MCP2515 CAN Address
/*
Panic Address: 0x1xx
slave 0: 0x100, slave 1: 0x101, slave 2: 0x102, ..., slave 9: 0x109

Master Address: 0x2xx
slave 0: 0x200, slave 1: 0x201, slave 2: 0x202, ..., slave 9: 0x209 (send command data to slaves)
frame 0: 0x210, frame 1: 0x211, frame 2: 0x212, ..., frame 5: 0x215 (request data from slave)
(send 1 frame of each slaves together, 6 frames total)

Slave Address: 0x3xx
slave 0: 0x3x0, slave 1: 0x3x1, slave 2: 0x3x2, ..., slave 9: 0x3x9
frame 0: 0x30x, frame 1: 0x31x, frame 2: 0x32x, ..., frame 5: 0x35x
*/

#define CAN_INTERNAL_PANIC_ADDRESS 0x100
#define CAN_INTERNAL_MASTER_COMMAND_ADDRESS 0x200
#define CAN_INTERNAL_MASTER_REQUEST_ADDRESS 0x210
#define CAN_INTERNAL_SLAVE_ADDRESS 0x300

#define NUM_SLAVE 10
#define NUM_SLAVE_FRAME 6

#define CAN_TIMEOUT_MAX 500

class CanInternalHelper
{
public:
    CanInternalHelper(MCP2515 &can_internal_, AmsState &ams_, AmsHelper &ams_helper_);
    CanInternalHelper() = delete; // Delete the default constructor to prevent its use
    void packingMaskCellBalState(uint16_t &flag);
    void requestSlaveData(uint8_t frame_index);
    void drainCanBuffer();
    void packSlaveData(can_frame &rx_frame);
    // bool isCommunicationTimeoutOld();
    bool isDelayedFrame(can_frame &frame);
    void sendSlaveRequest(uint8_t slave_index);
    void sendSlaveCommand(uint8_t slave_index);

private:
    MCP2515 &can_internal;
    AmsState &ams;
    AmsHelper &ams_helper;
};

//  Master Command Byte bits
#define MASTERCMD_DISCHARGE_STATE_BIT 0x01
#define MASTERCMD_CELLBAL_STATE_BIT 0x02
#define MASTERCMD_CELLBAL_ODD_BIT 0x04
#define MASTERCMD_SEQTOGGLE_BIT 0x08

#endif // CAN_INTERNAL_HELPER_HPP