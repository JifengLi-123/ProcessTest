/**
 * @file fan_controller.cpp
 * @brief 冷却ファン制御ユニット（FanController）の実装（APP レイヤー）
 */
#include "app/hvac/fan_controller.hpp"

namespace app
{

// C++14 では odr 使用される static constexpr メンバにクラス外定義が必要
constexpr std::uint8_t FanController::FAN_LEVEL_MAX;
constexpr float FanController::DUTY_PER_LEVEL;
constexpr float FanController::STOP_DUTY_PERCENT;

FanController::FanController(pf::vapi::IPwmService& pwmService,
                             pf::vapi::IIgnitionService& ignitionService) noexcept
    : pwmService_{pwmService},
      ignitionService_{ignitionService}
{
}

FanStatus FanController::setFanLevel(std::uint8_t level) noexcept
{
    // ① 入力検証（防御的プログラミング）
    if (level > FAN_LEVEL_MAX)
    {
        return FanStatus::OutOfRange;
    }

    // ② IG 状態確認。Unknown（取得失敗）は Off と同等に扱う（フェールセーフ）
    if (ignitionService_.GetIgnitionState() != pf::vapi::IgnitionState::On)
    {
        // 安全停止はベストエフォート（失敗しても IgnitionOff を優先して返す）
        (void)pwmService_.SetDutyCycle(STOP_DUTY_PERCENT);
        return FanStatus::IgnitionOff;
    }

    // ③ 出力（戻り値 false = HW 異常。必ず処理する）
    const float duty{DUTY_PER_LEVEL * static_cast<float>(level)};
    if (!pwmService_.SetDutyCycle(duty))
    {
        return FanStatus::HwError;
    }

    return FanStatus::Ok;
}

}  // namespace app
