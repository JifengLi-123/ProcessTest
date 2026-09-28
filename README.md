# ProcessTest — A-SPICE SWE プロセス サンプルプロジェクト

`~/software-development-process-work-instructions`（作業指示書）の SWE.1〜SWE.6 プロセスを、車載プロダクトリポジトリを模した **PF（Firmware / Vehicle API）・APP・ECU 統合** の 3 層構成で実演するサンプルです。APP は PF が提供する**車載 API**（契約面）のみに依存します。

題材: 目標回転数 [0〜6000 rpm] を PWM デューティ比 [5〜95 %] に線形変換し、IG 状態を確認のうえ出力するモータ制御（ASIL-B 想定・AUTOSAR C++14 準拠）。

## ディレクトリ構成

```
ProcessTest/
├── .devcontainer/                 # VS Code 用。docker/dev.Dockerfile で全員同一環境
├── docker/                        # dev / ci イメージ定義（SUP.8 対象。README 参照）
├── .github/
│   ├── workflows/build.yml        # PR時: ビルド+レイヤー依存チェック+SWE.4〜6テスト+カバレッジゲート
│   ├── workflows/image.yml        # docker/ 変更時のイメージ再ビルド・配布
│   ├── ISSUE_TEMPLATE/            # 工程別 Issue フォーム（swe3〜swe6。ラベル自動付与）
│   ├── PULL_REQUEST_TEMPLATE.md   # 成果物ID・04-05章番号・SW要求ID欄（PR = 13-19 Review Record）
│   └── CODEOWNERS                 # pf/** → PFチーム、app/** → APPチーム（レビュー独立性）
├── CMakeLists.txt                 # トップ: project 定義とサブディレクトリ登録のみ
├── CMakePresets.json              # ビルド構成の固定（host-debug/coverage/release, target-arm）
├── cmake/
│   ├── toolchain/                 # host-gcc.cmake / arm-none-eabi.cmake
│   ├── CompilerWarnings.cmake     # 共通警告フラグ（-Werror）
│   └── Coverage.cmake             # C0/C1 計測オプション
├── pf/                            # ═══ PFレイヤー（オーナー: PFチーム）═══
│   ├── firmware/                  # HW依存層。include/pf/fw/ は Vehicle API にのみ公開
│   │   ├── include/pf/fw/         #   FW-IF-001 IPwmHw / FW-IF-002 IIgnSignal + SIL実装ヘッダ
│   │   └── src/                   #   SIL実装（実機ではHW実装に置換）
│   └── vehicle_api/
│       ├── include/pf/vapi/       # ★APPに公開する唯一の契約面（17-08対応・Doxygen契約記述）
│       └── src/                   #   実装（Firmwareへ委譲。fwは前方宣言+PRIVATEリンクで隠蔽）
├── app/                           # ═══ APPレイヤー（オーナー: APP各チーム）═══
│   └── motor_control/             # MOD-001。リンク先は pf_vapi のみ
├── ecu/                           # ═══ 統合ルート（01-50 Integrated Software）═══
│   └── src/main.cpp               # 全レイヤーの wiring（生成・注入）。ホストSILデモ実行可能
├── test/                          # ホストビルドでのみ有効（BUILD_TESTING）
│   ├── unit/                      # SWE.4（ctest -L swe4）
│   │   ├── mocks/vapi_mock/       #   車載APIモック（APPのSWE.4はこれで完結。PFチーム管理）
│   │   ├── mocks/fw_mock/         #   Firmwareモック（Vehicle API自体のSWE.4用）
│   │   ├── app/motor_control/     #   SWE4-TC-001〜013
│   │   └── pf/vehicle_api/        #   SWE4-TC-101〜106
│   ├── integration/               # SWE.5（ctest -L swe5）: 3レイヤー統合
│   └── qualification/             # SWE.6（ctest -L swe6）: SW要求ベース・ホスト実行サブセット
├── tools/
│   ├── static-analysis/           # Parasoft/QAC ルール設定の置き場（運用ルールはREADME）
│   └── doxygen/Doxyfile           # APIリファレンス生成設定
├── scripts/
│   └── check_layer_deps.sh        # アーキテクチャ制約のCI強制（app→fw依存の禁止 等）
├── docs/                          # プロセス成果物（Docs as Code。成果物ID対応表は docs/README.md）
│   ├── requirements/              # 17-11 SW要求仕様書・17-50 検証基準（SWE.1）
│   ├── design/                    # 04-04・04-05×2分冊・uml/*.puml（SWE.2/3）
│   ├── test/swe4〜swe6/           # 08-52・08-50・13-50（SWE.4〜6）
│   └── traceability/              # 13-22 トレーサビリティ記録
└── build/                         # 生成物出力先（git管理外）
```

