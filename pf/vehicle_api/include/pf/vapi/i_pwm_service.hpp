/**
 * @file i_pwm_service.hpp
 * @brief 車載API: PWM 出力サービスインターフェース（SW-IF-001）
 *
 * @details **APP 層に公開する唯一の契約面（17-08 / 17-00-B 対応）の一つ。**
 *          PF Vehicle API 層が提供し、APP はこの抽象 I/F のみに依存する。
 *          実機では pf::vapi::PwmService（内部で Firmware 層を使用）が、
 *          ユニットテストでは vapi_mock が本 I/F を実装する。
 *
 *          対応 I/F 仕様: 04-04 §4 SW-IF-001（SW-REQ-200）
 *
 * @note 車載 API（PF 公開 I/F）のメソッド名は PascalCase とし、
 *       APP 内部 API（camelCase）と区別する。
 */
#ifndef PROCESSTEST_PF_VAPI_I_PWM_SERVICE_HPP
#define PROCESSTEST_PF_VAPI_I_PWM_SERVICE_HPP

namespace pf
{
namespace vapi
{

/**
 * @brief PWM 出力サービス（車載 API）
 */
class IPwmService
{
public:
    virtual ~IPwmService() = default;

    /**
     * @brief PWM デューティ比を出力する。
     *
     * @details 副作用: 実装クラスにおいて Firmware 経由の PWM レジスタ
     *          書き込みが発生する（HW 制御）。
     *
     * @param[in] dutyCyclePercent 出力する PWM デューティ比 [%]
     *                             Range: 0.0f ≤ dutyCyclePercent ≤ 100.0f
     * @return true: 出力成功 / false: 範囲外入力または HW 異常
     *
     * @pre  dutyCyclePercent が 0.0f〜100.0f の範囲内であること
     * @post true 返却時、指定デューティ比が HW へ反映されている
     *
     * @see app::MotorController::applyTargetSpeed
     */
    virtual bool SetDutyCycle(float dutyCyclePercent) = 0;

protected:
    IPwmService() = default;
    IPwmService(const IPwmService&) = default;
    IPwmService& operator=(const IPwmService&) = default;
    IPwmService(IPwmService&&) = default;
    IPwmService& operator=(IPwmService&&) = default;
};

}  // namespace vapi
}  // namespace pf

#endif  // PROCESSTEST_PF_VAPI_I_PWM_SERVICE_HPP
