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
    case ELFCLASS32:
      tfm::printfln("ELF32"); break;
    case ELFCLASS64:
      tfm::printfln("ELF64"); break;
    default:
      throw 1; break;
    }

    tfm::printf("ELF file encoding: ");
    switch (elf_reader.get_encoding()) {
    case ELFDATA2LSB: tfm::printfln("Little endian"); break;
    case ELFDATA2MSB: tfm::printfln("Big endian"); break;
    }

    auto sec_number = elf_reader.sections.size();
    tfm::printfln("Number of sections: %d", sec_number);
    for (uint32_t i = 0; i < sec_number; ++i) {
      const auto psection = elf_reader.sections[i];
      tfm::printfln("%2d %-25s %-7d 0x8%x", i, psection->get_name(), psection->get_size(), psection->get_address());
    }

    auto seg_number = elf_reader.segments.size();
    tfm::printfln("Number of segments: %d", seg_number);
    for (uint32_t i = 0; i < seg_number; ++i) {
      const auto psegment = elf_reader.segments[i];
      tfm::printfln("%2d 0x8%x 0x%-8x 0x%-4x %3d", i, psegment->get_flags(), psegment->get_virtual_address(),
                    psegment->get_file_size(), psegment->get_memory_size());
    }
  }
  catch (int e) {
    switch (e) {
    case 1: tfm::printfln("Not an ELF file"); break;
    default: break;
    }
  }



  return 0;
}

