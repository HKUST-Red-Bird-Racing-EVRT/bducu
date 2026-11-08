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
    uint16_t temperature_min = UINT16_MAX; // Should be 0?
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
    if (digitalRead(PIN_SDC) != SDC_CLOSED)
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
        if (ams.raw_ac_voltage < AC_MIN_START_VOLTAGE_THRESHOLD)
        {
            ams.fault_flags |= UNDERVOLTAGE_FAULT_BIT;
            digitalWrite(PIN_AMS_ERR, HIGH); // Set AMS_ERR pin high if any fault is active
        }

        else if (digitalRead(PIN_SDC) == SDC_CLOSED && !ams.fault_flags)
        {
            setAIR(AIRState::PRECHARGE);
        }

        break;
    }

    case AIRState::PRECHARGE:
    {
        // Handle PRECHARGE state
        if (ams.raw_ac_voltage > SHUTDOWN_VOLTAGE_THRESHOLD && ams.raw_tc_voltage >= ams.raw_ac_voltage * TS_AC_RATIO)
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
        if (ams.raw_tc_voltage < SHUTDOWN_VOLTAGE_THRESHOLD)
        {
            setAIR(AIRState::INIT);
        }

        break;
    }
    }
}

constexpr uint8_t AmsHelper::scaleSOCToVCU(uint8_t soc)
{   // SOC in percentage (0-100)
    /*
    constexpr uint8_t soc_100 = 227;
    constexpr uint8_t soc_0 = 27;
    constexpr uint8_t epsilon = 2;
    return static_cast<uint8_t>(soc + soc_0 * epsilon);
    */
    return LinearInterp<uint8_t, uint8_t, uint16_t, 2>(SOC_TO_VCU_TABLE).interp(soc);
}

constexpr uint8_t AmsHelper::scaleACTSVoltageToVCU(uint16_t value)
{
    // Analog Read
    /*
    constexpr uint16_t max_voltage = 600; // 600V
    constexpr uint8_t min_voltage = 90; // 90V
    constexpr uint16_t max_analog_read = (double)max_voltage / VOLTAGE_DIVSOR / VCC * ADC_RESOLUTION; // Maximum analog read value: 966
    constexpr uint16_t min_analog_read = (double)min_voltage / VOLTAGE_DIVSOR / VCC * ADC_RESOLUTION; // Minimum analog read value: 144
    constexpr uint8_t epsilon = 2;
    return static_cast<uint8_t> (value - min_analog_read) / epsilon;
    */
    return LinearInterp<uint16_t, uint8_t, uint16_t, 2>(ACTS_VOLTAGE_TO_VCU_TABLE).interp(value);
}

constexpr int8_t AmsHelper::scaleHallSensorToVCU(int8_t coloumb_count)
{
    // Hall Sensor
    /*
    constexpr int8_t max_count = 127;
    constexpr int8_t min_count = -128;
    constexpr uint8_t epsilon = 1;
    return static_cast<int8_t> (coloumb_count - min_count) / epsilon;
    */
    return LinearInterp<int8_t, int8_t, int16_t, 2>(HALL_SENSOR_TO_VCU_TABLE).interp(coloumb_count);
}

constexpr uint8_t AmsHelper::scaleMaxMinVoltageToVCU(uint16_t value)
{
    // Actual Voltage in mV
    /*
    constexpr uint16_t max_voltage = 4300;
    constexpr uint8_t min_voltage = 3500;
    constexpr double epsilon = 25/8;
    return static_cast<uint8_t> (value - min_voltage) / epsilon;
    */
    return LinearInterp<uint16_t, uint8_t, uint16_t, 2>(MAX_MIN_VOLTAGE_TO_VCU_TABLE).interp(value);
}

constexpr uint8_t AmsHelper::scaleMaxMinTemperatureToVCU(uint16_t value)
{
    // Analog Read
    constexpr uint8_t max_temperature = 100;
    constexpr uint8_t min_temperature = 0;
    constexpr uint8_t epsilon = 1;
    return static_cast<uint8_t> (value - min_temperature) / epsilon;
}