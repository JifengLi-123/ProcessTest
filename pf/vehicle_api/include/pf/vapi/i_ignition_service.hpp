/**
 * @file i_ignition_service.hpp
 * @brief 車載API: イグニッション（IG）状態サービスインターフェース（SW-IF-002）
 *
 * @details **APP 層に公開する唯一の契約面（17-08 / 17-00-B 対応）の一つ。**
 *          実機では pf::vapi::IgnitionService（内部で Firmware 層の
 *          IG 信号読み取りを使用）が、ユニットテストでは vapi_mock が
 *          本 I/F を実装する。
 *
 *          対応 I/F 仕様: 04-04 §4 SW-IF-002（SW-REQ-201）
 */
#ifndef PROCESSTEST_PF_VAPI_I_IGNITION_SERVICE_HPP
#define PROCESSTEST_PF_VAPI_I_IGNITION_SERVICE_HPP

#include <cstdint>

namespace pf
{
namespace vapi
{

/**
 * @brief イグニッション状態（車両状態としての解釈値）
 *
 * @details Unknown は PF 内部での信号取得失敗（断線・AD 異常等）を表す。
 *          APP は Unknown を IG-OFF と同等に扱うこと（フェールセーフ方針。
 *          04-05 §3.2 決定表参照）。
 */
enum class IgnitionState : std::uint8_t
{
    Off     = 0U,  ///< IG-OFF
    On      = 1U,  ///< IG-ON
    Unknown = 2U   ///< 取得失敗（フェールセーフ対象）
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

}  // namespace vapi
}  // namespace pf

#endif  // PROCESSTEST_PF_VAPI_I_IGNITION_SERVICE_HPP
