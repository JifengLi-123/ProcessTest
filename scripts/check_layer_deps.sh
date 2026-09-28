#!/usr/bin/env bash
# アーキテクチャ整合チェック（04-04 §1.2 アーキテクチャ制約の CI 強制）
#   1. APP 層から Firmware ヘッダ（pf/fw/...）の include を禁止
#   2. APP 層の CMakeLists から pf_fw への直接リンクを禁止
#   3. PF 層から APP 層への逆依存（include）を禁止
#   4. APP 層は車載 API の契約 I/F（i_*.hpp・vapi_version.hpp）のみ include 可
#   5. 契約バージョン（vapi_version.hpp ↔ cmake/AppModule.cmake）の同期確認
# CMake 側の担保（pf_vapi が pf_fw を PRIVATE リンク）と合わせて二重に強制する。
# 使い方: リポジトリルートで scripts/check_layer_deps.sh
set -u

cd "$(dirname "$0")/.."
violations=0

echo "[check_layer_deps] 1/5 APP -> Firmware include check"
if grep -rn --include='*.hpp' --include='*.cpp' -E '#include\s*"pf/fw/' app/; then
    echo "  NG: APP 層から Firmware ヘッダを include しています（pf/vapi のみ使用可）"
    violations=1
fi

echo "[check_layer_deps] 2/5 APP -> pf_fw link check"
# コメント行（先頭が # の行）は除外して判定する
link_hits="$(grep -rn --include='CMakeLists.txt' -w 'pf_fw' app/ | grep -vE ':[0-9]+:\s*#' || true)"
if [ -n "${link_hits}" ]; then
    echo "${link_hits}"
    echo "  NG: APP 層のターゲットが pf_fw にリンクしています（pf_vapi のみリンク可）"
    violations=1
fi

echo "[check_layer_deps] 3/5 PF -> APP reverse-dependency check"
if grep -rn --include='*.hpp' --include='*.cpp' -E '#include\s*"app/' pf/; then
    echo "  NG: PF 層が APP 層に依存しています（依存方向は APP -> PF のみ）"
    violations=1
fi

echo "[check_layer_deps] 4/5 APP -> vapi implementation-header include check"
# APP が include してよいのは契約面（i_*.hpp）と vapi_version.hpp のみ。
# 実装ヘッダ（pwm_service.hpp 等）の include は SOTA のリンク独立性を壊すため禁止。
impl_incl="$(grep -rn --include='*.hpp' --include='*.cpp' -E '#include\s*"pf/vapi/' app/ \
    | grep -vE 'pf/vapi/(i_[a-z0-9_]+|vapi_version)\.hpp' || true)"
if [ -n "${impl_incl}" ]; then
    echo "${impl_incl}"
    echo "  NG: APP は契約 I/F（pf/vapi/i_*.hpp・vapi_version.hpp）のみ include 可です"
    violations=1
fi

echo "[check_layer_deps] 5/5 vapi contract version sync check"
# 契約バージョンは vapi_version.hpp（コード側）と AppModule.cmake（SOVERSION 側）で
# 二重管理しているため、両者の一致を機械的に検証する（SOTA 互換性管理の要）。
ver_hdr="pf/vehicle_api/include/pf/vapi/vapi_version.hpp"
hdr_major="$(grep -oE 'VAPI_VERSION_MAJOR\{[0-9]+U\}' "${ver_hdr}" | grep -oE '[0-9]+' || true)"
hdr_minor="$(grep -oE 'VAPI_VERSION_MINOR\{[0-9]+U\}' "${ver_hdr}" | grep -oE '[0-9]+' || true)"
cmk_major="$(grep -E '^set\(PF_VAPI_VERSION_MAJOR ' cmake/AppModule.cmake | grep -oE '[0-9]+' || true)"
cmk_minor="$(grep -E '^set\(PF_VAPI_VERSION_MINOR ' cmake/AppModule.cmake | grep -oE '[0-9]+' || true)"
if [ -z "${hdr_major}" ] || [ -z "${hdr_minor}" ] || [ -z "${cmk_major}" ] || [ -z "${cmk_minor}" ]; then
    echo "  NG: 契約バージョン定義が見つかりません（${ver_hdr} / cmake/AppModule.cmake）"
    violations=1
elif [ "${hdr_major}" != "${cmk_major}" ] || [ "${hdr_minor}" != "${cmk_minor}" ]; then
    echo "  NG: 契約バージョン不一致: vapi_version.hpp=v${hdr_major}.${hdr_minor} / AppModule.cmake=v${cmk_major}.${cmk_minor}"
    echo "      両ファイルを同時に更新してください（互換性破壊は MAJOR+1 = SOVERSION 更新）"
    violations=1
else
    echo "  contract v${hdr_major}.${hdr_minor} (in sync)"
fi

if [ "${violations}" -ne 0 ]; then
    echo "[check_layer_deps] FAILED: アーキテクチャ整合違反があります"
    exit 1
fi

echo "[check_layer_deps] OK: アーキテクチャ整合違反なし"
