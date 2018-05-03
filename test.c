#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include"/usr/include/elf.h"

int main()
{       
	/* 打开目标文件，读取ELF头部，得到最初的入口地址*/
	FILE *fp;
	Elf64_Ehdr *e_header; //使用Elf64_Ehdr类型的结构体pheader来存储文件头
	fp = fopen("hello_world","r"); //已只读的方式r打开目标文件
	fread(e_header, sizeof(Elf64_Ehdr), 1, fp); //从文件中读取一块大小为sizeof(Elf64_Phdr)的内容到pheader中
	origin_entry = e_header->e_entry; //确定入口地址
	
	/*读取ELF头部后，读取程序头部*/
	Elf64_Phdr *p_header;
	for (i=0; i<(int)e_header->e_phnum; i++) //e_phnum Program header table entrycount
	{    
		fread = (p_header, sizeof(Elf64_Phdr), 1, fp);
		/*如果入口地址在段的内存地址中*/
		if ((p_header->p_paddr < origin_entry) && ((p_header->p_paddr + p_header->p_filesz) > origin_entry))// p_paddr段在内存中的物理地址 p_filesz该段的结区的总字长
		{
			p_header_vaddr =  p_header->vaddr; //找到程序段,保存段的虚拟地址
			p_header_size = p_header->filesz; //保存段的大小
		}
	}

	pass = lseek(fp,(int)p_header->e_shoff - sizeof(e_header) - (int)p_header->e_phnum*sizeof(p_header), SEEK_CUR); //指针跳过程序头部和节区定位到头部
	Elf64_Shdr *s_header;
	for (i=0; i<(int)p_header-> e_shnum; i++)
	{
		fread(s_header, sizeof(Elf64_Shdr), 1, fp);
		if ((s_header->sh_addr+s_header->sh_size) == (p_header_vaddr+p_header_size))
		{
			entry_section_offset = s_header->sh_offset;
			entry_section_size = s_header->sh_size;
			new_entry = s_header->sh_addr + s_header->sh_size; //新的入口地址
		}
	}

	

	
	

	
