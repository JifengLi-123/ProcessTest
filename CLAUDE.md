# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## リポジトリの目的

`~/software-development-process-work-instructions`（A-SPICE 作業指示書）の SWE.1〜SWE.6 プロセスを、車載プロダクトリポジトリを模した構成で実演するサンプル。題材はモータ制御（目標回転数 0〜6000 rpm → PWM デューティ比 5〜95 % の線形変換、IG-OFF 時の安全停止。ASIL-B 想定・AUTOSAR C++14 準拠）。

## アーキテクチャ（厳守事項）

```
ecu/ （統合ルート = 01-50。wiring のみ。唯一 PF 具象に触れてよい）
 └─ app/ （APP層: motor_control, hvac。契約面のみに依存）
     └─ pf/vehicle_api/ （車載API。include/pf/vapi/ = APPに見せる唯一の契約面 = 17-08）
         └─ pf/firmware/ （HW依存層。APPからは不可視）
```

- **APP が include してよいのは `pf/vapi/i_*.hpp` と `vapi_version.hpp` のみ**。`pf/fw/` と vapi 実装ヘッダ（`pwm_service.hpp` 等）は禁止
- APP のリンク先は `pf_vapi_if`（ヘッダのみの契約面ターゲット）のみ。`pf_fw`・`pf_vapi`（実装）へのリンク禁止
- 依存はコンストラクタ注入（参照メンバ・非所有）。オブジェクト生成・配線は `ecu/src/main.cpp` でのみ行う
- fw 隠蔽の実装: `pf_vapi` が `pf_fw` を PRIVATE リンク + vapi 公開ヘッダは fw 型を前方宣言のみで参照
- 以上は `scripts/check_layer_deps.sh`（CI 必須ステップ、5 項目）で機械的に強制される
- 命名: PF 公開 I/F（車載 API・FW-IF）のメソッドは PascalCase、APP 内部 API は camelCase

## コーディング規約

- AUTOSAR C++14: 例外不使用（public 関数は noexcept、異常はエラーコード返却）・動的メモリ確保禁止・ブレース初期化・enum class（uint8_t 基底）・マジックナンバー禁止（名前付き constexpr）
- サイクロマティック複雑度 10 以下
- 契約プログラミング: 全 API の Doxygen に `@pre`/`@post`・`@param` の `Range:`・副作用を記載（design-by-contract ガイドライン準拠）
- NaN 対策の値域チェックは否定形比較 `!(x >= MIN && x <= MAX)` を使用（**ターゲットビルドで `-ffast-math`/`-Ofast` は禁止**。NaN チェックが消える）
- C++14 のため odr 使用される static constexpr メンバにはクラス外定義が必要

## ビルド・テスト・検証コマンド

```bash
# 新規 APP の作成（必ずこの雛形生成を使うこと。手作業でディレクトリを作らない）
python3 scripts/new_app.py <name>    # 規約準拠の骨格を一括生成（C0/C1 100% の状態で生まれる）
                                     # test/CMakeLists.txt への登録は不要（自動発見）

# 選択ビルド（推奨。PF と契約面テストは常時ビルド）
python3 scripts/build.py                                # 全 APP
python3 scripts/build.py --apps hvac --test             # 指定 APP のみ（ecu/SWE.5/6 自動スキップ）
python3 scripts/build.py --changed --test               # git 差分から対象 APP を自動判定（CI が使用）
python3 scripts/build.py --preset host-sota --package dist/   # SOTA 配布単位（APP .so）生成

# プリセット直接（generator は Ninja 固定）
cmake --preset host-coverage && cmake --build --preset host-coverage -j
ctest --test-dir build/host-coverage -L swe4|swe5|swe6  # 工程別テスト
./build/host-coverage/ecu/ecu_app                       # 01-50 SIL デモ

# アーキテクチャ整合チェック（コミット前に必ず実行）
scripts/check_layer_deps.sh

# カバレッジ（ASIL-B: 実装ソース src/ で C0/C1 100% 必須。CI でゲート。gcovr==8.6 に pin）
gcovr --root . --filter 'app/[^/]+/src/' --filter 'pf/vehicle_api/src/' \
      --exclude-throw-branches --exclude-unreachable-branches --txt --txt-metric branch
```

## 契約バージョン管理（SOTA）

