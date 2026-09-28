/**
 * @file pwm_service.hpp
 * @brief PWM 出力サービスの実装クラス（MOD-002 PwmService）
 *
 * @details SW-IF-001 の製品実装。Firmware 層（FW-IF-001 IPwmHw）へ委譲する。
 *          本ヘッダは Firmware 型を**前方宣言のみ**で参照するため、
 *          APP 層が本ヘッダを include しても Firmware ヘッダには到達しない
 *          （fw 隠蔽。04-04 アーキテクチャ制約）。
 *          構築（依存の注入）は統合ルート（ecu/）が行う。
 *
 *          対応詳細設計: 04-05-PT01-PFVAPI §3（FUNC-101）
 */
#ifndef PROCESSTEST_PF_VAPI_PWM_SERVICE_HPP
#define PROCESSTEST_PF_VAPI_PWM_SERVICE_HPP

#include "pf/vapi/i_pwm_service.hpp"

namespace pf
{
namespace fw
{
class IPwmHw;  // 前方宣言（Firmware ヘッダを APP へ伝播させない）
}

namespace vapi
{

/**
 * @brief PWM 出力サービス実装
 *
 * @details API 境界での入力値域検証（防御的プログラミング）を行ったうえで
 *          Firmware へ委譲する。動的メモリ確保なし。
 */
class PwmService final : public IPwmService
{
public:
    /// 車載 API として受理するデューティ比の下限 [%]（CONST-101）
    static constexpr float API_DUTY_MIN{0.0F};
    /// 車載 API として受理するデューティ比の上限 [%]（CONST-102）
    static constexpr float API_DUTY_MAX{100.0F};

    /**
     * @brief Firmware の PWM ハードウェア抽象を注入して構築する。
     *
     * @param[in,out] pwmHw PWM ハードウェア抽象（本オブジェクトの生存期間中、
     *                      有効であること。所有権は移動しない）
     */
    explicit PwmService(fw::IPwmHw& pwmHw) noexcept;

    /**
     * @brief PWM デューティ比を出力する。
     *
     * @details 値域検証後、Firmware（FW-IF-001）へ委譲する。
     *          否定形の比較により NaN も棄却する（防御的プログラミング）。
     *
     * @param[in] dutyCyclePercent 出力する PWM デューティ比 [%]
     *                             Range: 0.0f ≤ dutyCyclePercent ≤ 100.0f
     * @return true: 出力成功 / false: 範囲外・NaN または HW 異常
     *
     * @pre  dutyCyclePercent が [API_DUTY_MIN, API_DUTY_MAX] の範囲内
     * @post false 返却時（範囲外）、HW への書き込みは発生しない
     */
    bool SetDutyCycle(float dutyCyclePercent) override;

private:
    fw::IPwmHw& pwmHw_;  ///< Firmware PWM ハードウェア抽象（非所有）
};

}  // namespace vapi
}  // namespace pf

#endif  // PROCESSTEST_PF_VAPI_PWM_SERVICE_HPP