## ビルドとテスト（CMake Presets）

```bash
# 構成〜ビルド（カバレッジ計測付き。初回は GoogleTest 1.14.0 を FetchContent 取得）
cmake --preset host-coverage
cmake --build --preset host-coverage -j

# 工程別テスト実行（CI と同じラベル指定）
ctest --preset host-coverage -L swe4    # SWE.4 ユニット検証（19ケース）
ctest --preset host-coverage -L swe5    # SWE.5 統合検証（5ケース）
ctest --preset host-coverage -L swe6    # SWE.6 適格性テスト・ホストサブセット（6ケース）

# 統合ソフトウェア（01-50）の SIL デモ実行
./build/host-coverage/ecu/ecu_app

# アーキテクチャ制約チェック（CI 必須ステップと同一）
scripts/check_layer_deps.sh

# カバレッジ集計（CI では gcovr --fail-under-line/branch 100 でゲート）
gcovr --root . --filter 'app/' --filter 'pf/vehicle_api/' --branches --txt
```

## アーキテクチャ上の要点

| 論点 | 実現方法 |
|------|---------|
| 契約面の一元化 | APP に見せる I/F は `pf/vehicle_api/include/pf/vapi/` のみ（17-08 対応。@pre/@post の Doxygen 契約記述） |
| Firmware の隠蔽 | ① pf_vapi が pf_fw を **PRIVATE リンク** ② vapi 公開ヘッダは fw 型を**前方宣言のみ**で参照 ③ `scripts/check_layer_deps.sh` が CI で include/リンク違反を検出 |
| 統合の一元化 | オブジェクト生成・注入は `ecu/`（Composition Root）のみ。01-50 の生成単位 |
| テスト戦略 | APP の SWE.4 は vapi_mock で完結 / Vehicle API の SWE.4 は fw_mock / SWE.5〜6 は Firmware SIL とのフルスタック |
| レビュー独立性 | CODEOWNERS でレイヤー別オーナーを自動アサイン（SWE.3 Step 7） |
| ビルド再現性 | CMakePresets.json + docker イメージで全員・CI が同一構成（SUP.8） |

## プロセス実演のポイント

| 作業指示書の要求 | 本サンプルでの実現 |
|-----------------|------------------|
| 設計先行（Design Before Code） | 04-05（2分冊）の FUNC/ERR/状態機械/決定表とコードが 1 対 1 対応 |
| 契約プログラミング（@pre/@post） | 全 API の Doxygen コメントに記載。テストケースを契約から導出 |
| AUTOSAR C++14 必須ルール | 例外不使用（noexcept）・動的メモリ確保なし・ブレース初期化・マジックナンバー排除・enum class |
| 複雑度 ≤ 10 | 最大 5（`applyTargetSpeed`） |
| 車載特有要求（IG-ON/OFF） | IG-OFF/信号異常時の安全停止（SW-REQ-PWR-001）。信号ライン→車両状態の解釈は Vehicle API が担当 |
| カバレッジ要件（ASIL-B） | app/ + pf/vehicle_api/ で C0/C1/Call 100 % 実測達成。CI ゲートで維持 |
| トレーサビリティ | SYS-REQ→SW-REQ→MOD/FUNC→コード→TC→VC の全鎖を 13-22 に記録 |
| チケット駆動・PR レビュー | 工程別 Issue フォーム + PR テンプレート（PR の Approve = 13-19） |
| AI/LLM 使用記録 | 各成果物の「AI/LLM使用記録」に Claude Code 使用と人手確認を明記 |

## 注意事項

- 本リポジトリは**プロセス実演用サンプル**です。SYS-REQ・Issue/PR 番号・CODEOWNERS のチーム名は仮置きです。
- 成果物 ID の表記揺れ（17-00-A/B/C ↔ 17-11/17-08/17-50 等）は [docs/README.md](docs/README.md) の対応表で解決しています。
- 実プロジェクトでは、Issue 発行 → ブランチ作成（`feature/<Issue番号>-...`）→ PR レビューの手順を必ず踏んでください（direct push 禁止）。
