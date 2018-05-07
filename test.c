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

	/* 打开目标文件，读取ELF头部，得到最初的入口地址*/
	Elf64_Ehdr *e_header = (Elf64_Ehdr*)malloc(sizeof(Elf64_Ehdr)); //使用Elf64_Ehdr类型的结构体eheader来存储文件头
	FILE *fp = fopen("../hello_world","rb+"); //已只读的方式r打开目标文件
	fread(e_header, sizeof(Elf64_Ehdr), 1, fp); //从文件中读取一块大小为sizeof(Elf64_Phdr)的内容到pheader中
	Elf64_Addr origin = e_header->e_entry; //确定入口地址

	/*读取ELF头部后，读取程序头部*/
    int p_num;
	Elf64_Phdr *p_header = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr));
	for (int i = 0; i < (int) e_header->e_phnum; i++) //e_phnum Program header table entrycount
	{
		fread(p_header, sizeof(Elf64_Phdr), 1, fp);
		/*如果入口地址在段的内存地址中*/
		if ((p_header->p_paddr < origin) &&
			((p_header->p_paddr + p_header->p_filesz) > origin))// p_paddr段在内存中的物理地址 p_filesz该段的结区的总字长
		{
			p_header_vaddr = p_header->p_vaddr; //找到程序段,保存段的虚拟地址
			p_header_size = p_header->p_filesz; //保存段的大小
            p_num = i;

		}
	}

    fseek(fp,(int)(e_header->e_shoff) - sizeof(Elf64_Ehdr) - (int)e_header->e_phnum * sizeof(Elf64_Phdr), SEEK_CUR);//指针跳过程序头部和节区定位到头部

    Elf64_Shdr *s_header = (Elf64_Shdr*)malloc(sizeof(Elf64_Shdr));
    Elf64_Shdr *after_sh = (Elf64_Shdr*)malloc(sizeof(Elf64_Shdr));
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

    for (int j = section_num + 1;j<(int)e_header->e_shnum;j++){
        fread(after_sh, sizeof(Elf64_Shdr),1,fp);
        after_sh->sh_offset = after_sh->sh_offset + 4096;
    }

    p_header->p_filesz = p_header->p_filesz + 4096;
	p_header->p_memsz = p_header->p_memsz + 4096; //增加段的大小
    s_header->sh_size = entry_section_size + 4096; //增加节区的大小
	e_header->e_entry = new_entry;
	e_header->e_shoff = e_header->e_shoff+4096;

	char nop[] = {0x90};
	char helloworld[] = {
	                      0xbd,0x76,0x53,0x19,0x04,
	                      0x48,
                          0x65,0x6c,
                          0x6c,
                          0x20,0x57,0x6f,
                          0x72,0x6c,
                          0x64,0x21,0x0a,
                          0x48,0xc7,0xc0,0x01,0x00,0x00,0x00,
                          0x48,0xc7,0xc7,0x01,0x00,0x00,0x00,
                          0x48,0xc7,0xc6,0x78,0x00,0x40,0x00,
                          0x48,0xc7,0xc2,0x0d,0x00,0x00,0x00,
                          0xbd,0xe9};

    fclose(fp);

	int c;
	FILE *source,*write,*keep;
	source = fopen("../hello_world", "r");
	write = fopen("../rewrite","w+");//打开文件。
    keep = fopen("../keep","w+");

    while((c = fgetc(source))!=EOF)//循环读文件到文件尾。
    {
        fputc(c, write);//写目标文件
    }

    fseek(source,entry_section_offset+entry_section_size,SEEK_SET);//从偏移位置开始读取
    while((c = fgetc(source))!=EOF)
    {
        fputc(c,keep);
    }
    fclose(source);

    if(-1 == fseek(write,entry_section_offset+entry_section_size,SEEK_SET))
    {
        printf("fseek error");
    }

    if(1 != fwrite(helloworld,sizeof(helloworld),1,write))
    {
        printf("fwrite error 1");
    }

	for(int i; i <4096 - sizeof(helloworld);i++)
    {
        fwrite(nop, sizeof(nop),1,write);
    }

    fseek(keep,0,SEEK_SET);

    while((c=fgetc(keep))!=EOF)
    {
        fputc(c,write);
    }
    fflush(write);
    fseek(write,0,SEEK_SET);
    fwrite(e_header, sizeof(e_header),1,write);
    fseek(write,(p_num-1)*sizeof(Elf64_Phdr),SEEK_CUR);
    fwrite(p_header, sizeof(p_header),1,write);
    fseek(write,(int)(e_header->e_shoff) - sizeof(Elf64_Ehdr) - (int)e_header->e_phnum * sizeof(Elf64_Phdr),SEEK_CUR);
    fwrite(s_header, sizeof(s_header),1,write);

    fclose(write);
    fclose(keep);

    //调试用
    Elf64_Ehdr *test_e_header = (Elf64_Ehdr*)malloc(sizeof(Elf64_Ehdr)); //使用Elf64_Ehdr类型的结构体eheader来存储文件头it 
    FILE *test = fopen("../testheader","r+"); //已只读的方式r打开目标文件
    fwrite(e_header, sizeof(e_header),1,test);
    fflush(test);
    fseek(test,0,SEEK_SET);
    fread(test_e_header, sizeof(Elf64_Ehdr), 1, test); //从文件中读取一块大小为sizeof(Elf64_Phdr)的内容到pheader中
    fclose(test);
}



	

	
	

	
