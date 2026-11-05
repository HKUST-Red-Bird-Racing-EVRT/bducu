#ifndef AMSHELPER_HPP
#define AMSHELPER_HPP

#include "AmsState.hpp"
#include "BoardConfig.h"
#include <Arduino.h>

#define TIME_SETUP 1000
#define DEFAULT_VOLTAGE_MAX 4250
#define DEFAULT_VOLTAGE_MIN 3700
constexpr uint8_t VOLTAGE_DIVSOR = 127;
constexpr uint16_t mV_PER_V = 1000;
constexpr float VCC = 5.0;
constexpr uint16_t ADC_RESOLUTION = 1023;
constexpr uint16_t AC_MIN_START_VOLTAGE_THRESHOLD = (double)VOLTAGE_UV * NUM_VCELL * NUM_SLAVE / VOLTAGE_DIVSOR / mV_PER_V / VCC * ADC_RESOLUTION; // 3700mV * 14 * 10 / 127 = 4070mV

class AmsHelper {
    public:
        AmsHelper(AmsState &ams_);
        AmsHelper() = delete; // Delete the default constructor to prevent its use
        void resetCellBal();
        void updateMaxMinCellVoltages();
        void updateMaxMinTemperatures();
        void updateBatterySOC(int8_t hall_lo, int8_t hall_hi);
        void setAIR(AIRState new_state);
        void updateAIR();

    private:
        AmsState &ams;
        uint32_t precharge_start = 0; // millis() when PRECHARGE was entered

};

#endif // AMSHELPER_HPP