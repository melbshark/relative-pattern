#include "cap.h"
#include "trace.h"

#include "../../parsing_helper.h"
#include <pin.H>
#include "../tinyformat.h"

#include <fstream>
#include <cassert>

extern auto normalize_hex_string (const std::string& input) -> std::string;

auto real_value_of_reg (const dyn_reg_t& reg_val) -> ADDRINT
{
  auto reg_size = REG_Size(std::get<0>(reg_val));
  assert((reg_size == 1) || (reg_size == 2) || (reg_size == 4) || (reg_size == 8));

  auto real_val = ADDRINT{0};
  switch (reg_size) {
  case 1:
    real_val = std::get<1>(reg_val).byte[0];
    break;

  case 2:
    real_val = std::get<1>(reg_val).word[0];
    break;

  case 4:
    real_val = std::get<1>(reg_val).dword[0];
    break;

  case 8:
    real_val = std::get<1>(reg_val).qword[0];
    break;
  }

  return real_val;
};

auto real_value_of_mem (const std::pair<dyn_mem_t, ADDRINT>& mem_val) -> ADDRINT
{
  auto mem_size = std::get<1>(std::get<0>(mem_val));
  assert((mem_size == 1) || (mem_size == 2) || (mem_size == 4) || (mem_size == 8));

  auto real_val = ADDRINT{0};
  switch (mem_size) {
  case 1:
    real_val = static_cast<uint8_t>(std::get<1>(mem_val));
    break;

  case 2:
    real_val = static_cast<uint16_t>(std::get<1>(mem_val));
    break;

  case 4:
    real_val = static_cast<uint32_t>(std::get<1>(mem_val));
    break;

  case 8:
    real_val = static_cast<uint64_t>(std::get<1>(mem_val));
    break;
  }

  return real_val;
}

auto save_in_simple_format (std::ofstream& output_stream) -> void
{
  tfm::printfln("trace length %d", trace.size());

  for (const dyn_ins_t& ins : trace) {
    auto ins_addr = std::get<INS_ADDRESS>(ins);
    tfm::format(output_stream, "%-12s %-40s\n", normalize_hex_string(StringFromAddrint(ins_addr)),
                cached_ins_at_addr[ins_addr]->disassemble);
  }

//  std::for_each(trace.begin(), trace.end(), [&output_stream](decltype(trace)::const_reference ins)
//  {
//    auto ins_addr = std::get<INS_ADDRESS>(ins);
//    tfm::format(output_stream, "%-12s %-40s", normalize_hex_string(StringFromAddrint(ins_addr)),
//                cached_ins_at_addr[ins_addr]->disassemble);

//    tfm::format(output_stream, "  RR: ");
//    for (const auto& reg_val : std::get<INS_READ_REGS>(ins)) {
//      tfm::format(output_stream, "[%s:%s]", REG_StringShort(std::get<0>(reg_val)),
//                  normalize_hex_string(StringFromAddrint(real_value_of_reg(reg_val))));
//    }

//    tfm::format(output_stream, "  RW: ");
//    for (const auto& reg_val : std::get<INS_WRITE_REGS>(ins)) {
//      tfm::format(output_stream, "[%s:%s]", REG_StringShort(std::get<0>(reg_val)),
//                  normalize_hex_string(StringFromAddrint(real_value_of_reg(reg_val))));
//    }

//    tfm::format(output_stream, "  MR: ");
//    for (const auto & mem_val : std::get<INS_READ_MEMS>(ins)) {
//      tfm::format(output_stream, "[%s:%d:%s]", normalize_hex_string(StringFromAddrint(std::get<0>(std::get<0>(mem_val)))),
//                  std::get<1>(std::get<0>(mem_val)), normalize_hex_string(StringFromAddrint(real_value_of_mem(mem_val))));
//    }

//    tfm::format(output_stream, "  MW: ");
//    for (const auto & mem_val : std::get<INS_WRITE_MEMS>(ins)) {
//      tfm::format(output_stream, "[%s:%d:%s]", normalize_hex_string(StringFromAddrint(std::get<0>(std::get<0>(mem_val)))),
//                  std::get<1>(std::get<0>(mem_val)), normalize_hex_string(StringFromAddrint(real_value_of_mem(mem_val))));
//    }

//    if (cached_ins_at_addr[ins_addr]->is_syscall) {
//      auto concret_info = std::get<INS_CONCRETE_INFO>(ins);
//      switch (concret_info.which())
//      {
//      case 0: /* SYS_OPEN */
//      {
//        auto open_concret_info = boost::get<sys_open_info_t>(concret_info);
//        tfm::format(output_stream, " ID: %d[%s:%d]", sys_open_info_t::id,
//                    open_concret_info.path_name, open_concret_info.file_desc);
//        break;
//      }

//      case 1: /* SYS_READ */
//      {
//        auto read_concret_info = boost::get<sys_read_info_t>(concret_info);
//        tfm::format(output_stream, "  ID: %d[%s:%d:%d:%c]", sys_read_info_t::id,
//                    normalize_hex_string(StringFromAddrint(read_concret_info.buffer_addr)),
//                    read_concret_info.buffer_length, read_concret_info.read_length,
//                    read_concret_info.buffer.get()[0]);
//        break;
//      }

//      case 2: /* SYS_WRITE */
//      {
//        auto write_concret_info = boost::get<sys_write_info_t>(concret_info);
//        tfm::format(output_stream, " ID: %d[%s:%d:%d:%c]", sys_open_info_t::id,
//                    normalize_hex_string(StringFromAddrint(write_concret_info.buffer_addr)),
//                    write_concret_info.buffer_length, write_concret_info.write_length,
//                    write_concret_info.buffer.get()[0]);
//        break;
//      }

//      case 3: /* SYS_OTHER */
//      {
//        auto other_concret_info = boost::get<sys_other_info_t>(concret_info);
//        tfm::format(output_stream, " ID: %d", other_concret_info.real_id);
//        break;
//      }
//      }
//    }

//    tfm::format(output_stream, "\n");
//  });
  return;
}

