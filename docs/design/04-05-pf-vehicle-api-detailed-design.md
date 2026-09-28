# SW詳細設計書（SW Detailed Design）— PF Vehicle API（MOD-002 / MOD-003）

> **成果物ID:** 04-05 SW Detailed Design
> **参照プロセス:** SWE.3 SW Detailed Design and Unit Construction
> **注:** サンプル成果物。APP 側（MOD-001）の詳細設計は [04-05-PT01-MOD001](./04-05-sw-detailed-design.md) を参照。

---

## 文書管理情報

| 項目 | 内容 |
|------|------|
| 文書番号 | 04-05-PT01-PFVAPI |
| バージョン | v1.0 |
| ステータス | Approved |
| 対象モジュール | MOD-002 PwmService / MOD-003 IgnitionService（PF Vehicle API 層） |
| 作成者 | Dev（PF チーム） |
| 作成日 | 2026-09-28 |
| 承認者 | Lead Developer（PF チーム） |
| 承認日 | 2026-09-28 |
| 前工程成果物リンク | [04-04 SWアーキテクチャ設計書](./04-04-sw-architectural-design.md) §4 |
| 次工程成果物リンク | 11-05: `pf/vehicle_api/` / [08-50 ユニットテスト仕様書](../test/swe4/08-50-unit-test-specification.md)（SWE4-TC-101〜106） |

---

## 1. モジュール概要

| 項目 | MOD-002 | MOD-003 |
|------|---------|---------|
| モジュール名 | PwmService | IgnitionService |
| ファイル名 | `pf/vehicle_api/src/pwm_service.cpp` / `include/pf/vapi/pwm_service.hpp` | `pf/vehicle_api/src/ignition_service.cpp` / `include/pf/vapi/ignition_service.hpp` |
| 責務 | 車載 API（SW-IF-001）の実装。API 境界の値域検証と Firmware（FW-IF-001）への委譲 | 車載 API（SW-IF-002）の実装。IG 信号ライン状態（FW-IF-002）の車両状態への解釈 |
| ASILレベル | ASIL-B | ASIL-B |
| 割り当てSW要求 | SW-REQ-200 | SW-REQ-201 |
| 依存 | `pf::fw::IPwmHw`（コンストラクタ注入） | `pf::fw::IIgnSignal`（コンストラクタ注入） |

> **fw 隠蔽の実装方式:** 公開ヘッダでは Firmware 型を前方宣言のみで参照し（メンバは参照保持）、Firmware ヘッダの include は .cpp 内に限定する。これにより pf_vapi を PUBLIC include する APP へ Firmware の include パスが伝播しない（04-04 §1.2）。

---

## 2. データ設計

| 定数ID | 名称 | 値 | 型 | 用途 | 関連要求ID |
|--------|------|----|----|------|------------|
| CONST-101 | `PwmService::API_DUTY_MIN` | 0.0f | float | 車載 API が受理するデューティ比下限 [%] | SW-REQ-200 |
| CONST-102 | `PwmService::API_DUTY_MAX` | 100.0f | float | 車載 API が受理するデューティ比上限 [%] | SW-REQ-200 |

型定義（`pf::vapi::IgnitionState`・`pf::fw::IgnLineLevel`）は契約面ヘッダの Doxygen を参照。

---

## 3. 関数設計

### 3.1 関数一覧

| 関数ID | 関数名 | シグネチャ | 責務概要 | 割り当てSW要求 | 関連I/F | ASIL |
|-------|-------|---------|---------|-------------|--------|------|
| FUNC-101 | `PwmService::SetDutyCycle` | `bool SetDutyCycle(float dutyCyclePercent)` | 値域検証と Firmware への委譲 | SW-REQ-200 | SW-IF-001 / FW-IF-001 | ASIL-B |
| FUNC-102 | `IgnitionService::GetIgnitionState` | `IgnitionState GetIgnitionState()` | ライン状態の車両状態への解釈 | SW-REQ-201 | SW-IF-002 / FW-IF-002 | ASIL-B |

