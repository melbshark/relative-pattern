#include <string>
#include <iostream>
#include <cstdint>

#include <elfio/elfio.hpp>
#include "tinyformat.h"

auto dump(const uint8_t* section_data, uint32_t section_addr, uint32_t section_size,
          uint32_t start_dumping_addr, uint32_t stop_dumping_addr, const std::string& output_file) -> void;

int main(int argc, char* argv[])
{
  ELFIO::elfio elf_reader;
  elf_reader.load(argv[1]);

  try {
    tfm::printf("ELF file class: ");
    switch (elf_reader.get_class()) {
      case ELFCLASS32: tfm::printfln("ELF32"); break;
      case ELFCLASS64: tfm::printfln("ELF64"); break;
      default: throw 1; break;
    }

    tfm::printf("ELF file encoding: ");
    switch (elf_reader.get_encoding()) {
      case ELFDATA2LSB: tfm::printfln("Little endian"); break;
      case ELFDATA2MSB: tfm::printfln("Big endian"); break;
      default: throw 2; break;
    }

    auto sec_number = elf_reader.sections.size();
    tfm::printfln("Number of sections: %d", sec_number);
//    for (uint32_t i = 0; i < sec_number; ++i) {
//      const auto psection = elf_reader.sections[i];
//      tfm::printfln("%2d %-25s %-7d 0x8%x",
//                    i, psection->get_name(), psection->get_size(), psection->get_address());
//    }

    auto seg_number = elf_reader.segments.size();
    tfm::printfln("Number of segments: %d", seg_number);
//    for (uint32_t i = 0; i < seg_number; ++i) {
//      const auto psegment = elf_reader.segments[i];
//      tfm::printfln("%2d 0x8%x 0x%-8x 0x%-4x %3d",
//                    i, psegment->get_flags(), psegment->get_virtual_address(),
//                    psegment->get_file_size(), psegment->get_memory_size());
//    }

    auto section_name = std::string(argv[2]);
    auto data_base_address = 0;
    auto data = (const uint8_t*){nullptr};
    auto data_size = uint32_t{0};

    for (uint32_t i = 0; i < sec_number; ++i) {
      auto psection = elf_reader.sections[i];

      if (psection->get_name() == section_name) {
        data_base_address = psection->get_address();
        data = reinterpret_cast<const uint8_t*>(psection->get_data());
        data_size = psection->get_size();
        break;
      }
    }

    //=== dumping data
    if (data == nullptr) throw 3;
    tfm::printfln("Section %s of size %d bytes found at range [0x%x, 0x%x)", section_name,
                  data_size, data_base_address, data_base_address + data_size);

    auto start_dumping_address = std::stoul(argv[3], nullptr, 0x10);
    auto stop_dumping_address = std::stoul(argv[4], nullptr, 0x10);
    auto dumped_data_file = std::string(argv[5]);

    dump(data, data_base_address, data_size, start_dumping_address, stop_dumping_address, dumped_data_file);
  }
  catch (int e) {
    switch (e) {
      case 1: tfm::printfln("Invalid file class"); break;
      case 2: tfm::printfln("Invalid file encoding"); break;
      case 3: tfm::printfln("Cannot read data from section"); break;
      default: break;
    }
  }
  catch (const std::exception& expt) {
    tfm::printfln("%s", expt.what());
  }

  return 0;
}

