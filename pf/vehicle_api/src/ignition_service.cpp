/**
 * @file ignition_service.cpp
 * @brief IG 状態サービスの実装（MOD-003）
 *
 * @details 対応詳細設計: 04-05-PT01-PFVAPI §3（FUNC-102）
 */
#include "pf/vapi/ignition_service.hpp"

#include "pf/fw/i_ign_signal.hpp"

namespace pf
{
namespace vapi
{

IgnitionService::IgnitionService(fw::IIgnSignal& ignSignal) noexcept
    : ignSignal_{ignSignal}
{
}

IgnitionState IgnitionService::GetIgnitionState()
{
    // ライン状態 → 車両状態の解釈（決定表: High→On / Low→Off / Fault→Unknown）
    const fw::IgnLineLevel level{ignSignal_.ReadIgnLine()};

    IgnitionState state{IgnitionState::Unknown};
    if (level == fw::IgnLineLevel::High)
    {
        state = IgnitionState::On;
    }
    else if (level == fw::IgnLineLevel::Low)
    {
        state = IgnitionState::Off;
    }
    else
    {
        // Fault および未定義値は Unknown（フェールセーフ側）とする
        state = IgnitionState::Unknown;
    }
    return state;
}

}  // namespace vapi
}  // namespace pf
