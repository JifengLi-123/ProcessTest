# ユニットテスト仕様書（Test Specification — SWE.4）

> **成果物ID:** 08-50 Test Specification
> **参照プロセス:** SWE.4 Software Unit Verification
> **注:** サンプル成果物。テストコード `test/unit/test_motor_controller.cpp` と 1 対 1 に対応する。

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 08-50-PT01-SWE4 |
| バージョン | v1.0 |
| ステータス | Approved |
| 対象テスト計画 | [08-52-PT01-SWE4](./08-52-unit-test-plan.md) |
| 対象モジュール | MOD-001 MotorController |
| 作成者 | TVE |
| 承認者 | QAE |
| 承認日 | 2026-09-28 |
| 前工程成果物リンク | [04-05 SW詳細設計書](../swe3/04-05-sw-detailed-design.md) |
| 次工程成果物リンク | [13-50 テスト結果](./13-50-unit-test-result.md) |

## ユニットテストケース

前提条件（共通）: 車載 API 2 種（SW-IF-001/002）はモックに差し替え。IG 状態モックのデフォルトは On。

| テストケースID | テスト名称 | 観点区分 | テスト目的 | 対象関数 | 前提条件 / 初期状態 | 入力データ | 期待結果 | 合否判定条件 | 対応SW要求ID | 対応詳細設計 | ステータス |
|-------------|---------|---------|---------|--------|----------------|---------|---------|------------|-----------|-----------|----------|
| SWE4-TC-001 | 中央値の線形変換 | 正常系 | 正常系：変換式の代表点確認 | `calculateDutyCycle()` | init() 済み | 3000.0 rpm | Ok / duty 50.0 % | 戻り値==Ok かつ duty==50.0 かつ受理値==3000.0 | SW-REQ-001 | FUNC-002 | passed |
| SWE4-TC-002 | 値域内境界値の変換 | 正常系 | 正常系：境界値（最小・最大）での変換確認 | `calculateDutyCycle()` | init() 済み | 0.0 / 6000.0 rpm | duty 5.0 % / 95.0 % | 各境界で期待値一致 | SW-REQ-001 | FUNC-002 | passed |
| SWE4-TC-003 | 負値入力の棄却 | 異常系 | 異常系：範囲外（負値）の棄却と状態保持 | `calculateDutyCycle()` | init() 済み | -0.1 rpm | OutOfRange / 出力・状態不変 | 戻り値==OutOfRange かつ duty 不変かつ受理値不変 | SW-REQ-002 | FUNC-002 / ERR-001 | passed |
| SWE4-TC-004 | 上限超過入力の棄却 | 異常系 | 異常系：範囲外（上限超過）の棄却 | `calculateDutyCycle()` | init() 済み | 6000.1 rpm | OutOfRange / duty 不変 | 同上 | SW-REQ-002 | FUNC-002 / ERR-001 | passed |
| SWE4-TC-005 | NaN 入力の棄却 | 異常系 | 異常系：非数値入力の棄却（防御的プログラミング） | `calculateDutyCycle()` | init() 済み | NaN | OutOfRange / duty 不変 | 同上 | SW-REQ-002 | FUNC-002 / ERR-001 | passed |
| SWE4-TC-006 | 未初期化時の拒否 | 異常系 | 異常系：初期化前の制御要求拒否 | `calculateDutyCycle()` | init() 未実行 | 1000.0 rpm | NotInitialized / duty 不変 | 戻り値==NotInitialized かつ duty 不変 | SW-REQ-003 | FUNC-002 / ERR-002 | passed |
| SWE4-TC-007 | Running への状態遷移 | 正常系 | 正常系：Uninitialized→Ready→Running 遷移と出力値確認 | `init()` / `applyTargetSpeed()` | 構築直後 | 1500.0 rpm | 状態遷移・duty 27.5 % 出力 | 各状態が仕様どおりかつモック出力値==27.5 | SW-REQ-004 | FUNC-001/003 / §4 | passed |
| SWE4-TC-008 | HW 失敗時の Error 遷移 | 異常系 | 異常系：PWM 出力失敗時のフェール動作 | `applyTargetSpeed()` | init() 済み・モック失敗設定 | 1500.0 rpm | HwError / Error 状態 / 以降拒否 | 戻り値==HwError かつ状態==Error かつ後続要求==NotInitialized | SW-REQ-005 | FUNC-003 / ERR-003 | passed |
| SWE4-TC-009 | Error からの復帰 | 正常系 | 正常系：init() による Error 復帰と制御再開 | `init()` / `applyTargetSpeed()` | Error 状態 | init 後 2000.0 rpm | Ready 復帰・受理値クリア・制御再開 | 状態==Ready かつ受理値==0.0、再開後 Ok | SW-REQ-005 | FUNC-001 / §4 | passed |
| SWE4-TC-010 | 範囲外時の HW 非出力 | 異常系 | 異常系：棄却された要求が HW に到達しないこと | `applyTargetSpeed()` | init() 済み | -1.0 rpm | OutOfRange / モック呼び出し 0 回 | モック呼び出し回数==0 かつ状態==Ready | SW-REQ-002 | FUNC-003 | passed |
| SWE4-TC-011 | IG-OFF 時の安全停止 | 異常系 | 異常系：IG-OFF 検出時の安全停止動作 | `applyTargetSpeed()` | Running 状態・IG モック Off | 3000.0 rpm | IgnitionOff / 0.0 % 出力 / Ready 遷移 | 戻り値==IgnitionOff かつ出力==0.0 かつ状態==Ready かつ受理値==0.0 | SW-REQ-PWR-001 | FUNC-003 / ERR-004 | passed |
| SWE4-TC-012 | IG 取得失敗の安全側処理 | 異常系 | 異常系：Unknown を IG-OFF と同等に扱うこと | `applyTargetSpeed()` | init() 済み・IG モック Unknown | 3000.0 rpm | IgnitionOff / 0.0 % 出力 | 同上（フェールセーフ確認） | SW-REQ-PWR-001 | FUNC-003 / FMEA-002 | passed |
| SWE4-TC-013 | 安全停止時の HW 失敗 | 異常系 | 異常系：安全停止出力の失敗時 Error 遷移 | `applyTargetSpeed()` | IG モック Off・PWM モック失敗設定 | 3000.0 rpm | HwError / Error 状態 | 戻り値==HwError かつ状態==Error | SW-REQ-005, PWR-001 | FUNC-003 / ERR-003 | passed |

## テストカバレッジサマリ（実行後に記入）

| モジュール / 関数名 | C0（ステートメント） | C1（ブランチ） | Call | 目標達成 |
|----------------|----------------|-----------|-------|---------|
| `motor_controller.cpp`（全関数） | 100 % | 100 % | 100 % | 達成（ASIL-B: C0/C1 100 % 必須） |
