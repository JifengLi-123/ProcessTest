# ProcessTest — SWE プロセスサンプルプロジェクト（PF / APP 2 レイヤー構成）
# 対応工程: SWE.3（ビルド）/ SWE.4〜6（テスト実行）
# 本ファイルは SUP.8（構成管理）の対象として Git 管理する。
cmake_minimum_required(VERSION 3.14)
project(ProcessTest
    VERSION 0.1.0
    DESCRIPTION "A-SPICE SWE process sample: motor control unit (PF/APP layers)"
    LANGUAGES CXX)

# AUTOSAR C++14 Guidelines 準拠のため C++14 を固定する（A1-1-1）
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_COVERAGE "Enable coverage instrumentation (SWE.4 C0/C1)" OFF)

# ---------------------------------------------------------------------------
# PF レイヤー: 車載 API（公開 I/F ヘッダ）
#   実機ではベンダ SDK / AUTOSAR AP の API をラップする実装がリンクされる。
# ---------------------------------------------------------------------------
add_library(pf_api INTERFACE)
target_include_directories(pf_api INTERFACE src/pf/include)

# ---------------------------------------------------------------------------
# PF レイヤー: SIL 実装（ホスト検証用。SWE.5/SWE.6 で使用）
# ---------------------------------------------------------------------------
add_library(pf_sil STATIC
    src/pf/sil/sil_pwm_service.cpp
    src/pf/sil/sil_ignition_service.cpp)
target_include_directories(pf_sil PUBLIC src/pf/sil)
target_link_libraries(pf_sil PUBLIC pf_api)
target_compile_options(pf_sil PRIVATE -Wall -Wextra -Wpedantic -Werror)

# ---------------------------------------------------------------------------
# APP レイヤー: 11-05 Software Unit（SWE.3。MOD-001 MotorController）
# ---------------------------------------------------------------------------
add_library(app_motor_control STATIC
    src/app/motor_control/motor_controller.cpp)
target_include_directories(app_motor_control PUBLIC src/app/motor_control)
target_link_libraries(app_motor_control PUBLIC pf_api)
target_compile_options(app_motor_control PRIVATE -Wall -Wextra -Wpedantic -Werror)

if(ENABLE_COVERAGE)
    target_compile_options(app_motor_control PRIVATE --coverage -O0 -g)
    target_link_options(app_motor_control PUBLIC --coverage)
endif()

# ---------------------------------------------------------------------------
# GoogleTest（SWE.4〜6 テストフレームワーク）
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
# gtest のビルド設定（親プロジェクトのフラグを汚染しない）
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)

# テスト実行ファイルを定義する共通関数
function(add_swe_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${ARGN} GTest::gtest_main)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    gtest_discover_tests(${target})
endfunction()

# SWE.4 ユニット検証（APP 単体。車載 API はテスト内モックに差し替え）
add_swe_test(unit_tests test/unit/test_motor_controller.cpp
    app_motor_control)

# SWE.5 統合検証（APP + PF SIL 実装の統合）
add_swe_test(integration_tests test/integration/test_integration.cpp
    app_motor_control pf_sil)

# SWE.6 適格性テスト（統合済み構成に対する要求ベーステスト）
add_swe_test(qualification_tests test/qualification/test_qualification.cpp
    app_motor_control pf_sil)
