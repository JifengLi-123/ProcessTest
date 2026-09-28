# テスト定義（ホストビルドでのみ有効。SWE.4〜SWE.6）
# 工程はラベル（swe4 / swe5 / swe6）とテスト名プレフィックスで識別する。
# 実行例: ctest --preset host-coverage -L swe4

# ---------------------------------------------------------------------------
# GoogleTest（テストフレームワーク。製品コードにはリンクしない）
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# ---------------------------------------------------------------------------
# モックライブラリ
#   vapi_mock: 車載 API（契約面）のモック。APP の SWE.4 はこれで完結する。
#              契約面との乖離を防ぐため PF チームがオーナー（CODEOWNERS）。
#   fw_mock  : Firmware I/F のモック。Vehicle API 自体の SWE.4 に使用する。
# ---------------------------------------------------------------------------
add_library(vapi_mock INTERFACE)
target_include_directories(vapi_mock INTERFACE unit/mocks)
target_link_libraries(vapi_mock INTERFACE pf_vapi)

add_library(fw_mock INTERFACE)
target_include_directories(fw_mock INTERFACE unit/mocks)
target_link_libraries(fw_mock INTERFACE pf_fw)

# ---------------------------------------------------------------------------
# テストターゲット共通定義
#   工程ラベルをテスト名プレフィックス・CTest ラベルとして付与する
# ---------------------------------------------------------------------------
function(add_swe_test target label source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${ARGN} GTest::gtest_main)
    target_compile_options(${target} PRIVATE -Wall -Wextra)
    gtest_discover_tests(${target}
        TEST_PREFIX "${label}."
        PROPERTIES LABELS "${label}")
endfunction()

# SWE.4 ユニット検証
add_swe_test(unit_app_motor_control swe4
    unit/app/motor_control/test_motor_controller.cpp
    app_motor_control vapi_mock)

add_swe_test(unit_pf_vehicle_api swe4
    unit/pf/vehicle_api/test_vehicle_api.cpp
    pf_vapi fw_mock)

# SWE.5 統合検証（APP + Vehicle API + Firmware SIL）
add_swe_test(integration_tests swe5
    integration/test_integration.cpp
    app_motor_control pf_fw)

# SWE.6 適格性テスト（ホスト実行可能サブセット。フルスタック構成）
add_swe_test(qualification_tests swe6
    qualification/test_qualification.cpp
    app_motor_control pf_fw)
