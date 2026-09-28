# 開発用イメージ（全開発者共通環境。.devcontainer から参照）
# バージョン更新ルール: docker/README.md 参照（SUP.8 管理対象）
FROM ubuntu:24.04

# ツールチェーン（バージョンは Quality Plan 08-13 に記録すること）
#   - g++ 13 / cmake / ninja : ホストビルド・SWE.3
#   - gdb                    : デバッグ
#   - gcovr                  : カバレッジ集計（SWE.4）
#   - doxygen + graphviz     : API リファレンス生成（tools/doxygen/Doxyfile）
#   - git / ca-certificates  : バージョン管理・gtest FetchContent
# 実機ターゲットを扱う場合はクロスコンパイラ（gcc-arm-none-eabi）を追加する。
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        g++ \
        cmake \
        ninja-build \
        gdb \
        gcovr \
        doxygen \
        graphviz \
        git \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# devcontainer 用の非 root ユーザー
RUN useradd -m -s /bin/bash vscode
USER vscode
WORKDIR /workspace
