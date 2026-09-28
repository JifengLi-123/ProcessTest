/**
 * @file i_ignition_service.hpp
 * @brief 車載API: イグニッション（IG）状態サービスインターフェース（PF レイヤー）
 *
 * @details PF レイヤーが APP レイヤーへ提供する車載 API の一つ。
 *          実機では車両状態管理サービス（IG 信号／電源管理 API）を
 *          ラップする実装が注入され、ホスト検証（SIL）では
 *          pf::sil::SilIgnitionService が注入される。
 *
 *          対応 I/F 仕様: 04-04 §4 SW-IF-002（SW-REQ-201）
 */
#ifndef PROCESSTEST_PF_I_IGNITION_SERVICE_HPP
#define PROCESSTEST_PF_I_IGNITION_SERVICE_HPP

#include <cstdint>

namespace pf
{

/**
 * @brief イグニッション状態
 *
 * @details Unknown は PF 内部での信号取得失敗（通信断等）を表す。
 *          APP は Unknown を IG-OFF と同等に扱うこと（フェールセーフ方針。
 *          04-05 §3.2 決定表参照）。
 */
enum class IgnitionState : std::uint8_t
{
    Off     = 0U,  ///< IG-OFF
    On      = 1U,  ///< IG-ON
    Unknown = 2U   ///< 取得失敗（通信断等）
};

/**
 * @brief IG 状態取得サービス（車載 API）
 */
class IIgnitionService
{
public:
    virtual ~IIgnitionService() = default;

    /**
     * @brief 現在のイグニッション状態を取得する。
     *
     * @details 副作用なし（読み取りのみ）。
     *
     * @return 現在の IgnitionState。信号取得に失敗した場合は
     *         IgnitionState::Unknown を返す。
     *
     * @see app::MotorController::applyTargetSpeed
     */
    virtual IgnitionState GetIgnitionState() = 0;

protected:
    IIgnitionService() = default;
    IIgnitionService(const IIgnitionService&) = default;
    IIgnitionService& operator=(const IIgnitionService&) = default;
    IIgnitionService(IIgnitionService&&) = default;
    IIgnitionService& operator=(IIgnitionService&&) = default;
};

}  // namespace pf

#endif  // PROCESSTEST_PF_I_IGNITION_SERVICE_HPP
