# app_motor_control — MOD-001 MotorController（11-05 Software Unit）
# add_app_module が契約面（pf_vapi_if）のみへのリンク・警告・カバレッジ・
# SOTA 用 install（APPS_SHARED=ON 時）を一括設定する（cmake/AppModule.cmake）。
add_app_module(motor_control
    src/motor_controller.cpp)
