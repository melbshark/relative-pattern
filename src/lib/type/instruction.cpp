#include "instruction.h"

instruction::instruction(const INS& ins)
{
  this->address     = INS_Address(ins);
  this->next_address = INS_NextAddress(ins);
//  this->opcode      = INS_Mnemonic(ins);
  this->opcode_size = INS_Size(ins);
  this->disassemble = INS_Disassemble(ins);

  // including image, routine
  auto img                = IMG_FindByAddress(this->address);
  this->including_image   = IMG_Valid(img) ? IMG_Name(img) : "";
//  this->including_routine = RTN_FindNameByAddress(this->address);

  PIN_LockClient();
  auto routine = RTN_FindByAddress(this->address);
  PIN_UnlockClient();

  if (RTN_Valid(routine)) {
    auto routine_mangled_name = RTN_Name(routine);
    this->including_routine_name = PIN_UndecorateSymbolName(routine_mangled_name, UNDECORATION_NAME_ONLY);
  }
  else this->including_routine_name = "";

  // has fall through
  this->has_fall_through = INS_HasFallThrough(ins);

  // is call, ret or syscall
  this->is_call    = INS_IsCall(ins);
  this->is_branch  = INS_IsBranch(ins);
  this->is_ret     = INS_IsRet(ins);
  this->is_syscall = INS_IsSyscall(ins);

  this->category = static_cast<xed_category_enum_t>(INS_Category(ins));
  this->iclass = static_cast<xed_iclass_enum_t>(INS_Opcode(ins));

  // read registers
  auto read_reg_number = INS_MaxNumRRegs(ins);
  for (decltype(read_reg_number) reg_id = 0; reg_id < read_reg_number; ++reg_id) {
    this->src_registers.push_back(INS_RegR(ins, reg_id));
  }

  // written registers
  auto written_reg_number = INS_MaxNumWRegs(ins);
  for (decltype(written_reg_number) reg_id = 0; reg_id < written_reg_number; ++reg_id) {
    this->dst_registers.push_back(INS_RegW(ins, reg_id));
  }

  auto is_special_reg = [](const REG& reg) -> bool {
    return (reg >= REG_MM_BASE);
  };

  this->is_special = std::any_of(std::begin(this->src_registers), std::end(this->src_registers), is_special_reg) ||
      std::any_of(std::begin(this->dst_registers), std::end(this->dst_registers), is_special_reg) ||
      (this->category == XED_CATEGORY_X87_ALU) || (this->category == XED_CATEGORY_LOGICAL_FP) ||
      (this->iclass == XED_ICLASS_XEND) || (this->iclass == XED_ICLASS_PUSHA) ||
      (this->iclass == XED_ICLASS_PUSHAD) || (this->iclass == XED_ICLASS_PUSHF) ||
      (this->iclass == XED_ICLASS_PUSHFD) || (this->iclass == XED_ICLASS_PUSHFQ) ||
      (this->iclass == XED_ICLASS_RDTSC) || (this->iclass == XED_ICLASS_SKINIT) || (this->iclass == XED_ICLASS_RDPMC);

  // is memory read, write
  this->is_memory_read    = INS_IsMemoryRead(ins);
  this->is_memory_write   = INS_IsMemoryWrite(ins);
  this->has_memory_read_2 = INS_HasMemoryRead2(ins);

//  if (this->address == 0x639219) {
//    tfm::printfln("initialize %s:%s:%d:%d\n", StringFromAddrint(this->address), this->disassemble, this->iclass, XED_ICLASS_PUSHAD);
//  }

//  tfm::printfln("instruction initialized %s : %s", StringFromAddrint(this->address), this->disassemble);
}
