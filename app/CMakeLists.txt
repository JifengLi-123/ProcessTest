# APP レイヤー（オーナー: APP 各チーム。.github/CODEOWNERS 参照）
# 各 APP は pf_vapi（車載 API 契約面）のみにリンクできる。
# pf_fw へのリンク・include は禁止（scripts/check_layer_deps.sh が CI で検出）。
add_subdirectory(motor_control)
