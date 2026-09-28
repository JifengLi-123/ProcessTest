/**
 * @file test_qualification.cpp
 * @brief ソフトウェア適格性テスト（SWE.6）— ホスト実行可能サブセット
 *
 * @details 対応テスト仕様: 08-50（docs/test/swe6/08-50-qualification-test-specification.md）
 *          SW 要求仕様書（17-11）の各 SW-REQ を、検証基準（17-50）の
 *          合否判定条件に基づき、統合済みソフトウェア（ecu/ と同一 wiring の
 *          フルスタック構成）に対して確認する。
 *          実プロジェクトでは実機/HIL 環境での適格性テストを別途実施する
 *          （docs/README.md の SWE.6 実施方針参照）。
 */
#include <gtest/gtest.h>

#include "app/motor_control/motor_controller.hpp"
#include "pf/fw/sil_ign_signal.hpp"
#include "pf/fw/sil_pwm_hw.hpp"
#include "pf/vapi/ignition_service.hpp"
#include "pf/vapi/pwm_service.hpp"

namespace
{

using app::ControlStatus;
using app::MotorController;
using app::MotorState;
using pf::fw::IgnLineLevel;
using pf::fw::SilIgnSignal;
using pf::fw::SilPwmHw;
using pf::vapi::IgnitionService;
using pf::vapi::PwmService;

class QualificationTest : public ::testing::Test
{
protected:
    // 統合ルート（ecu/）と同一の wiring によるフルスタック構成
    SilPwmHw pwmHw{};
    SilIgnSignal ignSignal{};  // デフォルト High（IG-ON 相当）
    PwmService pwmService{pwmHw};
    IgnitionService ignitionService{ignSignal};
    MotorController controller{pwmService, ignitionService};
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
        EXPECT_NEAR(pwmHw.GetLastDuty(), p.expectedDuty, 0.01F)
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
    EXPECT_FLOAT_EQ(pwmHw.GetLastDuty(), 50.0F);
    EXPECT_FLOAT_EQ(controller.getLastCommandedSpeed(), 3000.0F);
}

// SWE6-TC-003（SW-REQ-003 / VC-003）: 未初期化時の動作拒否
TEST_F(QualificationTest, TC003_SwReq003_UninitializedRejection)
{
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F),
              ControlStatus::NotInitialized);
    // 合否判定条件: HW への出力が発生しないこと
    EXPECT_TRUE(pwmHw.GetHistory().empty());
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

    pwmHw.InjectFailure(true);
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::HwError);
    EXPECT_EQ(controller.getState(), MotorState::Error);

    // Error 状態では新たな制御要求を受理しないこと
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F),
              ControlStatus::NotInitialized);

    // init() による復帰後、制御を再開できること
    pwmHw.InjectFailure(false);
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    EXPECT_EQ(controller.applyTargetSpeed(1000.0F), ControlStatus::Ok);
    EXPECT_EQ(controller.getState(), MotorState::Running);
}

// SWE6-TC-006（SW-REQ-PWR-001 / VC-006）: IG-OFF 時の安全停止
TEST_F(QualificationTest, TC006_SwReqPwr001_IgnitionOffSafeStop)
{
    ASSERT_EQ(controller.init(), ControlStatus::Ok);
    ASSERT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::Ok);

    // IG-OFF（ライン Low）: 制御要求は受理されず、停止デューティ比が出力されること
    ignSignal.SetLevel(IgnLineLevel::Low);
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);
    EXPECT_FLOAT_EQ(pwmHw.GetLastDuty(), MotorController::STOP_DUTY_PERCENT);
    EXPECT_EQ(controller.getState(), MotorState::Ready);

    // IG 信号取得失敗（ライン Fault → Unknown）も IG-OFF と同等に扱うこと
    ignSignal.SetLevel(IgnLineLevel::Fault);
    EXPECT_EQ(controller.applyTargetSpeed(3000.0F), ControlStatus::IgnitionOff);
}

}  // namespace
