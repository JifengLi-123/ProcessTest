# CI 用イメージ（dev の最小サブセット + 静的解析 CLI）
# .github/workflows/build.yml の実行環境として使用する（SUP.8 管理対象）
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        g++ \
        cmake \
        ninja-build \
        gcovr \
        git \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 静的解析 CLI（Parasoft C/C++test / Helix QAC）はライセンスサーバー設定とともに
# 社内レジストリのベースイメージから取り込む（本サンプルでは省略）。
# 設定ファイルは tools/static-analysis/ を参照。

WORKDIR /workspace
