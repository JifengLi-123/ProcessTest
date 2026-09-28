# 統合テスト結果（Test Result — SWE.5）

> **成果物ID:** 13-50 Test Result
> **参照プロセス:** SWE.5 Software Integration and Integration Verification

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 13-50-PT01-SWE5 |
| バージョン | v1.1 |
| 対象テスト仕様 | [08-50-PT01-SWE5](./08-50-integration-test-specification.md) |
| 統合対象（01-50） | `ecu_app`（統合ルート）/ `integration_tests`（同一 wiring のテストバイナリ） |
| 実施者 | TVE |
| 実施日 | 2026-09-28 |
| 合否判定者 | TVE / QAE（人手判定） |
| テスト環境 | Linux (WSL2) / GCC 13.3.0 / GoogleTest 1.14.0 |
| 実行コマンド | `ctest --preset host-coverage -L swe5` |

## 実行結果サマリ

| 項目 | 結果 |
|------|------|
| 総テストケース数 | 5（SWE5-TC-001〜005） |
| passed | 5 |
| failed / blocked / skipped | 0 / 0 / 0 |
| 総合判定 | **合格** |

## 確認事項

- レイヤー間 I/F（SW-IF-001/002・FW-IF-001/002）整合性: 04-04 §4 の定義と実装が一致することを確認（孤立 I/F なし）。
- レイヤー依存制約: CI の `scripts/check_layer_deps.sh` が違反 0 件で通過。
- 統合ビルド: `ecu_app`（01-50）がビルド成功し、SIL デモシナリオの実行を確認。
- 検出不具合: なし（Problem Record 起票なし）。
- ベースライン登録: サンプルのため省略（実プロジェクトでは SUP.8 手順に従い CI が登録する）。
