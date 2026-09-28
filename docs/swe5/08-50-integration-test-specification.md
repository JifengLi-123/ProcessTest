# 統合テスト仕様書（Test Specification — SWE.5）

> **成果物ID:** 08-50 Test Specification
> **参照プロセス:** SWE.5 Software Integration and Integration Verification
> **注:** サンプル成果物。テストコード `test/integration/test_integration.cpp` と 1 対 1 に対応する。

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 08-50-PT01-SWE5 |
| バージョン | v1.0 |
| ステータス | Approved |
| 対象 | APP レイヤー（MOD-001）+ PF レイヤー SIL 実装（MOD-101/102）の統合 |
| 作成者 | TVE |
| 承認者 | QAE |
| 前工程成果物リンク | [04-04 SWアーキテクチャ設計書](../swe2/04-04-sw-architectural-design.md)（統合対象 I/F: SW-IF-001/002） |
| 次工程成果物リンク | [13-50 テスト結果](./13-50-integration-test-result.md) |

## 統合方針

- 統合単位: APP（MotorController）× PF（SilPwmService / SilIgnitionService）。ボトムアップ統合。
- 確認観点: レイヤー間 I/F（車載 API）の整合性、PF 側値域検証との突き合わせ、車両状態シナリオ（IG サイクル）、故障注入時の統合挙動。
- 統合ビルド（01-50 相当）: CI により `integration_tests` 実行ファイルとして生成。

## 統合テストケース

| テストケースID | テスト名称 | 観点区分 | テスト目的 | 対象I/F | 入力データ | 期待結果 | 合否判定条件 | 対応成果物 | ステータス |
|-------------|---------|---------|---------|--------|---------|---------|------------|-----------|----------|
| SWE5-TC-001 | レイヤー間 I/F 整合性 | 正常系 | APP の全出力が PF 車載 API に受理されること | SW-IF-001 | 値域内 6 点（0〜6000 rpm）掃引 | 全要求 Ok・PF 受理値が [5.0, 95.0] % 内 | 全点で戻り値==Ok かつ PF 履歴値が範囲内 | 04-04 §4 | passed |
| SWE5-TC-002 | 連続コマンドシーケンス | 正常系 | 加減速シーケンスの統合動作 | SW-IF-001 | 1000→4000→2000 rpm | 各 20.0/65.0/35.0 % が順に出力・Running 維持 | PF 履歴が期待順・期待値と一致 | 04-04 §4.2 | passed |
| SWE5-TC-003 | 異常コマンドの遮断 | 異常系 | 棄却された要求が PF へ到達しないこと | SW-IF-001 | 正常・範囲外混在シーケンス | 正常 2 件のみ PF 到達 | PF 履歴件数==2 かつ値一致 | 04-05 ERR-001 | passed |
| SWE5-TC-004 | IG サイクルシナリオ | 正常系 | IG-ON→OFF→ON の車両状態シナリオ統合動作 | SW-IF-001/002 | IG 状態を切り替えつつ制御要求 | OFF で安全停止（0.0 %）・ON 復帰で制御再開 | 各フェーズの戻り値・状態・出力値が仕様どおり | SW-REQ-PWR-001 | passed |
| SWE5-TC-005 | PF 故障注入と復帰 | 異常系 | PF 故障時の APP フェール動作と復帰の統合確認 | SW-IF-001 | 故障注入 ON→OFF | HwError/Error 遷移 → init() 後に制御再開 | 戻り値・状態遷移が仕様どおり | 04-05 ERR-003 | passed |

## テスト結果

実行結果は [13-50 統合テスト結果](./13-50-integration-test-result.md) を参照（5/5 passed）。
