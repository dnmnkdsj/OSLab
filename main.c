//
// Created by elvis on 5/3/18.
//
#include <stdio.h>
#include <elf.h>\

#include <stdlib.h>
#include <memory.h>

char nop[]={0x90};//ass: nop
//char parasize[] = {0xbd, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe5};//ass: mov $0x00000000, %ebp

char parasize[] = {0x48,0xc7,0xc3,0x30,0x04,0x40,0x00,0xff,0xe3};

//char parasize[] = {0x48,0xc7,0xc3,0x30,0x04,0x40,0x00,0xff,0xe3,0xff,0x24,0x25,0x30,0x04,0x40,0x00,
//        0x48,0xc7,0xc0,0x01,0x00,0x00,0x00,0x48,0xc7,0xc3,0x00,0x00,0x00,0x00,0xcd,0x80};
//0000000000000000 <_start>:
//0:	48 c7 c3 30 04 40 00 	mov    $0x400430,%rbx
//7:	ff e3                	jmpq   *%rbx
//9:	ff 24 25 30 04 40 00 	jmpq   *0x400430
//10:	48 c7 c0 01 00 00 00 	mov    $0x1,%rax
//17:	48 c7 c3 00 00 00 00 	mov    $0x0,%rbx
//1e:	cd 80                	int    $0x80

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
    char elf_phdrs[p_ehdr->e_phnum][sizeof(Elf64_Phdr)];//all program headers
    Elf64_Phdr *p_phdr;
    Elf64_Addr program_head_vaddr;//the exact segment's address
    Elf64_Xword program_head_size;
    int segment_id = 0;//identify the exact executable segment
    for (int i = 0; i < (int) p_ehdr->e_phnum; i++) {//e_phnum is the number of program headers
        p_phdr = (Elf64_Phdr *) elf_phdrs[i];
        if(fread(elf_phdrs[i], sizeof(Elf64_Phdr), 1, source)!=1){
            printf("Read source file program header fail!\n");
        }
        if (p_phdr->p_paddr < origi_entry &&
                (p_phdr->p_paddr + p_phdr->p_filesz) > origi_entry) {//find the exact program segment
            program_head_vaddr = p_phdr->p_vaddr;
            program_head_size = p_phdr->p_filesz;
            segment_id = i;

            p_phdr->p_filesz += 4096;
            p_phdr->p_memsz += 4096;
        }
        if ((segment_id != 0) && i > segment_id) {
            p_phdr->p_offset += 4096;
        }
        if (i ==5) {
            p_phdr->p_offset -= 4096;
        }
        if (i == 6) {
            p_phdr->p_offset -= 4096;
        }
    }


    //step3: find the LAST one section of this segment as the name of A,according to segment's offset and size
    if(-1== fseek(source,//move file IO position to section position
                 (int) p_ehdr->e_shoff - sizeof(elf_ehdr) - (int) p_ehdr->e_phnum * sizeof(Elf64_Phdr),
                 SEEK_CUR)){
        printf("Move file position to section position fail!\n");
        return 45;
    }


    char elf_shdrs[p_ehdr->e_shnum][sizeof(Elf64_Shdr)];//store section headers
    Elf64_Shdr *p_shdr;
    Elf64_Off entry_section_offset;
    Elf64_Xword entry_section_size;
    Elf64_Xword new_entry = 8011;
    int section_id;
    for (int i = 0; i < (int) p_ehdr->e_shnum; i++) {
        p_shdr = (Elf64_Shdr *) elf_shdrs[i];
        fread(elf_shdrs[i], sizeof(Elf64_Shdr), 1, source);
        if (p_shdr->sh_addr + p_shdr->sh_size == (program_head_vaddr + program_head_size)) {//find the exact section
            entry_section_offset = p_shdr->sh_offset;
            entry_section_size = p_shdr->sh_size;
            new_entry = p_shdr->sh_addr+p_shdr->sh_size;//set new entry address as section A 's end address
            p_shdr->sh_size += 4096;//change section A's size
            section_id = i;
        }
        if (p_shdr->sh_addr + p_shdr->sh_size > (program_head_vaddr + program_head_size)) {
            if (i != section_id) {
                p_shdr->sh_offset += 4096;
            }
        }
    }

    //step4: include code into section A
//    struct _jump * jump = (struct _jump *)parasize;
//    jump->addr = origi_entry;

    //step5: enlarge the size of the segment by the length of insert code
    //step6: change section A 's section header
    //step7: change headers of all sections which are below section A
    //step8: change ELF header's entry address aiming to the insert code
    p_ehdr->e_entry = new_entry;

    //write file
    fwrite(elf_ehdr, sizeof(elf_ehdr), 1, target);

    for (int i = 0; i < p_ehdr->e_phnum; i++) {
        fwrite(elf_phdrs[i], sizeof(Elf64_Phdr), 1, target);
    }

    char data_tmp[p_ehdr->e_shoff - (p_ehdr->e_phnum * sizeof(Elf64_Phdr) + p_ehdr->e_phoff)];
    fseek(source,p_ehdr->e_phnum * sizeof(Elf64_Phdr) + p_ehdr->e_phoff,SEEK_SET);
    fread(data_tmp, sizeof(data_tmp), 1, source);
    fwrite(data_tmp, sizeof(data_tmp), 1, target);

    for (int i = 0; i < p_ehdr->e_shnum; i++) {
        fwrite(elf_shdrs[i], sizeof(Elf64_Shdr), 1, target);
    }

    //get Elf file size
    fseek(source, 0, SEEK_END);
    long file_size = ftell(source);
    fseek(source, p_ehdr->e_shoff + p_ehdr->e_shnum * sizeof(Elf64_Shdr), SEEK_SET);

    char data_tmp1[new_entry - (p_ehdr->e_shoff + p_ehdr->e_shnum * sizeof(Elf64_Shdr))];
    fread(data_tmp1, sizeof(data_tmp1), 1, source);
    fwrite(data_tmp1, sizeof(data_tmp1), 1, target);

    //insert code
    fwrite(parasize, sizeof(parasize), 1, target);
    for (int i = 0; i < 4096 - sizeof(parasize); i++) {
        fwrite(nop, 1, 1, target);
    }

    char data_tmp2[file_size - (new_entry + 4096)];
    fread(data_tmp2, sizeof(data_tmp2), 1, source);
    fwrite(data_tmp2, sizeof(data_tmp2), 1, target);

    printf("write file succeed!\n");

    fclose(target);
    fclose(source);
    return 0;
}