auto save_virtual_trace (std::ofstream& output_stream) -> void
{
  for (const dyn_ins_t& ins : trace) {
    auto ins_address = std::get<INS_ADDRESS>(ins);

      if (ins_address == 0x42d1de) {
        for (const auto& reg_val : std::get<INS_READ_REGS>(ins)) {
          if (REG_StringShort(std::get<0>(reg_val)) == "esi") {
            tfm::format(output_stream, "0x%x ", real_value_of_reg(reg_val));
            break;
          }
        }
      }
  }
  return;
}


//static auto set_trace_header (parser::trace_t& trace) noexcept -> void
//{
//  auto p_header = trace.mutable_header();

//  static_assert((sizeof(ADDRINT) == 4) || (sizeof(ADDRINT) == 8), "address size not supported");

//  switch (sizeof(ADDRINT)) {
//  case 4:
//    p_header->set_architecture(parser::X86);
//    p_header->set_address_size(parser::BIT32);
//    break;

//  case 8:
//    p_header->set_architecture(parser::X86_64);
//    p_header->set_address_size(parser::BIT64);
//    break;
//  }
//  return;
//}


//static auto add_trace_module (parser::trace_t& trace, const std::string& module_name) noexcept -> void
//{
//  // add a body element
//  auto p_body = trace.add_body();
//  p_body->set_typeid_(parser::METADATA);

//  // set this body as metadata
//  p_body->clear_instruction();
//  auto p_metadata = p_body->mutable_metadata();

//  // set the metadata as a module
//  p_metadata->set_typeid_(parser::MODULE_TYPE);
//  p_metadata->clear_exception_metadata();
//  p_metadata->clear_layer_metadata();
////  p_metadata->clear_exception();

//  // update info for the module
//  auto p_module = p_metadata->mutable_module_metadata();
//  p_module->set_name(module_name);

//  return;
//}


//static auto add_trace_instruction (parser::trace_t& trace, const dyn_ins_t& ins) noexcept -> void
//{
//  auto ins_address = std::get<INS_ADDRESS>(ins);
//  auto p_static_ins = cached_ins_at_addr[ins_address];

//  // add a new body as an instruction
//  auto p_ins_body = trace.add_body();
//  p_ins_body->set_typeid_(parser::INSTRUCTION);
//  p_ins_body->clear_metadata();

//  // create an instruction for this body, and set some information
//  auto p_instruction = p_ins_body->mutable_instruction();
//  p_instruction->set_thread_id(std::get<INS_THREAD_ID>(ins));
//  p_instruction->set_opcode(reinterpret_cast<uint8_t*>(ins_address), p_static_ins->opcode_size);
////  for (auto i = 0; i < inst_at_addr[ins_address]->opcode_size; ++i) {
////    tfm::printf("%02x ", reinterpret_cast<uint8_t*>(ins_address)[i]);
////  }
////  tfm::printf("\n");

//  auto p_ins_addr = p_instruction->mutable_address();

//  static_assert(((sizeof(ADDRINT) == 4) || (sizeof(ADDRINT) == 8)), "address size must be 32 or 64 bit");

