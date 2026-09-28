/**
 * @file test_qualification.cpp
 * @brief ソフトウェア適格性テスト（SWE.6）
 *
 * @details 対応テスト仕様: 08-50（docs/swe6/08-50-qualification-test-specification.md）
 *          SW 要求仕様書（17-11）の各 SW-REQ を、検証基準（17-50）の
 *          合否判定条件に基づき、統合済みソフトウェア（APP + PF SIL）に
 *          対して確認する。テストケース ID（SWE6-TC-NNN）は SW-REQ / VC と
 *          対応付ける。
 */
#include <gtest/gtest.h>

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

class QualificationTest : public ::testing::Test
{
protected:
    SilPwmService pwm{};
    SilIgnitionService ignition{};  // デフォルト IG-ON
    MotorController controller{pwm, ignition};
};

// SWE6-TC-001（SW-REQ-001 / VC-001）: 目標回転数の線形変換
TEST_F(QualificationTest, TC001_SwReq001_LinearConversion)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    // 検証基準 VC-001: 0/25/50/75/100% 点で期待デューティ比 ±0.01% 以内
    struct TestPoint
    {
        float speedRpm;
        float expectedDuty;
    };
    const TestPoint points[]{{0.0F, 5.0F},
                             {1500.0F, 27.5F},
                             {3000.0F, 50.0F},
                             {4500.0F, 72.5F},
                             {6000.0F, 95.0F}};

    for (const TestPoint& p : points)
    {
        ASSERT_EQ(controller.applyTargetSpeed(p.speedRpm), ControlStatus::Ok);
        EXPECT_NEAR(pwm.GetLastDutyCycle(), p.expectedDuty, 0.01F)
            << "speedRpm=" << p.speedRpm;
    }
}

// SWE6-TC-002（SW-REQ-002 / VC-002）: 範囲外入力の棄却と出力保持
TEST_F(QualificationTest, TC002_SwReq002_OutOfRangeRejection)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    ASSERT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::Ok);

    EXPECT_EQ(controller.applyTargetSpeed(-1.0F), ControlStatus::OutOfRange);
    EXPECT_EQ(controller.applyTargetSpeed(6000.1F), ControlStatus::OutOfRange);

    // 合否判定条件: 直前の正常出力（50%）と受理値（3000rpm）が保持されること
    EXPECT_FLOAT_EQ(pwm.GetLastDutyCycle(), 50.0F);
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 3000.0F);
}

// SWE6-TC-003（SW-REQ-003 / VC-003）: 未初期化時の動作拒否
TEST_F(QualificationTest, TC003_SwReq003_UninitializedRejection)
{
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F),
              ControlStatus::NotInitialized);
    // 合否判定条件: PF（HW）への出力が発生しないこと
    EXPECT_TRUE(pwm.GetHistory().empty());
}

// SWE6-TC-004（SW-REQ-004 / VC-004）: 状態遷移仕様の充足
TEST_F(QualificationTest, TC004_SwReq004_StateMachine)
{
    EXPECT_EQ(controller.getState(), MotorState::Uninitialized);
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Ready);
    ASSERT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
}

// SWE6-TC-005（SW-REQ-005 / VC-005）: HW 異常時のフェール動作と復帰
TEST_F(QualificationTest, TC005_SwReq005_HwErrorAndRecovery)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);

    pwm.InjectFailure(true);
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::HwError);
    EXPECT_EQ(controller.getState(), MotorState::Error);

    // Error 状態では新たな制御要求を受理しないこと
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F),
              ControlStatus::NotInitialized);

    // init() による復帰後、制御を再開できること
    pwm.InjectFailure(false);
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
}

// SWE6-TC-006（SW-REQ-PWR-001 / VC-006）: IG-OFF 時の安全停止
TEST_F(QualificationTest, TC006_SwReqPwr001_IgnitionOffSafeStop)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    ASSERT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::Ok);

    // IG-OFF: 制御要求は受理されず、停止デューティ比が出力されること
    ignition.SetIgnitionState(IgnitionState::Off);
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);
    EXPECT_FLOAT_EQ(pwm.GetLastDutyCycle(), MotorController::STOP_DUTY_PERCENT);
    EXPECT_EQ(controller.getState(), MotorState::Ready);

    // IG 状態取得失敗（Unknown）も IG-OFF と同等に扱うこと（フェールセーフ）
    ignition.SetIgnitionState(IgnitionState::Unknown);
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);
}

}  // namespace
