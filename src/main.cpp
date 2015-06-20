
#include "parsing_helper.h"
#include <pin.H>

#include "lib/tinyformat.h"
#include "lib/type/instruction.h"
#include "lib/cap/cap.h"

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <algorithm>

namespace windows
{
#include <Windows.h>
#include <WinBase.h>
#include <WinInet.h>
#include <io.h>
}

#define PIN_INIT_FAILED 1
#define UNUSED_DATA 0

/*====================================================================================================================*/
/*                                                command line handling functions                                     */
/*====================================================================================================================*/


//KNOB<uint32_t> trace_length_knob (KNOB_MODE_WRITEONCE, "pintool", "le", "1000", "length of trace");

//KNOB<bool> follow_call           (KNOB_MODE_WRITEONCE, "pintool", "fc", "true", "following calls");

KNOB<ADDRINT> start_address_knob                              (KNOB_MODE_WRITEONCE, "pintool", "start",
                                                               "0x0", "tracing start address");

KNOB<ADDRINT> stop_address_knob                               (KNOB_MODE_WRITEONCE, "pintool", "stop",
                                                               "0x0", "tracing stop address");

KNOB<ADDRINT> skip_full_address_knob                          (KNOB_MODE_APPEND, "pintool", "skip-full",
                                                               "0x0", "skipping call address");

KNOB<ADDRINT> skip_selective_address_knob                     (KNOB_MODE_APPEND, "pintool", "skip-selective",
                                                               "0x0", "skipping call address but select syscalls");

KNOB<ADDRINT> skip_auto_address_knob                          (KNOB_MODE_APPEND, "pintool", "skip-auto",
                                                               "0x0", "skipping called address");

KNOB<UINT32> loop_count_knob                                  (KNOB_MODE_WRITEONCE, "pintool", "loop-count", "1", "loop count");

KNOB<string> input_file                                       (KNOB_MODE_WRITEONCE, "pintool", "conf",
                                                               "binsec.conf", "configuration file, for parameterized analysis");

KNOB<string> output_file                                      (KNOB_MODE_WRITEONCE, "pintool", "out",
                                                               "trace.msg", "output file, for resulted trace");

KNOB<bool> output_trace_format                                (KNOB_MODE_WRITEONCE, "pintool", "format",
                                                               "false", "output trace format, 1: protobuf, 0: simple");

const static auto option_default_filename = std::string("9bcbb99f-0eb6-4d28-a876-dea762f5021d");
KNOB<string> option_file                                      (KNOB_MODE_WRITEONCE, "pintool", "opt",
                                                               "9bcbb99f-0eb6-4d28-a876-dea762f5021d", "option file, for parameter");

const static auto trace_dot_default_filename = std::string("c33553b1-57ab-4922-99dc-515eb50e5f51");
KNOB<string> trace_dot_file                                   (KNOB_MODE_WRITEONCE, "pintool", "dot",
                                                               "c33553b1-57ab-4922-99dc-515eb50e5f51", "output dot file, for trace");

const static auto trace_bb_dot_default_filename = std::string("793ded05-53e4-49d2-9d77-faa7f48a4217");
KNOB<string> trace_bb_dot_file                                (KNOB_MODE_WRITEONCE, "pintool", "dot-bb",
                                                               "793ded05-53e4-49d2-9d77-faa7f48a4217", "output file, for basic block graph");

const static auto trace_bb_default_filename = std::string("0acc4fd8-acca-418c-9384-d0dd60ac85c9");
KNOB<string> trace_bb_file                                    (KNOB_MODE_WRITEONCE, "pintool", "trace-bb",
                                                               "0acc4fd8-acca-418c-9384-d0dd60ac85c9", "output file, for basic block trace");

std::ofstream vtrace_logfile = std::ofstream("vtrace.log", std::ofstream::out | std::ofstream::trunc);

/*====================================================================================================================*/
/*                                                     support functions                                              */
/*====================================================================================================================*/

auto get_reg_from_name (const std::string& reg_name) -> REG
{
  auto upper_reg_name = boost::to_upper_copy(reg_name);
  std::underlying_type<REG>::type reg_id;
  for (reg_id = REG_INVALID_ ; reg_id < REG_LAST; ++reg_id) {
    if (boost::to_upper_copy(REG_StringShort((REG)reg_id)) == upper_reg_name) {
      break;
    }
  }
  return (REG)reg_id;
}


