# カバレッジ計測（SWE.4: ASIL-B は C0/C1 100% 必須。gcov/gcovr で計測）
# ENABLE_COVERAGE=ON のとき対象ターゲットに計測フラグを付与する。
# 使い方: enable_coverage(<target>)
option(ENABLE_COVERAGE "Enable coverage instrumentation (SWE.4 C0/C1)" OFF)

function(enable_coverage target)
    if(ENABLE_COVERAGE)
        target_compile_options(${target} PRIVATE --coverage -O0 -g)
        target_link_options(${target} PUBLIC --coverage)
    endif()
endfunction()
