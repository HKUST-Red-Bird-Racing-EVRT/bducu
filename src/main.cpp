/**
 * @file main.cpp
 * @author Oscar (oscarckh0818@gmail.com),
 *         Chiho (nganchihon@gmail.com),
 *         Planeson (carson.cpk@proton.me)
 * @brief Main entry point for the Battery Disconnect Unit Control Unit (BDUCU) firmware.
 * @version 0.2
 * @date 2026-11-10
 *
 * @copyright Copyright (c) 2026 Red Bird Racing
 *
 */

#include <Arduino.h>

// ignore -Wpedantic warnings for mcp2515.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <mcp2515.h>
#pragma GCC diagnostic pop

#include "BoardConfig.h"
#include "AmsState.hpp"
#include "Ams.hpp"
#include "CanInternal.hpp"
#include "CanExternal.hpp"

MCP2515 can_internal(PIN_CAN1_CS);
MCP2515 can_external(PIN_CAN0_CS);
can_frame rx_slave_frame;
can_frame tx_temp_frame;

AmsState ams_state;
Ams ams(ams_state);
CanInternal can_internal_helper(can_internal, ams_state, ams);
CanExternal can_external_helper(can_external, ams_state, ams);

void setup()
{
  // put your setup code here, to run once:

  can_internal.reset();
  can_internal.setBitrate(CAN_500KBPS, MCP_20MHZ);
  can_internal.setNormalMode();

  can_external.reset();
  can_external.setBitrate(CAN_250KBPS, MCP_20MHZ);
  can_external.setNormalMode();

  pinMode(PIN_HALL_HI, INPUT);
  pinMode(PIN_HALL_LO, INPUT);
  pinMode(PIN_AIR_NEG, OUTPUT);
  pinMode(PIN_AIR_POS, OUTPUT);
  pinMode(PIN_AIR_PRE, OUTPUT);
  pinMode(PIN_AMS_ERR, OUTPUT);
  pinMode(PIN_SDC, INPUT);

  digitalWrite(PIN_AMS_ERR, HIGH); // Active LOW, set AMS_ERR pin HIGH initially
  can_external_helper.sendDebug(); // Send initial debug information to the host (VCU or CHARGER)

  ams.setAIR(AIRState::INIT);      // Set initial AIR state to PRECHARGE
  //can_external_helper.waitHost();  // Determine if the host is VCU or CHARGER

  can_frame charger_frame = {
    0x18FF50E5 | CAN_EFF_FLAG, // Extended ID for charger
    8,                         // Data length
    0x88,
    0x08,
    0x0A,
    0x00,
    0x01,
    0,0,0
  };

  can_external.sendMessage(&charger_frame);

  ams.setAIR(AIRState::PRECHARGE); // Set initial AIR state to PRECHARGE
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

  ams_state.raw_ac_voltage = analogRead(PIN_AC_ISO); // Read raw AC voltage
  ams_state.raw_tc_voltage = analogRead(PIN_TC_ISO); // Read raw TC voltage

  uint16_t current = analogRead(PIN_HALL_HI);
  can_frame hall_frame = {
      0x699,
      2,
      current & 0xFF,
      (current >> 8) & 0xFF};

  can_external.sendMessage(&hall_frame);

  if (false && !ams_state.host_vcu)
  {
    can_external_helper.readBalanceAllow();
  }
  else
  {
    ams_state.balance_allowed = false;
  }

  can_external_helper.sendState(); // Send the current state to the host (VCU or CHARGER)
  can_external_helper.sendDebug(); // Send debug information to the host (VCU or CHARGER)

  ams.updateAIR(); // Update the AIR state based on conditions

  // for (uint8_t frame_index = 0; frame_index < NUM_SLAVE_FRAME; ++frame_index)
  // {
  //   ams_state.fault_flags &= ~CAN_TIMEOUT_FAULT_BIT; // Clear the CAN timeout fault flag if no timeout occurred
  //   can_internal_helper.requestSlaveData(frame_index);
  //   can_internal_helper.drainCanBuffer();
  // }

  can_frame rx_slave = {0};
  while (can_internal.readMessage(&rx_slave) == MCP2515::ERROR_OK)
  {
    if (rx_slave.can_id == 0x100 && (rx_slave.data[0] & 0b1100))
    {
      digitalWrite(PIN_AMS_ERR, LOW);
    }
  }

  // if (ams_state.fault_flags)
  // {
  //   digitalWrite(PIN_AMS_ERR, LOW);
  // }
  // /*else
  // {
  //   digitalWrite(PIN_AMS_ERR, HIGH);
  // }
  // */

  // for (uint8_t slave_index = 0; slave_index < NUM_SLAVE; ++slave_index)
  // {
  //   can_external_helper.sendChargerIndividualData();
  // }

  // // Normal operation, e.g., send data to VCU, perform other tasks, etc.
  // ams.updateMaxMinCellVoltages();
  // ams.updateMaxMinTemperatures();
  // ams.updateBatterySOC(analogRead(PIN_HALL_LO), analogRead(PIN_HALL_HI));

  // // Command Slaves
  // for (uint8_t slave_index = 0; slave_index < NUM_SLAVE; ++slave_index)
  // {
  //   can_internal_helper.sendSlaveCommand(slave_index);
  // }

  // if (!ams_state.host_vcu) // If the host is a charger, send charger data
  // {
  //   can_external_helper.sendChargerOverallData();
  // }

  // else if (millis() - ams_state.ams_data_timestamp >= 100)
  // {
  //   can_external_helper.sendVCUAMSData();
  //   ams_state.ams_data_timestamp = millis(); // Update the timestamp after sending data
  // }

  // else if (millis() - ams_state.bducu_timestamp >= 10)
  // {
  //   can_external_helper.sendVCUBDUCUData();
  //   ams_state.bducu_timestamp = millis(); // Update the timestamp after sending data
  // }
}