auto load_option_from_file (const std::string& filename) -> void
{
  std::ifstream opt_file(filename.c_str(), std::ifstream::in);

  if (opt_file.is_open()) {
    auto line = std::string();
    while (std::getline(opt_file, line)) {
      line = boost::trim_copy(line);

      if (line.front() != '#') {
        auto field = std::vector<std::string>();
        boost::split(field, line, boost::is_any_of(","), boost::token_compress_on);

        auto unconverted_idx = std::size_t{0};
        if (field.size() == 2) {
          if (field[0] == "start") {
            auto opt_start = std::stoul(field[1], &unconverted_idx, 16);
            cap_set_start_address(opt_start);
            tfm::printfln("add start address: %s", StringFromAddrint(opt_start));
          }

          if (field[0] == "stop") {
            auto opt_stop = std::stoul(field[1], &unconverted_idx, 16);
            cap_set_stop_address(opt_stop);
            tfm::printfln("add stop address: %s", StringFromAddrint(opt_stop));
          }

          if (field[0] == "skip-full") {
            auto opt_skip_full = std::stoul(field[1], &unconverted_idx, 16);
            cap_add_full_skip_call_address(opt_skip_full);
            tfm::printfln("add skip full address: %s", StringFromAddrint(opt_skip_full));
          }

          if (field[0] == "skip-selective") {
            auto opt_skip_select = std::stoul(field[1], &unconverted_idx, 16);
            cap_add_selective_skip_address(opt_skip_select);
          }

          if (field[0] == "skip-auto") {
            auto opt_skip_auto = std::stoul(field[1], &unconverted_idx, 16);
            cap_add_auto_skip_call_addresses(opt_skip_auto);
            tfm::printfln("add skip auto address: %s", StringFromAddrint(opt_skip_auto));
          }
        }
      }
    }
  }

  return;
}


auto load_configuration_from_file (const std::string& filename) -> void
{
  std::ifstream config_file(filename.c_str(), std::ifstream::in);
  auto line = std::string();
  while (std::getline(config_file, line)) {

    line = boost::trim_copy(line);

    if (line.front() != '#') {

      auto field = std::vector<std::string>();
      boost::split(field, line, boost::is_any_of(","), boost::token_compress_on);

      //    tfm::printf("address: %s order: %s info: %s value: %s before/after: %s\n", field[0], field[1], field[2], field[3]);
      //    PIN_ExitProcess(0);

      auto unconverted_idx = std::size_t{0};

      if (std::count(field[2].begin(), field[2].end(), ':') == 1) {

        auto addr_val_strs = std::vector<std::string>();
        boost::split(addr_val_strs, field[2], boost::is_any_of(":"), boost::token_compress_on);

        cap_add_patched_memory_value(std::stoul(field[0], &unconverted_idx, 16),         // address of the instruction
                                     std::stoul(field[1]),                               // execution order
                                     (field[4] == "1"),                                  // patching point (false = before, true = after)
                                     std::stoul(addr_val_strs[0], &unconverted_idx, 16), // memory address
                                     std::stoul(addr_val_strs[1]),                       // memory size
                                     std::stoul(field[3], &unconverted_idx, 16)          // memory value
                                     );

        tfm::printf("need to patch memory address %s of size %d by value %s\n",
                    StringFromAddrint(std::stoul(addr_val_strs[0], &unconverted_idx, 16)),
                    std::stoul(addr_val_strs[1]),
                    StringFromAddrint(std::stoul(field[3], &unconverted_idx, 16))
                    );
      }
      else {
        assert(std::count(field[2].begin(), field[2].end(), ':') == 2);

        auto reg_lo_hi_pos_strs = std::vector<std::string>();
        boost::split(reg_lo_hi_pos_strs, field[2], boost::is_any_of(":"), boost::token_compress_on);

        cap_add_patched_register_value(std::stoul(field[0], &unconverted_idx, 16), // address of the instruction
                                       std::stoul(field[1]),                       // execution order of the instruction
                                       (field[4] == "1"),                          // patching point (false = before, true = after)
                                       get_reg_from_name(reg_lo_hi_pos_strs[0]),   // register name
                                       std::stoul(reg_lo_hi_pos_strs[1]),          // low bit position
                                       std::stoul(reg_lo_hi_pos_strs[2]),          // hight bit position
                                       std::stoul(field[3], &unconverted_idx, 16)  // register value
            );

        tfm::printf("need to patch %s [%s-%s] with value %d\n",
                    reg_lo_hi_pos_strs[0], reg_lo_hi_pos_strs[1], reg_lo_hi_pos_strs[2],
                    std::stoul(field[3], &unconverted_idx, 16));
      }
    }
  }
  return;
}


