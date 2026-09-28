/**
 * @file main.cpp
 * @brief 統合ルート（Composition Root）— 01-50 Integrated Software のホスト SIL デモ
 *
 * @details 全レイヤー（Firmware SIL → Vehicle API → APP）を合成する唯一の場所。
 *          レイヤー間の依存注入（wiring）はここでのみ行う。
 *          実機ビルドでは Firmware SIL 実装をターゲット HW 実装に置き換え、
 *          RTOS 初期化・周期タスク起動を行う main に置き換わる。
 *
 *          本デモはホスト実行専用のため、標準出力への表示を使用する
 *          （製品コード（pf/app のライブラリ側）には I/O を含めない）。
 */
#include <cstdio>

#include "app/motor_control/motor_controller.hpp"
#include "pf/fw/sil_ign_signal.hpp"
#include "pf/fw/sil_pwm_hw.hpp"
#include "pf/vapi/ignition_service.hpp"
#include "pf/vapi/pwm_service.hpp"

namespace
{

/// ControlStatus を表示用文字列へ変換する（デモ専用）
const char* toString(app::ControlStatus status)
{
    switch (status)
    {
        case app::ControlStatus::Ok:             return "Ok";
        case app::ControlStatus::NotInitialized: return "NotInitialized";
        case app::ControlStatus::OutOfRange:     return "OutOfRange";
        case app::ControlStatus::HwError:        return "HwError";
        case app::ControlStatus::IgnitionOff:    return "IgnitionOff";
        default:                                 return "?";
    }
}

/// 1 コマンド分の要求と結果を表示する（デモ専用）
void command(app::MotorController& controller, pf::fw::SilPwmHw& pwmHw,
             float speedRpm)
{
    const app::ControlStatus result{controller.applyTargetSpeed(speedRpm)};
    std::printf("  applyTargetSpeed(%7.1f rpm) -> %-14s (last duty: %5.1f %%)\n",
                static_cast<double>(speedRpm), toString(result),
                static_cast<double>(pwmHw.GetLastDuty()));
}

}  // namespace

int main()
{
    // --- 統合（wiring）: Firmware SIL → Vehicle API → APP -------------------
    pf::fw::SilPwmHw pwmHw{};
    pf::fw::SilIgnSignal ignSignal{};

    pf::vapi::PwmService pwmService{pwmHw};
    pf::vapi::IgnitionService ignitionService{ignSignal};

    app::MotorController controller{pwmService, ignitionService};

    // --- SIL デモシナリオ ---------------------------------------------------
    std::printf("[1] IG-ON: init & normal control\n");
    (void)controller.init();
    command(controller, pwmHw, 3000.0F);   // Ok: 50.0 %
    command(controller, pwmHw, 6000.0F);   // Ok: 95.0 %

    std::printf("[2] Out-of-range command is rejected\n");
    command(controller, pwmHw, 7000.0F);   // OutOfRange（出力保持）

    std::printf("[3] IG-OFF: safe stop\n");
    ignSignal.SetLevel(pf::fw::IgnLineLevel::Low);
    command(controller, pwmHw, 3000.0F);   // IgnitionOff: 0.0 %

    std::printf("[4] IG-ON again: control resumes\n");
    ignSignal.SetLevel(pf::fw::IgnLineLevel::High);
    command(controller, pwmHw, 1500.0F);   // Ok: 27.5 %

    std::printf("[5] HW failure injection & recovery\n");
    pwmHw.InjectFailure(true);
    command(controller, pwmHw, 1500.0F);   // HwError → Error 状態
    command(controller, pwmHw, 1500.0F);   // NotInitialized（要求拒否）
    pwmHw.InjectFailure(false);
    (void)controller.init();
    command(controller, pwmHw, 1500.0F);   // Ok（復帰）

    return 0;
}
