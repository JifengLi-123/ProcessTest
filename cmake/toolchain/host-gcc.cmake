# ホストビルド用ツールチェーン（ユニットテスト・SIL 実行用。SUP.8 管理対象）
# ホスト＝開発マシン（Linux x86_64）。docker/dev.Dockerfile の GCC を前提とする。
set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