auto load_configuration_and_options () -> void
{
  cap_initialize();

  cap_set_start_address(start_address_knob.Value());
  cap_set_stop_address(stop_address_knob.Value());

  for (uint32_t i = 0; i < skip_full_address_knob.NumberOfValues(); ++i) {
    cap_add_full_skip_call_address(skip_full_address_knob.Value(i));
  }

  for (uint32_t i = 0; i < skip_selective_address_knob.NumberOfValues(); ++i) {
    cap_add_selective_skip_address(skip_selective_address_knob.Value(i));
  }

  for (uint32_t i = 0; i < skip_auto_address_knob.NumberOfValues(); ++i) {
    cap_add_auto_skip_call_addresses(skip_auto_address_knob.Value(i));
  }

  load_configuration_from_file(input_file.Value());

  if (option_file.Value() != option_default_filename) {
    tfm::printfln("load parameters from file %s...", option_file.Value());
    load_option_from_file(option_file.Value());
  }

  cap_set_loop_count(loop_count_knob.Value());

  cap_initialize_state();

//  assert(start_address != 0x0);

//  cap_verify_parameters();

//  tfm::printfln("code cache block size: %d", CODECACHE_BlockSize());
//  tfm::printfln("code cache limit size: %d", CODECACHE_CacheSizeLimit());

  return;
}

auto stop_pin (INT32 code, VOID* data) -> VOID
{
  tfm::printfln("save trace...");
  tfm::format(vtrace_logfile, "save trace\n");

  cap_save_trace_to_file(output_file.Value(), output_trace_format.Value());

  if (trace_dot_file.Value() != trace_dot_default_filename) {
    tfm::printfln("save trace to dot file %s...", trace_dot_file.Value());
    cap_save_trace_to_dot_file(trace_dot_file.Value());
  }

  if (trace_bb_dot_file.Value() != trace_bb_dot_default_filename) {
    tfm::printfln("save basic block CFG to dot file %s...", trace_bb_dot_file.Value());
    cap_save_basic_block_trace_to_dot_file(trace_bb_dot_file.Value());
  }

  if (trace_bb_file.Value() != trace_bb_default_filename) {
    tfm::printfln("save basic block trace to file %s...", trace_bb_file.Value());
    cap_save_basic_block_trace_to_file(trace_bb_file.Value());
  }

  return;
}

auto reattach_console () -> void
{
  if (windows::AttachConsole((windows::DWORD) - 1)) {
    auto hCrt = windows::_open_osfhandle((long)windows::GetStdHandle((windows::DWORD) - 11), 0);
    auto hf = _fdopen(hCrt, "w");
    *stdout = *hf;
    setvbuf(stdout, NULL, _IONBF, 0);
  }
  return;
}


/*====================================================================================================================*/
/*                                                      main function                                                 */
/*====================================================================================================================*/


auto main(int argc, char* argv[]) -> int
{
  reattach_console();

  // symbol of the binary should be initialized first
  tfm::printfln("initialize image symbols...");
//  tfm::format(vtrace_logfile, "initialize image symbols...");
  PIN_InitSymbols();

  if (PIN_Init(argc, argv)) {
    tfm::printfln("%s", KNOB_BASE::StringKnobSummary());
    PIN_ExitProcess(PIN_INIT_FAILED);
  }
  else {
    tfm::printfln("initialize Pin success...");

    tfm::printfln("load configuration and options...");
    load_configuration_and_options();
//    tfm::printfln("add start function...");
//    PIN_AddApplicationStartFunction(load_configuration_and_options, UNUSED_DATA);

//    INS_AddInstrumentFunction(cap_patch_instrunction_information, UNUSED_DATA);
//    INS_AddInstrumentFunction(cap_get_instruction_information, UNUSED_DATA);

    tfm::printfln("pre-processing instructions...");
//    IMG_AddInstrumentFunction(cap_img_mode_get_ins_info, UNUSED_DATA);

    tfm::printfln("register trace-based instruction instrumentation...");
//    TRACE_AddInstrumentFunction(cap_trace_mode_patch_ins_info, UNUSED_DATA);
    TRACE_AddInstrumentFunction(cap_trace_mode_get_ins_info, UNUSED_DATA);

    tfm::printfln("register syscall instruction instrumentation...");
//    PIN_AddSyscallEntryFunction(cap_get_syscall_entry_info, UNUSED_DATA);
//    PIN_AddSyscallExitFunction(cap_get_syscall_exit_info, UNUSED_DATA);

    tfm::printfln("add fini function");
    PIN_AddFiniFunction(stop_pin, UNUSED_DATA);

    tfm::printfln("pass control to Pin...");
    PIN_StartProgram();
  }

  // this return command never executes
  return 0;
}
