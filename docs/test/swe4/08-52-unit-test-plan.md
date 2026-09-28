# ユニットテスト計画（Test Plan — SWE.4）

> **成果物ID:** 08-52 Test Plan
> **参照プロセス:** SWE.4 Software Unit Verification
> **注:** サンプル成果物。

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 08-52-PT01-SWE4 |
| バージョン | v1.1 |
| ステータス | Approved |
| 対象 | MOD-001（APP）/ MOD-002・MOD-003（PF Vehicle API） |
| 作成者 | TVE |
| 作成日 | 2026-09-28 |
| 承認者 | QAE |
| 承認日 | 2026-09-28 |
| 前工程成果物リンク | [04-05 MOD-001](../../design/04-05-sw-detailed-design.md) / [04-05 PFVAPI](../../design/04-05-pf-vehicle-api-detailed-design.md) / [17-11](../../requirements/17-11-sw-requirements-specification.md) |
| 次工程成果物リンク | [08-50 テスト仕様書](./08-50-unit-test-specification.md) / [13-50 テスト結果](./13-50-unit-test-result.md) |

### 改訂履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|---------|-------|
| v1.0 | 2026-09-28 | 初版作成（MOD-001 のみ） | TVE |
| v1.1 | 2026-09-28 | 3 層構成再編に伴い PF Vehicle API（MOD-002/003）を対象に追加 | TVE |

## 1. テスト方針

| 項目 | 内容 |
|------|------|
| テスト対象 | 11-05: `app/motor_control/`（MOD-001）および `pf/vehicle_api/`（MOD-002/003）。いずれも ASIL-B |
| テスト環境 | ホスト Linux（docker/dev イメージ。GCC 13 / CMake Presets / GoogleTest 1.14.0） |
| テスト手法 | 要求ベース + 契約（@pre/@post）ベースのテストケース導出。境界値分析・状態遷移テスト・決定表テスト・故障注入 |
| 依存の扱い（モック戦略） | **APP（MOD-001）**: 車載 API を `vapi_mock` に差し替え、Firmware 不要で完結する。**Vehicle API（MOD-002/003）**: Firmware I/F を `fw_mock` に差し替える。モックは PF チームが管理（CODEOWNERS） |
| カバレッジ目標 | **ASIL-B: C0 100 %・C1 100 %（必須）**。対象は `app/` + `pf/vehicle_api/`。CI の gcovr ゲートで強制（--fail-under-line/branch 100） |
| 未達時の扱い | 目標値は下げず、逸脱根拠文書（15-01）で正当化する |
| 合否判定 | 全テストケース passed かつカバレッジ目標達成。最終判定は TVE / QAE の人手で行う |
| 実行方法 | `ctest --preset host-coverage -L swe4`（CI で PR ごとに自動実行） |

## 2. スケジュール・体制（サンプル）

| 作業 | 担当 | 期日 |
|------|------|------|
| テスト仕様作成（08-50） | TVE | 2026-09-28 |
| テスト実施・結果記録（13-50） | TVE | 2026-09-28 |
| 成果物適合確認 | QAE | 2026-09-28 |
