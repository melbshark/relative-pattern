#include "operand.h"

operand::operand(ADDRINT addr)
{
  this->value = addr;
}

operand::operand(REG reg)
{
  this->value = reg;
}
