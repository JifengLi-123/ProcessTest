/**
 * @file sil_pwm_hw.cpp
 * @brief PWM ハードウェアの SIL 実装
 */
#include "pf/fw/sil_pwm_hw.hpp"

namespace pf
{
namespace fw
{

// C++14 では odr 使用される static constexpr メンバにクラス外定義が必要
constexpr float SilPwmHw::HW_DUTY_MIN;
constexpr float SilPwmHw::HW_DUTY_MAX;

bool SilPwmHw::WriteDuty(float dutyPercent)
{
    if (injectFailure_)
    {
        return false;
    }

    // 実機レジスタ相当の値域検証: 範囲外・NaN は書き込み失敗とする
    if (!((dutyPercent >= HW_DUTY_MIN) && (dutyPercent <= HW_DUTY_MAX)))
    {
        return false;
    }

    lastDuty_ = dutyPercent;
    history_.push_back(dutyPercent);
    return true;
}

float SilPwmHw::GetLastDuty() const noexcept
{
    return lastDuty_;
}

const std::vector<float>& SilPwmHw::GetHistory() const noexcept
{
    return history_;
}

void SilPwmHw::InjectFailure(bool enable) noexcept
{
    injectFailure_ = enable;
}

}  // namespace fw
}  // namespace pf
