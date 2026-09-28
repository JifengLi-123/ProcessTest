/**
 * @file motor_controller.cpp
 * @brief モータ制御ユニット（MOD-001 MotorController）の実装（APP レイヤー）
 *
 * @details 対応詳細設計: 04-05-PT01-MOD001 §3（関数設計）・§4（状態機械設計）
 *          サイクロマティック複雑度: 全関数 10 以下（SWE.3 Step 4.4）
 */
#include "app/motor_control/motor_controller.hpp"

namespace app
{

// C++14 では odr 使用される static constexpr メンバにクラス外定義が必要（A3-3-2 関連）
constexpr float MotorController::MOTOR_MAX_SPEED_RPM;
constexpr float MotorController::DUTY_MIN;
constexpr float MotorController::DUTY_MAX;
constexpr float MotorController::STOP_DUTY_PERCENT;

MotorController::MotorController(pf::vapi::IPwmService& pwmService,
                                 pf::vapi::IIgnitionService& ignitionService) noexcept
    : pwmService_{pwmService},
      ignitionService_{ignitionService},
      state_{MotorState::Uninitialized},
      lastCommandedSpeed_{0.0F}
{
}

ControlStatus MotorController::init() noexcept
{
    state_ = MotorState::Ready;
    lastCommandedSpeed_ = 0.0F;
    return ControlStatus::Ok;
}

ControlStatus MotorController::calculateDutyCycle(float inputSpeed,
                                                  float& dutyCycle) noexcept
{
    // 前提条件チェック 1: 初期化済みであること（ERR-002）
    if ((state_ == MotorState::Uninitialized) || (state_ == MotorState::Error))
    {
        return ControlStatus::NotInitialized;
    }

    // 前提条件チェック 2: 入力値域（ERR-001）
    // 否定形の比較により NaN 入力も OutOfRange として棄却する（防御的プログラミング）
    if (!((inputSpeed >= 0.0F) && (inputSpeed <= MOTOR_MAX_SPEED_RPM)))
    {
        return ControlStatus::OutOfRange;
    }

    // 線形変換（FUNC-002）: [0, MAX_SPEED] → [DUTY_MIN, DUTY_MAX]
    const float speedRatio{inputSpeed / MOTOR_MAX_SPEED_RPM};
    dutyCycle = DUTY_MIN + (speedRatio * (DUTY_MAX - DUTY_MIN));

    // 後条件: 正常受理した回転数を記録する
    lastCommandedSpeed_ = inputSpeed;

    return ControlStatus::Ok;
}

ControlStatus MotorController::applyTargetSpeed(float inputSpeed) noexcept
{
    float dutyCycle{0.0F};

    // 手順 1: デューティ比算出（未初期化・範囲チェック込み。FUNC-002）
    const ControlStatus calcResult{calculateDutyCycle(inputSpeed, dutyCycle)};
    if (calcResult != ControlStatus::Ok)
    {
        return calcResult;
    }

    // 手順 2: IG 状態確認（車載 API SW-IF-002。SW-REQ-PWR-001）
    // Unknown（取得失敗）は IG-OFF と同等に扱う（フェールセーフ方針）
    const pf::vapi::IgnitionState ignition{ignitionService_.GetIgnitionState()};
    if (ignition != pf::vapi::IgnitionState::On)
    {
        // 安全停止: 停止デューティ比を出力し Ready へ遷移する（ERR-004）
        if (!pwmService_.SetDutyCycle(STOP_DUTY_PERCENT))
        {
            state_ = MotorState::Error;
            return ControlStatus::HwError;
        }
        state_ = MotorState::Ready;
        lastCommandedSpeed_ = 0.0F;
        return ControlStatus::IgnitionOff;
    }

    // 手順 3: PWM 出力（車載 API SW-IF-001。ERR-003: 失敗時は Error 状態へ遷移）
    if (!pwmService_.SetDutyCycle(dutyCycle))
    {
        state_ = MotorState::Error;
        return ControlStatus::HwError;
    }

    // 手順 4: 出力成功 → Running へ遷移
    state_ = MotorState::Running;
    return ControlStatus::Ok;
}

MotorState MotorController::getState() const noexcept
{
    return state_;
}

float MotorController::getLastCommandedSpeed() const noexcept
{
    return lastCommandedSpeed_;
}

}  // namespace app
