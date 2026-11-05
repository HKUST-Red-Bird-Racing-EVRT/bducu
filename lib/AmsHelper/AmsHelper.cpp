#include "AmsHelper.hpp"

AmsHelper::AmsHelper(AmsState &ams_)
    : ams(ams_)
{
    // Constructor implementation
}

void AmsHelper::resetCellBal()
{
    ams.cellbal_active = false;
    ams.cellbal_odd = false;
    ams.voltage_min = DEFAULT_VOLTAGE_MIN;
    ams.voltage_max = DEFAULT_VOLTAGE_MAX;
}

void AmsHelper::updateMaxMinCellVoltages()
{
    uint16_t voltage_min = UINT16_MAX;
    uint16_t voltage_max = 0;
    for (uint8_t slave_index = 0; slave_index < NUM_SLAVE; ++slave_index)
    {

        for (uint8_t cell_index = 0; cell_index < NUM_VCELL; ++cell_index)
        {
            if (ams.cell_voltages[slave_index][cell_index] < voltage_min)
            {
                voltage_min = ams.cell_voltages[slave_index][cell_index];
            }
            if (ams.cell_voltages[slave_index][cell_index] > voltage_max)
            {
                voltage_max = ams.cell_voltages[slave_index][cell_index];
            }
        }
    }
    ams.voltage_min = voltage_min;
    ams.voltage_max = voltage_max;
}

void AmsHelper::updateMaxMinTemperatures()
{
    uint16_t temperature_min = UINT16_MAX;
    uint16_t temperature_max = 0;
    for (uint8_t slave_index = 0; slave_index < NUM_SLAVE; ++slave_index)
    {

        for (uint8_t ntc_index = 0; ntc_index < NUM_NTC; ++ntc_index)
        {
            if (ams.ntc_temperatures[slave_index][ntc_index] < temperature_min)
            {
                temperature_min = ams.ntc_temperatures[slave_index][ntc_index];
            }
            if (ams.ntc_temperatures[slave_index][ntc_index] > temperature_max)
            {
                temperature_max = ams.ntc_temperatures[slave_index][ntc_index];
            }
        }
    }
    ams.temperature_min = temperature_min;
    ams.temperature_max = temperature_max;
}

void AmsHelper::updateBatterySOC(int8_t hall_lo, int8_t hall_hi)
{
    ams.coulomb_count = hall_hi - hall_lo; // Calculate the coulomb count based on hall sensor readings
    // TODO: Implement a more accurate SOC calculation based on coulomb count and battery capacity
}

void AmsHelper::setAIR(AIRState new_state)
{
    ams.air_state = new_state;
    switch (ams.air_state)
    {
    case AIRState::INIT:
        // Handle INIT state
        digitalWrite(PIN_AIR_NEG, LOW);
        digitalWrite(PIN_AIR_POS, LOW);
        digitalWrite(PIN_AIR_PRE, LOW);
        break;

    case AIRState::PRECHARGE:
        // Handle PRECHARGE state
        precharge_start = millis();
        digitalWrite(PIN_AIR_NEG, HIGH);
        digitalWrite(PIN_AIR_POS, LOW);
        digitalWrite(PIN_AIR_PRE, HIGH);
        break;

    case AIRState::ACTIVE:
        // Handle ACTIVE state
        // Delay 100 after INIT, Voltage > 50% (Threshold)
        digitalWrite(PIN_AIR_NEG, HIGH);
        digitalWrite(PIN_AIR_POS, HIGH);
        digitalWrite(PIN_AIR_PRE, LOW);
        break;

    case AIRState::DISCHARGE:
        // Handle DISCHARGE state
        // Discharge itself is hardware (K5 shorts MCU+ to MCU-)
        digitalWrite(PIN_AIR_NEG, LOW);
        digitalWrite(PIN_AIR_POS, LOW);
        digitalWrite(PIN_AIR_PRE, LOW);
        break;
    }
}

void AmsHelper::updateAIR()
{
    // Applies to all states, if SDC is closed, go to DISCHARGE state
    if (digitalRead(PIN_SDC) == LOW)
    {
        ams.sdc_active = false;
        ams.fault_flags |= SDC_FAULT_BIT;
        setAIR(AIRState::DISCHARGE);
    }

    switch (ams.air_state)
    {
    case AIRState::INIT:
    {
        // powered on = SDC closed, and no fault active
        if (analogRead(PIN_AC_ISO) < AC_MIN_START_VOLTAGE_THRESHOLD)
        {
            ams.fault_flags |= UNDERVOLTAGE_FAULT_BIT;
            digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin high if any fault is active
        }

        else if (digitalRead(PIN_SDC) == HIGH && !ams.fault_flags)
        {
            setAIR(AIRState::PRECHARGE);
        }

        break;
    }

    case AIRState::PRECHARGE:
    {
        // Handle PRECHARGE state
        uint16_t voltage_ac_iso = analogRead(PIN_AC_ISO);
        uint16_t voltage_tc_iso = analogRead(PIN_TC_ISO);

        if (voltage_ac_iso > SHUTDOWN_VOLTAGE_THRESHOLD && voltage_tc_iso >= voltage_ac_iso * TS_AC_RATIO)
        {
            setAIR(AIRState::ACTIVE);
        }
        else if (millis() - precharge_start > PRECHARGE_TIMEOUT)
        {
            // TS never reached the ratio, stop precharging and send fault
            ams.fault_flags |= PRECHARGE_TIMEOUT_FAULT_BIT;
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
        // Handle DISCHARGE state
        uint16_t voltage_tc_iso = analogRead(PIN_TC_ISO);

        if (voltage_tc_iso < SHUTDOWN_VOLTAGE_THRESHOLD)
        {
            setAIR(AIRState::INIT);
        }

        break;
    }
    }
}