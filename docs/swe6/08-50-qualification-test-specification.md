# 適格性テスト仕様書（Test Specification — SWE.6）

> **成果物ID:** 08-50 Test Specification
> **参照プロセス:** SWE.6 Software Qualification Test（SW適格性テスト）
> **注:** サンプル成果物。テストコード `test/qualification/test_qualification.cpp` と 1 対 1 に対応する。

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 08-50-PT01-SWE6 |
| バージョン | v1.0 |
| ステータス | Approved |
| 対象 | 統合済みソフトウェア（APP + PF SIL） |
| 作成者 | TVE |
| 承認者 | PO / QAE |
| 前工程成果物リンク | [17-11 SW要求仕様書](../swe1/17-11-sw-requirements-specification.md) / [17-50 検証基準](../swe1/17-50-verification-criteria.md) |
| 次工程成果物リンク | [13-50 テスト結果](./13-50-qualification-test-result.md) |

## テスト方針

SWE.6 は **SW 要求仕様書（17-11）に対する適格性確認**である。各テストケースは SW-REQ と検証基準（17-50 の VC）に 1 対 1 で対応付け、合否判定条件は VC の定義に従う。

## 適格性テストケース

| テストケースID | 対応SW要求ID | 対応検証基準ID | テスト名称 | 観点区分 | 実施内容 | 期待結果 | 合否判定条件 | ステータス |
|-------------|-----------|-------------|---------|---------|---------|---------|------------|----------|
| SWE6-TC-001 | SW-REQ-001 | VC-001 | 線形変換の適格性 | 正常系 | 値域の 0/25/50/75/100 % 点で出力を確認 | 5.0/27.5/50.0/72.5/95.0 % | 誤差 ±0.01 % 以内 | passed |
| SWE6-TC-002 | SW-REQ-002 | VC-002 | 範囲外入力棄却の適格性 | 異常系 | 負値・上限超過を入力 | OutOfRange・直前出力保持 | 出力値 50.0 %・受理値 3000 rpm が不変 | passed |
| SWE6-TC-003 | SW-REQ-003 | VC-003 | 未初期化拒否の適格性 | 異常系 | 初期化前に制御要求 | NotInitialized・HW 出力なし | PF 出力履歴が空であること | passed |
| SWE6-TC-004 | SW-REQ-004 | VC-004 | 状態遷移の適格性 | 正常系 | 初期化→制御要求で状態観測 | Uninitialized→Ready→Running | 各状態が仕様どおり | passed |
| SWE6-TC-005 | SW-REQ-005 | VC-005 | HW異常フェール動作の適格性 | 異常系 | 故障注入→要求→復帰 | HwError/Error→拒否→init() 復帰 | 遷移・拒否・復帰すべて仕様どおり | passed |
| SWE6-TC-006 | SW-REQ-PWR-001 | VC-006 | IG-OFF 安全停止の適格性 | 正常系・異常系 | IG-OFF / Unknown で制御要求 | IgnitionOff・0.0 % 出力・Ready 遷移 | 停止出力値・状態・戻り値が仕様どおり（Unknown 含む） | passed |

## 要求カバレッジ

全 SW 要求（機能・電源状態）に対して適格性テストケースが 1 件以上対応（100 %）。I/F 要求（SW-REQ-200/201）は統合構成での実行自体により間接検証（SWE5-TC-001 参照）。
