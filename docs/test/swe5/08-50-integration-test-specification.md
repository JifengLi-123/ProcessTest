# 統合テスト仕様書（Test Specification — SWE.5）

> **成果物ID:** 08-50 Test Specification
> **参照プロセス:** SWE.5 Software Integration and Integration Verification
> **注:** サンプル成果物。テストコード `test/integration/test_integration.cpp` と 1 対 1 に対応する。

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 08-50-PT01-SWE5 |
| バージョン | v1.1 |
| ステータス | Approved |
| 対象 | 3 レイヤー統合: APP（MOD-001）+ Vehicle API（MOD-002/003）+ Firmware SIL（MOD-201/202） |
| 作成者 | TVE |
| 承認者 | QAE |
| 前工程成果物リンク | [04-04 SWアーキテクチャ設計書](../../design/04-04-sw-architectural-design.md)（統合対象 I/F: SW-IF-001/002・FW-IF-001/002） |
| 次工程成果物リンク | [13-50 テスト結果](./13-50-integration-test-result.md) |

## 統合方針

- 統合単位: APP × Vehicle API × Firmware SIL のボトムアップ統合。wiring は統合ルート（`ecu/src/main.cpp`）と同一とする。
- 確認観点: レイヤー間 I/F（車載 API・FW-IF）の整合性、IG 信号ライン→車両状態の解釈を含むシナリオ、故障注入時の統合挙動。
- 統合ビルド（01-50）: CI が `ecu_app` および `integration_tests` として生成（`cmake --build --preset host-coverage`）。

## 統合テストケース

| テストケースID | テスト名称 | 観点区分 | テスト目的 | 対象I/F | 入力データ | 期待結果 | 合否判定条件 | 対応成果物 | ステータス |
|-------------|---------|---------|---------|--------|---------|---------|------------|-----------|----------|
| SWE5-TC-001 | レイヤー間 I/F 整合性 | 正常系 | APP の全出力が Vehicle API・Firmware を通り HW に到達すること | SW-IF-001 / FW-IF-001 | 値域内 6 点（0〜6000 rpm）掃引 | 全要求 Ok・HW 受理値が [5.0, 95.0] % 内 | 全点で戻り値==Ok かつ HW 履歴値が範囲内 | 04-04 §4 | passed |
| SWE5-TC-002 | 連続コマンドシーケンス | 正常系 | 加減速シーケンスの統合動作 | SW-IF-001 / FW-IF-001 | 1000→4000→2000 rpm | 各 20.0/65.0/35.0 % が順に HW へ到達・Running 維持 | HW 履歴が期待順・期待値と一致 | 04-04 §4.3 | passed |
| SWE5-TC-003 | 異常コマンドの遮断 | 異常系 | 棄却された要求がレイヤー境界を越えないこと | SW-IF-001 | 正常・範囲外混在シーケンス | 正常 2 件のみ HW 到達 | HW 履歴件数==2 かつ値一致 | 04-05 ERR-001 | passed |
| SWE5-TC-004 | IG サイクルシナリオ | 正常系 | IG-ON→OFF→信号異常→ON の車両状態シナリオ統合動作（ライン状態解釈を含む） | SW-IF-002 / FW-IF-002 / SW-IF-001 | ライン High→Low→Fault→High で制御要求 | OFF/Fault で安全停止（0.0 %）・ON 復帰で制御再開 | 各フェーズの戻り値・状態・HW 出力値が仕様どおり | SW-REQ-PWR-001 / SW-REQ-201 | passed |
| SWE5-TC-005 | Firmware 故障注入と復帰 | 異常系 | HW 故障時の APP フェール動作と復帰の統合確認 | FW-IF-001 経由の全経路 | 故障注入 ON→OFF | HwError/Error 遷移 → init() 後に制御再開 | 戻り値・状態遷移が仕様どおり | 04-05 ERR-003 | passed |

## テスト結果

実行結果は [13-50 統合テスト結果](./13-50-integration-test-result.md) を参照（5/5 passed）。
