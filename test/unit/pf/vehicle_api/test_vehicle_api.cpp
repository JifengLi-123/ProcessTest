/**
 * @file test_vehicle_api.cpp
 * @brief Vehicle API 層（MOD-002 PwmService / MOD-003 IgnitionService）
 *        ユニットテスト（SWE.4）
 *
 * @details 対応テスト仕様: 08-50（docs/test/swe4/08-50-unit-test-specification.md
 *          の SWE4-TC-101〜106）。
 *          Vehicle API 自体の SWE.4 では Firmware I/F を fw_mock に差し替える。
 */
#include <gtest/gtest.h>

#include <limits>

#include "fw_mock/mock_ign_signal.hpp"
#include "fw_mock/mock_pwm_hw.hpp"
#include "pf/vapi/ignition_service.hpp"
#include "pf/vapi/pwm_service.hpp"

namespace
{

using pf::fw::IgnLineLevel;
using pf::vapi::IgnitionService;
using pf::vapi::IgnitionState;
using pf::vapi::PwmService;

// SWE4-TC-101（正常系）: ライン High → IgnitionState::On
TEST(IgnitionServiceTest, TC101_HighLineMapsToOn)
{
    mocks::MockIgnSignal signal{};
    IgnitionService service{signal};

    signal.level = IgnLineLevel::High;
    EXPECT_EQ(service.GetIgnitionState(), IgnitionState::On);
}

// SWE4-TC-102（正常系）: ライン Low → IgnitionState::Off
TEST(IgnitionServiceTest, TC102_LowLineMapsToOff)
{
    mocks::MockIgnSignal signal{};
    IgnitionService service{signal};

    signal.level = IgnLineLevel::Low;
    EXPECT_EQ(service.GetIgnitionState(), IgnitionState::Off);
}

// SWE4-TC-103（異常系）: ライン Fault → IgnitionState::Unknown（フェールセーフ）
TEST(IgnitionServiceTest, TC103_FaultLineMapsToUnknown)
{
    mocks::MockIgnSignal signal{};
    IgnitionService service{signal};

    signal.level = IgnLineLevel::Fault;
    EXPECT_EQ(service.GetIgnitionState(), IgnitionState::Unknown);
}

// SWE4-TC-104（異常系）: 範囲外・NaN デューティ比は棄却し Firmware を呼ばない
TEST(PwmServiceTest, TC104_OutOfRangeDutyRejectedWithoutHwAccess)
{
    mocks::MockPwmHw hw{};
    PwmService service{hw};

    EXPECT_FALSE(service.SetDutyCycle(-0.1F));
    EXPECT_FALSE(service.SetDutyCycle(100.1F));
    EXPECT_FALSE(service.SetDutyCycle(std::numeric_limits<float>::quiet_NaN()));

    // 合否判定条件: Firmware への書き込みが一切発生しないこと
    EXPECT_EQ(hw.callCount, 0);
}

// SWE4-TC-105（正常系）: 値域内デューティ比は Firmware へそのまま委譲される
TEST(PwmServiceTest, TC105_InRangeDutyDelegatedToFirmware)
{
    mocks::MockPwmHw hw{};
    PwmService service{hw};

    EXPECT_TRUE(service.SetDutyCycle(0.0F));
    EXPECT_FLOAT_EQ(hw.lastDuty, 0.0F);

    EXPECT_TRUE(service.SetDutyCycle(50.5F));
    EXPECT_FLOAT_EQ(hw.lastDuty, 50.5F);

    EXPECT_TRUE(service.SetDutyCycle(100.0F));
    EXPECT_FLOAT_EQ(hw.lastDuty, 100.0F);

    EXPECT_EQ(hw.callCount, 3);
}

// SWE4-TC-106（異常系）: Firmware 書き込み失敗は false として伝播する
TEST(PwmServiceTest, TC106_FirmwareFailurePropagates)
{
    mocks::MockPwmHw hw{};
    PwmService service{hw};

    hw.simulateHwFailure = true;
    EXPECT_FALSE(service.SetDutyCycle(50.0F));
    EXPECT_EQ(hw.callCount, 1);
}

}  // namespace
