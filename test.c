#include<stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include"/usr/include/elf.h"

int main() {
	Elf64_Addr p_header_vaddr;
	Elf64_Addr p_header_size;


	Elf64_Ehdr *e_header = (Elf64_Ehdr*)malloc(sizeof(Elf64_Ehdr));
	FILE *fp = fopen("../hello_world","r+");
	fread(e_header, sizeof(Elf64_Ehdr), 1, fp);
	Elf64_Addr origin = e_header->e_entry; //确定入口地址

	/*读取ELF头部后，读取程序头部*/

    int p_num;
	Elf64_Phdr *p_header = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr));
	for (int i = 0; i < (int) e_header->e_phnum; i++)
	{
		fread(p_header, sizeof(Elf64_Phdr), 1, fp);
		/*如果入口地址在段的内存地址中*/
		if ((p_header->p_paddr < origin) &&
			((p_header->p_paddr + p_header->p_filesz) > origin))
		{
            p_num = i;
			p_header_vaddr = p_header->p_vaddr;
			p_header_size = p_header->p_filesz;
           break;

		}
	}

    fseek(fp,(int)(e_header->e_shoff), SEEK_SET);

    Elf64_Shdr *s_header = (Elf64_Shdr*)malloc(sizeof(Elf64_Shdr));
    Elf64_Off entry_section_offset;
    Elf64_Xword entry_section_size;
    Elf64_Addr new_entry;

    int section_num;
    for (int i = 0; i < (int)e_header->e_shnum; i++) {
		fread(s_header, sizeof(Elf64_Shdr), 1, fp);
		if (s_header->sh_addr+s_header->sh_size == (p_header_vaddr + p_header_size)) {
		    section_num = i; //记录当前的节区号
			entry_section_offset = s_header->sh_offset; // 该字段指出该节所在的位置;该字段的值是该节的第一个字节在文件中的位置,即:相对于文件开始位置处的偏移量;
			entry_section_size = s_header->sh_size;
			new_entry = s_header->sh_addr + s_header->sh_size; //新的入口地址
            break;
		}
	}

    p_header->p_filesz = p_header->p_filesz + 4096;
	p_header->p_memsz = p_header->p_memsz+4096;
    s_header->sh_size = entry_section_size + 4096;
	e_header->e_entry = new_entry;
	e_header->e_shoff = e_header->e_shoff+4096;


	char nop[] = {0x90};
	char helloworld[] = {
	                     //0x48,0x89,0xe5,
	                      //0x48,0xc7,0xc0,0x05,0x00,0x00,0x00,
                          //0x48,0xc7,0xc3,0x01,0x00,0x00,0x00,
                          //0x48,0xc7,0xc1,0x00,0x00,0x00,0x00,
                          //0x48,0xc7,0xc2,0xb6,0x01,0x00,0x00,
                          //0xcd,0x80,
                         // 0x48,0xc7,0xc0,0x04,0x00,0x00,0x00,
                         // 0x48,0xc7,0xc1,0x45,0x07,0x40,0x00,
                         // 0x48,0xc7,0xc2,0x0d,0x00,0x00,0x00,
                         // 0xcd,0x80,
                         // 0x48,0xc7,0xc0,0x06,0x00,0x00,0x00,
                         // 0xcd,0x80,
                          0x48,0xc7,0xc3,0x30,0x04,0x40,0x00,
                          0xff,0xe3,
                          //0x48,0x65,0x6c,0x6c,0x6f,0x20,0x57,
                          //0x6f,0x72,0x6c,0x64,0x21,0x0a
			};

    fclose(fp);

	int c,num,num2;
	FILE *source,*write,*keep;
	source = fopen("../hello_world","r+");
	write = fopen("../rewrite","r+");//打开文件。
    keep = fopen("../keep","r+");

    fseek(source,entry_section_offset+entry_section_size,SEEK_SET);//从偏移位置开始读取
    while((num = fgetc(source))!=EOF)
    {
        fputc(num,keep);
    }

    fseek(source,0,SEEK_SET);
    while((c = fgetc(source))!=EOF)//循环读文件到文件尾。
    {
        fputc(c, write);//写目标文件
    }

    fseek(write,0,SEEK_SET);//重写e_header
    fwrite(e_header, sizeof(Elf64_Ehdr),1,write);

    fseek(write,e_header->e_phoff+p_num* sizeof(Elf64_Phdr),SEEK_SET);//重写p_header
    fwrite(p_header, sizeof(Elf64_Phdr),1,write);

    Elf64_Phdr *after_p_header = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr));//重写p_header之后的节区偏移量
    fseek(source,e_header->e_phoff+ sizeof(Elf64_Phdr)*(p_num+1),SEEK_SET);
    for(int n = 0; n <(int)e_header->e_phnum-p_num-1;n++)
    {
        if(n!=4&&n!=2&&n!=3)
        {
            fread(after_p_header, sizeof(Elf64_Phdr),1,source);
            after_p_header->p_offset = after_p_header->p_offset+4096;
            fwrite(after_p_header, sizeof(Elf64_Phdr),1,write);
        } else{
            fread(after_p_header, sizeof(Elf64_Phdr),1,source);
            after_p_header->p_offset = after_p_header->p_offset;
            fwrite(after_p_header, sizeof(Elf64_Phdr),1,write);
        }

    }

    fseek(write,entry_section_offset+entry_section_size,SEEK_SET);//重写section
    fwrite(helloworld, sizeof(helloworld),1,write);
    for(int i=0; i <4096 - sizeof(helloworld);i++)
    {
        fwrite(nop, sizeof(nop),1,write);
    }

    fseek(keep,0,SEEK_SET);
    while((num2 = fgetc(keep))!=EOF)//循环读文件到文件尾。
    {
        fputc(num2, write);//写目标文件
    }

    fseek(write,e_header->e_shoff+section_num*sizeof(Elf64_Shdr),SEEK_SET);
    fwrite(s_header, sizeof(Elf64_Shdr),1,write);

    Elf64_Shdr *after_s_header = (Elf64_Shdr*)malloc(sizeof(Elf64_Shdr));//重写p_header之后的节区偏移量
    fseek(source,(int)e_header->e_shoff-4096 + (section_num+1)*sizeof(Elf64_Shdr),SEEK_SET);
    for(int m = 0; m <(int)e_header->e_shnum-section_num-1;m++)
    {
        fread(after_s_header, sizeof(Elf64_Shdr),1,source);
        after_s_header->sh_offset = after_s_header->sh_offset+4096;
        fwrite(after_s_header, sizeof(Elf64_Shdr),1,write);
    }

    fclose(source);
    fclose(write);
    fclose(keep);


}



	

	
	

	
