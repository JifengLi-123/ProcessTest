/**
 * @file mock_ign_signal.hpp
 * @brief Firmware FW-IF-002（IIgnSignal）のユニットテスト用モック
 *
 * @details Vehicle API 層自体の SWE.4（pf_vapi のユニットテスト）に使用する。
 */
#ifndef PROCESSTEST_TEST_FW_MOCK_IGN_SIGNAL_HPP
#define PROCESSTEST_TEST_FW_MOCK_IGN_SIGNAL_HPP

#include "pf/fw/i_ign_signal.hpp"

namespace mocks
{

/// IG 信号読み取りモック（ライン状態を自在に設定可能）
class MockIgnSignal final : public pf::fw::IIgnSignal
{
public:
    pf::fw::IgnLineLevel ReadIgnLine() override
    {
        return level;
    }

    pf::fw::IgnLineLevel level{pf::fw::IgnLineLevel::High};  ///< 模擬するライン状態
};

}  // namespace mocks

#endif  // PROCESSTEST_TEST_FW_MOCK_IGN_SIGNAL_HPP
