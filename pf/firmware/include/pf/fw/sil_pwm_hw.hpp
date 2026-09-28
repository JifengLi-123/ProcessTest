/**
 * @file sil_pwm_hw.hpp
 * @brief PWM ハードウェアの SIL（Software-in-the-Loop）実装（MOD-201）
 *
 * @details ホスト検証（SWE.5/SWE.6・ecu SIL デモ）用の Firmware 実装。
 *          実機ではタイマ/PWM レジスタ書き込み実装（ターゲットビルド専用
 *          ソース）に置き換える。レジスタ書き込みの代わりに出力履歴を
 *          記録し、故障注入（InjectFailure）で HW 異常系を検証可能にする。
 */
#ifndef PROCESSTEST_PF_FW_SIL_PWM_HW_HPP
#define PROCESSTEST_PF_FW_SIL_PWM_HW_HPP

#include <vector>

#include "pf/fw/i_pwm_hw.hpp"

namespace pf
{
namespace fw
{

/**
 * @brief PWM ハードウェア SIL 実装
 */
class SilPwmHw final : public IPwmHw
{
public:
    /// PWM レジスタが受理するデューティ比の下限 [%]
    static constexpr float HW_DUTY_MIN{0.0F};
    /// PWM レジスタが受理するデューティ比の上限 [%]
    static constexpr float HW_DUTY_MAX{100.0F};

    /**
     * @brief デューティ比を書き込む（SIL: 履歴に記録する）。
     *
     * @param[in] dutyPercent 書き込むデューティ比 [%]
     *                        Range: 0.0f ≤ dutyPercent ≤ 100.0f
     * @return true: 成功 / false: 範囲外・NaN または故障注入中
     */
    bool WriteDuty(float dutyPercent) override;

    /**
     * @brief 最後に書き込まれたデューティ比を取得する（検証用）。
     * @return 最後に受理したデューティ比 [%]。未書き込み時は -1.0f
     */
    float GetLastDuty() const noexcept;

    /**
     * @brief 書き込み履歴を取得する（検証用）。
     * @return 受理された全デューティ比の履歴
     */
    const std::vector<float>& GetHistory() const noexcept;

    /**
     * @brief HW 異常の模擬を設定する（検証用フォールトインジェクション）。
     * @param[in] enable true: 以降の WriteDuty が失敗する
     */
    void InjectFailure(bool enable) noexcept;

private:
    std::vector<float> history_{};
    float lastDuty_{-1.0F};
    bool injectFailure_{false};
};

}  // namespace fw
}  // namespace pf

#endif  // PROCESSTEST_PF_FW_SIL_PWM_HW_HPP
