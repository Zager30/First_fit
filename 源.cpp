#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include<time.h>

#define Free 0 //空闲状态
#define Busy 1 //已用状态
#define OK 1    //完成
#define ERROR 0 //出错
#define MAX_length 1024

struct BLOCK  //空闲区的结构体
{
    int id;             //分区的序号
    int size;           //分区的大小
    int startAddr;      //分区的开始位置
    bool status;        //分区的状态：FREE为空闲，BUSY为被占用
    int pid;            //如果该分区被占用，则存放占用进程的id; 否则为-1
    struct Block* prev;  //指向前面一块内存分区
    struct Block* next;   //指向后面一块内存分区
};

struct PCB {
    int pid;            //进程的序号
    int neededMem;      //需要的内存分区大小
    int status;         //1: 内存分配成功；-1：分配失败
    int blockID;        //如果分配成功，保存占用分区的id,否则为-1
    struct PCB* next;   //指向下一个PCB
};
// 线性表的双向链表存储结构
typedef struct DuLNode
{
    BLOCK data;
    struct DuLNode* prior; //前趋指针
    struct DuLNode* next;  //后继指针
}DuLNode, * DuLinkList;

DuLinkList block_first; //头结点
DuLinkList block_last;  //尾结点
PCB tmpPCB[10] = {};
int BlockID=0;  //分区序号，初始为0

int Initblock()//开创带头结点的内存空间链表
{
    block_first = (DuLinkList)malloc(sizeof(DuLNode));
    block_last = (DuLinkList)malloc(sizeof(DuLNode));
    block_first->prior = NULL;
    block_first->next = block_last;
    block_last->prior = block_first;
    block_last->next = NULL;

    block_first->data.size = 0;
    block_first->data.status = Busy; //头结点不会被使用，定义为占用状态防止分区合并失败

    block_last->data.startAddr = 0;
    block_last->data.size = MAX_length;
    block_last->data.status = Free;
    return OK;
}

//首次适应算法
int First_fit(int id,int request)
{
    //为进程开辟新空间且初始化
    DuLinkList temp = (DuLinkList)malloc(sizeof(DuLNode));
    DuLinkList p0 = (DuLinkList)malloc(sizeof(DuLNode));
    temp->data.size = request;
    temp->data.status = Busy;
    
    DuLNode* p = block_first->next;
    p->data.id = 0;
    while (p)
    {
        if (p->data.status == Free && p->data.size == request)//有大小恰好合适的空闲块
        {
            p->data.status = Busy;
            p->data.id=BlockID;
            return OK;
            break;
        }
        if (p->data.status == Free && p->data.size > request)//有空闲块能满足需求且有剩余
        {
            //在这块空闲分区的地址范围内随机产生划分的开始位置，划分出temp给进程使用，将temp插入后面剩余的空闲区p之前
            temp->data.startAddr = rand()%(p->data.size-request) + p->data.startAddr;
            temp->prior = p->prior;
            p->prior->next = temp;
            temp->next = p;
            p->prior = temp;
            
            //再在temp之前插入前面剩余的空闲区p0
            p0->prior = temp->prior;
            temp->prior->next = p0;
            p0->next = temp;
            temp->prior = p0;

            //修改分区大小、起始位置、状态、占用进程的序号等信息 
            p0->data.startAddr = p->data.startAddr;
            p->data.startAddr = temp->data.startAddr + request;
            p0->data.size = temp->data.startAddr - p0->data.startAddr;
            p->data.size =p->data.size-p0->data.size-temp->data.size;
            temp->data.pid = id;
            p0->data.status = Free;
            p->data.status = Free;

            //修改分区序号
            p0->data.id = p->data.id;
            p->data.id = BlockID + 1;
            temp->data.id = BlockID + 2;
            BlockID = temp->data.id;          
            return OK;
        }
        p = p->next;
    }
    return ERROR;
}
//显示内存分配情况
int show()
{
    DuLNode* p = block_first->next;
    p->data.id = 0;
    printf(" FreeBlkId  StartAddr  Size\n");
    while (p)
    {   //输出空闲分区的信息
        if (p->data.status == Free && p->data.size != 0)
            printf("\t%d\t%d\t%d\n", p->data.id, p->data.startAddr, p->data.size);
        if (p->data.size == MAX_length)
        {
            printf("***********************************************************\n");
            return OK;
        }
        p = p->next;
    }
    printf("\n");
    p = block_first->next;
    printf(" UsedBlkId  StartAddr  Size  ProcID\n");
    while (p)
    {   //输出被进程占用的分区信息
        if (p->data.status == Busy)
            printf("\t%d\t%d\t%d\t%d\n",p->data.id, p->data.startAddr, p->data.size, p->data.pid);
        p = p->next;
    }
    printf("***********************************************************\n");
    return OK;
}
//主存回收
int free(int id)
{
    DuLNode* p = block_first->next;
    while (p->data.pid!=id)
    {
        p = p->next;
    }  //找到pid=id的分区
    p->data.status = Free;  //修改状态为空闲
    if (p->prior->data.status == Free && p->next->data.status == Busy)//与前一个空闲区相邻，则合并
    {
        p->prior->data.size += p->data.size;
        p->prior->next = p->next;
        p->next->prior = p->prior;
    }
    if (p->next->data.status == Free && p->prior->data.status == Busy)//与后一个空闲区相邻，则合并
    {
        p->data.size += p->next->data.size;

        if (p->next->next)
        {
            p->next->next->prior = p;
            p->next = p->next->next;
        }
        else
            p->next = p->next->next;
    }
    if (p->prior->data.status == Free && p->next->data.status == Free)//前后的空闲区均为空，合并
    {
        p->prior->data.size += p->data.size + p->next->data.size;
        if (p->next->next)
        {
            p->next->next->prior = p->prior;
            p->prior->next = p->next->next;
        }
        else
            p->prior->next = p->next->next;
    }
    printf("Recycling used memory block for process %d\n", id);
    show();
    return OK;
}
//分配主存
int alloc(int id, int request)
{
    if (First_fit(id, request) == OK)
    {
        tmpPCB[id].status = 1;  //修改进程状态为内存分配成功
        printf("Allocating memory for process %d, memory requirement = %d\n", id, request);
        show();
    }
    else printf("Memory allocation failed for process %d\n", id);
    return OK;
}
//主函数
int main()
{
    int i;
    srand((unsigned)time(NULL));  //随机数种子
    Initblock(); //开创空闲分区链表
    printf("    ProcID\tNeededMem\n");
    for (i = 0; i < 10; i++)
    {
        tmpPCB[i].pid = i;
        tmpPCB[i].status = -1;  //初始状态为未分配内存
        tmpPCB[i].neededMem = rand() % 101 + 100;  //所需的内存大小是100~200的随机数
        printf("\t%d\t%d\n", tmpPCB[i].pid, tmpPCB[i].neededMem);
    }
    show();
    for (i = 0; i < 10; i++)
    {
        alloc(tmpPCB[i].pid, tmpPCB[i].neededMem);
    }  //分配内存
    printf("\nAllocating finished.\n");
    show();
    for (i = 0; i < 10; i++)
    {
        if (tmpPCB[i].status == 1)
            free(i);
    }  //回收内存
    printf("Recycling finished.\n");
    return 0;
}