### 3.2 FUNC-101：`PwmService::SetDutyCycle`

| 種別 | 条件 |
|------|------|
| 事前条件 | dutyCyclePercent ∈ [API_DUTY_MIN, API_DUTY_MAX] |
| 事後条件 | true 返却時: 指定値が Firmware へ書き込まれている。false（範囲外）返却時: Firmware への書き込みは発生しない |

**処理:** 否定形の値域比較（NaN も棄却）→ 合格時のみ `pf::fw::IPwmHw::WriteDuty()` へ委譲し、結果をそのまま返す（ERR-101: 範囲外は false / HW 失敗は false 伝播）。

### 3.3 FUNC-102：`IgnitionService::GetIgnitionState`

**決定表（ライン状態 → 車両状態の解釈）**

| 入力（IgnLineLevel） | 出力（IgnitionState） | 備考 |
|---------------------|----------------------|------|
| High | On | |
| Low | Off | |
| Fault | Unknown | フェールセーフ側（APP は Off と同等に扱う） |
| 上記以外（未定義値） | Unknown | 防御的プログラミング |

| 種別 | 条件 |
|------|------|
| 事前条件 | なし |
| 事後条件 | 戻り値は IgnitionState の定義値のいずれか。副作用なし |

### 3.4 エラー処理・例外処理設計

| エラーID | 対象 | エラー種別 | 検出方法 | 対処方法 | 通知方法 |
|----------|------|------------|----------|----------|----------|
| ERR-101 | FUNC-101 | 異常入力（範囲外・NaN） | 否定形の値域比較 | Firmware へ委譲せず失敗を返す | 戻り値 false |
| ERR-102 | FUNC-101 | HW 異常 | FW-IF-001 戻り値 | 失敗をそのまま伝播 | 戻り値 false |
| ERR-103 | FUNC-102 | 信号取得失敗（Fault） | FW-IF-002 戻り値 | Unknown へ解釈（フェールセーフ） | 戻り値 Unknown |

例外: 全関数とも送出しない（A15-0-1。仮想 I/F のため noexcept 指定はなし。実装内で例外を発生させる処理を含まない）。

---

## 4. トレーサビリティ

| SW要求ID | I/F仕様ID | 詳細設計 | ソースコードファイル | 関数 | ユニットテスト | ステータス |
|---------|-----------|---------|--------------------|------|---------------|----------|
| SW-REQ-200 | SW-IF-001 / FW-IF-001 | FUNC-101 / §3.2 | `pf/vehicle_api/src/pwm_service.cpp` | `SetDutyCycle()` | SWE4-TC-104〜106 | traced |
| SW-REQ-201 | SW-IF-002 / FW-IF-002 | FUNC-102 / §3.3 | `pf/vehicle_api/src/ignition_service.cpp` | `GetIgnitionState()` | SWE4-TC-101〜103 | traced |

---

## 5. テスト容易性設計

| 対象 | 依存先 | 依存注入方法 | モック | 単体テスト方法 |
|------|--------|--------------|--------|----------------|
| PwmService | pf::fw::IPwmHw | コンストラクタ引数（参照注入） | `test/unit/mocks/fw_mock/mock_pwm_hw.hpp` | GoogleTest。委譲値・非委譲（範囲外時）・失敗伝播を検証 |
| IgnitionService | pf::fw::IIgnSignal | 同上 | `test/unit/mocks/fw_mock/mock_ign_signal.hpp` | GoogleTest。決定表の全行を検証 |

**サイクロマティック複雑度:** FUNC-101 = 2、FUNC-102 = 3。逸脱なし。

---

## 6. AI/LLM使用記録

| 記録項目 | 内容 |
|---------|------|
| 使用ツール | Claude Code |
| 活用範囲 | 本書ドラフト生成・ソースコード・ユニットテストコード生成 |
| 最終確認者 | Dev / Lead Developer（PF チーム。人手レビュー） |
| 最終確認日 | 2026-09-28 |
