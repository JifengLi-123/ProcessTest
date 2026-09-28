/**
 * @file test_fan_controller.cpp
 * @brief FanController ユニットテスト（SWE.4）
 *
 * @details 選択ビルド・SOTA デモ用 APP（app/hvac）のテスト。
 *          正式な 08-50 テスト仕様体系の見本は motor_control 側を参照。
 *          車載 API（契約面）は vapi_mock に差し替える。
 */
#include <gtest/gtest.h>

#include "app/hvac/fan_controller.hpp"
#include "vapi_mock/mock_ignition_service.hpp"
#include "vapi_mock/mock_pwm_service.hpp"

namespace
{

using app::FanController;
using app::FanStatus;
using pf::vapi::IgnitionState;

class FanControllerTest : public ::testing::Test
{
protected:
    mocks::MockPwmService pwm{};
    mocks::MockIgnitionService ignition{};  // デフォルト IG-ON
    FanController fan{pwm, ignition};
};

// HVAC-TC-001（正常系）: 各レベルが 25% 刻みのデューティ比に変換される
TEST_F(FanControllerTest, TC001_LevelMapsToDuty)
{
    EXPECT_EQ(fan.setFanLevel(0U), FanStatus::Ok);
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, 0.0F);

    EXPECT_EQ(fan.setFanLevel(2U), FanStatus::Ok);
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, 50.0F);

    EXPECT_EQ(fan.setFanLevel(FanController::FAN_LEVEL_MAX), FanStatus::Ok);
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, 75.0F);
}

// HVAC-TC-002（異常系）: 範囲外レベルは棄却され HW 出力しない
TEST_F(FanControllerTest, TC002_OverMaxLevelRejected)
{
    EXPECT_EQ(fan.setFanLevel(4U), FanStatus::OutOfRange);
    EXPECT_EQ(pwm.callCount, 0);
}

// HVAC-TC-003（異常系）: IG-OFF / 取得失敗時は安全停止して IgnitionOff
TEST_F(FanControllerTest, TC003_IgnitionOffTriggersSafeStop)
{
    ignition.state = IgnitionState::Off;
    EXPECT_EQ(fan.setFanLevel(2U), FanStatus::IgnitionOff);
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, FanController::STOP_DUTY_PERCENT);

    ignition.state = IgnitionState::Unknown;  // フェールセーフ: Off と同等
    EXPECT_EQ(fan.setFanLevel(2U), FanStatus::IgnitionOff);
}

// HVAC-TC-004（異常系）: PWM 出力失敗は HwError として通知される
TEST_F(FanControllerTest, TC004_HwFailurePropagates)
{
    pwm.simulateHwFailure = true;
    EXPECT_EQ(fan.setFanLevel(1U), FanStatus::HwError);
}

}  // namespace
