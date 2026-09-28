#!/usr/bin/env python3
"""
new_app.py - 新規 APP モジュールの雛形生成（AI 主導開発用スキャフォールド）

リポジトリ規約（CLAUDE.md / 04-04 §1.2）に準拠した APP の骨格を一括生成する:
    app/<name>/CMakeLists.txt                                  … add_app_module 登録
    app/<name>/include/app/<name>/<name>_controller.hpp        … 契約面のみに依存するクラス雛形
    app/<name>/src/<name>_controller.cpp                       … 実装雛形（C0/C1 100% で生まれる）
    test/unit/app/<name>/test_<name>_controller.cpp            … SWE.4 テスト雛形（vapi_mock 使用）

test/CMakeLists.txt への登録は不要（unit/app/<name>/*.cpp を自動発見して登録する）。

使い方:
    python3 scripts/new_app.py cooling_fan
    python3 scripts/build.py --apps cooling_fan --test      # 生成直後にビルド・テスト確認
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
RESERVED = {"pf", "app", "ecu", "test", "docs", "build", "cmake",
            "scripts", "tools", "ALL", "NONE"}

CMAKELISTS = """\
# app_@NAME@ — @PASCAL@Controller（11-05 Software Unit）
# add_app_module が契約面（pf_vapi_if）のみへのリンク・警告・カバレッジ・
# SOTA 用 install（APPS_SHARED=ON 時）を一括設定する（cmake/AppModule.cmake）。
add_app_module(@NAME@
    src/@NAME@_controller.cpp)
"""

HEADER = """\
/**
 * @file @NAME@_controller.hpp
 * @brief @PASCAL@Controller のクラス定義（APP レイヤー）
 *
 * @details TODO: モジュールの責務を記述する。
 *
 * @note 設計先行（Design Before Code）: 機能を実装する前に 04-05 詳細設計書を
 *       作成しレビュー承認を得ること。本ファイルは scripts/new_app.py が生成した
 *       雛形であり、applySafeState() は安全停止のサンプル実装である。
 *       依存規則: include 可能なのは契約面（pf/vapi/i_*.hpp・vapi_version.hpp）のみ。
 */
#ifndef PROCESSTEST_APP_@UPPER@_CONTROLLER_HPP
#define PROCESSTEST_APP_@UPPER@_CONTROLLER_HPP

#include <cstdint>

#include "pf/vapi/i_ignition_service.hpp"
#include "pf/vapi/i_pwm_service.hpp"

namespace app
{

/// @PASCAL@ 制御 API の戻り値（エラーコード）
enum class @PASCAL@Status : std::uint8_t
{
    Ok          = 0U,  ///< 正常終了
    HwError     = 1U,  ///< PWM 出力サービス失敗
    IgnitionOff = 2U   ///< IG-OFF/取得失敗により処理を実施せず
};

/**
 * @brief @PASCAL@ 制御クラス（雛形）
 *
 * @details 車載 API はコンストラクタで注入する（依存性注入。
 *          ユニットテストでは vapi_mock に差し替える）。
 *          動的メモリ確保を行わない（AUTOSAR A18-5-1/A18-5-2）。
 */
class @PASCAL@Controller final
{
public:
    /// 安全停止時に出力するデューティ比 [%]
    static constexpr float STOP_DUTY_PERCENT{0.0F};

    /**
     * @brief 車載 API（PWM 出力・IG 状態サービス）を注入して構築する。
     *
     * @param[in,out] pwmService      PWM 出力サービス（非所有。生存期間は統合ルートが保証）
     * @param[in,out] ignitionService IG 状態サービス（同上）
     */
    @PASCAL@Controller(pf::vapi::IPwmService& pwmService,
                       pf::vapi::IIgnitionService& ignitionService) noexcept;

    /**
     * @brief 安全状態（停止デューティ比）を出力する（サンプル実装）。
     *
     * @details IG-ON のときのみ停止デューティ比を出力する。
     *          Unknown（取得失敗）は IG-OFF と同等に扱う（フェールセーフ方針）。
     *          TODO: 04-05 の詳細設計に基づき本来の制御処理へ置き換えること。
     *
     * @return @PASCAL@Status::Ok / HwError / IgnitionOff
     *
     * @pre  なし
     * @post Ok 返却時、STOP_DUTY_PERCENT が出力されている
     */
    @PASCAL@Status applySafeState() noexcept;

private:
    pf::vapi::IPwmService& pwmService_;            ///< 車載 API: PWM 出力サービス（非所有）
    pf::vapi::IIgnitionService& ignitionService_;  ///< 車載 API: IG 状態サービス（非所有）
};

}  // namespace app

#endif  // PROCESSTEST_APP_@UPPER@_CONTROLLER_HPP
"""

SOURCE = """\
/**
 * @file @NAME@_controller.cpp
 * @brief @PASCAL@Controller の実装（APP レイヤー）
 *
 * @details TODO: 対応する 04-05 詳細設計書の文書番号・章番号を記載する。
 */
#include "app/@NAME@/@NAME@_controller.hpp"

