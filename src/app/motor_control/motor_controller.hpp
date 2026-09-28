/**
 * @file motor_controller.hpp
 * @brief モータ制御ユニット（MOD-001 MotorController）のクラス定義（APP レイヤー）
 *
 * @details 目標回転数 [rpm] を PWM デューティ比 [%] に線形変換し、
 *          PF レイヤーの車載 API（IPwmService）経由で出力する。
 *          IG 状態は車載 API（IIgnitionService）で確認し、IG-OFF 時は
 *          出力を安全停止する（SW-REQ-PWR-001）。
 *          AUTOSAR C++14 Guidelines 準拠（例外不使用・動的メモリ確保なし）。
 *
 *          対応詳細設計: 04-05 §1〜§4（docs/swe3/04-05-sw-detailed-design.md）
 *          割り当て SW 要求: SW-REQ-001〜005, SW-REQ-PWR-001
 *          使用車載 API: SW-IF-001（IPwmService）/ SW-IF-002（IIgnitionService）
 */
#ifndef PROCESSTEST_APP_MOTOR_CONTROLLER_HPP
#define PROCESSTEST_APP_MOTOR_CONTROLLER_HPP

#include <cstdint>

#include "pf/i_ignition_service.hpp"
#include "pf/i_pwm_service.hpp"

namespace app
{

/**
 * @brief モータ制御 API の戻り値（エラーコード）
 *
 * @details 04-05 §3.3 エラー処理設計に対応する。例外は送出しない方針
 *          （AUTOSAR A15-0-1: 例外は真に例外的な状況のみ）のため、
 *          すべての異常はエラーコードで呼び出し側へ通知する。
 */
enum class ControlStatus : std::uint8_t
{
    Ok             = 0U,  ///< 正常終了
    NotInitialized = 1U,  ///< 未初期化状態での操作要求（ERR-002）
    OutOfRange     = 2U,  ///< 入力値が有効範囲外（ERR-001）
    HwError        = 3U,  ///< PWM 出力サービス失敗（ERR-003）
    IgnitionOff    = 4U   ///< IG-OFF/取得失敗により出力を安全停止（ERR-004）
};

/**
 * @brief モータ制御ユニットの内部状態
 *
 * @details 04-05 §4 状態機械設計に対応する。
 */
enum class MotorState : std::uint8_t
{
    Uninitialized = 0U,  ///< 初期状態（init() 未実行）
    Ready         = 1U,  ///< 初期化完了・出力停止中
    Running       = 2U,  ///< PWM 出力中
    Error         = 3U   ///< HW 異常検出（init() による復帰待ち）
};

/**
 * @brief モータ目標回転数を PWM デューティ比へ変換し車載 API 経由で出力する制御クラス
 *
 * @details クラス不変条件（04-05 §3.2 参照）:
 *          - lastCommandedSpeed_ は常に [0.0f, MOTOR_MAX_SPEED_RPM] の範囲内
 *          - state_ は MotorState の定義値のいずれか
 *
 *          依存する車載 API（PWM 出力サービス・IG 状態サービス）は
 *          コンストラクタで注入する（依存性注入。テスト時はモックへ差し替え）。
 *          本クラスは動的メモリ確保を行わない（AUTOSAR A18-5-1/A18-5-2）。
 *
 * @invariant lastCommandedSpeed_ ∈ [0.0f, MOTOR_MAX_SPEED_RPM]
 * @invariant state_ ∈ {Uninitialized, Ready, Running, Error}
 *
 * @see pf::IPwmService
 * @see pf::IIgnitionService
 */
class MotorController final
{
public:
    /// 目標回転数の上限 [rpm]（SW-REQ-001。CONST-001）
    static constexpr float MOTOR_MAX_SPEED_RPM{6000.0F};
    /// PWM デューティ比の下限 [%]（SW-REQ-001。CONST-002）
    static constexpr float DUTY_MIN{5.0F};
    /// PWM デューティ比の上限 [%]（SW-REQ-001。CONST-003）
    static constexpr float DUTY_MAX{95.0F};
    /// 安全停止時に出力するデューティ比 [%]（SW-REQ-PWR-001。CONST-004）
    static constexpr float STOP_DUTY_PERCENT{0.0F};

    /**
     * @brief 車載 API（PWM 出力・IG 状態サービス）を注入してコントローラを構築する。
     *
     * @details 構築直後の状態は Uninitialized であり、init() が呼ばれるまで
     *          制御 API（calculateDutyCycle / applyTargetSpeed）は
     *          NotInitialized を返す。副作用なし。
     *
     * @param[in,out] pwmService      PWM 出力サービス（本オブジェクトの生存期間中、
     *                                有効であること。所有権は移動しない）
     * @param[in,out] ignitionService IG 状態サービス（同上）
     */
    MotorController(pf::IPwmService& pwmService,
                    pf::IIgnitionService& ignitionService) noexcept;

    /**
     * @brief コントローラを初期化し Ready 状態へ遷移する。
     *
     * @details Uninitialized または Error 状態から Ready 状態へ遷移させる。
     *          Error 状態からの復帰経路を兼ねる（04-05 §4）。
     *          副作用: state_ の更新、lastCommandedSpeed_ の 0.0f クリア。
     *
     * @return ControlStatus::Ok（本サンプルでは常に成功）
     *
     * @pre  なし（任意の状態から呼び出し可能）
     * @post getState() == MotorState::Ready
     * @post getLastCommandedSpeed() == 0.0f
     */
    ControlStatus init() noexcept;

