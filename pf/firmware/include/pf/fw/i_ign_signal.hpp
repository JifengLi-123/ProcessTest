/**
 * @file i_ign_signal.hpp
 * @brief Firmware 内部I/F: IG 信号ライン読み取り抽象（FW-IF-002）
 *
 * @details PF Firmware 層が Vehicle API 層にのみ公開する低レベル I/F。
 *          **APP 層からの include は禁止**（04-04 アーキテクチャ制約）。
 *          Firmware 層は電気的なライン状態（Low/High/Fault）を扱い、
 *          車両状態（IG-ON/OFF）への解釈は Vehicle API 層
 *          （pf::vapi::IgnitionService）が行う。
 */
#ifndef PROCESSTEST_PF_FW_I_IGN_SIGNAL_HPP
#define PROCESSTEST_PF_FW_I_IGN_SIGNAL_HPP

#include <cstdint>

namespace pf
{
namespace fw
{

/**
 * @brief IG 信号ラインの電気的状態
 */
enum class IgnLineLevel : std::uint8_t
{
    Low   = 0U,  ///< ライン Low（IG-OFF 相当）
    High  = 1U,  ///< ライン High（IG-ON 相当）
    Fault = 2U   ///< 読み取り失敗（断線・AD 変換異常等）
};

/**
 * @brief IG 信号読み取りインターフェース
 */
class IIgnSignal
{
public:
    virtual ~IIgnSignal() = default;

    /**
     * @brief IG 信号ラインの現在の電気的状態を読み取る。
     *
     * @details 副作用なし（読み取りのみ）。
     *
     * @return 現在の IgnLineLevel。読み取りに失敗した場合は Fault を返す。
     *
     * @see pf::vapi::IgnitionService
     */
    virtual IgnLineLevel ReadIgnLine() = 0;

protected:
    IIgnSignal() = default;
    IIgnSignal(const IIgnSignal&) = default;
    IIgnSignal& operator=(const IIgnSignal&) = default;
    IIgnSignal(IIgnSignal&&) = default;
    IIgnSignal& operator=(IIgnSignal&&) = default;
};

}  // namespace fw
}  // namespace pf

#endif  // PROCESSTEST_PF_FW_I_IGN_SIGNAL_HPP