- 車載 API 契約バージョンは **2 か所同時更新**: `pf/vehicle_api/include/pf/vapi/vapi_version.hpp` と `cmake/AppModule.cmake`（不一致は check_layer_deps.sh 5/5 が検出）
- 互換性破壊（I/F 変更）→ MAJOR+1（= APP .so の SOVERSION）。後方互換追加 → MINOR+1
- 契約（`pf/vapi/` の I/F）変更は SWE.2 扱い: `docs/design/04-04` §4 の改訂 + PF チームレビューが必要
- APP .so は PF 実装へのリンク依存を持たない（ldd で libc/libstdc++ のみ）— この性質を壊さないこと

## プロセス成果物

- 成果物 ID 対応表・配置は `docs/README.md` 参照（17-00-A=17-11 等の表記揺れ対応）
- 設計先行: 実装前に 04-05 の該当章を更新しレビュー承認を得る。コードと 04-05 は 1 対 1 対応を維持
- 新規テストケースは `docs/test/sweN/08-50-*.md` の TC 表と 1 対 1 で追加。`docs/traceability/13-22` も更新
- PR の Approve = 13-19 Review Record（PULL_REQUEST_TEMPLATE.md の必須欄を埋める）
- 成果物に AI（Claude Code）を使用したら各文書の「AI/LLM使用記録」を更新（最終確認は人手）

## 実施済み作業の記録（2026-09-28）

1. **初期サンプル作成**: 作業指示書を読み込み、モータ制御を題材に SWE.1〜6 の成果物一式＋C++14 コード＋GoogleTest を作成
2. **PF/APP 2 層化**: ユーザー指示により車載 API（PWM 出力・IG 状態）前提の構成へ変更。IG-OFF 安全停止（SW-REQ-PWR-001）追加
3. **product-repo 構成へ再編**: ユーザー提案の構成案を取り込み 3 層化（pf/firmware・pf/vehicle_api・app）+ ecu 統合ルート新設。devcontainer / docker / CMakePresets / CODEOWNERS / 工程別 Issue フォーム / PR テンプレート / check_layer_deps.sh / docs 再配置（requirements・design・test/swe4-6・traceability）を整備
4. **SOTA・選択ビルド対応**: `~/development/cmakeBuild.py` のパターンを踏襲し `scripts/build.py`（--apps / --changed / --package）と `BUILD_APPS` ゲートを導入。契約面 `pf_vapi_if` を実装から分離し APP のリンク独立性を確立、`host-sota` プリセット（APPS_SHARED=ON、SOVERSION=契約 MAJOR）追加。デモ用 2 つ目の APP `app/hvac`（FanController）追加。CI は PR で差分 APP のみビルド
5. **ツールチェーン堅牢化**: GoogleTest に URL_HASH（SHA256）追加 / `.gitattributes`（LF 固定）追加 / 契約バージョン同期チェック（check 5/5）追加 / プリセットの generator を Ninja に固定
6. **AI 主導開発向けスキャフォールド**: `scripts/new_app.py`（新規 APP の規約準拠雛形を一括生成。生成直後から C0/C1 100%）を追加。`test/CMakeLists.txt` の APP 別ユニットテスト登録を自動発見方式（`unit/app/<name>/*.cpp` を GLOB）へ変更し、APP 追加時の手編集を撤廃。include 構造は「モジュール毎 `include/<名前空間パス>/`」を維持する決定（AI の機械検査性・衝突耐性を優先。CLAUDE.md 冒頭の厳守事項参照）
- 検証状態: 全 34 テスト合格（SWE.4=23・SWE.5=5・SWE.6=6）、対象 4 ソース C0/C1/Call 100%、レイヤーチェック 5/5 合格、SOTA .so の PF 非依存を ldd で確認済み。スキャフォールドは使い捨て APP（body_light）で生成→9 テスト合格→カバレッジ 100%→撤去まで検証済み

## 環境メモ・既知の TODO

- ninja はこの WSL2 環境では `~/.local/bin/ninja`（v1.12.1、GitHub リリースから導入）。sudo・pip は使用不可だった
- `target-arm` プリセットは**プレースホルダ**（クロスコンパイラ・リンカスクリプト・fw 実機実装・startup が未整備。リンクまでは通らない）
- CI（build.yml）は暫定で ubuntu-latest + apt。本命は `docker/ci.Dockerfile` イメージの digest 固定実行（TODO）
- 静的解析（Parasoft/QAC）と MC/DC 計測ツールは未導入（ASIL C/D 到達前に必要。tools/static-analysis/README.md 参照）
- GitHub 未 push（ローカル git のみ・未コミット）。コミット時は Issue 番号入りブランチ（`feature/<番号>-...`）を切ること
- 上位のシステム要求（SYS-REQ）・Issue/PR 番号・CODEOWNERS のチーム名は仮置き
