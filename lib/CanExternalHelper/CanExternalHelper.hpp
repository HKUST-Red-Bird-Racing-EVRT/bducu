#ifndef CAN_EXTERNAL_HELPER_HPP
#define CAN_EXTERNAL_HELPER_HPP

#include "AmsState.hpp"
#include "AmsHelper.hpp"
#include "BoardConfig.h"

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

//  MCP2515 CAN Address
/*
Panic Address: 0x100 (to be determined)
Host Request Address: 0x250 (to be determined)
Battery Stat Address: 0x400
Temperature Stat Address: 0x410
Determine Host Address (Determine host is VCU/CHARGER): 0x500
*/

#define CAN_EXTERNAL_PANIC_ADDRESS 0x100
#define CAN_EXTERNAL_VCU_ADDRESS 0x300
#define CAN_EXTERNAL_CHARGER_BATTERY_STAT_ADDRESS 0x400
#define CAN_EXTERNAL_CHARGER_TEMPERATURE_STAT_ADDRESS 0x410
#define CAN_EXTERNAL_DETERMINE_HOST_ADDRESS 0x500
#define CAN_EXTERNAL_WAITING_TS_ACT_ADDRESS 0x690
#define CAN_EXTERNAL_STATE_ADDRESS 0x691
#define CAN_EXTERNAL_DEBUG_ADDRESS 0x692

#define CAN_HOST_TIMEOUT 500 // ms

class CanExternalHelper
{
public:
    CanExternalHelper(MCP2515 &can_external_, AmsState &ams_, AmsHelper &ams_helper_);
    CanExternalHelper() = delete; // Delete the default constructor to prevent its use
    void waitHost();
    void sendVCUData();
    void sendChargerData();
    void sendWaitingTsAct();
    void sendState();
    void sendDebug();

private:
    MCP2515 &can_external;
    AmsState &ams;
    AmsHelper &ams_helper;
};

#endif // CAN_EXTERNAL_HELPER_HPP