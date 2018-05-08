//
// Created by elvis on 5/3/18.
//
#include <stdio.h>
#include <elf.h>\

#include <stdlib.h>
#include <memory.h>

char nop[]={0x90};//ass: nop
char parasize[] = {0xbd, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe5};//ass: mov $0x00000000, %ebp

struct _jump{
    char opcode_mov;
    Elf64_Addr addr;
    short opcode_imp;
}__attribute__((packed));


int main(){
    FILE *source = fopen("../hello_world", "r");//open source elf file
    if (!source) {
        printf("Open source file error!\n");
    }
    FILE *target = fopen("../target", "w+");
    if(!target) {
        printf("Open target file error!\n");
    }

    //step1: find source file's entry address
    char elf_ehdr[sizeof(Elf64_Ehdr)];//store string of elf header
    Elf64_Ehdr *p_ehdr = (Elf64_Ehdr *) elf_ehdr;//p_ehdr pointer aiming at elf's header
    if (fread(elf_ehdr, sizeof(elf_ehdr), 1, source)!=1) {//read source elf file
        printf("Read source file ELF header fail!\n");
    }
    Elf64_Addr origi_entry = p_ehdr->e_entry;


    //step2: find executable segment according to entry address
    char elf_phdr[sizeof(Elf64_Phdr)];
    char elf_phdrs[p_ehdr->e_phnum][sizeof(Elf64_Phdr)];//all program headers
    Elf64_Phdr *p_phdr = (Elf64_Phdr *) elf_phdr;
    Elf64_Addr program_head_vaddr = 8011;//the exact segment's address
    Elf64_Xword program_head_size = 8011;
    for (int i = 0; i < (int) p_ehdr->e_phnum; i++) {//e_phnum is the number of program headers
        if(fread(elf_phdr, sizeof(elf_phdr), 1, source)!=1){
            printf("Read source file program header fail!\n");
        }
        memcpy(elf_phdrs[i], elf_phdr, sizeof(Elf64_Phdr));
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
    char elf_shdrs[p_ehdr->e_shnum][sizeof(Elf64_Shdr)];
    Elf64_Shdr *p_shdr = (Elf64_Shdr *) elf_shdr;
    Elf64_Off entry_section_offset;
    Elf64_Xword entry_section_size;
    Elf64_Xword new_entry = 8011;
    for (int i = 0; i < (int) p_ehdr->e_shnum; i++) {
        fread(elf_shdr, sizeof(elf_shdr), 1, source);
        memcpy(elf_shdrs[i], elf_shdr, sizeof(Elf64_Shdr));
        if (p_shdr->sh_addr + p_shdr->sh_size == (program_head_vaddr + program_head_size)) {//find the exact section
            entry_section_offset = p_shdr->sh_offset;
            entry_section_size = p_shdr->sh_size;
            new_entry = p_shdr->sh_addr+p_shdr->sh_size;//set new entry address as section A 's end address
            p_shdr->sh_size += 4096;//change section A's size
        }
    }

    //step4: include code into section A
    struct _jump * jump = (struct _jump *)parasize;
    jump->addr = origi_entry;

    //todo fwrite parasize
    //write(newfile,parasize,sizeof(parasize);
    //todo fwrite nop into 4096
    //for(i-0;i<page_size - sizeof(parasize);i++)
    //write(newfile,nop,1);

    //step5: enlarge the size of the segment by the length of insert code
    p_phdr->p_filesz += 4096;
    p_phdr->p_memsz += 4096;
    //step6: change section A 's section header
    //step7: change headers of all sections which are below section A
    for (int i = 0; i < (int) p_ehdr->e_shnum; i++) {
        p_shdr = (Elf64_Shdr *) elf_shdrs[i];
        if (p_shdr->sh_addr + p_shdr->sh_size > (program_head_vaddr + program_head_size)) {
            p_shdr->sh_offset += 4096;
        }
    }
    //step8: change ELF header's entry address aiming to the insert code
    p_ehdr->e_entry = new_entry;

    //write file
    fwrite(elf_ehdr, sizeof(elf_ehdr), 1, target);
    for (int i = 0; i < p_ehdr->e_phnum; i++) {
        fwrite(elf_phdrs[i], sizeof(elf_phdr), 1, target);
    }
    char data_tmp[p_ehdr->e_shoff - (p_ehdr->e_phnum * sizeof(elf_phdr) + p_ehdr->e_phoff)];
    fseek(source,p_ehdr->e_phnum * sizeof(elf_phdr) + p_ehdr->e_phoff,SEEK_SET);
    fread(data_tmp, sizeof(data_tmp), 1, source);
    fwrite(data_tmp, sizeof(data_tmp), 1, target);
    for (int i = 0; i < p_ehdr->e_shnum; i++) {
        fwrite(elf_shdrs[i], sizeof(elf_shdr), 1, target);
    }

    fseek(source, 0, SEEK_END);
    long file_size = ftell(source);
    fseek(source, p_ehdr->e_shoff + p_ehdr->e_shnum * sizeof(Elf64_Shdr), SEEK_SET);
    char data_tmp1[file_size - (p_ehdr->e_shoff + p_ehdr->e_shnum * sizeof(Elf64_Shdr))];
    fread(data_tmp1, sizeof(data_tmp1), 1, source);
    fwrite(data_tmp1, sizeof(data_tmp1), 1, target);

    printf("write file succeed!\n");

    fclose(target);
    fclose(source);
    return 0;
}
