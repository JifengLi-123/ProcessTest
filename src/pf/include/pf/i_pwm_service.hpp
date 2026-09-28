/**
 * @file i_pwm_service.hpp
 * @brief 車載API: PWM 出力サービスインターフェース（PF レイヤー）
 *
 * @details PF レイヤーが APP レイヤーへ提供する車載 API の一つ。
 *          実機では AUTOSAR Adaptive Platform / ベンダ SDK の
 *          PWM 制御 API をラップする実装（HAL 実装）が注入され、
 *          ホスト検証（SIL）では pf::sil::SilPwmService が注入される。
 *
 *          対応 I/F 仕様: 04-04 §4 SW-IF-001（SW-REQ-200）
 *
 * @note 車載 API（PF 公開 I/F）のメソッド名は PascalCase とし、
 *       APP 内部 API（camelCase）と区別する。
 */
#ifndef PROCESSTEST_PF_I_PWM_SERVICE_HPP
#define PROCESSTEST_PF_I_PWM_SERVICE_HPP

namespace pf
{

/**
 * @brief PWM 出力サービス（車載 API）
 *
 * @details APP からの PWM デューティ比出力要求を HW へ反映する。
 */
class IPwmService
{
public:
    virtual ~IPwmService() = default;

    /**
     * @brief PWM デューティ比を出力する。
     *
     * @details 副作用: 実装クラスにおいて PWM レジスタ／デバイスファイルへの
     *          書き込みが発生する（HW 制御）。
     *
     * @param[in] dutyCyclePercent 出力する PWM デューティ比 [%]
     *                             Range: 0.0f ≤ dutyCyclePercent ≤ 100.0f
     * @return true: 出力成功 / false: HW 異常または範囲外により出力失敗
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

}  // namespace pf

#endif  // PROCESSTEST_PF_I_PWM_SERVICE_HPP
