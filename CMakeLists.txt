# ProcessTest — A-SPICE SWE プロセスサンプル（PF / APP / ECU 統合構成）
# トップレベル: project 定義・共通オプション・サブディレクトリ登録のみを行う。
# ビルド構成は CMakePresets.json で固定する（SUP.8 管理対象）。
cmake_minimum_required(VERSION 3.21)
project(ProcessTest
    VERSION 0.3.0
    DESCRIPTION "A-SPICE SWE process sample: motor control (PF/APP/ECU)"
    LANGUAGES CXX)

# AUTOSAR C++14 Guidelines 準拠のため C++14 を固定する（A1-1-1）
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 選択ビルド: ビルド対象 APP を指定する（CI の時間削減。scripts/build.py 参照）
#   ALL  : app/ 配下の全 APP（デフォルト）
#   NONE : APP を一切ビルドしない（PF のみ。docs 変更時の最小ビルド等）
#   リスト: セミコロン区切りの APP 名（例: -DBUILD_APPS=motor_control;hvac）
set(BUILD_APPS "ALL" CACHE STRING "Apps to build: ALL, NONE, or semicolon-separated app names")

# 共通モジュール（警告フラグ・カバレッジ計測・APP モジュール定義）
include(cmake/CompilerWarnings.cmake)
include(cmake/Coverage.cmake)
include(cmake/AppModule.cmake)

# レイヤー登録（依存方向: app → pf/vehicle_api → pf/firmware。ecu が統合）
add_subdirectory(pf)
add_subdirectory(app)

# BUILD_APPS に存在しない APP 名が指定されていないか検証する
if(NOT BUILD_APPS STREQUAL "ALL" AND NOT BUILD_APPS STREQUAL "NONE")
    foreach(requested ${BUILD_APPS})
        if(NOT requested IN_LIST ENABLED_APPS)
            message(FATAL_ERROR
                "BUILD_APPS に不明な APP が指定されました: '${requested}'"
                "（app/ 配下に存在しません）")
        endif()
    endforeach()
endif()

add_subdirectory(ecu)

# テスト（ホストビルドでのみ有効。target-arm プリセットでは BUILD_TESTING=OFF）
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
