#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "mpi.h"

#define opsCount 10000

typedef struct {
	uint64_t rank : 11;
	uint64_t offset : 53;
} nodePtr;

typedef struct {
	int val;
	nodePtr next;
} node;

typedef struct{
	nodePtr dummy;
	nodePtr head;
	nodePtr tail;
} Queue;

static node** allocNodes = NULL; 
static node** allocNodesTmp = NULL; 
static int allocNodeSize = 0; 
static int allocNodeCount = 0; 
static const nodePtr nullPtr = {2047, (MPI_Aint)MPI_BOTTOM};

int succEnq = 0;
int succDeq = 0;
int failedDeq = 0;
int totalElementCount = 0;

uint64_t allocElem(int val, int rank, MPI_Win win) {
	MPI_Aint disp;
	node* allocNode;

	MPI_Alloc_mem(sizeof(node), MPI_INFO_NULL, &allocNode);

	allocNode->val = val;
	allocNode->next = nullPtr;

	MPI_Win_attach(win, allocNode, sizeof(node));

	if (allocNodeCount == allocNodeSize) {
		allocNodeSize += 100;
		allocNodesTmp = (node**)realloc(allocNodes, allocNodeSize * sizeof(node*));
		if (allocNodesTmp != NULL)
			allocNodes = allocNodesTmp;
		else {
			printf("Error while allocating memory!\n");
			return 0;
		}
	}
	
	allocNodes[allocNodeCount] = allocNode;
	allocNodeCount++;
	MPI_Get_address(allocNode, &disp);

	return disp;
}

nodePtr getTail(Queue q, MPI_Win win)
{
	nodePtr tail = { 0 }, curNodePtr = { 0 };

	MPI_Fetch_and_op(NULL, (void*)&tail, MPI_LONG_LONG, 0,
		q.tail.offset + offsetof(node, next), MPI_NO_OP, win);
	MPI_Win_flush(0, win);

	return tail;
}

void enq(int val, int rank, Queue q, MPI_Win win)
{
	nodePtr newNode = { 0 }, result = { 0 }, tmpTail = { 0 }, tmpTailUpdated = { 0 }, tailNext = { 0 };
	
	newNode.rank = rank;
	newNode.offset = allocElem(val, rank, win);

	while(1){
		tmpTail = getTail(q, win);

		MPI_Compare_and_swap((void*)&newNode, (void*)&nullPtr, (void*)&result, MPI_LONG_LONG,
			tmpTail.rank, tmpTail.offset + offsetof(node, next), win);
		MPI_Win_flush(tmpTail.rank, win);

		if(result.rank == nullPtr.rank && result.offset == nullPtr.offset){
			MPI_Compare_and_swap((void*)&newNode, (void*)&tmpTail, (void*)&result, MPI_LONG_LONG,
				0, q.tail.offset + offsetof(node, next), win);
			MPI_Win_flush(0, win);
			succEnq++;
			return;
		} else {
			tailNext = getTail(q, win);
			MPI_Compare_and_swap((void*)&tailNext, (void*)&tmpTail, (void*)&result, MPI_LONG_LONG,
				0, q.tail.offset + offsetof(node, next), win);
			MPI_Win_flush(0, win);
		}
	}
}

nodePtr getHead(Queue q, MPI_Win win)
{
	nodePtr result = { 0 };

	MPI_Fetch_and_op(NULL, (void*)&result, MPI_LONG_LONG, 0,
		q.head.offset + offsetof(node, next), MPI_NO_OP, win);
	MPI_Win_flush(0, win);

	return result;
}

nodePtr getNextHead(nodePtr head, MPI_Win win)
{
	nodePtr result = { 0 };

	if(head.rank == nullPtr.rank) return head;

	MPI_Fetch_and_op(NULL, (void*)&result, MPI_LONG_LONG, head.rank,
		head.offset + offsetof(node, next), MPI_NO_OP, win);
	MPI_Win_flush(head.rank, win);

	return result;
}

int readVal(nodePtr ptr, MPI_Win win)
{
	int result = 0;
	MPI_Get((void*)&result, 1, MPI_INT, ptr.rank, ptr.offset + offsetof(node, val),
		1, MPI_INT, win);
	MPI_Win_flush(ptr.rank, win);
	printf("Val %d\n", result);
}

void deq(Queue q, MPI_Win win)
{
	nodePtr tail = { 0 }, head = { 0 }, afterHead = { 0 }, result = { 0 };

	while(1){
		head = getHead(q, win);
		tail = getTail(q, win);
		afterHead = getNextHead(head, win);

		if(tail.rank == head.rank && tail.offset == head.offset){
			if(afterHead.rank == nullPtr.rank && afterHead.offset == nullPtr.offset) { 
				return;
			} else {
				MPI_Compare_and_swap((void*)&afterHead, (void*)&tail, (void*)&result, MPI_LONG_LONG,
					0, q.tail.offset + offsetof(node, next), win); 
				MPI_Win_flush(0, win);
			}
		} else {
			MPI_Compare_and_swap((void*)&afterHead, (void*)&head, (void*)&result, MPI_LONG_LONG,
				0, q.head.offset + offsetof(node, next), win); 
			MPI_Win_flush(0, win);
			if(result.rank == head.rank && result.offset == head.offset) {
				//readVal(afterHead, win);
				succDeq++; 
				return;
			}
		}
	}
}

