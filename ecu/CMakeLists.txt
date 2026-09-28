# ecu_app — 統合ソフトウェア（01-50 Integrated Software）の生成ターゲット
# 統合ルート（Composition Root）として全レイヤーをここで合成する。
# 統合ルートのみ Firmware（pf_fw）の具象実装に触れてよい（wiring のため）。
# 本サンプルはホスト SIL デモ実行ファイル。実機ビルド（--preset target-arm）では
# Firmware をターゲット HW 実装に差し替えたファームウェアイメージとなる。
add_executable(ecu_app
    src/main.cpp)
target_link_libraries(ecu_app PRIVATE app_motor_control pf_fw)

set_project_warnings(ecu_app)
