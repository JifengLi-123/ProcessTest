# ユニットテスト結果（Test Result — SWE.4）

> **成果物ID:** 13-50 Test Result（13-25 Verification Results を兼ねる簡略サンプル）
> **参照プロセス:** SWE.4 Software Unit Verification

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 13-50-PT01-SWE4 |
| バージョン | v1.0 |
| 対象テスト仕様 | [08-50-PT01-SWE4](./08-50-unit-test-specification.md) |
| 実施者 | TVE |
| 実施日 | 2026-09-28 |
| 合否判定者 | TVE / QAE（人手判定） |
| テスト環境 | Linux (WSL2) / GCC 13.3.0 / CMake 3.28.3 / GoogleTest 1.14.0 |
| 実行コマンド | `cmake -B build -DENABLE_COVERAGE=ON && cmake --build build && ctest --test-dir build -R unit_tests` |

## 1. 実行結果サマリ

| 項目 | 結果 |
|------|------|
| 総テストケース数 | 13 |
| passed | 13 |
| failed / blocked / skipped | 0 / 0 / 0 |
| 総合判定 | **合格** |

全 13 ケース（SWE4-TC-001〜013）の個別結果は [08-50 テスト仕様書](./08-50-unit-test-specification.md) のステータス列（すべて passed）を参照。

## 2. カバレッジ計測結果（gcov）

対象: `src/app/motor_control/motor_controller.cpp`

| 指標 | 実測値 | 目標（ASIL-B） | 判定 |
|------|--------|---------------|------|
| C0（Lines executed） | 100.00 %（42/42 行） | 100 %（必須） | OK |
| C1（Branches taken at least once） | 100.00 %（16/16 分岐） | 100 %（必須） | OK |
| Call（Calls executed） | 100.00 %（4/4） | —（参考） | OK |

> 逸脱なし。15-01 逸脱根拠文書は不要。

## 3. 検出不具合

なし（Problem Record 起票なし）。

## 4. 特記事項

- 全テストは CI（`.github/workflows/ci.yml`）で PR ごとに自動実行される。
- テスト合否の最終判定は人手（TVE/QAE）で実施（AI への委任禁止事項）。
