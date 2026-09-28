# pf_fw — Firmware 層（HW 依存実装。本サンプルは SIL 実装のみ）
# ヘッダは Vehicle API 層・統合ルート（ecu/）・テストにのみ公開する。
# APP 層への伝播は pf_vapi 側の PRIVATE リンクで遮断し、
# scripts/check_layer_deps.sh が include 違反を CI で検出する。
add_library(pf_fw STATIC
    src/sil_pwm_hw.cpp
    src/sil_ign_signal.cpp)
target_include_directories(pf_fw PUBLIC include)

set_project_warnings(pf_fw)
enable_coverage(pf_fw)
