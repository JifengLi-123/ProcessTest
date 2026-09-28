/**
 * @file mock_ignition_service.hpp
 * @brief 車載API SW-IF-002（IIgnitionService）のユニットテスト用モック
 *
 * @details APP 各チームの SWE.4 はこのモックで完結する（Firmware 不要）。
 *          本ディレクトリ（vapi_mock）は PF チームがオーナーとして管理する。
 */
#ifndef PROCESSTEST_TEST_VAPI_MOCK_IGNITION_SERVICE_HPP
#define PROCESSTEST_TEST_VAPI_MOCK_IGNITION_SERVICE_HPP

#include "pf/vapi/i_ignition_service.hpp"

namespace mocks
{

/// IG 状態サービスモック（状態を自在に設定可能）
class MockIgnitionService final : public pf::vapi::IIgnitionService
{
public:
    pf::vapi::IgnitionState GetIgnitionState() override
    {
        return state;
    }

    pf::vapi::IgnitionState state{pf::vapi::IgnitionState::On};  ///< 模擬する IG 状態
};

}  // namespace mocks

#endif  // PROCESSTEST_TEST_VAPI_MOCK_IGNITION_SERVICE_HPP
