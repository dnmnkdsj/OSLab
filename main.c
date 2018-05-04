//
// Created by elvis on 5/3/18.
//
#include <stdio.h>
#include <elf.h>
#include <zconf.h>
#include <unistd.h>
#include <fcntl.h>

//todo usr fseek replace the lseek
char nop[]={0x90};//ass: nop
char parasize[] = {0xbd, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe5};//ass: mov $0x00000000, %ebp

struct _jump{
    char opcode_mov;
    int addr;
    short opcode_imp;
}__attribute__((packed));

int main(){

    //step1: find source file's entry address
    FILE *source = fopen("../hello_world", "r");//open source elf file
//    int source = open("../hello_world", O_RDONLY);
    if (!source) {
        printf("Open source file error!\n");
        return 45;
    }
    char elf_ehdr[sizeof(Elf64_Ehdr)];//store string of elf header
    Elf64_Ehdr *p_ehdr = (Elf64_Ehdr *) elf_ehdr;//p_ehdr pointer aiming at elf's header
    if (fread(elf_ehdr, sizeof(elf_ehdr), 1, source)!=1) {//read source elf file
        printf("Read source file ELF header fail!\n");
        return 45;
    }
    Elf64_Addr origi_entry = p_ehdr->e_entry;


    //step2: find executable segment according to entry address
    char elf_phdr[sizeof(Elf64_Phdr)];
    Elf64_Phdr *p_phdr = (Elf64_Phdr *) elf_phdr;
    Elf64_Addr program_head_vaddr;//store data about the exact segment
    Elf64_Xword program_head_size;
    for (int i = 0; i < (int) p_ehdr->e_phnum; i++) {//e_phnum is the number of program headers
        if(fread(elf_phdr, sizeof(elf_phdr), 1, source)!=1){//read program headers
            printf("Read source file program header fail!\n");
            return 45;
        }
        if (p_phdr->p_paddr < origi_entry &&
                (p_phdr->p_paddr + p_phdr->p_filesz) > origi_entry) {//find the exact program segment
            program_head_vaddr = p_phdr->p_vaddr;
            program_head_size = p_phdr->p_filesz;
        }
    }


    //step3: find the LAST one section of this segment as the name of A,according to segment's offset and size
    if(-1== fseek(source,//move file IO position to section position
                 (int) p_ehdr->e_shoff - sizeof(elf_ehdr) - (int) p_ehdr->e_phnum * sizeof(elf_phdr),
                 SEEK_CUR)){
        printf("Move file position to section position fail!\n");
        return 45;
    }
    char elf_shdr[sizeof(Elf64_Shdr)];//store section headers
    Elf64_Shdr *p_shdr = (Elf64_Shdr *) elf_shdr;
    Elf64_Off entry_section_offset;
    Elf64_Xword entry_section_size;
    Elf64_Xword new_entry;
    for (int i = 0; i < (int) p_ehdr->e_shnum; i++) {
        fread(elf_shdr, sizeof(elf_shdr), 1, source);
        if (p_shdr->sh_addr + p_shdr->sh_size == (program_head_vaddr + program_head_size)) {//find the exact section
            entry_section_offset = p_shdr->sh_offset;
            entry_section_size = p_shdr->sh_size;
            new_entry = p_shdr->sh_addr+p_shdr->sh_size;//set new entry address as section A 's end address
        }
    }

    //step4: include code into section A
    struct _jump * jump = (struct _jump *)parasize;
    jump->addr = origi_entry;
    //todo fwrite parasize 
    //todo fwrite nop into 4096
    //step5: enlarge the size of the section by the length of insert code
    //step6: change section A 's section header
    //step7: change headers of all sections which are below section A
    //step8: change ELF header's entry address aiming to the insert code

    fclose(source);
    return 0;
}
