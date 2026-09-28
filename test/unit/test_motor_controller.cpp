/**
 * @file test_motor_controller.cpp
 * @brief MotorController ユニットテスト（SWE.4）
 *
 * @details 対応テスト仕様: 08-50（docs/swe4/08-50-unit-test-specification.md）
 *          テストケース ID（SWE4-TC-NNN）は 08-50 と 1 対 1 に対応する。
 *          詳細設計（04-05）の @pre / @post から導出した境界値・異常系を含む。
 *          依存する車載 API（PF レイヤー I/F）はモックに差し替える
 *          （04-05 §8 テスト容易性設計）。
 */
#include <gtest/gtest.h>

#include <limits>

#include "motor_controller.hpp"
#include "pf/i_ignition_service.hpp"
#include "pf/i_pwm_service.hpp"

namespace
{

using app::ControlStatus;
using app::MotorController;
using app::MotorState;
using pf::IgnitionState;

/// PWM 出力サービス（車載 API SW-IF-001）のモック
class MockPwmService final : public pf::IPwmService
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

/// IG 状態サービス（車載 API SW-IF-002）のモック
class MockIgnitionService final : public pf::IIgnitionService
{
public:
    IgnitionState GetIgnitionState() override
    {
        return state;
    }

    IgnitionState state{IgnitionState::On};  ///< 模擬する IG 状態
};

class MotorControllerTest : public ::testing::Test
{
protected:
    MockPwmService pwm{};
    MockIgnitionService ignition{};
    MotorController controller{pwm, ignition};
};

// SWE4-TC-001（正常系）: 中央値 3000rpm → デューティ比 50%
TEST_F(MotorControllerTest, TC001_NominalSpeedProducesLinearDuty)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    float duty{0.0F};
    EXPECT_EQ(controller.calculateDutyCycle(3000.0F, duty), ControlStatus::Ok);
    EXPECT_FLOAT_EQ(duty, 50.0F);
    // 後条件: lastCommandedSpeed が更新される
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 3000.0F);
}

// SWE4-TC-002（正常系）: 値域内境界値 0rpm → DUTY_MIN / 6000rpm → DUTY_MAX
TEST_F(MotorControllerTest, TC002_BoundaryValuesMapToDutyLimits)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    float duty{0.0F};
    EXPECT_EQ(controller.calculateDutyCycle(0.0F, duty), ControlStatus::Ok);
    EXPECT_FLOAT_EQ(duty, MotorController::DUTY_MIN);

    EXPECT_EQ(controller.calculateDutyCycle(MotorController::MOTOR_MAX_SPEED_RPM,
                                            duty),
              ControlStatus::Ok);
    EXPECT_FLOAT_EQ(duty, MotorController::DUTY_MAX);
}

// SWE4-TC-003（異常系）: 負値入力 → OutOfRange、出力・内部状態は不変
TEST_F(MotorControllerTest, TC003_NegativeSpeedRejected)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    float duty{-99.0F};
    EXPECT_EQ(controller.calculateDutyCycle(-0.1F, duty),
              ControlStatus::OutOfRange);
    // 後条件: エラー時 dutyCycle と内部状態は変化しない
    EXPECT_FLOAT_EQ(duty, -99.0F);
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 0.0F);
}

// SWE4-TC-004（異常系）: 上限超過入力 → OutOfRange
TEST_F(MotorControllerTest, TC004_OverMaxSpeedRejected)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    float duty{-99.0F};
    EXPECT_EQ(controller.calculateDutyCycle(6000.1F, duty),
              ControlStatus::OutOfRange);
    EXPECT_FLOAT_EQ(duty, -99.0F);
}

// SWE4-TC-005（異常系）: NaN 入力 → OutOfRange（防御的プログラミング）
TEST_F(MotorControllerTest, TC005_NanSpeedRejected)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    const float nan{std::numeric_limits<float>::quiet_NaN()};
    float duty{-99.0F};
    EXPECT_EQ(controller.calculateDutyCycle(nan, duty),
              ControlStatus::OutOfRange);
    EXPECT_FLOAT_EQ(duty, -99.0F);
}

