/**
 * @file no_slave_test.cpp
 * @brief Testing without AMS slaves: same as main.cpp but slave polling removed, so the AIR / precharge logic can run
 * DANGER: no cell voltage / temperature monitoring at all, only for testing with a power supply
 * flash with: pio run -e no_slave_test -t upload
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

#warning "no_slave_test: no cell monitoring"

MCP2515 can_internal(PIN_CAN1_CS);
MCP2515 can_external(PIN_CAN0_CS);

AmsState ams;
AmsHelper ams_helper(ams);
CanExternalHelper can_external_helper(can_external, ams, ams_helper);

void setup()
{
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
  pinMode(TS_ACT, INPUT);

  ams_helper.setAIR(AIRState::INIT); // Set initial AIR state to INIT
  can_external_helper.setHost(); // Determine if the host is VCU or CHARGER
  uint8_t wait_count = 0;
  while (!digitalRead(TS_ACT))
  {
    if (wait_count == 0)
    {
      can_external_helper.sendWaitingTsAct();
    }
    wait_count = (wait_count + 1) % 10;
    // Wait for the TS_ACT pin to go HIGH before proceeding
    delay(10); // Small delay to avoid busy waiting
  }
}

void loop()
{
  // put your main code here, to run repeatedly:

  static uint32_t air_state_millis = 0;
  if (millis() - air_state_millis >= AIR_STATE_PERIOD)
  {
    can_external_helper.sendState(); // send AIR
    can_external_helper.sendDebug(); // send voltages, faults

    air_state_millis = millis();
  }

  ams_helper.updateAIR(); // Update the AIR state based on conditions

  if (ams.fault_flags)
  {
    digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin high if any fault is active
  }

  ams_helper.updateBatterySOC(analogRead(PIN_HALL_LO), analogRead(PIN_HALL_HI));

  if (millis() - ams.timestamp >= 1000) // Send data to VCU every 1 second
  {
    can_external_helper.sendHostData();
    ams.timestamp = millis(); // Update the timestamp after sending data
  }
}