    /**
     * @brief モータ目標回転数に対する PWM デューティ比を算出する。
     *
     * @details 線形変換式（04-05 §3.2 FUNC-002）:
     *          dutyCycle = DUTY_MIN
     *                    + (inputSpeed / MOTOR_MAX_SPEED_RPM) * (DUTY_MAX - DUTY_MIN)
     *          前提条件違反時はエラーコードを返し、dutyCycle および内部状態を
     *          変更しない（ASIL-B 方針: エラーコード返却で呼び出し側に回復を委ねる）。
     *          NaN 入力は範囲チェックで OutOfRange として棄却される（防御的プログラミング）。
     *          IG 状態の確認は行わない（出力を伴わない純粋な算出処理のため）。
     *          副作用: 正常時のみ lastCommandedSpeed_ を更新する。HW 出力は行わない。
     *
     * @param[in]  inputSpeed 目標回転数 [rpm]
     *                        Range: 0.0f ≤ inputSpeed ≤ MOTOR_MAX_SPEED_RPM
     * @param[out] dutyCycle  算出された PWM デューティ比 [%]
     *                        Range: DUTY_MIN ≤ dutyCycle ≤ DUTY_MAX（正常終了時）
     * @return ControlStatus::Ok             正常終了
     * @return ControlStatus::NotInitialized 未初期化（Uninitialized / Error 状態）
     * @return ControlStatus::OutOfRange     inputSpeed が範囲外または NaN
     *
     * @pre  init() が完了していること（getState() が Ready または Running）
     * @pre  inputSpeed >= 0.0f
     * @pre  inputSpeed <= MOTOR_MAX_SPEED_RPM
     * @post 正常終了時、dutyCycle は [DUTY_MIN, DUTY_MAX] の範囲内
     * @post 正常終了時、getLastCommandedSpeed() == inputSpeed
     * @post エラー時、dutyCycle と内部状態は変化しない
     * @post 本関数呼び出し前後でヒープ使用量は変化しない
     *
     * @see applyTargetSpeed
     */
    ControlStatus calculateDutyCycle(float inputSpeed, float& dutyCycle) noexcept;

    /**
     * @brief 目標回転数からデューティ比を算出し、IG 状態を確認のうえ
     *        車載 API（PWM 出力サービス）経由で出力する。
     *
     * @details 処理順序（04-05 §3.2 FUNC-003）:
     *          1. calculateDutyCycle() でデューティ比を算出（未初期化・範囲チェック込み）
     *          2. IIgnitionService::GetIgnitionState() で IG 状態を確認
     *             - IG-OFF / Unknown（取得失敗）の場合: STOP_DUTY_PERCENT を出力して
     *               安全停止し Ready 状態へ遷移、IgnitionOff を返す（SW-REQ-PWR-001。
     *               Unknown を Off と同等に扱うのはフェールセーフ方針）
     *          3. IPwmService::SetDutyCycle() で算出値を出力
     *             - 出力失敗時: Error 状態へ遷移し HwError を返す（ERR-003）
     *          4. 成功時: Running 状態へ遷移
     *          Error 状態からは init() でのみ復帰できる。
     *          副作用: state_ / lastCommandedSpeed_ の更新、PWM HW への出力。
     *
     * @param[in] inputSpeed 目標回転数 [rpm]
     *                       Range: 0.0f ≤ inputSpeed ≤ MOTOR_MAX_SPEED_RPM
     * @return ControlStatus::Ok             正常終了（PWM 出力完了）
     * @return ControlStatus::NotInitialized 未初期化（Uninitialized / Error 状態）
     * @return ControlStatus::OutOfRange     inputSpeed が範囲外または NaN
     * @return ControlStatus::HwError        PWM 出力サービス失敗
     * @return ControlStatus::IgnitionOff    IG-OFF/取得失敗により安全停止を実施
     *
     * @pre  init() が完了していること（getState() が Ready または Running）
     * @pre  inputSpeed >= 0.0f
     * @pre  inputSpeed <= MOTOR_MAX_SPEED_RPM
     * @post 正常終了時、getState() == MotorState::Running
     * @post IG-OFF 時、getState() == MotorState::Ready かつ
     *       getLastCommandedSpeed() == 0.0f（安全停止済み）
     * @post HW 異常時、getState() == MotorState::Error
     *
     * @see calculateDutyCycle
     * @see pf::IPwmService::SetDutyCycle
     * @see pf::IIgnitionService::GetIgnitionState
     */
    ControlStatus applyTargetSpeed(float inputSpeed) noexcept;

    /**
     * @brief 現在の内部状態を取得する。
     * @return 現在の MotorState。副作用なし。
     */
    MotorState getState() const noexcept;

    /**
     * @brief 最後に正常受理した目標回転数を取得する。
     * @return 最後に正常受理した目標回転数 [rpm]
     *         Range: 0.0f ≤ 戻り値 ≤ MOTOR_MAX_SPEED_RPM。副作用なし。
     */
    float getLastCommandedSpeed() const noexcept;

private:
    pf::IPwmService& pwmService_;            ///< 車載 API: PWM 出力サービス（非所有）
    pf::IIgnitionService& ignitionService_;  ///< 車載 API: IG 状態サービス（非所有）
    MotorState state_;                       ///< 現在の状態（04-05 §4）
    float lastCommandedSpeed_;               ///< 最後に正常受理した目標回転数 [rpm]
};

}  // namespace app

#endif  // PROCESSTEST_APP_MOTOR_CONTROLLER_HPP
