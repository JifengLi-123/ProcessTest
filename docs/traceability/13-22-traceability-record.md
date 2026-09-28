# トレーサビリティ記録（Traceability Record）

> **成果物ID:** 13-22 Traceability Record（別名表記: 13-51）
> **参照プロセス:** SWE.1〜SWE.6（共通成果物）
> **注:** サンプル成果物。上位のシステム要求（SYS-REQ-NNN）は仮置きである。

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 13-22-PT01-MOTORCTRL |
| バージョン | v1.1 |
| 対象工程 | SWE.1〜SWE.6 |
| 作成者 | CM |
| 作成日 | 2026-09-28 |
| 承認者 | QAE |

### 改訂履歴

| バージョン | 日付 | 変更内容 | 変更者 |
|-----------|------|---------|-------|
| v1.0 | 2026-09-28 | 初版作成 | CM |
| v1.1 | 2026-09-28 | 3 層構成再編（Vehicle API 実装モジュール・SWE4-TC-1xx 追加、パス更新） | CM |

---

## 1. 全工程トレーサビリティマトリクス

システム要求 → SW要求 → アーキテクチャ → 詳細設計 → ソースコード → テスト → 検証基準 の全鎖を示す。

| システム要求ID（仮置き） | SW要求ID（17-11） | アーキ要素（04-04） | I/F仕様ID | 詳細設計（04-05） | ソースコード（11-05） | ユニットテスト（SWE.4） | 統合テスト（SWE.5） | 適格性テスト（SWE.6） | 検証基準（17-50） | ステータス |
|------------------------|------------------|--------------------|-----------|------------------|----------------------|------------------------|--------------------|----------------------|------------------|----------|
| SYS-REQ-010 | SW-REQ-001 | MOD-001 | N/A | MOD001 FUNC-002 / §3.2 | `app/motor_control/src/motor_controller.cpp` `calculateDutyCycle()` | SWE4-TC-001, 002 | SWE5-TC-001, 002 | SWE6-TC-001 | VC-001 | verified |
| SYS-REQ-012 | SW-REQ-002 | MOD-001 | N/A | MOD001 FUNC-002 / ERR-001 | 同上 | SWE4-TC-003, 004, 005, 010 | SWE5-TC-003 | SWE6-TC-002 | VC-002 | verified |
| SYS-REQ-012 | SW-REQ-003 | MOD-001 | N/A | MOD001 FUNC-001, 002 / ERR-002 | `init()` / `calculateDutyCycle()` | SWE4-TC-006 | — | SWE6-TC-003 | VC-003 | verified |
| SYS-REQ-010 | SW-REQ-004 | MOD-001 | N/A | MOD001 §4 状態機械 | `MotorState` / 各関数 | SWE4-TC-007 | SWE5-TC-002 | SWE6-TC-004 | VC-004 | verified |
| SYS-REQ-012 | SW-REQ-005 | MOD-001 | SW-IF-001 | MOD001 FUNC-003 / ERR-003 | `applyTargetSpeed()` | SWE4-TC-008, 009, 013 | SWE5-TC-005 | SWE6-TC-005 | VC-005 | verified |
| SYS-REQ-011 | SW-REQ-PWR-001 | MOD-001 | SW-IF-001, 002 | MOD001 FUNC-003 / ERR-004 | `applyTargetSpeed()` | SWE4-TC-011, 012 | SWE5-TC-004 | SWE6-TC-006 | VC-006 | verified |
| —（I/F要求） | SW-REQ-200 | MOD-002（COMP-002） | SW-IF-001 / FW-IF-001 | PFVAPI FUNC-101 / §3.2 | `pf/vehicle_api/src/pwm_service.cpp` `SetDutyCycle()`・契約面 `pf/vapi/i_pwm_service.hpp` | SWE4-TC-104, 105, 106 | SWE5-TC-001 | （統合構成で間接） | VC-001〜006 | verified |
| —（I/F要求） | SW-REQ-201 | MOD-003（COMP-002） | SW-IF-002 / FW-IF-002 | PFVAPI FUNC-102 / §3.3 | `pf/vehicle_api/src/ignition_service.cpp` `GetIgnitionState()`・契約面 `pf/vapi/i_ignition_service.hpp` | SWE4-TC-101, 102, 103 | SWE5-TC-004 | SWE6-TC-006 | VC-006 | verified |

---

## 2. 上向きトレース（孤立要素確認）

| 本工程要素 | 上位対応 | 孤立フラグ | 孤立根拠 |
|-----------|---------|-----------|---------|
| SW-REQ-001〜005, PWR-001 | SYS-REQ-010/011/012 | — | |
| SW-REQ-200, 201 | 17-08 IF-REQ-001/002（仮置き） | — | |
| MOD001 FUNC-001〜005 / PFVAPI FUNC-101〜102 | SW-REQ-001〜005, PWR-001, 200, 201 | — | |
| MOD-201, MOD-202（Firmware SIL） | — | 孤立（許容） | 検証環境用実装。製品要求由来ではなくテスト戦略（08-52）由来。実機では HW 実装に置換 |
| ecu/main.cpp（統合ルート） | — | 孤立（許容） | wiring のみ（ロジックなし）。01-50 の生成単位として SWE.5 で検証 |

---

## 3. カバレッジサマリ

| 指標 | 値 | 目標 | 判定 |
|------|-----|------|------|
| システム要求カバレッジ（下向き） | 3/3 (100 %) | 100 % | OK |
| SW要求 → 詳細設計トレース率 | 8/8 (100 %) | 100 % | OK |
| SW要求 → ユニットテストトレース率 | 8/8 (100 %) | 100 % | OK |
| SW要求 → 適格性テストトレース率 | 6/6 (100 %)（I/F 要求は SWE.4/5 で検証） | 100 % | OK |
| 孤立要素数（根拠なし） | 0 件 | 0 件 | OK |
