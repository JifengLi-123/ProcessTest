/**
 * @file test_integration.cpp
 * @brief ソフトウェア統合テスト（SWE.5）
 *
 * @details 対応テスト仕様: 08-50（docs/swe5/08-50-integration-test-specification.md）
 *          APP レイヤー（MotorController）と PF レイヤー（SIL 実装の車載 API:
 *          SilPwmService / SilIgnitionService）を統合し、レイヤー間 I/F
 *          （SW-IF-001 / SW-IF-002）の整合性と統合後の振る舞いを検証する。
 *          ユニットテストと異なり、PF 側の値域検証を含めた
 *          コンポーネント間の相互作用を確認する。
 */
#include <gtest/gtest.h>

#include <vector>

#include "motor_controller.hpp"
#include "sil_ignition_service.hpp"
#include "sil_pwm_service.hpp"

namespace
{

using app::ControlStatus;
using app::MotorController;
using app::MotorState;
using pf::IgnitionState;
using pf::sil::SilIgnitionService;
using pf::sil::SilPwmService;

class IntegrationTest : public ::testing::Test
{
protected:
    SilPwmService pwm{};
    SilIgnitionService ignition{};  // デフォルト IG-ON
    MotorController controller{pwm, ignition};
};

// SWE5-TC-001（正常系）: I/F 整合性 — APP の全出力が PF 車載 API に受理される
TEST_F(IntegrationTest, TC001_AllAppOutputsAcceptedByPfApi)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    // 値域内の代表点・境界値を掃引し、全てレイヤー間 I/F 越しに受理されること
    const std::vector<float> speeds{0.0F, 1.0F, 1500.0F, 3000.0F,
                                    5999.0F, 6000.0F};
    for (const float speed : speeds)
    {
        EXPECT_EQ(controller.applyTargetSpeed(speed), ControlStatus::Ok)
            << "speed=" << speed;
    }
    EXPECT_EQ(pwm.GetHistory().size(), speeds.size());

    // PF が受理した値は常に [DUTY_MIN, DUTY_MAX] の範囲内（04-05 後条件）
    for (const float duty : pwm.GetHistory())
    {
        EXPECT_GE(duty, MotorController::DUTY_MIN);
        EXPECT_LE(duty, MotorController::DUTY_MAX);
    }
}

// SWE5-TC-002（正常系）: 連続コマンドシーケンスの統合動作
TEST_F(IntegrationTest, TC002_ContinuousCommandSequence)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    // 加速 → 減速のシーケンスで状態が Running を維持すること
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.applyTargetSpeed(4000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.applyTargetSpeed(2000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 2000.0F);

    ASSERT_EQ(pwm.GetHistory().size(), 3U);
    EXPECT_FLOAT_EQ(pwm.GetHistory()[0], 20.0F);  // 1000/6000 → 20.0%
    EXPECT_FLOAT_EQ(pwm.GetHistory()[1], 65.0F);  // 4000/6000 → 65.0%
    EXPECT_FLOAT_EQ(pwm.GetHistory()[2], 35.0F);  // 2000/6000 → 35.0%
}

// SWE5-TC-003（異常系）: 範囲外コマンド混在時、正常コマンドのみ PF へ到達
TEST_F(IntegrationTest, TC003_InvalidCommandsDoNotReachPf)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.applyTargetSpeed(-100.0F), ControlStatus::OutOfRange);
    EXPECT_EQ(controller.applyTargetSpeed(7000.0F), ControlStatus::OutOfRange);
    EXPECT_EQ(controller.applyTargetSpeed(6000.0F), ControlStatus::Ok);

    // 異常コマンドはレイヤー間 I/F へ到達しないこと
    ASSERT_EQ(pwm.GetHistory().size(), 2U);
    EXPECT_FLOAT_EQ(pwm.GetHistory()[0], 50.0F);
    EXPECT_FLOAT_EQ(pwm.GetHistory()[1], MotorController::DUTY_MAX);
}

// SWE5-TC-004（正常系）: IG-ON → IG-OFF → IG-ON の車両状態シナリオ統合動作
//                        （SW-REQ-PWR-001）
TEST_F(IntegrationTest, TC004_IgnitionCycleScenario)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    // IG-ON: 通常制御
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);

    // IG-OFF: 安全停止（停止デューティ比 0% が PF へ出力される）
    ignition.SetIgnitionState(IgnitionState::Off);
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);
    EXPECT_EQ(controller.getState(), MotorState::Ready);
    EXPECT_FLOAT_EQ(pwm.GetLastDutyCycle(), MotorController::STOP_DUTY_PERCENT);

    // IG-ON 復帰: 制御再開
    ignition.SetIgnitionState(IgnitionState::On);
    EXPECT_EQ(controller.applyTargetSpeed(1500.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
    EXPECT_FLOAT_EQ(pwm.GetLastDutyCycle(), 27.5F);
}

// SWE5-TC-005（異常系）: PF 故障注入 → APP のフェール動作 → 復帰の統合確認
TEST_F(IntegrationTest, TC005_PfFailureAndRecovery)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    pwm.InjectFailure(true);
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::HwError);
    EXPECT_EQ(controller.getState(), MotorState::Error);

    pwm.InjectFailure(false);
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
}

}  // namespace
