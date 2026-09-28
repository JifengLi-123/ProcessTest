# 開発・CI 用 Docker イメージ

> 本ディレクトリは **SUP.8（構成管理）の対象**。イメージのバージョン（タグ・ダイジェスト）は Quality Plan（08-13）に記録する。

## イメージ一覧

| イメージ | Dockerfile | 用途 |
|---------|-----------|------|
| `processtest-dev` | `dev.Dockerfile` | 開発者共通環境（VS Code devcontainer から参照）。クロスコンパイラ・デバッガ・ドキュメント生成を含む |
| `processtest-ci` | `ci.Dockerfile` | CI 実行環境（dev の最小サブセット + 静的解析 CLI）。`.github/workflows/build.yml` で使用 |

## 取得手順

```bash
# 社内レジストリ（GHCR）から取得
docker pull ghcr.io/<org>/<repo>/processtest-dev:<tag>

# ローカルビルド（devcontainer は自動でビルドする）
docker build -f docker/dev.Dockerfile -t processtest-dev docker/
```

## バージョン更新ルール

1. Dockerfile の変更は `chore/<Issue番号>-...` ブランチで行い、PR レビューを経てマージする
2. マージ後、`.github/workflows/image.yml` が自動でイメージをビルドし GHCR へ push する
3. ツールチェーン（コンパイラ等）のバージョン変更は **Change Request（13-16 / SUP.10）** の対象とし、
   Quality Plan（08-13）の記録を更新する
4. 機能安全対象プロジェクトでは、コンパイラ変更時にツール適格化（ISO 26262 Part 8 §11）の
   要否を機能安全担当（FSS）と合意する

## ベースイメージ更新の影響確認

イメージ更新 PR では CI（build.yml）の全工程テストが通ることを確認する。
ローカル環境との乖離を防ぐため、開発者は devcontainer の再ビルドを行うこと。
