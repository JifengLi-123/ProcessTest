/**
 * @file pwm_service.cpp
 * @brief PWM 出力サービスの実装（MOD-002）
 *
 * @details 対応詳細設計: 04-05-PT01-PFVAPI §3（FUNC-101）
 */
#include "pf/vapi/pwm_service.hpp"

#include "pf/fw/i_pwm_hw.hpp"

namespace pf
{
namespace vapi
{

// C++14 では odr 使用される static constexpr メンバにクラス外定義が必要
constexpr float PwmService::API_DUTY_MIN;
constexpr float PwmService::API_DUTY_MAX;

PwmService::PwmService(fw::IPwmHw& pwmHw) noexcept
    : pwmHw_{pwmHw}
{
}

bool PwmService::SetDutyCycle(float dutyCyclePercent)
{
    // API 境界の値域検証（ERR-101）: 否定形比較で NaN も棄却する
    if (!((dutyCyclePercent >= API_DUTY_MIN) && (dutyCyclePercent <= API_DUTY_MAX)))
    {
        return false;
    }

    // Firmware へ委譲（FW-IF-001）。HW 異常は false で伝播する
    return pwmHw_.WriteDuty(dutyCyclePercent);
}

}  // namespace vapi
}  // namespace pf
