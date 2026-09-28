# ProcessTest — A-SPICE SWE プロセス サンプルプロジェクト

`~/software-development-process-work-instructions`（作業指示書）の SWE.1〜SWE.6 プロセスを、最小規模のモータ制御ユニットを題材に実演するサンプルです。**PF（プラットフォーム）/ APP（アプリケーション）の 2 レイヤー構成**とし、APP は PF が提供する**車載 API**（PWM 出力サービス・IG 状態サービス）を使用します。

## 題材

目標回転数 [0〜6000 rpm] を PWM デューティ比 [5〜95 %] に線形変換し、IG 状態を確認のうえ車載 API 経由で出力するモータ制御ユニット（ASIL-B 想定・AUTOSAR C++14 準拠）。

## ディレクトリ構成（プロセス定義から導出）

作業指示書の「成果物トレーサビリティ早見表」に基づき、工程ごとに成果物を配置しています。

```
ProcessTest/
├── docs/                       # 文書成果物（Docs as Code）
│   ├── swe1/                   # SWE.1 SW要求分析
│   │   ├── 17-11-sw-requirements-specification.md
│   │   └── 17-50-verification-criteria.md
│   ├── swe2/                   # SWE.2 アーキテクチャ設計
│   │   └── 04-04-sw-architectural-design.md      # PF/APP レイヤー・車載API定義
│   ├── swe3/                   # SWE.3 詳細設計 & 実装
│   │   └── 04-05-sw-detailed-design.md
│   ├── swe4/                   # SWE.4 ユニット検証
│   │   ├── 08-52-unit-test-plan.md
│   │   ├── 08-50-unit-test-specification.md
│   │   └── 13-50-unit-test-result.md             # C0/C1 100% 達成記録
│   ├── swe5/                   # SWE.5 統合 & 統合検証
│   │   ├── 08-50-integration-test-specification.md
│   │   └── 13-50-integration-test-result.md
│   ├── swe6/                   # SWE.6 SW適格性テスト
│   │   ├── 08-50-qualification-test-specification.md
│   │   └── 13-50-qualification-test-result.md
│   └── common/                 # 全工程共通成果物
│       └── 13-22-traceability-record.md          # 要求→設計→コード→テストの全鎖
├── src/                        # 11-05 Software Unit（SWE.3）
│   ├── pf/                     # PFレイヤー
│   │   ├── include/pf/         #   車載API（公開I/F。実機ではAUTOSAR AP/ベンダSDKをラップ）
│   │   │   ├── i_pwm_service.hpp        # SW-IF-001 PWM出力サービス
│   │   │   └── i_ignition_service.hpp   # SW-IF-002 IG状態サービス
│   │   └── sil/                #   SIL実装（ホスト検証用。実機ではHAL実装に置換）
│   └── app/                    # APPレイヤー
│       └── motor_control/      #   MOD-001 MotorController
├── test/
│   ├── unit/                   # SWE.4（車載APIはモックに差し替え）
│   ├── integration/            # SWE.5（APP + PF SIL 統合）
│   └── qualification/          # SWE.6（SW要求ベース）
├── CMakeLists.txt              # SWE.3/5 ビルド定義（SUP.8 管理対象）
└── .github/workflows/ci.yml    # CI/CD（SWE.3 Step 4.3）
```

## ビルドとテスト

```bash
# 構成（カバレッジ計測有効）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON

# ビルド（SWE.3。初回は GoogleTest 1.14.0 を FetchContent で取得）
cmake --build build -j

# 工程別テスト実行
ctest --test-dir build -R unit_tests           # SWE.4（13ケース）
ctest --test-dir build -R integration_tests    # SWE.5（5ケース）
ctest --test-dir build -R qualification_tests  # SWE.6（6ケース）

# カバレッジ計測（SWE.4: ASIL-B は C0/C1 100% 必須）
./build/unit_tests
gcov -b -c build/CMakeFiles/app_motor_control.dir/src/app/motor_control/motor_controller.cpp.gcno
```

## プロセス実演のポイント

| 作業指示書の要求 | 本サンプルでの実現 |
|-----------------|------------------|
| 設計先行（Design Before Code） | 04-05 の FUNC/ERR/状態機械定義とコードが 1 対 1 対応 |
| 契約プログラミング（@pre/@post） | 全 API の Doxygen コメントに記載。テストケースを契約から導出 |
| テスト可能設計（依存性注入） | 車載 API 2 種をコンストラクタ注入。ユニットテストはモック、統合テストは PF SIL 実装 |
| AUTOSAR C++14 必須ルール | 例外不使用（noexcept）・動的メモリ確保なし・ブレース初期化・マジックナンバー排除・enum class |
| 複雑度 ≤ 10 | 最大 5（`applyTargetSpeed`） |
| 車載特有要求（IG-ON/OFF） | IG-OFF/取得失敗時の安全停止（SW-REQ-PWR-001）。フェールセーフ方針を実装 |
| カバレッジ要件（ASIL-B） | ユニットテストで C0/C1/Call 100 % を実測達成 |
| トレーサビリティ | SYS-REQ→SW-REQ→MOD/FUNC→コード→TC→VC の全鎖を 13-22 に記録 |
| AI/LLM 使用記録 | 各成果物の「AI/LLM使用記録」に Claude Code 使用と人手確認を明記 |

## 注意事項

- 本リポジトリは**プロセス実演用サンプル**です。上位のシステム要求（SYS-REQ）・GitHub Issue/PR 番号は仮置きです。
- 実プロジェクトでは、Issue 発行 → ブランチ作成（`feature/<Issue番号>-...`）→ PR レビュー（13-19 を兼ねる）の手順を必ず踏んでください（direct push 禁止）。
- ドキュメント成果物の作成には Claude Code（AI）を使用し、最終確認は人手で行う運用を前提としています（AI/LLM 活用ガイドライン準拠）。
