# 共通警告フラグ（SWE.3 Step 5 静的解析の前段として最低限の品質ゲート）
# 使い方: set_project_warnings(<target>)
function(set_project_warnings target)
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Werror)
endfunction()
