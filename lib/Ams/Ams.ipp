/**
 * @file Ams.ipp
 * @author Planeson (carson.cpk@proton.me)
 * @brief Implementation of Ams constexpr methods
 * @version 1.0
 * @date 2026-11-09
 * 
 * @copyright Copyright (c) 2026 Red Bird Racing
 * 
 */

#include "Ams.hpp"

constexpr uint8_t Ams::scaleSOCToVCU(uint8_t soc)
{   // SOC in percentage (0-100)
    /*
    constexpr uint8_t soc_100 = 227;
    constexpr uint8_t soc_0 = 27;
    constexpr uint8_t epsilon = 2;
    return static_cast<uint8_t>(soc + soc_0 * epsilon);
    */
    return LinearInterp<uint8_t, uint8_t, uint16_t, 2>(SOC_TO_VCU_TABLE).interp(soc);
}

constexpr uint8_t Ams::scaleACTSVoltageToVCU(uint16_t value)
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

constexpr int8_t Ams::scaleHallSensorToVCU(int8_t coloumb_count)
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

constexpr uint8_t Ams::scaleMaxMinVoltageToVCU(uint16_t value)
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

constexpr uint8_t Ams::scaleMaxMinTemperatureToVCU(uint16_t value)
{
    // Analog Read
    constexpr uint8_t max_temperature = 100;
    constexpr uint8_t min_temperature = 0;
    constexpr uint8_t epsilon = 1;
    return static_cast<uint8_t> (value - min_temperature) / epsilon;
}