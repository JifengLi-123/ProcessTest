# Vehicle API 層 — 契約面と実装を別ターゲットに分離する（SOTA 対応）
#
#   pf_vapi_if : 契約面（include/pf/vapi/ のヘッダのみ。INTERFACE ターゲット）
#                APP がリンクするのはこれだけ。実装コードを含まないため、
#                APP の成果物（.a/.so）は PF 実装へのリンク依存を持たない。
#   pf_vapi    : 実装（PwmService / IgnitionService）。統合ルート（ecu/）と
#                統合テストのみがリンクする。
#
# ★アーキテクチャ制約の強制ポイント（04-04 §1.2）:
#   pf_fw を PRIVATE リンクすることで、Firmware のヘッダ（include パス）が
#   利用者へ推移的に公開されない。公開ヘッダは Firmware 型を前方宣言のみで参照する。
add_library(pf_vapi_if INTERFACE)
target_include_directories(pf_vapi_if INTERFACE include)

add_library(pf_vapi STATIC
    src/pwm_service.cpp
    src/ignition_service.cpp)
target_link_libraries(pf_vapi
    PUBLIC pf_vapi_if
    PRIVATE pf_fw)

set_project_warnings(pf_vapi)
enable_coverage(pf_vapi)
