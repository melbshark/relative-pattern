#ifndef OPERAND_H
#define OPERAND_H

#include <pin.H>

#include <boost/variant.hpp>
#include <memory>
#include <vector>

class operand
{
 public:
  boost::variant<ADDRINT, REG> value;

 public:
  operand(ADDRINT addr);
  operand(REG reg);
};

typedef std::shared_ptr<operand> p_operand_t;
typedef std::vector<p_operand_t> p_operands_t;

#endif
