/**
 * @file sil_ign_signal.hpp
 * @brief IG 信号読み取りの SIL（Software-in-the-Loop）実装（MOD-202）
 *
 * @details ホスト検証（SWE.5/SWE.6・ecu SIL デモ）用の Firmware 実装。
 *          実機では GPIO/AD 変換読み取り実装に置き換える。
 *          テストシナリオからライン状態を設定できる。
 */
#ifndef PROCESSTEST_PF_FW_SIL_IGN_SIGNAL_HPP
#define PROCESSTEST_PF_FW_SIL_IGN_SIGNAL_HPP

#include "pf/fw/i_ign_signal.hpp"

namespace pf
{
namespace fw
{

/**
 * @brief IG 信号読み取り SIL 実装
 */
class SilIgnSignal final : public IIgnSignal
{
public:
    /**
     * @brief 現在のライン状態を返す（SIL: SetLevel で設定した値）。
     * @return 設定されている IgnLineLevel（デフォルト: High = IG-ON 相当）
     */
    IgnLineLevel ReadIgnLine() override;

    /**
     * @brief ライン状態を設定する（検証用）。
     * @param[in] level 模擬する IgnLineLevel
     */
    void SetLevel(IgnLineLevel level) noexcept;

private:
    IgnLineLevel level_{IgnLineLevel::High};
};

}  // namespace fw
}  // namespace pf

#endif  // PROCESSTEST_PF_FW_SIL_IGN_SIGNAL_HPP