// SWE4-TC-006（異常系）: 未初期化状態での算出要求 → NotInitialized
TEST_F(MotorControllerTest, TC006_UninitializedRejected)
{
    float duty{-99.0F};
    EXPECT_EQ(controller.getState(), MotorState::Uninitialized);
    EXPECT_EQ(controller.calculateDutyCycle(1000.0F, duty),
              ControlStatus::NotInitialized);
    EXPECT_FLOAT_EQ(duty, -99.0F);
}

// SWE4-TC-007（正常系）: 状態遷移 Uninitialized → Ready → Running
TEST_F(MotorControllerTest, TC007_StateTransitionToRunning)
{
    EXPECT_EQ(controller.getState(), MotorState::Uninitialized);

    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Ready);

    EXPECT_EQ(controller.applyTargetSpeed(1500.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
    // 車載 API へ出力されたデューティ比の確認（1500/6000 → 27.5%）
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, 27.5F);
    EXPECT_EQ(pwm.callCount, 1);
}

// SWE4-TC-008（異常系）: PWM 出力サービス失敗 → HwError・Error 状態へ遷移
TEST_F(MotorControllerTest, TC008_HwFailureTransitionsToError)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    pwm.simulateHwFailure = true;

    EXPECT_EQ(controller.applyTargetSpeed(1500.0F), ControlStatus::HwError);
    EXPECT_EQ(controller.getState(), MotorState::Error);

    // Error 状態では制御 API は NotInitialized を返す
    float duty{0.0F};
    EXPECT_EQ(controller.calculateDutyCycle(1000.0F, duty),
              ControlStatus::NotInitialized);
}

// SWE4-TC-009（正常系）: Error 状態からの init() 復帰
TEST_F(MotorControllerTest, TC009_RecoveryFromErrorByInit)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    pwm.simulateHwFailure = true;
    ASSERT_EQ(controller.applyTargetSpeed(1500.0F), ControlStatus::HwError);
    ASSERT_EQ(controller.getState(), MotorState::Error);

    pwm.simulateHwFailure = false;
    EXPECT_EQ(controller.init(), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Ready);
    // 後条件: init() 後 lastCommandedSpeed は 0.0f にクリアされる
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 0.0F);

    EXPECT_EQ(controller.applyTargetSpeed(2000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
}

// SWE4-TC-010（異常系）: applyTargetSpeed の範囲外入力では HW 出力しない
TEST_F(MotorControllerTest, TC010_OutOfRangeDoesNotTouchHardware)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    EXPECT_EQ(controller.applyTargetSpeed(-1.0F), ControlStatus::OutOfRange);
    EXPECT_EQ(pwm.callCount, 0);
    EXPECT_EQ(controller.getState(), MotorState::Ready);
}

// SWE4-TC-011（異常系）: IG-OFF 時は安全停止（0% 出力・Ready 遷移）
//                        （SW-REQ-PWR-001）
TEST_F(MotorControllerTest, TC011_IgnitionOffTriggersSafeStop)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    ASSERT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::Ok);
    ASSERT_EQ(controller.getState(), MotorState::Running);

    ignition.state = IgnitionState::Off;
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);

    // 後条件: 停止デューティ比が出力され Ready 状態・受理値 0 となる
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, MotorController::STOP_DUTY_PERCENT);
    EXPECT_EQ(controller.getState(), MotorState::Ready);
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 0.0F);
}

// SWE4-TC-012（異常系）: IG 状態取得失敗（Unknown）は IG-OFF と同等に扱う
//                        （フェールセーフ方針）
TEST_F(MotorControllerTest, TC012_IgnitionUnknownTreatedAsOff)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    ignition.state = IgnitionState::Unknown;

    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, MotorController::STOP_DUTY_PERCENT);
    EXPECT_EQ(controller.getState(), MotorState::Ready);
}

// SWE4-TC-013（異常系）: IG-OFF 安全停止時の出力失敗 → HwError・Error 遷移
TEST_F(MotorControllerTest, TC013_SafeStopHwFailureTransitionsToError)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    ignition.state = IgnitionState::Off;
    pwm.simulateHwFailure = true;

    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::HwError);
    EXPECT_EQ(controller.getState(), MotorState::Error);
}

}  // namespace