//  switch (sizeof(ADDRINT)) {
//  case 4:
//    p_ins_addr->set_typeid_(parser::BIT32);
//    p_ins_addr->set_value_32(ins_address);
//    p_ins_addr->clear_value_64();
//    break;

//  case 8:
//    p_ins_addr->set_typeid_(parser::BIT64);
//    p_ins_addr->set_value_64(ins_address);
//    p_ins_addr->clear_value_32();
//    break;
//  }

//  enum REG_T { REG_READ = 0, REG_WRITE = 1 };
//  auto add_registers = [&p_instruction, &ins, &p_static_ins](REG_T reg_type) -> void
//  {
//    const auto & regs = (reg_type == REG_READ) ? p_static_ins->src_registers : p_static_ins->dst_registers;
//    auto value_of_reg = (reg_type == REG_READ) ? std::get<INS_READ_REGS>(ins) : std::get<INS_WRITE_REGS>(ins);
//    auto reg_typeid = (reg_type == REG_READ) ? parser::REGREAD : parser::REGWRITE;

//    std::for_each(std::begin(regs), std::end(regs), [&](REG pin_reg)
//    {
//      // create a new concrete info
//      auto p_new_con_info = p_instruction->add_concrete_info();

//      // set corresponding type for the concrete info (REGLOAD or REGSTORE)
//      p_new_con_info->set_typeid_(reg_typeid);

//      // allocate a new register for the concrete info, set its name
//      auto p_new_reg =
//          (reg_type == REG_READ) ? p_new_con_info->mutable_read_register() : p_new_con_info->mutable_write_register();
//      p_new_reg->set_name(REG_StringShort(pin_reg));

//      // then set its value
//      auto p_reg_value = p_new_reg->mutable_value();
//      switch (REG_Width(pin_reg)) { // or we can use REG_Size
//      case REGWIDTH_8:
//        p_reg_value->set_typeid_(parser::BIT8);
//        p_reg_value->set_value_8(value_of_reg[pin_reg].byte[0]);
//        break;

//      case REGWIDTH_16:
//        p_reg_value->set_typeid_(parser::BIT16);
//        p_reg_value->set_value_16(value_of_reg[pin_reg].word[0]);
//        break;

//      case REGWIDTH_32:
//        p_reg_value->set_typeid_(parser::BIT32);
//        p_reg_value->set_value_32(value_of_reg[pin_reg].dword[0]);
//        break;

//      case REGWIDTH_64:
//        p_reg_value->set_typeid_(parser::BIT64);
//        p_reg_value->set_value_64(value_of_reg[pin_reg].qword[0]);
//        break;

//      default:
//        break;
//      }
//    });
//    return;
//  };

//  enum MEM_T { MEM_READ = 0, MEM_WRITE = 1 };
//  auto add_mems = [&p_instruction, &ins](MEM_T mem_type) -> void
//  {
//    assert((mem_type == MEM_READ) || (mem_type == MEM_WRITE));

//    auto mems = (mem_type == MEM_READ) ? std::get<INS_READ_MEMS>(ins) : std::get<INS_WRITE_MEMS>(ins);

//    for (auto const& addr_val : mems) {
//      // add a new concrete info and set it by the memory instance
//      auto new_mem_con_info = p_instruction->add_concrete_info();
//      switch (mem_type) {
//      case MEM_READ:
//        new_mem_con_info->set_typeid_(parser::MEMLOAD);
//        break;

//      case MEM_WRITE:
//        new_mem_con_info->set_typeid_(parser::MEMSTORE);
//        break;
//      }

//      // add a new memory instance
//      auto new_mem = (mem_type == MEM_READ) ?
//            new_mem_con_info->mutable_load_memory() : new_mem_con_info->mutable_store_memory();
//      auto new_mem_addr = new_mem->mutable_address();
//      auto new_mem_val = new_mem->mutable_value();

//      auto pin_mem_addr_size = std::get<0>(addr_val);
//      auto pin_mem_addr      = std::get<0>(pin_mem_addr_size);
//      auto pin_mem_size      = std::get<1>(pin_mem_addr_size);
//      auto pin_mem_val       = std::get<1>(addr_val);

//      static_assert((sizeof(ADDRINT) == 4) || (sizeof(ADDRINT) == 8), "address size not supported");

//      switch (sizeof(ADDRINT)) {
//      case 4:
//        new_mem_addr->set_typeid_(parser::BIT32);
//        new_mem_addr->set_value_32(pin_mem_addr);
//        break;

