#include "cap.h"
#include "trace.h"

#include "../tinyformat.h"

#include <fstream>
#include <cassert>



/*====================================================================================================================*/
/*                                                     exported functions                                             */
/*====================================================================================================================*/

//auto cap_save_trace_to_file (const std::string& filename) noexcept -> void
//{
//  std::ofstream trace_file(filename.c_str(),
//                           std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

//  if (trace_file.is_open()) {
//    save_in_simple_format(trace_file);
////    save_in_protobuf_format(trace_file);

//    trace_file.close();
//  }
//  else {
//    tfm::printfln("cannot save to file %", filename);
//  }

//  return;
//}
