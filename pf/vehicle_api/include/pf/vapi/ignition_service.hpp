/**
 * @file ignition_service.hpp
 * @brief IG 状態サービスの実装クラス（MOD-003 IgnitionService）
 *
 * @details SW-IF-002 の製品実装。Firmware 層（FW-IF-002 IIgnSignal）が返す
 *          電気的ライン状態を車両状態（IgnitionState）へ解釈する。
 *          本ヘッダは Firmware 型を**前方宣言のみ**で参照する（fw 隠蔽）。
 *          構築（依存の注入）は統合ルート（ecu/）が行う。
 *
 *          対応詳細設計: 04-05-PT01-PFVAPI §3（FUNC-102）
 */
#ifndef PROCESSTEST_PF_VAPI_IGNITION_SERVICE_HPP
#define PROCESSTEST_PF_VAPI_IGNITION_SERVICE_HPP

#include "pf/vapi/i_ignition_service.hpp"

namespace pf
{
namespace fw
{
class IIgnSignal;  // 前方宣言（Firmware ヘッダを APP へ伝播させない）
}

namespace vapi
{

/**
 * @brief IG 状態サービス実装
 *
 * @details ライン状態の解釈規則（04-05-PT01-PFVAPI §3 決定表）:
 *          High → On / Low → Off / Fault → Unknown
 */
class IgnitionService final : public IIgnitionService
{
public:
    /**
     * @brief Firmware の IG 信号読み取り抽象を注入して構築する。
     *
     * @param[in,out] ignSignal IG 信号読み取り抽象（本オブジェクトの
     *                          生存期間中、有効であること。所有権は移動しない）
     */
    explicit IgnitionService(fw::IIgnSignal& ignSignal) noexcept;

    /**
     * @brief 現在のイグニッション状態を取得する。
     *
     * @details Firmware のライン状態を車両状態へ解釈して返す。副作用なし。
     *
     * @return IgnitionState（High→On / Low→Off / Fault→Unknown）
     *
     * @post 戻り値は IgnitionState の定義値のいずれか
     */
    IgnitionState GetIgnitionState() override;

private:
    fw::IIgnSignal& ignSignal_;  ///< Firmware IG 信号読み取り抽象（非所有）
};

}  // namespace vapi
}  // namespace pf

#endif  // PROCESSTEST_PF_VAPI_IGNITION_SERVICE_HPP
