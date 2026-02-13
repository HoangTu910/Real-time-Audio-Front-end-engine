# ── RTAFE example programs ───────────────────────────────────────────────────

# Offline file-based processing
add_executable(host_file_process host_file_process.c)
target_link_libraries(host_file_process PRIVATE rtafe_core)

# RTOS / ISR integration skeleton (compile-check only, not linked to HW)
add_executable(rtos_isr_skeleton rtos_isr_skeleton.c)
target_link_libraries(rtos_isr_skeleton PRIVATE rtafe_core)
