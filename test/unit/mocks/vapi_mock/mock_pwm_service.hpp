/**
 * @file mock_pwm_service.hpp
 * @brief 車載API SW-IF-001（IPwmService）のユニットテスト用モック
 *
 * @details APP 各チームの SWE.4 はこのモックで完結する（Firmware 不要）。
 *          契約面（pf/vapi）との乖離を防ぐため、本ディレクトリ（vapi_mock）は
 *          PF チームがオーナーとして管理する（.github/CODEOWNERS 参照）。
 */
#ifndef PROCESSTEST_TEST_VAPI_MOCK_PWM_SERVICE_HPP
#define PROCESSTEST_TEST_VAPI_MOCK_PWM_SERVICE_HPP

#include "pf/vapi/i_pwm_service.hpp"

namespace mocks
{

/// PWM 出力サービスモック（呼び出し記録・故障模擬）
class MockPwmService final : public pf::vapi::IPwmService
{
public:
    bool SetDutyCycle(float dutyCyclePercent) override
    {
        ++callCount;
        lastDutyCycle = dutyCyclePercent;
        return !simulateHwFailure;
    }

    bool simulateHwFailure{false};  ///< true で HW 異常を模擬
    float lastDutyCycle{-1.0F};     ///< 最後に出力されたデューティ比
    int callCount{0};               ///< SetDutyCycle 呼び出し回数
};

}  // namespace mocks

#endif  // PROCESSTEST_TEST_VAPI_MOCK_PWM_SERVICE_HPP
