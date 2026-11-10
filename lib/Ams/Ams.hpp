/**
 * @file Ams.hpp
 * @author Oscar (oscarckh0818@gmail.com),
 *         Planeson (carson.cpk@proton.me)
 * @brief Declaration of the AMS (Accumulator Management System) class and related constants.
 * @version 0.2
 * @date 2026-11-10
 * 
 * @copyright Copyright (c) 2026 Red Bird Racing
 * 
 */

#ifndef AMS_HPP
#define AMS_HPP

#include "AmsState.hpp"
#include "BoardConfig.h"
#include "Interp.hpp"
#include <Arduino.h>

#define TIME_SETUP 1000
#define DEFAULT_VOLTAGE_MAX 4250
#define DEFAULT_VOLTAGE_MIN 3700
constexpr uint8_t VOLTAGE_DIVSOR = 127;
constexpr uint16_t mV_PER_V = 1000;
constexpr float VCC = 5.0;
constexpr uint16_t ADC_RESOLUTION = 1023;
constexpr uint16_t AC_MIN_START_VOLTAGE_THRESHOLD = (double)VOLTAGE_UV * NUM_VCELL * NUM_SLAVE / VOLTAGE_DIVSOR / mV_PER_V / VCC * ADC_RESOLUTION; // 3700mV * 14 * 10 / 127 = 4070mV

constexpr TablePoint<uint8_t, uint8_t> SOC_TO_VCU_TABLE[2] = 
{
    {0, 27},
    {100, 227}
};

constexpr TablePoint<uint16_t, uint8_t> ACTS_VOLTAGE_TO_VCU_TABLE[2] = 
{
    {(uint16_t)(90 / VOLTAGE_DIVSOR / VCC * ADC_RESOLUTION), 0},    // Minimum Analog Read Value: 144
    {(uint16_t)(600 / VOLTAGE_DIVSOR / VCC * ADC_RESOLUTION), 255}  // Maximum Analog Read Value: 966
};

constexpr TablePoint<int8_t, int8_t> HALL_SENSOR_TO_VCU_TABLE[2] = 
{
    {-128, -128},
    {127, 127}
};

constexpr TablePoint<uint16_t, uint8_t> MAX_MIN_VOLTAGE_TO_VCU_TABLE[2] = 
{
    {3500, 0},
    {4300, 255}
};

constexpr TablePoint<uint16_t, uint8_t> MAX_MIN_TEMPERATURE_TO_VCU_TABLE[5] = 
{
    {0, 0},
    {25, 1},
    {50, 2},
    {75, 3},
    {100, 4}
};

class Ams
{
public:
    explicit Ams(AmsState &ams_state_);
    Ams() = delete; // Delete the default constructor to prevent its use
    void resetCellBal();
    void updateMaxMinCellVoltages();
    void updateMaxMinTemperatures();
    void updateBatterySOC(int8_t hall_lo, int8_t hall_hi);
    void setAIR(AIRState new_state);
    void updateAIR();
    constexpr uint8_t scaleSOCToVCU(uint8_t soc);
    constexpr uint8_t scaleACTSVoltageToVCU(uint16_t value);
    constexpr int8_t scaleHallSensorToVCU(int8_t coloumb_count);
    constexpr uint8_t scaleMaxMinVoltageToVCU(uint16_t value);
    constexpr uint8_t scaleMaxMinTemperatureToVCU(uint16_t value);

private:
    AmsState &ams_state;
    uint32_t precharge_start = 0; // millis() when PRECHARGE was entered
};

#include "Ams.ipp" // Include the implementation of constexpr methods

#endif // AMS_HPP