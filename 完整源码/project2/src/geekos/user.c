/*
 * Common user mode functions
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.50 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/vfs.h>
#include <geekos/tss.h>
#include <geekos/user.h>

/*
 * This module contains common functions for implementation of user
 * mode processes.
 */

/*
 * Associate the given user context with a kernel thread.
 * This makes the thread a user process.
 */
void Attach_User_Context(struct Kernel_Thread* kthread, struct User_Context* context)
{
    KASSERT(context != 0);
    kthread->userContext = context;

    Disable_Interrupts();

    /*
     * We don't actually allow multiple threads
     * to share a user context (yet)
     */
    KASSERT(context->refCount == 0);

    ++context->refCount;
    Enable_Interrupts();
}

/*
 * If the given thread has a user context, detach it
 * and destroy it.  This is called when a thread is
 * being destroyed.
 */
void Detach_User_Context(struct Kernel_Thread* kthread)
{
    struct User_Context* old = kthread->userContext;

    kthread->userContext = 0;

    if (old != 0) {
	int refCount;

	Disable_Interrupts();
        --old->refCount;
	refCount = old->refCount;
	Enable_Interrupts();

	/*Print("User context refcount == %d\n", refCount);*/
        if (refCount == 0)
            Destroy_User_Context(old);
    }
}

/*
 * Spawn a user process.
 * Params:
 *   program - the full path of the program executable file
 *   command - the command, including name of program and arguments
 *   pThread - reference to Kernel_Thread pointer where a pointer to
 *     the newly created user mode thread (process) should be
 *     stored
 * Returns:
 *   The process id (pid) of the new process, or an error code
 *   if the process couldn't be created.  Note that this function
 *   should return ENOTFOUND if the reason for failure is that
 *   the executable file doesn't exist.
 */
int Spawn(const char *program, const char *command, struct Kernel_Thread **pThread)
{
    /*
     * Hints:
     * - Call Read_Fully() to load the entire executable into a memory buffer
     * - Call Parse_ELF_Executable() to verify that the executable is
     *   valid, and to populate an Exe_Format data structure describing
     *   how the executable should be loaded
     * - Call Load_User_Program() to create a User_Context with the loaded
     *   program
     * - Call Start_User_Thread() with the new User_Context
     *
     * If all goes well, store the pointer to the new thread in
     * pThread and return 0.  Otherwise, return an error code.
     */

   //TODO("Spawn a process by reading an executable from a filesystem");
    //标记各函数的返回值，为0则表示成功，否则失败
    int result;
 
    //保存在内存缓冲中的用户程序可执行文件
    char *exeFileData = 0;
 
    //可执行文件的长度
    ulong_t exeFileLength;
 
    //调用Parse_ELF_Executable函数得到的可执行文件信息
    struct Exe_Format exeFormat;
 
    //指向User_Conetxt的指针
    struct User_Context *userContext = 0;
   
    //指向Kernel_Thread *pThread的指针
    struct Kernel_Thread *pNewThread = 0;
 
    //调用Read_Fully函数将名为program的可执行文件全部读入内存缓冲区
    if ((result = Read_Fully(program,(void **) &exeFileData, &exeFileLength)) != 0)
    {
       Print("Failed to Read File %s!\n", program);
       goto fail;
    }
 
    //调用Parse_ELF_Executable函数分析ELF格式文件
    if ((result = Parse_ELF_Executable(exeFileData, exeFileLength, &exeFormat)) != 0)
    {
       Print("Failed to Parse ELF File!\n");
       goto fail;
    }
 
    //调用Load_User_Program将可执行程序的程序段和数据段装入内存
    if ((result = Load_User_Program(exeFileData, exeFileLength, &exeFormat, command, &userContext)) != 0)
    {
       Print("Failed to Load User Program!\n");
       goto fail;
    }
 
    //在堆分配方式下释放内存并再次初始化exeFileData
    Free(exeFileData);
    exeFileData = 0;
 
    //调用Start_User_Thread函数创建一个进程并使其进入准备运行队列
    pNewThread = Start_User_Thread(userContext, false);
 
    //不是核心级进程(即为用户级进程)
    if (pNewThread != 0)
    {
       *pThread = pNewThread;
 
       //记录当前进程的ID
       result = pNewThread->pid;
    }
    else
    {
       //project2\include\geekos\errno.h
       //超出内存空间
       result = ENOMEM;
    }
 
    return result;
 
//如果新进程创建失败则注销User_Context对象
fail:
    if (exeFileData != 0)
    {
       //释放内存
       Free(exeFileData);
    }
 
    if (userContext != 0)
    {
       //销毁进程对象
       Destroy_User_Context(userContext);
    }
 
    return result;
     
}

/*
 * If the given thread has a User_Context,
 * switch to its memory space.
 *
 * Params:
 *   kthread - the thread that is about to execute
 *   state - saved processor registers describing the state when
 *      the thread was interrupted
 */
void Switch_To_User_Context(struct Kernel_Thread* kthread, struct Interrupt_State* state)
{
    /*
     * Hint: Before executing in user mode, you will need to call
     * the Set_Kernel_Stack_Pointer() and Switch_To_Address_Space()
     * functions.
     */
   // TODO("Switch to a new user address space, if necessary

      //指向User_Conetxt的指针，并初始化为准备切换的进程
   /* struct User_Context* userContext = kthread->userContext;
 
    //判断进程的类型，userContext为0表示此进程为核心态进程
    //如果是核心态进程，就不用切换地址空间
    if (0 == userContext)
    {
       return ;
    }
 
    //为用户态进程时则切换地址空间
    Switch_To_Address_Space(userContext);
 
    //设置内核堆栈指针
    //PAGE_POWER=12
    //PAGE_SIZE=(1<<PAGE_POWER)
    Set_Kernel_Stack_Pointer((ulong_t)(kthread->stackPage+PAGE_SIZE);*/
   
   static struct User_Context* s_currentUserContext;
    struct User_Context* userContext = kthread->userContext;
    KASSERT(!Interrupts_Enabled());
    //判断进程的类型，userContext为0表示此进程为核心态进程
    //如果是核心态进程，就不用切换地址空间
    if (0 == userContext)
    {
       return ;
    }
   if(userContext!=s_currentUserContext){
     ulong_t esp0;
    //为用户态进程时则切换地址空间
    Switch_To_Address_Space(userContext);
    esp0=((ulong_t)kthread->stackPage)+PAGE_SIZE;
    //设置内核堆栈指针
    //PAGE_POWER=12
    //PAGE_SIZE=(1<<PAGE_POWER)
    Set_Kernel_Stack_Pointer(esp0);
    s_currentUserContext=userContext;
  }
}
