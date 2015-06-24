#include <iostream>
#include <cstdint>

#include <elfio/elfio.hpp>
#include "tinyformat.h"

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

    //=== dumping jump table
    auto start_address = std::stoul(argv[2], nullptr, 0x10);
    auto stop_address = std::stoul(argv[3], nullptr, 0x10);
    if (stop_address < start_address) throw 3;

    auto rodata_base_address = 0;
    auto rodata  = (const char*){nullptr};

    for (uint32_t i = 0; i < sec_number; ++i) {
      auto psection = elf_reader.sections[i];
      if (psection->get_name() == ".rodata") {
        rodata_base_address = psection->get_address();
        rodata = psection->get_data();
        break;
      }
    }

    if (rodata == nullptr) throw 5;
    else {
      if (start_address < rodata_base_address) throw 4;
      else {
        auto start_entry = (const uint32_t*)(rodata + (start_address - rodata_base_address));
        auto stop_entry = (const uint32_t*)(rodata + (stop_address - rodata_base_address));

        tfm::printfln("Dumping %d entries in .rodata section (0x%x) of [0x%x, 0x%x)",
                      (stop_address - start_address) / sizeof(uint32_t),
                      rodata_base_address, start_address, stop_address);
        auto count = uint32_t{0};
        for (auto jmp_entry = start_entry; jmp_entry < stop_entry; ++jmp_entry) {
          auto jmp_entry_value = *jmp_entry;
          tfm::printf("0x%x ", jmp_entry_value);
          ++count; if (count % 4 == 0) tfm::printfln("");
        }
      }
    }
  }
  catch (int e) {
    switch (e) {
      case 1: tfm::printfln("Invalid file class"); break;
      case 2: tfm::printfln("Invalid file encoding"); break;
      case 3: tfm::printfln("Start address must be smaller than stop address"); break;
      case 4: tfm::printfln("Cannot read .rodata section"); break;
      case 5: tfm::printfln("Start address must be greater than base address of rodata section"); break;
      default: break;
    }
  }
  catch (const std::exception& expt) {
    tfm::printfln("%s", expt.what());
  }

  return 0;
}

