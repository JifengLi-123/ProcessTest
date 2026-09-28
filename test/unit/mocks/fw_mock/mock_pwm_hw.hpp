/**
 * @file mock_pwm_hw.hpp
 * @brief Firmware FW-IF-001（IPwmHw）のユニットテスト用モック
 *
 * @details Vehicle API 層自体の SWE.4（pf_vapi のユニットテスト）に使用する。
 *          APP のテストからは使用しない（APP は vapi_mock を使用）。
 */
#ifndef PROCESSTEST_TEST_FW_MOCK_PWM_HW_HPP
#define PROCESSTEST_TEST_FW_MOCK_PWM_HW_HPP

#include "pf/fw/i_pwm_hw.hpp"

namespace mocks
{

/// PWM ハードウェア抽象モック（呼び出し記録・故障模擬）
class MockPwmHw final : public pf::fw::IPwmHw
{
public:
    bool WriteDuty(float dutyPercent) override
    {
        ++callCount;
        lastDuty = dutyPercent;
        return !simulateHwFailure;
    }

    bool simulateHwFailure{false};  ///< true で HW 異常を模擬
    float lastDuty{-1.0F};          ///< 最後に書き込まれたデューティ比
    int callCount{0};               ///< WriteDuty 呼び出し回数
};

}  // namespace mocks

#endif  // PROCESSTEST_TEST_FW_MOCK_PWM_HW_HPP
