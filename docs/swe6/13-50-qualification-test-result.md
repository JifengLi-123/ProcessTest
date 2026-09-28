# 適格性テスト結果（Test Result — SWE.6）

> **成果物ID:** 13-50 Test Result
> **参照プロセス:** SWE.6 Software Qualification Test

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 13-50-PT01-SWE6 |
| バージョン | v1.0 |
| 対象テスト仕様 | [08-50-PT01-SWE6](./08-50-qualification-test-specification.md) |
| 実施者 | TVE |
| 実施日 | 2026-09-28 |
| 合否判定者 | TVE（判定）/ QAE（適合確認） |
| 受入承認 | PO / PM（最終リリース判定は人手必須・AI委任禁止） |
| テスト環境 | Linux (WSL2) / GCC 13.3.0 / GoogleTest 1.14.0 |
| 実行コマンド | `ctest --test-dir build -R qualification_tests` |

## 実行結果サマリ

| 項目 | 結果 |
|------|------|
| 総テストケース数 | 6（SWE6-TC-001〜006） |
| passed | 6 |
| failed / blocked / skipped | 0 / 0 / 0 |
| SW 要求カバレッジ | 6/6 要求（100 %） |
| 総合判定 | **合格** |

## 未解決問題

なし（Problem Record 0 件）。

## リリース判定（サンプル）

| 確認項目 | 結果 |
|---------|------|
| 全 SW 要求の検証完了（17-11 ステータス verified） | OK |
| SWE.4 カバレッジ目標達成（C0/C1 100 %） | OK |
| 未解決 Problem Record | 0 件 |
| 受入判定 | 合格（PO 承認。サンプルのため SPL.2 リリース工程は対象外） |