namespace app
{

// C++14 では odr 使用される static constexpr メンバにクラス外定義が必要
constexpr float @PASCAL@Controller::STOP_DUTY_PERCENT;

@PASCAL@Controller::@PASCAL@Controller(
    pf::vapi::IPwmService& pwmService,
    pf::vapi::IIgnitionService& ignitionService) noexcept
    : pwmService_{pwmService},
      ignitionService_{ignitionService}
{
}

@PASCAL@Status @PASCAL@Controller::applySafeState() noexcept
{
    // IG 状態確認。Unknown（取得失敗）は Off と同等に扱う（フェールセーフ）
    if (ignitionService_.GetIgnitionState() != pf::vapi::IgnitionState::On)
    {
        return @PASCAL@Status::IgnitionOff;
    }

    // 出力（戻り値 false = HW 異常。必ず処理する）
    if (!pwmService_.SetDutyCycle(STOP_DUTY_PERCENT))
    {
        return @PASCAL@Status::HwError;
    }

    return @PASCAL@Status::Ok;
}

}  // namespace app
"""

TEST = """\
/**
 * @file test_@NAME@_controller.cpp
 * @brief @PASCAL@Controller ユニットテスト（SWE.4）雛形
 *
 * @details 雛形実装（applySafeState）を C0/C1 100% でカバーする 3 ケース。
 *          機能追加時は 08-50 テスト仕様（docs/test/swe4/）に TC を追記し、
 *          本ファイルと 1 対 1 対応を維持すること。
 *          車載 API（契約面）は vapi_mock に差し替える。
 */
#include <gtest/gtest.h>

#include "app/@NAME@/@NAME@_controller.hpp"
#include "vapi_mock/mock_ignition_service.hpp"
#include "vapi_mock/mock_pwm_service.hpp"

namespace
{

using app::@PASCAL@Controller;
using app::@PASCAL@Status;
using pf::vapi::IgnitionState;

class @PASCAL@ControllerTest : public ::testing::Test
{
protected:
    mocks::MockPwmService pwm{};
    mocks::MockIgnitionService ignition{};  // デフォルト IG-ON
    @PASCAL@Controller controller{pwm, ignition};
};

// TC-001（正常系）: IG-ON 時に停止デューティ比が出力される
TEST_F(@PASCAL@ControllerTest, TC001_SafeStateOutputsStopDuty)
{
    EXPECT_EQ(controller.applySafeState(), @PASCAL@Status::Ok);
    EXPECT_FLOAT_EQ(pwm.lastDutyCycle, @PASCAL@Controller::STOP_DUTY_PERCENT);
    EXPECT_EQ(pwm.callCount, 1);
}

// TC-002（異常系）: IG-OFF / 取得失敗時は出力せず IgnitionOff
TEST_F(@PASCAL@ControllerTest, TC002_IgnitionOffRejected)
{
    ignition.state = IgnitionState::Off;
    EXPECT_EQ(controller.applySafeState(), @PASCAL@Status::IgnitionOff);

    ignition.state = IgnitionState::Unknown;  // フェールセーフ: Off と同等
    EXPECT_EQ(controller.applySafeState(), @PASCAL@Status::IgnitionOff);

    EXPECT_EQ(pwm.callCount, 0);
}

// TC-003（異常系）: PWM 出力失敗は HwError として通知される
TEST_F(@PASCAL@ControllerTest, TC003_HwFailurePropagates)
{
    pwm.simulateHwFailure = true;
    EXPECT_EQ(controller.applySafeState(), @PASCAL@Status::HwError);
}

}  // namespace
"""


def render(template: str, name: str) -> str:
    pascal = "".join(part.capitalize() for part in name.split("_"))
    return (template.replace("@NAME@", name)
                    .replace("@PASCAL@", pascal)
                    .replace("@UPPER@", name.upper()))


def write(path: Path, content: str) -> None:
    if path.exists():
        print(f"[new_app.py] 既に存在します: {path}")
        sys.exit(2)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")
    print(f"[new_app.py] 生成: {path.relative_to(ROOT)}")


def main() -> None:
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    name = sys.argv[1]

    if not re.fullmatch(r"[a-z][a-z0-9_]*", name):
        print(f"[new_app.py] APP 名は英小文字始まりの snake_case で指定してください: {name}")
        sys.exit(2)
    if name in RESERVED:
        print(f"[new_app.py] 予約語のため使用できません: {name}")
        sys.exit(2)
    if (ROOT / "app" / name).exists():
        print(f"[new_app.py] app/{name} は既に存在します")
        sys.exit(2)

    write(ROOT / "app" / name / "CMakeLists.txt", render(CMAKELISTS, name))
    write(ROOT / "app" / name / "include" / "app" / name / f"{name}_controller.hpp",
          render(HEADER, name))
    write(ROOT / "app" / name / "src" / f"{name}_controller.cpp", render(SOURCE, name))
    write(ROOT / "test" / "unit" / "app" / name / f"test_{name}_controller.cpp",
          render(TEST, name))

    print(f"""
[new_app.py] 完了: app/{name}

次の手順（プロセス準拠）:
  1. 詳細設計書を作成しレビュー承認を得る（設計先行。docs/design/04-05-*.md 参照）
  2. ビルド・テスト確認:  python3 scripts/build.py --apps {name} --test
  3. .github/CODEOWNERS に /app/{name}/ と /test/unit/app/{name}/ のオーナーを追記
  4. docs/traceability/13-22 にトレーサビリティ行を追加
  5. Issue 番号付きブランチ（feature/<番号>-add-{name}）で PR を作成
""")


if __name__ == "__main__":
    main()
