#ifndef CAP_H
#define CAP_H

#include <pin.H>

// support functions
auto cap_initialize                         () noexcept                                                       -> void;
auto cap_initialize_state                   () noexcept                                                       -> void;
auto cap_set_trace_length                   (uint32_t trace_length) noexcept                                  -> void;
auto cap_set_start_address                  (ADDRINT address) noexcept                                        -> void;
auto cap_set_stop_address                   (ADDRINT address) noexcept                                        -> void;
auto cap_add_full_skip_call_address         (ADDRINT address) noexcept                                        -> void;
auto cap_add_selective_skip_address         (ADDRINT address) noexcept                                        -> void;
auto cap_add_auto_skip_call_addresses       (ADDRINT address) noexcept                                        -> void;
auto cap_set_loop_count                     (uint32_t count) noexcept                                         -> void;
auto cap_verify_parameters                  ()                                                                -> void;

auto cap_add_patched_memory_value           (ADDRINT ins_address, UINT32 exec_order, bool be_or_af,
                                             ADDRINT mem_address, UINT8 mem_size, ADDRINT mem_value) noexcept -> void;
auto cap_add_patched_register_value         (ADDRINT ins_address, UINT32 exec_order, bool be_or_af,
                                             REG reg, UINT8 lo_pos, UINT8 hi_pos, ADDRINT reg_value) noexcept -> void;

// report functions
auto cap_save_trace_to_file                 (const std::string& filename, bool simple_or_proto) noexcept      -> void;
auto cap_load_trace_from_file               (const std::string& filename) noexcept                            -> void; // TODO or not TODO
auto cap_save_trace_to_dot_file             (const std::string& filename) noexcept                            -> void;
auto cap_save_basic_block_trace_to_dot_file (const std::string& filename) noexcept                            -> void;
auto cap_save_basic_block_trace_to_file     (const std::string& filename) noexcept                            -> void;

// instrumentation functions
// instruction mode
using ins_instrumentation_t = VOID (*)(INS, VOID*) /*std::add_pointer<VOID(INS, VOID*)>::type*/;
extern ins_instrumentation_t cap_ins_mode_get_ins_info;
extern ins_instrumentation_t cap_patch_instrunction_information;

// trace mode
using trace_instrumentation_t = VOID (*)(TRACE, VOID*);
extern trace_instrumentation_t cap_trace_mode_get_ins_info;
extern trace_instrumentation_t cap_trace_mode_patch_ins_info;

// img mode
using img_instrumentation_t = VOID (*)(IMG, VOID*);
extern img_instrumentation_t cap_img_mode_get_ins_info;

extern SYSCALL_ENTRY_CALLBACK cap_get_syscall_entry_info;
extern SYSCALL_EXIT_CALLBACK cap_get_syscall_exit_info;

//using syscall_instrumentation_t = VOID (*)(THREADID thread_id, const CONTEXT* p_context, SYSCALL_STANDARD std, VOID* data);
//extern ins_instrumentation_t cap_instrument_instruction_not_follow_call;
// the following functions are generated in compile time by template system
//auto cap_instrument_instruction_follow_call     (INS ins, VOID* data) -> VOID;
//auto cap_instrument_instruction_not_follow_call (INS ins, VOID* data) -> VOID;

#endif
