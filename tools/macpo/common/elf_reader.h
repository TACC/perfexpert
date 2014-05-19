/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 * 
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 * 
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef TOOLS_MACPO_COMMON_ELF_READER_H_
#define TOOLS_MACPO_COMMON_ELF_READER_H_

#include <bfd.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdint.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

class elf_reader_t {
 public:
    typedef struct {
        std::string filename;
        int line_number;
    } location_t;

 private:
    typedef struct {
        bfd_vma pc;
        asymbol **syms;
        bfd_boolean found;
        unsigned int line;
        const char *filename;
        const char *functionname;
    } object;

    typedef struct {
        int64_t size;
        int64_t address;
        std::string name;
    } symbol_t;

    typedef std::vector<symbol_t> symbol_list_t;

    bfd* abfd;
    asymbol** symbols;
    symbol_list_t symbol_list;

    static void find_address_in_section(bfd *abfd, asection *section,
            void *data) {
        bfd_vma vma;
        bfd_size_type size;

        object* obj = reinterpret_cast<object*>(data);

        if (obj->found) {
            return;
        }

        if ((bfd_get_section_flags(abfd, section) & SEC_ALLOC) == 0) {
            return;
        }

        vma = bfd_get_section_vma(abfd, section);
        if (obj->pc < vma) {
            return;
        }

        size = bfd_get_section_size(section);
        if (obj->pc >= vma + size) {
            return;
        }

        obj->found = bfd_find_nearest_line(abfd, section, obj->syms,
                obj->pc - vma, &(obj->filename), &(obj->functionname),
                &(obj->line));
    }

    bool init(const std::string& filename) {
        return init_reader(filename) == 0 &&
            load_symtab(filename) == 0;
    }

    int init_reader(const std::string& file_path) {
        bfd_init();
        abfd = bfd_openr(file_path.c_str(), "default");
        if (abfd == NULL) {
            return -1;
        }

        if (bfd_check_format(abfd, bfd_archive)) {
            return -2;
        }

        char **matching;
        if (bfd_check_format_matches(abfd, bfd_object, &matching) == FALSE) {
            return -3;
        }

        if (read_symbol_table() < 0) {
            return -4;
        }

        return 0;
    }

    int load_symtab(const std::string& file_name) {
        size_t data_size;
        int return_status = 0;
        char* buff_end = NULL;
        char* buff_start = NULL;

        Elf_Data* data = NULL;
        Elf_Scn* section = NULL;
        Elf64_Sym* symbol = NULL;
        Elf64_Sym* last_symbol = NULL;
        Elf64_Shdr* section_header = NULL;

        int elf_file = open(file_name.c_str(), O_RDONLY);
        elf_version(EV_CURRENT);

        Elf* elf = elf_begin(elf_file, ELF_C_READ, NULL);
        if (elf == NULL) {
            return_status = -1;
            goto close_and_exit;
        }

        section = elf_nextscn(elf, section);
        while (section) {
            section_header = elf64_getshdr(section);
            if (section_header == NULL) {
                goto loop_end;
            }

            if (section_header->sh_type != SHT_SYMTAB &&
                    section_header->sh_type != SHT_SYMTAB) {
                goto loop_end;
            }

            data = elf_getdata(section, data);
            // Check if there is no data in symbol table.
            if (data == NULL || data->d_size == 0) {
                goto loop_end;
            }

            data_size = data->d_size;
            buff_start = reinterpret_cast<char*>(data->d_buf);
            buff_end = buff_start + data_size;

            symbol = reinterpret_cast<Elf64_Sym*>(buff_start);
            last_symbol = reinterpret_cast<Elf64_Sym*>(buff_end);

            while (symbol < last_symbol) {
                size_t offset = static_cast<size_t>(symbol->st_name);
                char* name = elf_strptr(elf, section_header->sh_link, offset);

                symbol_t symbol_value;
                symbol_value.size = symbol->st_size;
                symbol_value.name = std::string(name);
                symbol_value.address = symbol->st_value;
                symbol_list.push_back(symbol_value);

                symbol += 1;
            }

    loop_end:
            section = elf_nextscn(elf, section);
        }

        elf_end(elf);

    close_and_exit:
        close(elf_file);
        return return_status;
    }

    int read_symbol_table() {
        int64_t symcount;
        unsigned int size;

        if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0) {
            fprintf(stderr, "Given file does not appear to contain symbols.\n");
            return -1;
        }

        void** syms_cast = reinterpret_cast<void**>(&(symbols));
        symcount = bfd_read_minisymbols(abfd, FALSE, syms_cast, &size);
        if (symcount == 0) {
            symcount = bfd_read_minisymbols(abfd, TRUE, syms_cast, &size);
        }

        if (symcount < 0) {
            return -1;
        }

        return 0;
    }

    void destroy() {
        if (abfd) {
            bfd_close(abfd);
            abfd = NULL;
        }

        if (symbols) {
            free(symbols);
            symbols = NULL;
        }
    }

 public:
    explicit elf_reader_t(const std::string& filename) {
        abfd = NULL;
        symbols = NULL;

        if (init(filename) != true) {
            // Reset the variables to make sure
            // we don't use a garbled state later.
            abfd = NULL;
            symbols = NULL;
        }
    }

    ~elf_reader_t() {
        destroy();
    }

    static std::string get_function_name(void* function_address) {
        Dl_info dl_info;
        std::string function_name = "unknown";
        if (dladdr(function_address, &dl_info) != 0 &&
                dl_info.dli_sname != NULL) {
            function_name = std::string(dl_info.dli_sname);
        }

        return function_name;
    }

    const location_t translate_address(bfd_vma addr) {
        location_t location = {
            /* .filename = */ "??",
            /* .line_number = */ 0
        };

        object obj = {0};
        obj.found = FALSE;
        obj.pc = addr;

        if (abfd == NULL) {
            return location;
        }

        bfd_map_over_sections(abfd, find_address_in_section, &obj);

        if (obj.filename != NULL) {
            const char *h;

            h = strrchr(obj.filename, '/');
            if (h != NULL)
                obj.filename = h + 1;
        }

        if (obj.found == FALSE || obj.filename == NULL) {
            obj.filename = "??";
            obj.line = 0;
        }

        location.filename = std::string(obj.filename);
        location.line_number = obj.line;

        return location;
    }
};

#endif  // TOOLS_MACPO_COMMON_ELF_READER_H_
