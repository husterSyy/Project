/*
 * ELF executable loading
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.29 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/ktypes.h>
#include <geekos/screen.h>  /* for debug Print() statements */
#include <geekos/pfat.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/elf.h>


/**
 * From the data of an ELF executable, determine how its segments
 * need to be loaded into memory.
 * @param exeFileData buffer containing the executable file
 * @param exeFileLength length of the executable file in bytes
 * @param exeFormat structure describing the executable's segments
 *   and entry address; to be filled in
 * @return 0 if successful, < 0 on error
 */
int Parse_ELF_Executable(char *exeFileData, ulong_t exeFileLength,
    struct Exe_Format *exeFormat)
{
    //TODO("Parse an ELF executable image");
         int i;
    elfHeader *head = (elfHeader*)exeFileData;
    programHeader *proHeader = (programHeader *)(exeFileData+head->phoff);
    KASSERT(exeFileData != NULL);
    KASSERT(exeFileLength > head->ehsize+head->phentsize*head->phnum);
    KASSERT(head->entry%4 == 0);
    exeFormat->numSegments = head->phnum;
    exeFormat->entryAddr = head->entry;
    for(i=0; i<head->phnum; i++)
{
        exeFormat->segmentList[i].offsetInFile = proHeader->offset;
        exeFormat->segmentList[i].lengthInFile = proHeader->fileSize;
        exeFormat->segmentList[i].startAddress = proHeader->vaddr;
        exeFormat->segmentList[i].sizeInMemory = proHeader->memSize;
        exeFormat->segmentList[i].protFlags = proHeader->flags;
        proHeader++;
}
return 0;
}

