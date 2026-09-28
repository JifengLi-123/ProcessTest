/**
 * @file vapi_version.hpp
 * @brief 車載 API（契約面）のバージョン定義
 *
 * @details SOTA（Software OTA）の互換性判定に使用する。
 *          - MAJOR: 契約の互換性を破壊する変更（I/F 削除・シグネチャ変更・
 *                   enum 値の意味変更等）でインクリメントする。
 *                   APP 共有ライブラリの SOVERSION と一致させる
 *          - MINOR: 後方互換のある追加（I/F 追加等）でインクリメントする
 *          値は cmake/AppModule.cmake の PF_VAPI_VERSION_* と一致させること。
 *          契約変更は 04-04 §4 の改訂（SWE.2）として管理する。
 */
#ifndef PROCESSTEST_PF_VAPI_VERSION_HPP
#define PROCESSTEST_PF_VAPI_VERSION_HPP

#include <cstdint>

namespace pf
{
namespace vapi
{

/// 車載 API 契約メジャーバージョン（互換性破壊で +1。= APP .so の SOVERSION）
constexpr std::uint8_t VAPI_VERSION_MAJOR{1U};
/// 車載 API 契約マイナーバージョン（後方互換の追加で +1）
constexpr std::uint8_t VAPI_VERSION_MINOR{0U};

}  // namespace vapi
}  // namespace pf

#endif  // PROCESSTEST_PF_VAPI_VERSION_HPP
