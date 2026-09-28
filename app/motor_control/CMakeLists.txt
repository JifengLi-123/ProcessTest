# app_motor_control — MOD-001 MotorController（11-05 Software Unit）
# リンク先は pf_vapi のみ（04-04 アーキテクチャ制約）。
add_library(app_motor_control STATIC
    src/motor_controller.cpp)
target_include_directories(app_motor_control PUBLIC include)
target_link_libraries(app_motor_control PUBLIC pf_vapi)

set_project_warnings(app_motor_control)
enable_coverage(app_motor_control)
