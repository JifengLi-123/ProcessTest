# APP モジュール共通定義（選択ビルド・SOTA 対応）
#
# APPS_SHARED=ON のとき、各 APP を共有ライブラリ（.so）としてビルドする。
#   - SOTA（Software OTA）の配布単位 = APP 1 つの .so
#   - SOVERSION は車載 API 契約バージョン（PF_VAPI_VERSION_MAJOR）に一致させる。
#     契約の互換性が保たれる限り、APP の .so 単体を差し替え可能
#   - APP は契約面（pf_vapi_if: ヘッダのみ）にしか依存しないため、
#     .so は PF 実装（pf_vapi/pf_fw）へのリンク依存を持たない
#
# 使い方: add_app_module(<name> <sources...>)
#   → ターゲット app_<name> を生成し、契約面リンク・警告・カバレッジ・
#     install（COMPONENT app_<name>）まで一括設定する。
option(APPS_SHARED "Build APP modules as shared libraries (SOTA unit)" OFF)

# 車載 API 契約バージョン（pf/vapi/vapi_version.hpp と一致させること）
set(PF_VAPI_VERSION_MAJOR 1)
set(PF_VAPI_VERSION_MINOR 0)

function(add_app_module name)
    set(target app_${name})

    if(APPS_SHARED)
        add_library(${target} SHARED ${ARGN})
        set_target_properties(${target} PROPERTIES
            VERSION   ${PF_VAPI_VERSION_MAJOR}.${PF_VAPI_VERSION_MINOR}
            SOVERSION ${PF_VAPI_VERSION_MAJOR}
            POSITION_INDEPENDENT_CODE ON)
        # SOTA 配布単位として COMPONENT ごとに install 可能にする
        #   例: cmake --install build/host-sota --component app_motor_control --prefix dist/
        install(TARGETS ${target}
            LIBRARY DESTINATION lib/apps
            COMPONENT ${target})
    else()
        add_library(${target} STATIC ${ARGN})
    endif()

    target_include_directories(${target} PUBLIC include)
    # 契約面（ヘッダのみ）にのみ依存。PF 実装へのリンク依存を持たせない
    target_link_libraries(${target} PUBLIC pf_vapi_if)

    set_project_warnings(${target})
    enable_coverage(${target})
endfunction()
