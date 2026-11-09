/**
 * @file main.cpp
 * @brief Main entry point for the AMS Master Controller
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
can_frame tx_temp_frame;

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

  pinMode(PIN_HALL_HI, INPUT);
  pinMode(PIN_HALL_LO, INPUT);
  pinMode(PIN_AIR_NEG, OUTPUT);
  pinMode(PIN_AIR_POS, OUTPUT);
  pinMode(PIN_AIR_PRE, OUTPUT);
  pinMode(PIN_AMS_ERR, OUTPUT);
  pinMode(PIN_SDC, INPUT);

  digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin low initially
  can_external_helper.sendDebug(); // Send initial debug information to the host (VCU or CHARGER)

  ams_helper.setAIR(AIRState::INIT); // Set initial AIR state to PRECHARGE
  can_external_helper.waitHost();     // Determine if the host is VCU or CHARGER
  ams_helper.setAIR(AIRState::PRECHARGE); // Set initial AIR state to PRECHARGE
  
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

  ams.raw_ac_voltage = analogRead(PIN_AC_ISO); // Read raw AC voltage
  ams.raw_tc_voltage = analogRead(PIN_TC_ISO); // Read raw TC voltage

  can_external_helper.sendState(); // Send the current state to the host (VCU or CHARGER)
  can_external_helper.sendDebug(); // Send debug information to the host (VCU or CHARGER)

  ams_helper.updateAIR(); // Update the AIR state based on conditions

  /*
  for (uint8_t frame_index = 0; frame_index < NUM_SLAVE_FRAME; ++frame_index)
  {
    // if frame dropped, keep looping for 3-4 times to request the slave data again, and drain the buffer
    uint8_t retry_count = 0;
    while (retry_count < 4)
    {
      ams.fault_flags &= ~CAN_TIMEOUT_FAULT_BIT; // Clear the CAN timeout fault flag if no timeout occurred
      can_internal_helper.requestSlaveData(frame_index);
      can_internal_helper.drainCanBuffer();
      if (ams.fault_flags & CAN_TIMEOUT_FAULT_BIT)
      {
        retry_count++;
      }
      else
      {
        break;
      }
    }
  }

  if (ams.fault_flags)
  {
    digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin high if any fault is active
  }
  /*else
  {
    digitalWrite(PIN_AMS_ERR, LOW); // Set AMS_ERR pin low if no fault is active
  }
  

  // Normal operation, e.g., send data to VCU, perform other tasks, etc.
  ams_helper.updateMaxMinCellVoltages();
  ams_helper.updateMaxMinTemperatures();
  ams_helper.updateBatterySOC(analogRead(PIN_HALL_LO), analogRead(PIN_HALL_HI));
  */

  if (!ams.host_vcu) // If the host is a charger, send charger data
  {
    can_external_helper.sendChargerData();
  }

  else if (millis() - ams.timestamp >= 1000) // Send data to VCU every 1 second
  {
    can_external_helper.sendVCUData();
    ams.timestamp = millis(); // Update the timestamp after sending data
  }
}
