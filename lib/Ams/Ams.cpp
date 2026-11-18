/**
 * @file Ams.cpp
 * @author Oscar (oscarckh0818@gmail.com),
 *         Chiho (nganchihon@gmail.com),
 *         Planeson (carson.cpk@proton.me)
 * @brief Implementation for the AMS (Accumulator Management System) class.
 * @version 0.2
 * @date 2026-11-10
 * 
 * @copyright Copyright (c) 2026 Red Bird Racing
 * 
 */

#include "Ams.hpp"

Ams::Ams(AmsState &ams_state_)
    : ams_state(ams_state_)
{
    // Constructor implementation
}

void Ams::resetCellBal()
{
    ams_state.cellbal_active = false;
    ams_state.cellbal_odd = false;
    ams_state.voltage_min = DEFAULT_VOLTAGE_MIN;
    ams_state.voltage_max = DEFAULT_VOLTAGE_MAX;
}

void Ams::updateMaxMinCellVoltages()
{
    uint16_t voltage_min = UINT16_MAX;
    uint16_t voltage_max = 0;
    for (uint8_t slave_index = 0; slave_index < NUM_SLAVE; ++slave_index)
    {

        for (uint8_t cell_index = 0; cell_index < NUM_VCELL; ++cell_index)
        {
            if (ams_state.cell_voltages[slave_index][cell_index] < voltage_min)
            {
                voltage_min = ams_state.cell_voltages[slave_index][cell_index];
            }
            if (ams_state.cell_voltages[slave_index][cell_index] > voltage_max)
            {
                voltage_max = ams_state.cell_voltages[slave_index][cell_index];
            }
        }
    }
    ams_state.voltage_min = voltage_min;
    ams_state.voltage_max = voltage_max;
}

void Ams::updateMaxMinTemperatures()
{
    uint16_t temperature_min = UINT16_MAX;
    uint16_t temperature_max = 0;
    for (uint8_t slave_index = 0; slave_index < NUM_SLAVE; ++slave_index)
    {

        for (uint8_t ntc_index = 0; ntc_index < NUM_NTC; ++ntc_index)
        {
            if (ams_state.ntc_temperatures[slave_index][ntc_index] < temperature_min)
            {
                temperature_min = ams_state.ntc_temperatures[slave_index][ntc_index];
            }
            if (ams_state.ntc_temperatures[slave_index][ntc_index] > temperature_max)
            {
                temperature_max = ams_state.ntc_temperatures[slave_index][ntc_index];
            }
        }
    }
    ams_state.temperature_min = temperature_min;
    ams_state.temperature_max = temperature_max;
}

void Ams::updateBatterySOC(int8_t hall_lo, int8_t hall_hi)
{
    ams_state.coulomb_count = hall_hi - hall_lo; // Calculate the coulomb count based on hall sensor readings
    // TODO: Implement a more accurate SOC calculation based on coulomb count and battery capacity
}

void Ams::setAIR(AIRState new_state)
{
    ams_state.air_state = new_state;
    switch (ams_state.air_state)
    {
    case AIRState::INIT:
        // Handle INIT state
        digitalWrite(PIN_AIR_NEG, HIGH);
        digitalWrite(PIN_AIR_POS, HIGH);
        digitalWrite(PIN_AIR_PRE, HIGH);
        break;

    case AIRState::PRECHARGE:
        // Handle PRECHARGE state
        precharge_start = millis();
        digitalWrite(PIN_AIR_NEG, LOW);
        digitalWrite(PIN_AIR_POS, HIGH);
        digitalWrite(PIN_AIR_PRE, LOW);
        break;

    case AIRState::ACTIVE:
        // Handle ACTIVE state
        // Delay 100 after INIT, Voltage > 50% (Threshold)
        digitalWrite(PIN_AIR_NEG, LOW);
        digitalWrite(PIN_AIR_POS, LOW);
        digitalWrite(PIN_AIR_PRE, HIGH);
        break;

    case AIRState::DISCHARGE:
        // Handle DISCHARGE state
        // Discharge itself is hardware (K5 shorts MCU+ to MCU-)
        digitalWrite(PIN_AIR_NEG, HIGH);
        digitalWrite(PIN_AIR_POS, HIGH);
        digitalWrite(PIN_AIR_PRE, HIGH);
        break;
    }
}

void Ams::updateAIR()
{
    // Applies to all states, if SDC is closed, go to DISCHARGE state
    if (digitalRead(PIN_SDC) != SDC_CLOSED)
    {
        ams_state.sdc_active = false;
        ams_state.fault_flags |= SDC_FAULT_BIT;
        setAIR(AIRState::DISCHARGE);
    }

    switch (ams_state.air_state)
    {
    case AIRState::INIT:
    {
        // powered on = SDC closed, and no fault active
        if (ams_state.raw_ac_voltage < AC_MIN_START_VOLTAGE_THRESHOLD)
        {
            ams_state.fault_flags |= UNDERVOLTAGE_FAULT_BIT;
            digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin high if any fault is active
        }

        else if (digitalRead(PIN_SDC) == SDC_CLOSED && !ams_state.fault_flags)
        {
            setAIR(AIRState::PRECHARGE);
        }

        break;
    }

    case AIRState::PRECHARGE:
    {
        // Handle PRECHARGE state
        if (ams_state.raw_ac_voltage > SHUTDOWN_VOLTAGE_THRESHOLD && ams_state.raw_tc_voltage >= ams_state.raw_ac_voltage * TS_AC_RATIO)
        {
            setAIR(AIRState::ACTIVE);
        }
        else if (millis() - precharge_start > PRECHARGE_TIMEOUT)
        {
            // TS never reached the ratio, stop precharging and send fault
            ams_state.fault_flags |= PRECHARGE_TIMEOUT_FAULT_BIT;
            setAIR(AIRState::DISCHARGE);
        }

        break;
    }

    case AIRState::ACTIVE:
    {
        // Handle ACTIVE state

        break;
    }

    case AIRState::DISCHARGE:
    {
        break;
    }
    }
}