//      case 8:
//        new_mem_addr->set_typeid_(parser::BIT64);
//        new_mem_addr->set_value_64(pin_mem_addr);
//        break;
//      }

//      assert((pin_mem_size == 1) || (pin_mem_size == 2) || (pin_mem_size == 4) || (pin_mem_size == 8));

//      switch (pin_mem_size) {
//      case 1:
//        new_mem_val->set_typeid_(parser::BIT8);
//        new_mem_val->set_value_8(pin_mem_val);
//        break;

//      case 2:
//        new_mem_val->set_typeid_(parser::BIT16);
//        new_mem_val->set_value_16(pin_mem_val);
//        break;

//      case 4:
//        new_mem_val->set_typeid_(parser::BIT32);
//        new_mem_val->set_value_32(pin_mem_val);
//        break;

//      case 8:
//        new_mem_val->set_typeid_(parser::BIT64);
//        new_mem_val->set_value_64(pin_mem_val);
//        break;
//      }
//    }
//    return;
//  };

//  // This lambda is verbose since lambda function is mono-morphic.
//  auto add_syscall_concrete_info = [&p_instruction, &ins](const concrete_info_t& syscall_info) -> void {

//    auto syscall_idx = syscall_info.which();
//    assert((0 <= syscall_idx) && (syscall_idx <= 3));

//    auto concrete_info = p_instruction->add_concrete_info();
//    concrete_info->set_typeid_(parser::SYSCALL);

//    auto sys_concrete_info = concrete_info->mutable_system_call();
//    auto sys_sup_info = sys_concrete_info->mutable_info();

//    switch (syscall_idx)
//    {
//    case 0: /* SYS_OPEN */
//    {
//      sys_concrete_info->set_id(SYS_OPEN);
//      sys_sup_info->set_typeid_(parser::OPEN_SYSCALL);

//      auto sys_open_info = boost::get<sys_open_info_t>(syscall_info);

//      auto sys_open_concrete_info = sys_sup_info->mutable_open_syscall();
//      sys_open_concrete_info->set_file_name(sys_open_info.path_name);
//      sys_open_concrete_info->set_flags(sys_open_info.flags);
//      sys_open_concrete_info->set_mode(sys_open_info.mode);
//      sys_open_concrete_info->set_file_descriptor(sys_open_info.file_desc);

//      break;
//    }

//    case 1: /* SYS_READ */
//    {
//      sys_concrete_info->set_id(SYS_READ);
//      sys_sup_info->set_typeid_(parser::READ_SYSCALL);

//      auto sys_read_info = boost::get<sys_read_info_t>(syscall_info);

//      auto sys_read_concrete_info = sys_sup_info->mutable_read_syscall();
//      sys_read_concrete_info->set_file_descriptor(sys_read_info.file_desc);

//      auto read_buffer = sys_read_concrete_info->mutable_buffer_address();
//      switch (sizeof(ADDRINT)) {
//      case 4:
//        read_buffer->set_typeid_(parser::BIT32);
//        read_buffer->set_value_32(sys_read_info.buffer_addr);
//        break;

//      case 8:
//        read_buffer->set_typeid_(parser::BIT64);
//        read_buffer->set_value_64(sys_read_info.buffer_addr);
//        break;
//      }

//      sys_read_concrete_info->set_count(sys_read_info.buffer_length);
//      sys_read_concrete_info->set_count_effective(sys_read_info.read_length);
//      sys_read_concrete_info->set_buffer_data(sys_read_info.buffer.get(), sys_read_info.read_length);

//      break;
//    }

//    case 2: /* SYS_WRITE */
//    {
//      sys_concrete_info->set_id(SYS_WRITE);
//      sys_sup_info->set_typeid_(parser::WRITE_SYSCALL);

//      auto sys_write_info = boost::get<sys_write_info_t>(syscall_info);

//      auto sys_write_concrete_info = sys_sup_info->mutable_write_sycall();
//      sys_write_concrete_info->set_file_descriptor(sys_write_info.file_desc);

//      auto write_buffer = sys_write_concrete_info->mutable_buffer_address();
//      switch (sizeof(ADDRINT)) {
//      case 4:
//        write_buffer->set_typeid_(parser::BIT32);
//        write_buffer->set_value_32(sys_write_info.buffer_addr);
//        break;

//      case 8:
//        write_buffer->set_typeid_(parser::BIT64);
//        write_buffer->set_value_64(sys_write_info.buffer_addr);
//        break;
//      }

