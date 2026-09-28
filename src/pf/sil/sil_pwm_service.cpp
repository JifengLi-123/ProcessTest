/**
 * @file sil_pwm_service.cpp
 * @brief PWM 出力サービスの SIL 実装
 */
#include "sil_pwm_service.hpp"

namespace pf
{
namespace sil
{

// C++14 では odr 使用される static constexpr メンバにクラス外定義が必要
constexpr float SilPwmService::HW_DUTY_MIN;
constexpr float SilPwmService::HW_DUTY_MAX;

bool SilPwmService::SetDutyCycle(float dutyCyclePercent)
{
    if (injectFailure_)
    {
        return false;
    }

    // 実機ドライバ相当の値域検証: 範囲外・NaN は HW 異常として拒否する
    if (!((dutyCyclePercent >= HW_DUTY_MIN) && (dutyCyclePercent <= HW_DUTY_MAX)))
    {
        return false;
    }

    lastDutyCycle_ = dutyCyclePercent;
    history_.push_back(dutyCyclePercent);
    return true;
}

float SilPwmService::GetLastDutyCycle() const noexcept
{
    return lastDutyCycle_;
}

const std::vector<float>& SilPwmService::GetHistory() const noexcept
{
    return history_;
}

void SilPwmService::InjectFailure(bool enable) noexcept
{
    injectFailure_ = enable;
}

}  // namespace sil
}  // namespace pf