void printQueue(int rank, Queue q, MPI_Win win)
{
	node curNode = {0};
	nodePtr curNodePtr = getNextHead(q.head, win);
	int i = -1;
	
	printf("Rank[%d]: Result Queue is: \n", rank);

	while (curNodePtr.offset != nullPtr.offset && curNodePtr.rank != nullPtr.rank) {

		MPI_Get((void*)&curNode, sizeof(node), MPI_BYTE,
			curNodePtr.rank, curNodePtr.offset, sizeof(node), MPI_BYTE, win);
		MPI_Win_flush(curNodePtr.rank, win);

		if(i != - 1)
			printf("------val %d was inserted by rank %d at displacement %x next rank %d next displacement %x------"
			"\n", curNode.val, curNodePtr.rank, curNodePtr.offset, curNode.next.rank, curNode.next.offset);
		i++;
		curNodePtr = curNode.next;
	}
	totalElementCount = i;
}

void init(int argc, char* argv[], int* procid, int* numproc, MPI_Win* win)
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, procid);
	MPI_Comm_size(MPI_COMM_WORLD, numproc);
	MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, win);
}

void showStat(int procid, int numproc, double elapsedTime, int ops)
{
	int results[3] = { 0 };
	int elemCount = 0;

	if(procid != 0) {
		results[0] = succEnq; results[1] = succDeq; results[2] = failedDeq;
		MPI_Send((void*)&results, 3, MPI_INT, 0, 0, MPI_COMM_WORLD);
	} else {
		elemCount += (succEnq - succDeq);
		for(int i = 1; i < numproc; i++){
			MPI_Recv((void*)&results, 3, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			elemCount += (results[0] - results[1]);
		}
	}
	if(procid == 0) {
		printf("Total element count = %d\n", totalElementCount);
		printf("Expected element count = %d\n", elemCount);
		printf("Test result: total elapsed time = %f ops/sec = %f\n", elapsedTime, ops/elapsedTime);
		totalElementCount == elemCount ? puts("Queue Integrity: True") : puts("Queue Integrity: False");
	}
}

void runTest(Queue q, MPI_Win win, int rank, int numproc, int testSize)
{
	double startTime, endTime, elapsedTime;

	srand(time(0) + rank);

	MPI_Win_lock_all(0, win);

	startTime = MPI_Wtime();

	for (int i = 0; i < testSize; i++) {
		if(rand() % 2 == 0) enq(i, rank, q, win);
		else deq(q, win);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	endTime = MPI_Wtime();

	elapsedTime = endTime - startTime;

	if(rank == 0) printQueue(rank, q, win);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Win_unlock_all(win);	

	showStat(rank, numproc, elapsedTime, numproc * testSize);

	for (int i = 0; i < allocNodeCount; i++) {
		MPI_Win_detach(win, allocNodes[i]);
		MPI_Free_mem(allocNodes[i]);
	}

	MPI_Win_free(&win);
	MPI_Finalize();
}

void testInit(int argc, char* argv[], int testSize)
{
	int rank, numproc, elemCount = 0;
	MPI_Win win;
	nodePtr head = { 0 }, tail = { 0 }, dummy = { 0 };
	Queue q = { 0 };

	init(argc, argv, &rank, &numproc, &win);
	if (rank == 0) {
		head.offset = allocElem(-1, 0, win); head.rank = 0;
		tail.offset = allocElem(-1, 0, win); tail.rank = 0;
		dummy.offset = allocElem(-1, 0, win); dummy.rank = 0;
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
		MPI_Put((void*)&dummy, sizeof(nodePtr), MPI_BYTE, 0, head.offset + offsetof(node, next),
			sizeof(nodePtr), MPI_BYTE, win);
		MPI_Put((void*)&dummy, sizeof(nodePtr), MPI_BYTE, 0, tail.offset + offsetof(node, next),
			sizeof(nodePtr), MPI_BYTE, win);
		MPI_Win_unlock(0, win);
		q.dummy = dummy;
		q.head = head;
		q.tail = tail;
	}

	MPI_Bcast(&head, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	MPI_Bcast(&tail, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
	MPI_Bcast(&dummy, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

	q.dummy = dummy;
	q.head = head;
	q.tail = tail;

	runTest(q, win, rank, numproc, testSize);
}

int main(int argc, char* argv[])
{
	testInit(argc, argv, opsCount);
	return 0;
}