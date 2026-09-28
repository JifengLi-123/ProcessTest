/**
 * @file sil_pwm_service.hpp
 * @brief PWM 出力サービスの SIL（Software-in-the-Loop）実装
 *
 * @details ホスト環境での統合検証（SWE.5）・適格性テスト（SWE.6）用の
 *          PF レイヤー実装。実機ドライバと同一の入力値域検証を行い、
 *          HW レジスタ書き込みの代わりに出力履歴を記録する。
 *          故障注入機能（injectFailure）によりHW異常系の検証を可能にする。
 *          実機ビルドでは本実装を HAL 実装に置き換える。
 *
 *          対応 I/F 仕様: 04-04 §4 SW-IF-001
 */
#ifndef PROCESSTEST_PF_SIL_PWM_SERVICE_HPP
#define PROCESSTEST_PF_SIL_PWM_SERVICE_HPP

#include <vector>

#include "pf/i_pwm_service.hpp"

namespace pf
{
namespace sil
{

/**
 * @brief PWM 出力サービス SIL 実装
 */
class SilPwmService final : public IPwmService
{
public:
    /// 実機 PWM HW が受理するデューティ比の下限 [%]
    static constexpr float HW_DUTY_MIN{0.0F};
    /// 実機 PWM HW が受理するデューティ比の上限 [%]
    static constexpr float HW_DUTY_MAX{100.0F};

    /**
     * @brief PWM デューティ比を出力する（SIL: 履歴に記録する）。
     *
     * @param[in] dutyCyclePercent 出力する PWM デューティ比 [%]
     *                             Range: 0.0f ≤ dutyCyclePercent ≤ 100.0f
     * @return true: 出力成功 / false: 範囲外または故障注入中
     */
    bool SetDutyCycle(float dutyCyclePercent) override;

    /**
     * @brief 最後に出力されたデューティ比を取得する（検証用）。
     * @return 最後に受理したデューティ比 [%]。未出力時は -1.0f
     */
    float GetLastDutyCycle() const noexcept;

    /**
     * @brief 出力履歴を取得する（検証用）。
     * @return 受理された全デューティ比の履歴
     */
    const std::vector<float>& GetHistory() const noexcept;

    /**
     * @brief HW 異常の模擬を設定する（検証用フォールトインジェクション）。
     * @param[in] enable true: 以降の SetDutyCycle が失敗する
     */
    void InjectFailure(bool enable) noexcept;

private:
    std::vector<float> history_{};
    float lastDutyCycle_{-1.0F};
    bool injectFailure_{false};
};

}  // namespace sil
}  // namespace pf

#endif  // PROCESSTEST_PF_SIL_PWM_SERVICE_HPP
