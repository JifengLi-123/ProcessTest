# ユニットテスト結果（Test Result — SWE.4）

> **成果物ID:** 13-50 Test Result（13-25 Verification Results を兼ねる簡略サンプル）
> **参照プロセス:** SWE.4 Software Unit Verification

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 13-50-PT01-SWE4 |
| バージョン | v1.1 |
| 対象テスト仕様 | [08-50-PT01-SWE4](./08-50-unit-test-specification.md) |
| 実施者 | TVE |
| 実施日 | 2026-09-28 |
| 合否判定者 | TVE / QAE（人手判定） |
| テスト環境 | Linux (WSL2) / GCC 13.3.0 / CMake 3.28.3 / GoogleTest 1.14.0 |
| 実行コマンド | `cmake --preset host-coverage && cmake --build --preset host-coverage && ctest --preset host-coverage -L swe4` |

## 1. 実行結果サマリ

| 対象 | ケース数 | passed | failed / blocked / skipped |
|------|---------|--------|---------------------------|
| MOD-001 MotorController（SWE4-TC-001〜013） | 13 | 13 | 0 / 0 / 0 |
| MOD-002/003 PF Vehicle API（SWE4-TC-101〜106） | 6 | 6 | 0 / 0 / 0 |
| **合計** | **19** | **19** | **0** |

**総合判定: 合格**（個別結果は [08-50](./08-50-unit-test-specification.md) のステータス列を参照）

## 2. カバレッジ計測結果（gcov）

| ソースファイル | C0（Lines） | C1（Branches taken） | Call | 目標（ASIL-B） | 判定 |
|----------------|------------|---------------------|------|---------------|------|
| `app/motor_control/src/motor_controller.cpp` | 100.00 %（42/42） | 100.00 %（16/16） | 100.00 %（4/4） | C0/C1 100 % | OK |
| `pf/vehicle_api/src/pwm_service.cpp` | 100.00 %（7/7） | 100.00 %（4/4） | 100.00 %（2/2） | C0/C1 100 % | OK |
| `pf/vehicle_api/src/ignition_service.cpp` | 100.00 %（12/12） | 100.00 %（4/4） | 100.00 %（2/2） | C0/C1 100 % | OK |

> 逸脱なし。15-01 逸脱根拠文書は不要。CI（build.yml）の gcovr ゲート（--fail-under-line/branch 100）で継続的に維持する。

## 3. 検出不具合

なし（Problem Record 起票なし）。

## 4. 特記事項

- 全テストは CI（`.github/workflows/build.yml`）で PR ごとに自動実行される（工程ラベル: `-L swe4`）。
- テスト合否の最終判定は人手（TVE/QAE）で実施（AI への委任禁止事項）。
