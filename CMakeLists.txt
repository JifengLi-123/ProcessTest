# ProcessTest — A-SPICE SWE プロセスサンプル（PF / APP / ECU 統合構成）
# トップレベル: project 定義・共通オプション・サブディレクトリ登録のみを行う。
# ビルド構成は CMakePresets.json で固定する（SUP.8 管理対象）。
cmake_minimum_required(VERSION 3.21)
project(ProcessTest
    VERSION 0.2.0
    DESCRIPTION "A-SPICE SWE process sample: motor control (PF/APP/ECU)"
    LANGUAGES CXX)

# AUTOSAR C++14 Guidelines 準拠のため C++14 を固定する（A1-1-1）
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 共通モジュール（警告フラグ・カバレッジ計測）
include(cmake/CompilerWarnings.cmake)
include(cmake/Coverage.cmake)

# レイヤー登録（依存方向: app → pf/vehicle_api → pf/firmware。ecu が統合）
add_subdirectory(pf)
add_subdirectory(app)
add_subdirectory(ecu)

# テスト（ホストビルドでのみ有効。target-arm プリセットでは BUILD_TESTING=OFF）
include(CTest)
if(BUILD_TESTING)
    add_subdirectory(test)
endif()
