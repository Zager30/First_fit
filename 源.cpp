#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include<time.h>

#define Free 0 //����״̬
#define Busy 1 //����״̬
#define OK 1    //���
#define ERROR 0 //����
#define MAX_length 1024

struct BLOCK  //�������Ľṹ��
{
    int id;             //���������
    int size;           //�����Ĵ�С
    int startAddr;      //�����Ŀ�ʼλ��
    bool status;        //������״̬��FREEΪ���У�BUSYΪ��ռ��
    int pid;            //����÷�����ռ�ã�����ռ�ý��̵�id; ����Ϊ-1
    struct Block* prev;  //ָ��ǰ��һ���ڴ����
    struct Block* next;   //ָ�����һ���ڴ����
};

struct PCB {
    int pid;            //���̵����
    int neededMem;      //��Ҫ���ڴ������С
    int status;         //1: �ڴ����ɹ���-1������ʧ��
    int blockID;        //�������ɹ�������ռ�÷�����id,����Ϊ-1
    struct PCB* next;   //ָ����һ��PCB
};
// ���Ա��˫������洢�ṹ
typedef struct DuLNode
{
    BLOCK data;
    struct DuLNode* prior; //ǰ��ָ��
    struct DuLNode* next;  //���ָ��
}DuLNode, * DuLinkList;

DuLinkList block_first; //ͷ���
DuLinkList block_last;  //β���
PCB tmpPCB[10] = {};
int BlockID=0;  //������ţ���ʼΪ0

int Initblock()//������ͷ�����ڴ�ռ�����
{
    block_first = (DuLinkList)malloc(sizeof(DuLNode));
    block_last = (DuLinkList)malloc(sizeof(DuLNode));
    block_first->prior = NULL;
    block_first->next = block_last;
    block_last->prior = block_first;
    block_last->next = NULL;

    block_first->data.size = 0;
    block_first->data.status = Busy; //ͷ��㲻�ᱻʹ�ã�����Ϊռ��״̬��ֹ�����ϲ�ʧ��

    block_last->data.startAddr = 0;
    block_last->data.size = MAX_length;
    block_last->data.status = Free;
    return OK;
}

//�״���Ӧ�㷨
int First_fit(int id,int request)
{
    //Ϊ���̿����¿ռ��ҳ�ʼ��
    DuLinkList temp = (DuLinkList)malloc(sizeof(DuLNode));
    DuLinkList p0 = (DuLinkList)malloc(sizeof(DuLNode));
    temp->data.size = request;
    temp->data.status = Busy;
    
    DuLNode* p = block_first->next;
    p->data.id = 0;
    while (p)
    {
        if (p->data.status == Free && p->data.size == request)//�д�Сǡ�ú��ʵĿ��п�
        {
            p->data.status = Busy;
            p->data.id=BlockID;
            return OK;
            break;
        }
        if (p->data.status == Free && p->data.size > request)//�п��п���������������ʣ��
        {
            //�������з����ĵ�ַ��Χ������������ֵĿ�ʼλ�ã����ֳ�temp������ʹ�ã���temp�������ʣ��Ŀ�����p֮ǰ
            temp->data.startAddr = rand()%(p->data.size-request) + p->data.startAddr;
            temp->prior = p->prior;
            p->prior->next = temp;
            temp->next = p;
            p->prior = temp;
            
            //����temp֮ǰ����ǰ��ʣ��Ŀ�����p0
            p0->prior = temp->prior;
            temp->prior->next = p0;
            p0->next = temp;
            temp->prior = p0;

            //�޸ķ�����С����ʼλ�á�״̬��ռ�ý��̵���ŵ���Ϣ 
            p0->data.startAddr = p->data.startAddr;
            p->data.startAddr = temp->data.startAddr + request;
            p0->data.size = temp->data.startAddr - p0->data.startAddr;
            p->data.size =p->data.size-p0->data.size-temp->data.size;
            temp->data.pid = id;
            p0->data.status = Free;
            p->data.status = Free;

            //�޸ķ������
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
//��ʾ�ڴ�������
int show()
{
    DuLNode* p = block_first->next;
    p->data.id = 0;
    printf(" FreeBlkId  StartAddr  Size\n");
    while (p)
    {   //������з�������Ϣ
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
    {   //���������ռ�õķ�����Ϣ
        if (p->data.status == Busy)
            printf("\t%d\t%d\t%d\t%d\n",p->data.id, p->data.startAddr, p->data.size, p->data.pid);
        p = p->next;
    }
    printf("***********************************************************\n");
    return OK;
}
//�������
int free(int id)
{
    DuLNode* p = block_first->next;
    while (p->data.pid!=id)
    {
        p = p->next;
    }  //�ҵ�pid=id�ķ���
    p->data.status = Free;  //�޸�״̬Ϊ����
    if (p->prior->data.status == Free && p->next->data.status == Busy)//��ǰһ�����������ڣ���ϲ�
    {
        p->prior->data.size += p->data.size;
        p->prior->next = p->next;
        p->next->prior = p->prior;
    }
    if (p->next->data.status == Free && p->prior->data.status == Busy)//���һ�����������ڣ���ϲ�
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
    if (p->prior->data.status == Free && p->next->data.status == Free)//ǰ��Ŀ�������Ϊ�գ��ϲ�
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
//��������
int alloc(int id, int request)
{
    if (First_fit(id, request) == OK)
    {
        tmpPCB[id].status = 1;  //�޸Ľ���״̬Ϊ�ڴ����ɹ�
        printf("Allocating memory for process %d, memory requirement = %d\n", id, request);
        show();
    }
    else printf("Memory allocation failed for process %d\n", id);
    return OK;
}
//������
int main()
{
    int i;
    srand((unsigned)time(NULL));  //���������
    Initblock(); //�������з�������
    printf("    ProcID\tNeededMem\n");
    for (i = 0; i < 10; i++)
    {
        tmpPCB[i].pid = i;
        tmpPCB[i].status = -1;  //��ʼ״̬Ϊδ�����ڴ�
        tmpPCB[i].neededMem = rand() % 101 + 100;  //������ڴ��С��100~200�������
        printf("\t%d\t%d\n", tmpPCB[i].pid, tmpPCB[i].neededMem);
    }
    show();
    for (i = 0; i < 10; i++)
    {
        alloc(tmpPCB[i].pid, tmpPCB[i].neededMem);
    }  //�����ڴ�
    printf("\nAllocating finished.\n");
    show();
    for (i = 0; i < 10; i++)
    {
        if (tmpPCB[i].status == 1)
            free(i);
    }  //�����ڴ�
    printf("Recycling finished.\n");
    return 0;
}
