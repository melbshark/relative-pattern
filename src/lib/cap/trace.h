#ifndef TRACE_H
#define TRACE_H

#include "../type/instruction.h"

#include <cstdint>
#include <cstdlib>
#include <algorithm>
#include <tuple>
#include <list>

#include <boost/variant.hpp>

typedef std::pair<REG, PIN_REGISTER> dyn_reg_t;
typedef std::pair<ADDRINT, UINT8>  dyn_mem_t;

typedef std::map<REG, PIN_REGISTER> dyn_regs_t;
typedef std::map<dyn_mem_t, ADDRINT> dyn_mems_t;

enum
{
  CAP_SYS_READ   = 3,
  CAP_SYS_WRITE  = 4,
  CAP_SYS_OPEN   = 5,
  CAP_SYS_SOCKET = 102,
  CAP_SYS_OTHER = 1000
};

template<uint32_t sys_id>
struct syscall_t
{
  enum { id = sys_id };
};

struct sys_read_info_t : syscall_t<CAP_SYS_READ>
{
  int                      file_desc;
  ADDRINT                  buffer_addr;
  size_t                   buffer_length;
  size_t                   read_length;
  std::shared_ptr<uint8_t> buffer;
};

struct sys_write_info_t : syscall_t<CAP_SYS_WRITE>
{
  int                      file_desc;
  ADDRINT                  buffer_addr;
  size_t                   buffer_length;
  size_t                   write_length;
  std::shared_ptr<uint8_t> buffer;
};

struct sys_open_info_t : syscall_t<CAP_SYS_OPEN>
{
  std::string path_name;
  int         flags;
  int         mode;
  int         file_desc;
};

struct sys_other_info_t : syscall_t<CAP_SYS_OTHER>
{
  uint32_t real_id;
  sys_other_info_t(uint32_t syscall_id) : real_id(syscall_id) {};
};

struct call_info_t
{
  ADDRINT     called_fun_addr;
  std::string called_fun_name;
  bool        is_traced;
};

typedef boost::variant
  <
    sys_open_info_t,  // which = 0
    sys_read_info_t,  // 1
    sys_write_info_t, // 2
    sys_other_info_t, // 3
    call_info_t       // 4
  > concrete_info_t;

typedef std::tuple<
  ADDRINT,        // address of instructions
  THREADID,       // id of containing thread
  dyn_regs_t,     // read registers
  dyn_regs_t,     // write registers
  dyn_mems_t,     // read memory addresses
  dyn_mems_t,     // write memory addresses
  concrete_info_t // concrete information
  > dyn_ins_t;

// list is prefered since new instructions will be added regularly
typedef std::list<dyn_ins_t> dyn_inss_t;

extern dyn_inss_t                trace;
extern map_address_instruction_t cached_ins_at_addr;

enum
  {
    INS_ADDRESS       = 0,
    INS_THREAD_ID     = 1,
    INS_READ_REGS     = 2,
    INS_WRITE_REGS    = 3,
    INS_READ_MEMS     = 4,
    INS_WRITE_MEMS    = 5,
    INS_CONCRETE_INFO = 6
  };

#endif // TRACE_H
