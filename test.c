//#include<stdio.h>
//#include<unistd.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <elf.h>
//#include"/usr/include/elf.h"
//
//int main() {
//	Elf64_Addr p_header_vaddr;
//	Elf64_Addr p_header_size;
//
//	/* 打开目标文件，读取ELF头部，得到最初的入口地址*/
//
//	Elf64_Ehdr *e_header = (Elf64_Ehdr*)malloc(sizeof(Elf64_Ehdr)); //使用Elf64_Ehdr类型的结构体eheader来存储文件头
//	FILE *fp = fopen("../hello_world","r"); //已只读的方式r打开目标文件
//	fread(e_header, sizeof(Elf64_Ehdr), 1, fp); //从文件中读取一块大小为sizeof(Elf64_Phdr)的内容到pheader中
//	Elf64_Addr origin = e_header->e_entry; //确定入口地址
//
//	/*读取ELF头部后，读取程序头部*/
//	Elf64_Phdr *p_header = (Elf64_Phdr*)malloc(sizeof(Elf64_Phdr));
//	for (int i = 0; i < (int) e_header->e_phnum; i++) //e_phnum Program header table entrycount
//	{
//		fread(p_header, sizeof(Elf64_Phdr), 1, fp);
//		/*如果入口地址在段的内存地址中*/
//		if ((p_header->p_paddr < origin) &&
//			((p_header->p_paddr + p_header->p_filesz) > origin))// p_paddr段在内存中的物理地址 p_filesz该段的结区的总字长
//		{
//			p_header_vaddr = p_header->p_vaddr; //找到程序段,保存段的虚拟地址
//			p_header_size = p_header->p_filesz; //保存段的大小
//		}
//	}
//
//    fseek(fp,(int)(e_header->e_shoff) - sizeof(e_header) - (int)e_header->e_phnum * sizeof(p_header), SEEK_CUR);//指针跳过程序头部和节区定位到头部
//
//	Elf64_Shdr *s_header = (Elf64_Shdr*)malloc(sizeof(Elf64_Shdr));
//    Elf64_Off entry_section_offset;
//    Elf64_Xword entry_section_size;
//    Elf64_Addr new_entry;
//
//    for (int i = 0; i < (int) e_header->e_shnum; i++) {
//		fread(s_header, sizeof(Elf64_Shdr), 1, fp);
//		if (s_header->sh_addr+s_header->sh_size == (p_header_vaddr + p_header_size)) {
//			entry_section_offset = s_header->sh_offset;
//			int t = s_header->sh_offset;
//			entry_section_size = s_header->sh_size;
//			new_entry = s_header->sh_addr + s_header->sh_size; //新的入口地址
//		}
//	}
//
//	char nop[] = {0x90};
//	char hello_world[] = {
//	                      0x48,
//                          0x65,0x6c,
//                          0x6c,
//                          0x6f,
//                          0x20,0x57,0x6f,
//                          0x72,0x6c,
//                          0x64,0x21,0x0a,
//                          0x48,0xc7,0xc0,0x01,0x00,0x00,0x00,
//                          0x48,0xc7,0xc7,0x01,0x00,0x00,0x00,
//                          0x48,0xc7,0xc6,0x78,0x00,0x40,0x00,
//                          0x48,0xc7,0xc2,0x0d,0x00,0x00,0x00,
//                          0xff,0xe5};
//
//	struct jumpto{
//		char data[41];
//		int addr;
//		short code_jump;
//	};
//	struct jumpto* jump = (struct jumpto*)hello_world;
//	jump->addr = origin; //最初的入口
//	FILE* newfile = fopen("../helloo","w"); ;
//	fwrite(hello_world, sizeof(hello_world),1,newfile);
//	for(int i; i <4096 - sizeof(hello_world);i++)
//    {
//        fwrite(nop, sizeof(nop),1,newfile);
//    }
//
//}
//
//
//
//
//
//
//
//
//
