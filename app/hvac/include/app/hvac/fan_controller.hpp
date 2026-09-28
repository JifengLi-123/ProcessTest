/**
 * @file fan_controller.hpp
 * @brief 冷却ファン制御ユニット（FanController）のクラス定義（APP レイヤー）
 *
 * @details 選択ビルド（BUILD_APPS）と SOTA（APP 単位の .so 差し替え）を実演する
 *          ための 2 つ目の APP サンプル。ファンレベル（0〜3）を PWM デューティ比
 *          （0/25/50/75 %）に変換し、IG-ON 時のみ車載 API 経由で出力する。
 *
 * @note 本 APP は構成デモ用の簡易サンプルであり、正式なプロセス成果物
 *       （04-05 詳細設計書・08-50 テスト仕様書等）の見本は app/motor_control を
 *       参照すること。依存規則は同一: 契約面（pf/vapi の i_*.hpp）のみ include 可。
 */
#ifndef PROCESSTEST_APP_HVAC_FAN_CONTROLLER_HPP
#define PROCESSTEST_APP_HVAC_FAN_CONTROLLER_HPP

#include <cstdint>

#include "pf/vapi/i_ignition_service.hpp"
#include "pf/vapi/i_pwm_service.hpp"

namespace app
{

/// ファン制御 API の戻り値（エラーコード）
enum class FanStatus : std::uint8_t
{
    Ok          = 0U,  ///< 正常終了
    OutOfRange  = 1U,  ///< ファンレベルが範囲外
    HwError     = 2U,  ///< PWM 出力サービス失敗
    IgnitionOff = 3U   ///< IG-OFF/取得失敗により出力を安全停止
};

/**
 * @brief ファンレベルを PWM デューティ比へ変換し車載 API 経由で出力する制御クラス
 */
class FanController final
{
public:
    /// ファンレベルの上限
    static constexpr std::uint8_t FAN_LEVEL_MAX{3U};
    /// レベル 1 あたりのデューティ比増分 [%]
    static constexpr float DUTY_PER_LEVEL{25.0F};
    /// 安全停止時に出力するデューティ比 [%]
    static constexpr float STOP_DUTY_PERCENT{0.0F};

    /**
     * @brief 車載 API（PWM 出力・IG 状態サービス）を注入して構築する。
     *
     * @param[in,out] pwmService      PWM 出力サービス（非所有。生存期間は統合ルートが保証）
     * @param[in,out] ignitionService IG 状態サービス（同上）
     */
    FanController(pf::vapi::IPwmService& pwmService,
                  pf::vapi::IIgnitionService& ignitionService) noexcept;

    /**
     * @brief ファンレベルに応じた PWM デューティ比を出力する。
     *
     * @param[in] level ファンレベル
     *                  Range: 0 ≤ level ≤ FAN_LEVEL_MAX
     * @return FanStatus::Ok / OutOfRange / HwError / IgnitionOff
     *
     * @pre  level <= FAN_LEVEL_MAX
     * @post Ok 返却時、level × DUTY_PER_LEVEL [%] が出力されている
     * @post IgnitionOff 返却時、STOP_DUTY_PERCENT が出力されている（ベストエフォート）
     */
    FanStatus setFanLevel(std::uint8_t level) noexcept;

private:
    pf::vapi::IPwmService& pwmService_;            ///< 車載 API: PWM 出力サービス（非所有）
    pf::vapi::IIgnitionService& ignitionService_;  ///< 車載 API: IG 状態サービス（非所有）
};

}  // namespace app

#endif  // PROCESSTEST_APP_HVAC_FAN_CONTROLLER_HPP
