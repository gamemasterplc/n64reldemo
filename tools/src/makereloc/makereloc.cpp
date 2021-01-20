#include <stdio.h>
#include <string>
#include "elfio/elfio.hpp"

int GetRelocSection(std::string name)
{
	if (name == ".rel.text" || name == ".text") {
		return 1;
	} else if (name == ".rel.data" || name == ".data") {
		return 2;
	} else if (name == ".rel.rodata" || name == ".rodata" || name == ".rodata.str1.4" || name == ".rodata.cst4") {
		return 3;
	}
	return -1;
}

std::string GetFileName(std::string path)
{
	size_t slash_pos = path.find_last_of("\\/", path.length());
	if (slash_pos != std::string::npos) {
		return path.substr(slash_pos + 1, path.length() - slash_pos);
	}
	return path;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("Usage: makereloc object out_rel\n");
		return 1;
	}
	ELFIO::elfio *reader = new ELFIO::elfio();
	if (!reader->load(argv[1])) {
		delete reader;
		return 1;
	}
	std::vector<uint32_t> relocs;
	ELFIO::Elf_Half num_sections = reader->sections.size();
	for (ELFIO::Elf_Half i = 0; i < num_sections; i++) {
		std::string rel_section_names[3] = { ".rel.text", ".rel.data" , ".rel.rodata" };
		ELFIO::section *section = reader->sections[i];
		if (section->get_type() == SHT_REL && std::find(&rel_section_names[0], &rel_section_names[3], section->get_name()) != &rel_section_names[3]) {
			int reloc_section = GetRelocSection(section->get_name());
			ELFIO::relocation_section_accessor reloc_accessor(*reader, section);
			for (ELFIO::Elf_Xword j = 0; j < reloc_accessor.get_entries_num(); j++) {
				ELFIO::Elf64_Addr offset;
				ELFIO::Elf_Word symbol;
				ELFIO::Elf_Word type;
				{
					ELFIO::Elf_Sxword addend;
					reloc_accessor.get_entry(j, offset, symbol, type, addend);
				}
				std::string sym_name;
				ELFIO::Elf_Half sym_shndx = SHN_UNDEF;
				{
					ELFIO::symbol_section_accessor symbol_accessor(*reader, reader->sections[(ELFIO::Elf_Half)section->get_link()]);
					ELFIO::Elf64_Addr value;
					ELFIO::Elf_Xword size;
					unsigned char bind;
					unsigned char type;
					unsigned char other;
					symbol_accessor.get_symbol(symbol, sym_name, value, size, bind, type, sym_shndx, other);
				}
				if (sym_shndx != SHN_UNDEF) {
					uint32_t rel_value = (reloc_section << 30)|((type & 0x3F) << 24)|(offset & 0xFFFFFF);
					relocs.push_back(rel_value);
				}
			}
		}
	}
	uint32_t section_sizes[5] = { 0, 0, 0, 0, 0 };
	for (ELFIO::Elf_Half i = 0; i < num_sections; i++) {
		std::string section_names[4] = { ".text", ".data" , ".rodata", ".bss" };
		ELFIO::section *section = reader->sections[i];
		std::string *string_ptr = std::find(&section_names[0], &section_names[4], section->get_name());
		if (string_ptr != &section_names[4]) {
			section_sizes[string_ptr-section_names] += section->get_size();
		}
	}
	delete reader;
	ELFIO::elfio writer;
	writer.create(ELFCLASS32, ELFDATA2MSB);
	writer.set_os_abi(ELFOSABI_NONE);
	writer.set_type(ET_REL);
	writer.set_machine(EM_MIPS);
	section_sizes[4] = relocs.size();
	uint8_t *elf_data = new uint8_t[20 + (relocs.size() * 4)];
	uint32_t elf_data_ofs = 0;
	for (uint32_t i = 0; i < 5; i++) {
		elf_data[elf_data_ofs++] = section_sizes[i] >> 24;
		elf_data[elf_data_ofs++] = (section_sizes[i] >> 16) & 0xFF;
		elf_data[elf_data_ofs++] = (section_sizes[i] >> 8) & 0xFF;
		elf_data[elf_data_ofs++] = section_sizes[i] & 0xFF;
	}
	for (uint32_t i = 0; i < relocs.size(); i++) {
		elf_data[elf_data_ofs++] = relocs[i] >> 24;
		elf_data[elf_data_ofs++] = (relocs[i] >> 16) & 0xFF;
		elf_data[elf_data_ofs++] = (relocs[i] >> 8) & 0xFF;
		elf_data[elf_data_ofs++] = relocs[i] & 0xFF;
	}
	ELFIO::section *ovl_sec = writer.sections.add(".rodata");
	ovl_sec->set_type(SHT_PROGBITS);
	ovl_sec->set_flags(SHF_ALLOC);
	ovl_sec->set_addr_align(0x10);
	ovl_sec->set_data((const char *)elf_data, 20 + (relocs.size() * 4));
	ELFIO::section *str_sec = writer.sections.add(".strtab");
	str_sec->set_type(SHT_STRTAB);
	str_sec->set_addr_align(0x1);
	ELFIO::string_section_accessor stra(str_sec);
	ELFIO::section *sym_sec = writer.sections.add(".symtab");
	sym_sec->set_type(SHT_SYMTAB);
	sym_sec->set_info(1);
	sym_sec->set_addr_align(0x4);
	sym_sec->set_entry_size(writer.get_default_entry_size(SHT_SYMTAB));
	sym_sec->set_link(str_sec->get_index());
	ELFIO::symbol_section_accessor syma(writer, sym_sec);
	std::string ovl_file = GetFileName(argv[1]);
	std::string start_sym;
	start_sym = "_" + ovl_file.substr(0, ovl_file.find_last_of(".")) + "SegmentReloc";
	syma.add_symbol(stra, start_sym.c_str(), 0x00000000, 0, STB_GLOBAL, STT_OBJECT, 0, ovl_sec->get_index());
	writer.save(argv[2]);
	return 0;
}