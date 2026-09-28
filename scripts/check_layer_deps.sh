#!/usr/bin/env bash
# レイヤー依存チェック（04-04 §1.2 アーキテクチャ制約の CI 強制）
#   1. APP 層から Firmware ヘッダ（pf/fw/...）の include を禁止
#   2. APP 層の CMakeLists から pf_fw への直接リンクを禁止
#   3. PF 層から APP 層への逆依存（include）を禁止
# CMake 側の担保（pf_vapi が pf_fw を PRIVATE リンク）と合わせて二重に強制する。
# 使い方: リポジトリルートで scripts/check_layer_deps.sh
set -u

cd "$(dirname "$0")/.."
violations=0

echo "[check_layer_deps] 1/3 APP -> Firmware include check"
if grep -rn --include='*.hpp' --include='*.cpp' -E '#include\s*"pf/fw/' app/; then
    echo "  NG: APP 層から Firmware ヘッダを include しています（pf/vapi のみ使用可）"
    violations=1
fi

echo "[check_layer_deps] 2/3 APP -> pf_fw link check"
# コメント行（先頭が # の行）は除外して判定する
link_hits="$(grep -rn --include='CMakeLists.txt' -w 'pf_fw' app/ | grep -vE ':[0-9]+:\s*#' || true)"
if [ -n "${link_hits}" ]; then
    echo "${link_hits}"
    echo "  NG: APP 層のターゲットが pf_fw にリンクしています（pf_vapi のみリンク可）"
    violations=1
fi

echo "[check_layer_deps] 3/3 PF -> APP reverse-dependency check"
if grep -rn --include='*.hpp' --include='*.cpp' -E '#include\s*"app/' pf/; then
    echo "  NG: PF 層が APP 層に依存しています（依存方向は APP -> PF のみ）"
    violations=1
fi

if [ "${violations}" -ne 0 ]; then
    echo "[check_layer_deps] FAILED: レイヤー依存違反があります"
    exit 1
fi

echo "[check_layer_deps] OK: レイヤー依存違反なし"
