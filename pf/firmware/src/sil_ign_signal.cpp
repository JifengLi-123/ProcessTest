/**
 * @file sil_ign_signal.cpp
 * @brief IG 信号読み取りの SIL 実装
 */
#include "pf/fw/sil_ign_signal.hpp"

namespace pf
{
namespace fw
{

IgnLineLevel SilIgnSignal::ReadIgnLine()
{
    return level_;
}

void SilIgnSignal::SetLevel(IgnLineLevel level) noexcept
{
    level_ = level;
}

}  // namespace fw
}  // namespace pf
