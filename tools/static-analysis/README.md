# 静的解析設定（SWE.3 Step 5）

> 本ディレクトリは **SUP.8（構成管理）の対象**。設定変更は `chore/` ブランチ + PR で行う。

## 使用ツール

| ツール | 用途 | 備考 |
|--------|------|------|
| Parasoft C/C++test | AUTOSAR C++14 Guidelines（397 ルール）準拠チェック・MC/DC 計測 | TÜV SÜD 認定。第一候補 |
| Helix QAC | 同上（代替） | Parasoft が使用できない環境向け |

## 配置ファイル（実プロジェクトで追加）

| ファイル | 内容 |
|---------|------|
| `cpptest.properties` | Parasoft 解析設定（ルールセット: AUTOSAR C++14 必須ルール） |
| `ruleset-autosar-cpp14.properties` | 有効ルール一覧・重大度マッピング |
| `suppressions/` | 逸脱根拠を記録した抑止設定（逸脱ごとに根拠コメント必須） |

> 本サンプルリポジトリには商用ツールのライセンスが無いため設定ファイルは未配置。
> 代替として CI では `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror`
> （cmake/CompilerWarnings.cmake）を品質ゲートとして適用している。

## 違反対処基準（SWE.3 Step 5）

| 違反種別 | 対処 |
|---------|------|
| 重大違反（Error/Critical） | 必ず修正。未修正での先行禁止 |
| 軽微違反（Warning/Advisory） | 修正、または逸脱根拠をツール上（suppressions/）に記録 |

## MC/DC カバレッジ（ASIL C/D の場合）

gcov は MC/DC を計測できないため、ASIL C/D のユニットには Parasoft C/C++test
または Bullseye 等の MC/DC 対応ツールを使用する（tools.md カバレッジ基準参照）。
本サンプル（ASIL-B）は C0/C1 を gcov/gcovr で計測している。
