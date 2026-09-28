# docs/ — プロセス成果物の置き場と成果物 ID 対応表

本リポジトリの文書成果物（Docs as Code）の配置と、成果物 ID の対応関係を定義する。

## 成果物 ID 対応表

作業指示書内では A-SPICE 標準 ID（17-11 等）と別名表記（17-00-A 等）が混在するため、本リポジトリでは以下のとおり対応付けて運用する。

| 別名表記 | A-SPICE ID | 成果物名 | 本リポジトリでの場所 |
|---------|-----------|---------|--------------------|
| 17-00-A | **17-11** | SW Requirements Specification | `docs/requirements/17-11-sw-requirements-specification.md` |
| 17-00-B | **17-08** | I/F Requirements Specification | **契約面そのもの**: `pf/vehicle_api/include/pf/vapi/`（Doxygen 契約記述）+ [04-04 §4](design/04-04-sw-architectural-design.md) の I/F 定義表 |
| 17-00-C | **17-50** | Verification Criteria | `docs/requirements/17-50-verification-criteria.md` |
| 13-51 | **13-22** | Traceability Record | `docs/traceability/13-22-traceability-record.md` |
| — | 04-04 | SW Architectural Design | `docs/design/04-04-sw-architectural-design.md` |
| — | 04-05 | SW Detailed Design | `docs/design/04-05-*.md`（モジュール群単位で分冊） |
| — | 11-05 | Software Unit（ソースコード） | `pf/` `app/` |
| — | 01-50 | Integrated Software | ビルド成果物 `ecu_app`（`ecu/` が生成定義） |
| — | 08-52 / 08-50 / 13-50 | テスト計画 / テスト仕様 / テスト結果 | `docs/test/swe4〜swe6/` |
| — | 13-19 | Review Record | **GitHub PR の Approve・コメントが兼ねる**（SUP.1 運用。`.github/PULL_REQUEST_TEMPLATE.md`） |
| — | 13-25 | Verification Results | `docs/test/swe4/13-50-unit-test-result.md` に統合（サンプル簡略化） |

## ディレクトリ構成

| ディレクトリ | 内容 | 主担当 |
|-------------|------|--------|
| `requirements/` | 17-11 SW要求仕様書・17-50 検証基準（SWE.1） | PO / Dev |
| `design/` | 04-04 アーキテクチャ設計書（SWE.2）・04-05 詳細設計書（SWE.3）・`uml/`（PlantUML 図。サブシステム単位で分割） | Arch / Dev |
| `test/swe4/` `test/swe5/` `test/swe6/` | 08-52 / 08-50 / 13-50（各検証工程） | TVE |
| `traceability/` | 13-22 トレーサビリティ記録（全工程共通） | CM |

## SWE.6（適格性テスト）の実施方針

| 区分 | 実施内容 | 実施環境 |
|------|---------|---------|
| ホスト実行サブセット | SW 要求ベースのテスト（`test/qualification/`）を CI で毎 PR 実行 | ホスト（SIL 構成 = `ecu/` と同一 wiring） |
| 実機適格性テスト | タイミング制約・HW 依存要求・E2E の確認 | 実機 / HIL 環境（本サンプル対象外。実プロジェクトでは HIL 環境側の手順書・結果を `docs/test/swe6/` に記録する） |

最終リリース判定（PO/PM）・Safety Case 確認（FSS）は人手必須であり AI へ委任しない（AI/LLM 活用ガイドライン）。
