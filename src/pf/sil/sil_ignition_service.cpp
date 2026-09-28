/**
 * @file sil_ignition_service.cpp
 * @brief IG 状態サービスの SIL 実装
 */
#include "sil_ignition_service.hpp"

namespace pf
{
namespace sil
{

IgnitionState SilIgnitionService::GetIgnitionState()
{
    return state_;
}

void SilIgnitionService::SetIgnitionState(IgnitionState state) noexcept
{
    state_ = state;
}

}  // namespace sil
}  // namespace pf
