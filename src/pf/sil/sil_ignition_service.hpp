/**
 * @file sil_ignition_service.hpp
 * @brief IG 状態サービスの SIL（Software-in-the-Loop）実装
 *
 * @details ホスト環境での統合検証（SWE.5）・適格性テスト（SWE.6）用の
 *          PF レイヤー実装。テストシナリオから IG 状態を設定できる。
 *          実機ビルドでは車両状態管理サービスをラップする実装に置き換える。
 *
 *          対応 I/F 仕様: 04-04 §4 SW-IF-002
 */
#ifndef PROCESSTEST_PF_SIL_IGNITION_SERVICE_HPP
#define PROCESSTEST_PF_SIL_IGNITION_SERVICE_HPP

#include "pf/i_ignition_service.hpp"

namespace pf
{
namespace sil
{

/**
 * @brief IG 状態サービス SIL 実装
 */
class SilIgnitionService final : public IIgnitionService
{
public:
    /**
     * @brief 現在の IG 状態を返す（SIL: SetIgnitionState で設定した値）。
     * @return 設定されている IgnitionState（デフォルト: On）
     */
    IgnitionState GetIgnitionState() override;

    /**
     * @brief IG 状態を設定する（検証用）。
     * @param[in] state 模擬する IgnitionState
     */
    void SetIgnitionState(IgnitionState state) noexcept;

private:
    IgnitionState state_{IgnitionState::On};
};

}  // namespace sil
}  // namespace pf

#endif  // PROCESSTEST_PF_SIL_IGNITION_SERVICE_HPP