//      sys_write_concrete_info->set_count(sys_write_info.buffer_length);
//      sys_write_concrete_info->set_count_effective(sys_write_info.write_length);
//      sys_write_concrete_info->set_buffer_data(sys_write_info.buffer.get(), sys_write_info.buffer_length);

//      break;
//    }

//    case 3: /* SYS_OTHER */
//    {
////      auto sys_other_info = boost::get<sys_other_info_t>(syscall_info);
////      sys_concrete_info->set_id(SYS_OTHER);
//      break;
//    }
//    }

//    return;
//  };

//  auto add_call_concrete_info = [&p_instruction, &ins](const concrete_info_t& call_info) -> void {
//    assert(call_info.which() == 4); // magic value from concrete_info_t in trace.h

//    auto call_real_info = boost::get<call_info_t>(call_info);

//    auto concrete_info = p_instruction->add_concrete_info();
//    concrete_info->set_typeid_(parser::CALL);

//    auto call_concrete_info = concrete_info->mutable_call();
//    call_concrete_info->set_func_name(call_real_info.called_fun_name);

//    call_concrete_info->set_is_traced(call_real_info.is_traced);

//    auto func_addr = call_concrete_info->mutable_func_addr();
//    switch (sizeof(ADDRINT)) {
//    case 4:
//      func_addr->set_typeid_(parser::BIT32);
//      func_addr->set_value_32(call_real_info.called_fun_addr);
//      break;

//    case 8:
//      func_addr->set_typeid_(parser::BIT64);
//      func_addr->set_value_64(call_real_info.called_fun_addr);
//      break;
//    }

//    return;
//  };

//  if (p_static_ins->is_special) {
//    auto concrete_info = p_instruction->add_concrete_info();
//    concrete_info->set_typeid_(parser::NOT_RETRIEVED);
//  }
//  else {
//    // set read/write registers
//    add_registers(REG_READ);
//    add_registers(REG_WRITE);

//    // set read/write memories
//    add_mems(MEM_READ);
//    add_mems(MEM_WRITE);

//    if (p_static_ins->is_syscall) {
//      auto syscall_info = std::get<INS_CONCRETE_INFO>(ins);
//      if (syscall_info.which() <= 2) { // 2 magic value from concrete_info_t in trace.h
//        add_syscall_concrete_info(syscall_info);
//      }
//    }

//    if (p_static_ins->is_call) {
//      auto call_info = std::get<INS_CONCRETE_INFO>(ins);
//      add_call_concrete_info(call_info);
//    }
//  }

//  return;
//}


//auto save_in_protobuf_format (std::ofstream& output_stream) -> void
//{
//  // create an instance of trace
//  auto trace_saver = parser::trace_t();

//  // set trace header
//  set_trace_header(trace_saver);

//  // set trace body
//  auto prev_module_name = std::string("");
////  tfm::printfln("trace size: %d", trace.size());
//  std::for_each(trace.begin(), trace.end(), [&](decltype(trace)::const_reference ins)
//  {
//    auto ins_address = std::get<INS_ADDRESS>(ins);
//    auto p_static_ins = cached_ins_at_addr[ins_address];

//    // add a new module if the current instruction locates in a new module
//    auto curr_module_name = p_static_ins->including_image;
//    if (prev_module_name != curr_module_name) {        // current instruction located in a new module
//      add_trace_module(trace_saver, curr_module_name); // add a new module
//      prev_module_name = curr_module_name;             // update the module name
//    }

//    // add a new instruction
//    add_trace_instruction(trace_saver, ins);
//  });

//  // save trace to file, close it, and free internal objects of protobuf
//  trace_saver.SerializeToOstream(&output_stream);
//  google::protobuf::ShutdownProtobufLibrary();

//  return;
//}


/*====================================================================================================================*/
/*                                                     exported functions                                             */
/*====================================================================================================================*/

auto cap_save_trace_to_file (const std::string& filename) -> void
{
  std::ofstream trace_file(filename.c_str(), std::ofstream::out | std::ofstream::trunc);

  if (trace_file.is_open()) {
   save_in_simple_format(trace_file);
//   save_in_protobuf_format(trace_file);
    trace_file.close();
  }
  else {
    tfm::printfln("cannot save to file %", filename);
  }

  return;
}

auto cap_save_virtual_trace_to_file (const std::string& filename) -> void
{
  std::ofstream virt_trace_file(filename.c_str(), std::ofstream::trunc);
  if (virt_trace_file.is_open()) {
    save_virtual_trace(virt_trace_file);
    virt_trace_file.close();
  }
  else {
    tfm::printfln("cannot save virtual trace to file %s", filename);
  }
}
