# ターゲット（車載 ECU）用クロスツールチェーン定義（SUP.8 管理対象）
# 注: 本サンプルリポジトリのホスト環境にはクロスコンパイラは含まれない。
#     docker/dev.Dockerfile に arm-none-eabi-gcc を追加したうえで
#     `cmake --preset target-arm` で使用する（CMakePresets.json 参照）。
#     実プロジェクトではコンパイラのバージョンを Quality Plan（08-13）に記録し、
#     ツール適格化（ISO 26262 Part 8 §11）の要否を機能安全担当と合意すること。
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# ベアメタル向け: リンクテストを静的ライブラリ生成で代替する
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
