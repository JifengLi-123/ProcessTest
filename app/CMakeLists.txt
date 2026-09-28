# APP レイヤー（オーナー: APP 各チーム。.github/CODEOWNERS 参照）
#
# 選択ビルド: ルートの BUILD_APPS（ALL / NONE / アプリ名のセミコロン区切りリスト）
# に従い、app/ 直下のサブディレクトリ（CMakeLists.txt を持つもの）を自動発見して
# 対象のみ add_subdirectory する。CI では scripts/build.py --changed が
# 変更のあった APP だけを指定する（ビルド時間の削減）。
#
# 各 APP は pf_vapi_if（車載 API 契約面）のみにリンクできる。
# pf_fw・pf_vapi 実装へのリンク・include は禁止（scripts/check_layer_deps.sh が CI で検出）。
set(ENABLED_APPS "")

file(GLOB app_dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
list(SORT app_dirs)
foreach(name ${app_dirs})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${name}
       AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}/CMakeLists.txt)
        if(BUILD_APPS STREQUAL "ALL" OR name IN_LIST BUILD_APPS)
            add_subdirectory(${name})
            list(APPEND ENABLED_APPS ${name})
        else()
            message(STATUS "APP skipped by BUILD_APPS: ${name}")
        endif()
    endif()
endforeach()

message(STATUS "Enabled apps: [${ENABLED_APPS}] (BUILD_APPS=${BUILD_APPS})")
set(ENABLED_APPS ${ENABLED_APPS} PARENT_SCOPE)
