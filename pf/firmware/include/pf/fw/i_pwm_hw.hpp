/**
 * @file i_pwm_hw.hpp
 * @brief Firmware 内部I/F: PWM ハードウェア抽象（FW-IF-001）
 *
 * @details PF Firmware 層が Vehicle API 層にのみ公開する低レベル I/F。
 *          **APP 層からの include は禁止**（04-04 アーキテクチャ制約。
 *          CMake の PRIVATE リンクと scripts/check_layer_deps.sh で強制）。
 *          実機ではタイマ/PWM レジスタへの書き込み実装、ホスト検証（SIL）では
 *          pf::fw::SilPwmHw が本 I/F を実装する。
 */
#ifndef PROCESSTEST_PF_FW_I_PWM_HW_HPP
#define PROCESSTEST_PF_FW_I_PWM_HW_HPP

namespace pf
{
namespace fw
{

/**
 * @brief PWM ハードウェア抽象インターフェース
 */
class IPwmHw
{
public:
    virtual ~IPwmHw() = default;

    /**
     * @brief PWM デューティ比をハードウェアレジスタへ書き込む。
     *
     * @details 副作用: 実装クラスにおいて PWM レジスタ書き込みが発生する。
     *
     * @param[in] dutyPercent 書き込むデューティ比 [%]
     *                        Range: 0.0f ≤ dutyPercent ≤ 100.0f
     * @return true: 書き込み成功 / false: HW 異常または範囲外
     *
     * @see pf::vapi::PwmService
     */
    virtual bool WriteDuty(float dutyPercent) = 0;

protected:
    IPwmHw() = default;
    IPwmHw(const IPwmHw&) = default;
    IPwmHw& operator=(const IPwmHw&) = default;
    IPwmHw(IPwmHw&&) = default;
    IPwmHw& operator=(IPwmHw&&) = default;
};

}  // namespace fw
}  // namespace pf

#endif  // PROCESSTEST_PF_FW_I_PWM_HW_HPP
