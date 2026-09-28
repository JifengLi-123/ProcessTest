# ecu_app — 統合ソフトウェア（01-50 Integrated Software）の生成ターゲット
# 統合ルート（Composition Root）として全レイヤーをここで合成する。
# 統合ルートのみ Firmware（pf_fw）と Vehicle API 実装（pf_vapi）の具象に触れてよい。
#
# 選択ビルド時: 配線対象の APP（motor_control）が BUILD_APPS に含まれない場合、
# 統合ターゲットはスキップされる（APP 単体の CI を高速化するため）。
if(NOT "motor_control" IN_LIST ENABLED_APPS)
    message(STATUS "ecu_app skipped: requires app 'motor_control' (BUILD_APPS=${BUILD_APPS})")
    return()
endif()

add_executable(ecu_app
    src/main.cpp)
target_link_libraries(ecu_app PRIVATE app_motor_control pf_vapi pf_fw)

set_project_warnings(ecu_app)
