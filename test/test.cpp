/**
 * @file test.cpp
 * @brief tesing code
 */

#include <Arduino.h>

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

#include "BoardConfig.h"
#include "AmsState.hpp"
#include "AmsHelper.hpp"
#include "CanInternalHelper.hpp"
#include "CanExternalHelper.hpp"

MCP2515 can_internal(PIN_CAN1_CS);
MCP2515 can_external(PIN_CAN0_CS);
can_frame rx_slave_frame;

AmsState ams;
AmsHelper ams_helper(ams);
CanInternalHelper can_internal_helper(can_internal, ams, ams_helper);
CanExternalHelper can_external_helper(can_external, ams, ams_helper);

void setup()
{
  // put your setup code here, to run once:

  can_internal.reset();
  can_internal.setBitrate(CAN_500KBPS, MCP_20MHZ);
  can_internal.setNormalMode();

  can_external.reset();
  can_external.setBitrate(CAN_500KBPS, MCP_20MHZ);
  can_external.setNormalMode();

  pinMode(PIN_CAN0_CS, OUTPUT);
  pinMode(PIN_CAN1_CS, OUTPUT);
  pinMode(PIN_INT_0, INPUT);
  pinMode(PIN_INT_1, INPUT);
  pinMode(PIN_HALL_LO, INPUT);
  pinMode(PIN_HALL_HI, INPUT);
  pinMode(PIN_AIR_NEG, OUTPUT);
  pinMode(PIN_AIR_POS, OUTPUT);
  pinMode(PIN_AIR_PRE, OUTPUT);
  pinMode(PIN_AMS_ERR, OUTPUT);
  pinMode(PIN_SDC, INPUT);

  ams_helper.setAIR(AIRState::INIT); // Set initial AIR state to PRECHARGE
}

void loop()
{
  // put your main code here, to run repeatedly:

  /*
  Later Implementation:
  - Master send command byte to slave
  - Before receiving data from slave, handle cell balancing odd/even states
  - Put recieved data into buffer, and update timestamp
  - If timeout, set fault flag and fault slave
  - If no timeout, communicate with vcu
  */

  ams_helper.updateAIR(); // Update the AIR state based on conditions
  

  if (ams.fault_flags)
  {
    digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin high if any fault is active
  }
}
