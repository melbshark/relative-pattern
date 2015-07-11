#include <cstdint>
#include <string>
#include <fstream>

#include <elfio/elfio.hpp>
#include "tinyformat.h"

auto dump(const uint8_t* section_data, uint32_t section_addr, uint32_t section_size,
          uint32_t start_dumping_addr, uint32_t stop_dumping_addr, const std::string& output_file) -> void
{
  try {
    if ((start_dumping_addr < section_addr) ||
        (stop_dumping_addr > section_addr + section_size)) throw 1;

    auto start_dumped_entry = (const uint8_t*)(section_data + (start_dumping_addr - section_addr));
    auto stop_dumped_entry = (const uint8_t*)(section_data + (stop_dumping_addr - section_addr));

    std::ofstream dumped_file(output_file.c_str(),
                              std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

//    auto count = uint32_t{0};
    for (auto dumped_entry = start_dumped_entry; dumped_entry < stop_dumped_entry; ++dumped_entry) {
      auto dumped_value = *dumped_entry;
      tfm::format(dumped_file, "0x%x; ", dumped_value);
//      ++count; if (count % 32 == 0) tfm::format(dumped_file, "\n");
    }

    dumped_file.close();
  } catch (int e) {
    tfm::printfln("[start, stop) range must be located in section");
  }
  catch (const std::exception& expt) {
    tfm::printfln("%s", expt.what());
  }

  return;
}
