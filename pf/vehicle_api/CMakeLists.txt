# pf_vapi — Vehicle API 層（APP に公開する唯一の契約面 + その実装）
#
# ★アーキテクチャ制約の強制ポイント（04-04 §1.2）:
#   pf_fw を PRIVATE リンクすることで、Firmware のヘッダ（include パス）が
#   pf_vapi の利用者（APP）へ推移的に公開されない。
#   公開ヘッダ（include/pf/vapi/）は Firmware 型を前方宣言のみで参照する。
add_library(pf_vapi STATIC
    src/pwm_service.cpp
    src/ignition_service.cpp)
target_include_directories(pf_vapi PUBLIC include)
target_link_libraries(pf_vapi PRIVATE pf_fw)

set_project_warnings(pf_vapi)
enable_coverage(pf_vapi)
