# テスト定義（ホストビルドでのみ有効。SWE.4〜SWE.6）
# 工程はラベル（swe4 / swe5 / swe6）とテスト名プレフィックスで識別する。
# 実行例: ctest --preset host-coverage -L swe4
#
# 選択ビルド対応: 各 APP のテストはその APP が ENABLED_APPS に含まれるときのみ
# 定義する。統合（SWE.5）・適格性（SWE.6）テストは配線対象の motor_control を
# 必要とするため、同様にゲートする。PF（vehicle_api）のテストは常に定義する。

# ---------------------------------------------------------------------------
# GoogleTest（テストフレームワーク。製品コードにはリンクしない）
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    # 取得物の完全性検証（サプライチェーン対策。バージョン更新時はハッシュも更新すること）
    URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# ---------------------------------------------------------------------------
# モックライブラリ
#   vapi_mock: 車載 API（契約面）のモック。APP の SWE.4 はこれで完結する。
#              契約面（pf_vapi_if）のみに依存し、PF 実装をリンクしない。
#   fw_mock  : Firmware I/F のモック。Vehicle API 自体の SWE.4 に使用する。
# ---------------------------------------------------------------------------
add_library(vapi_mock INTERFACE)
target_include_directories(vapi_mock INTERFACE unit/mocks)
target_link_libraries(vapi_mock INTERFACE pf_vapi_if)

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

# ---------------------------------------------------------------------------
# SWE.4 ユニット検証 — PF Vehicle API（常時）
# ---------------------------------------------------------------------------
add_swe_test(unit_pf_vehicle_api swe4
    unit/pf/vehicle_api/test_vehicle_api.cpp
    pf_vapi fw_mock)

# ---------------------------------------------------------------------------
# SWE.4 ユニット検証 — 各 APP（自動発見）
#   test/unit/app/<name>/*.cpp を APP ごとに自動登録する（選択ビルド対象のみ）。
#   新規 APP は scripts/new_app.py で雛形生成すれば追加登録は不要（構造＝登録）。
#   テストが無い APP は警告を出す（ASIL 対象はカバレッジゲートで別途検出される）。
# ---------------------------------------------------------------------------
foreach(app_name ${ENABLED_APPS})
    file(GLOB app_test_sources CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/unit/app/${app_name}/*.cpp)
    if(app_test_sources)
        add_swe_test(unit_app_${app_name} swe4
            "${app_test_sources}"
            app_${app_name} vapi_mock)
    else()
        message(WARNING
            "APP '${app_name}' にユニットテストがありません"
            "（test/unit/app/${app_name}/ が空。SWE.4 未実施）")
    endif()
endforeach()

# ---------------------------------------------------------------------------
# SWE.5 統合検証 / SWE.6 適格性テスト（配線対象 motor_control が必要）
# ---------------------------------------------------------------------------
if("motor_control" IN_LIST ENABLED_APPS)
    add_swe_test(integration_tests swe5
        integration/test_integration.cpp
        app_motor_control pf_vapi pf_fw)

    add_swe_test(qualification_tests swe6
        qualification/test_qualification.cpp
        app_motor_control pf_vapi pf_fw)
else()
    message(STATUS "SWE.5/SWE.6 tests skipped: requires app 'motor_control'")
endif()
