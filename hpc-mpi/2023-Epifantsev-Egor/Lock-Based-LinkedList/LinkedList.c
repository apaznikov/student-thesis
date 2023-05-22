#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define smallTest 256
#define mediumTest 1000
#define bigTest 2000
#define testType 1

typedef struct {
	int rank; 
	MPI_Aint disp;
} nodePtr;

typedef struct {
	int id;
	int val; 
	char logicalyDeleted;
	char canBeReclaimed;
	nodePtr next;
} node;

static node** allocNodes = NULL; 
static node** allocNodesTmp = NULL; 
static int allocNodeSize = 0; 
static int allocNodeCount = 0; 
static const nodePtr nullPtr = { -1, (MPI_Aint)MPI_BOTTOM };
int successIns = 0;
int failedIns = 0;
int successDel = 0;
int failedDel = 0;
int totalElementCount = 0;
int reclaimedNodes = 0;

MPI_Aint allocElem(int id, int val, int rank, MPI_Win win) {
	MPI_Aint disp;
	node* allocNode;

	MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
		for(int i = 0; i < allocNodeCount; i++){
			if(allocNodes[i]->canBeReclaimed == 1){
				allocNodes[i]->id = id;
				allocNodes[i]->val = val;
				allocNodes[i]->next = nullPtr;
				allocNodes[i]->logicalyDeleted = 0;
				allocNodes[i]->canBeReclaimed = 0;
				reclaimedNodes++;
				MPI_Get_address(allocNodes[i], &disp);
				MPI_Win_unlock(rank, win);
				return disp;
			}
	}
	MPI_Win_unlock(rank, win);

	MPI_Alloc_mem(sizeof(node), MPI_INFO_NULL, &allocNode);

	allocNode->id = id;
	allocNode->val = val;
	allocNode->next = nullPtr;
	allocNode->logicalyDeleted = 0;
	allocNode->canBeReclaimed = 0;

	MPI_Win_attach(win, allocNode, sizeof(node));

	if (allocNodeCount == allocNodeSize) {
		allocNodeSize += 100;
		allocNodesTmp = (node**)realloc(allocNodes, allocNodeSize * sizeof(node*));
		if (allocNodesTmp != NULL)
			allocNodes = allocNodesTmp;
		else {
			printf("Error while allocating memory!\n");
			return -1;
		}
	}

	allocNodes[allocNodeCount] = allocNode;
	allocNodeCount++;

	MPI_Get_address(allocNode, &disp);

	return disp;
}

void printList(int procid, nodePtr head, MPI_Win win)
{
	nodePtr curNodePtr = head;
	node curNode = { 0 };
	int i = -1, marked = 0;
	
	printf("Rank[%d]: Result list is: \n", procid);

	while (curNodePtr.rank != nullPtr.rank && curNodePtr.disp != nullPtr.disp) {

		MPI_Win_lock(MPI_LOCK_SHARED, curNodePtr.rank, 0, win);

		MPI_Get(&curNode, sizeof(node), MPI_BYTE,
			curNodePtr.rank, curNodePtr.disp, sizeof(node), MPI_BYTE, win);

		MPI_Win_unlock(curNodePtr.rank, win);

		if(curNode.logicalyDeleted == 1) marked ++;

		printf("-----id %d: val %d was inserted by rank %d at displacement %x marked %d next rank %d next displacement %x"
			"-----\n", curNode.id, curNode.val, curNodePtr.rank, curNodePtr.disp, curNode.logicalyDeleted,
			curNode.next.rank, curNode.next.disp);

		curNodePtr = curNode.next;
		i++;
	}

	totalElementCount = i;
}

nodePtr search(int id, nodePtr head, MPI_Win win)
{
	nodePtr curNodePtr = head, next = {0};
	node curNode = {0};

	while(curNodePtr.rank != -1 && curNodePtr.disp != (MPI_Aint)MPI_BOTTOM){
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, curNodePtr.rank, 0, win);
			
		MPI_Get((void*)&curNode, sizeof(node), MPI_BYTE, curNodePtr.rank, curNodePtr.disp,
			sizeof(node), MPI_BYTE, win);
		
		MPI_Win_flush(curNodePtr.rank, win);

		if(curNode.id == id) {
			MPI_Win_unlock(curNodePtr.rank, win);
			return curNodePtr;
		} else next = curNode.next;

		MPI_Win_unlock(curNodePtr.rank, win);
	
		curNodePtr = next;
	}

	return nullPtr;
}

void insertAfter(int id, int newVal, int key, int rank, nodePtr head, MPI_Win win)
{
	node curNode;
	nodePtr newNode, curNodePtr, fetched;

	curNodePtr = search(key, head, win);
	if(curNodePtr.rank == nullPtr.rank && curNodePtr.disp == nullPtr.disp) {
		failedIns++;
		return;
	}
	else {
		newNode.rank = rank;
		newNode.disp = allocElem(id, newVal, rank, win);
		if(newNode.disp == -1) return;

		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, curNodePtr.rank, 0, win);

		MPI_Get((void*)&curNode, sizeof(node), MPI_BYTE, curNodePtr.rank, curNodePtr.disp,
			sizeof(node), MPI_BYTE, win);

		MPI_Win_flush(curNodePtr.rank, win);

		if(curNode.logicalyDeleted != 1){
			MPI_Fetch_and_op(&newNode.rank, &fetched.rank, MPI_INT, curNodePtr.rank,
				curNodePtr.disp + offsetof(node, next.rank), MPI_REPLACE, win);
			
			MPI_Fetch_and_op(&newNode.disp, &fetched.disp, MPI_AINT, curNodePtr.rank,
				curNodePtr.disp + offsetof(node, next.disp), MPI_REPLACE, win);

			MPI_Win_flush(curNodePtr.rank, win);

			((node*)newNode.disp)->next.rank = fetched.rank;
			((node*)newNode.disp)->next.disp = fetched.disp;
			
			successIns++;
		} else {
			MPI_Win_unlock(curNodePtr.rank, win);
			failedIns++;
			return;
		}
		MPI_Win_unlock(curNodePtr.rank, win);
	}
}

int traverseAndDelete(nodePtr head, nodePtr nodeToDelete, nodePtr nextAfterDeleted, MPI_Win win)
{
	nodePtr curNodePtr = head, next = {0};
	node curNode = {0};

	while(curNodePtr.rank != -1 && curNodePtr.disp != (MPI_Aint)MPI_BOTTOM){
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, curNodePtr.rank, 0, win);
			
		MPI_Get((void*)&curNode, sizeof(node), MPI_BYTE, curNodePtr.rank, curNodePtr.disp,
			sizeof(node), MPI_BYTE, win);
		
		MPI_Win_flush(curNodePtr.rank, win);

		if(curNode.next.rank == nodeToDelete.rank && curNode.next.disp == nodeToDelete.disp) {
			if(curNode.logicalyDeleted == 0){
				MPI_Put((void*)&nextAfterDeleted, sizeof(nodePtr), MPI_BYTE, curNodePtr.rank,
				curNodePtr.disp + offsetof(node, next), sizeof(nodePtr), MPI_BYTE, win);
				MPI_Win_unlock(curNodePtr.rank, win);
				return 1;
			} else {
				MPI_Win_unlock(curNodePtr.rank, win);
				return 0;
			}
		} else {
			next = curNode.next;
			MPI_Win_unlock(curNodePtr.rank, win);
		}
		curNodePtr = next;
	}
}

void Delete(int key, nodePtr head, MPI_Win win)
{
	nodePtr curNodePtr, next;
	node curNode;
	char mark = 1, emptyMark = 0, result = 0, free = 1;
	int testDelete = 0;

	curNodePtr = search(key, head, win);
	if(curNodePtr.rank == nullPtr.rank && curNodePtr.disp == nullPtr.disp) {
		failedDel++;
		return;
	} else {
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, curNodePtr.rank, 0, win);

		MPI_Compare_and_swap((void*)&mark, (void*)&emptyMark, (void*)&result, 
			MPI_BYTE, curNodePtr.rank, curNodePtr.disp + offsetof(node, logicalyDeleted), win);
		
		MPI_Win_flush(curNodePtr.rank, win);

		if(result != 1) MPI_Get((void*)&next, sizeof(nodePtr), MPI_BYTE, curNodePtr.rank, curNodePtr.disp + 
			offsetof(node, next), sizeof(nodePtr), MPI_BYTE, win);

		MPI_Win_unlock(curNodePtr.rank, win);

		if(result != 1) {
			while(testDelete == 0){
				testDelete = traverseAndDelete(head, curNodePtr, next, win);
			}
			MPI_Win_lock(MPI_LOCK_EXCLUSIVE, curNodePtr.rank, 0, win);
				MPI_Put((void*)&free, 1, MPI_INT, curNodePtr.rank,
					curNodePtr.disp + offsetof(node, canBeReclaimed), 1, MPI_INT, win);
			MPI_Win_unlock(curNodePtr.rank, win);
			successDel++;
		} else failedDel++;	
	}
}

void init(int argc, char* argv[], int* procid, int* numproc, MPI_Win* win)
{
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, procid);
	MPI_Comm_size(MPI_COMM_WORLD, numproc);
	MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, win);
}

void showStat(int procid, int numproc, int testSize, double elapsedTime, int opsCount)
{
	int results[5] = { 0 };
	int elemCount = 0;

	if(procid != 0) {
		results[0] = successIns; results[1] = failedIns; results[2] = successDel; results[3] = failedDel, results[4] = reclaimedNodes;
		MPI_Send((void*)&results, 5, MPI_INT, 0, 0, MPI_COMM_WORLD);
	} else {
		printf("Rank %d success insert = %d, failed insert = %d, success delete = %d, failed delete = %d, reclaimed = %d\n", 
			procid, successIns, failedIns, successDel, failedDel, reclaimedNodes);
			elemCount = elemCount + successIns - successDel;
		for(int i = 1; i < numproc; i++){
			MPI_Recv((void*)&results, 5, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Rank %d success insert = %d, failed insert = %d, success delete = %d, failed delete = %d, reclaimed = %d\n", 
				i, results[0], results[1], results[2], results[3], results[4]);
			elemCount = elemCount + results[0] - results[2];
		}
	}
	if(procid == 0){
		printf("Total element count = %d\n", totalElementCount);
		printf("Expected element count = %d\n", elemCount);
		(totalElementCount == elemCount) ? printf("List Integrity: True\n") : printf("List Integrity: False\n");
		printf("Test result: total elapsed time = %f ops/sec = %f\n", elapsedTime, opsCount/elapsedTime);
	}
}

void runTest2(nodePtr head, MPI_Win win, int procid, int numproc, int testSize)
{
	double startTime, endTime, elapsedTime;

	srand(time(0) + procid);

	startTime = MPI_Wtime();

	for (int i = 0; i < testSize; i++) {
		if(procid == 0 || procid == 2) {
			insertAfter((procid + 3000) * i, 2000, -1, procid, head, win);
			Delete(i, head, win);
		}
		if(procid == 1 || procid == 3) {
			insertAfter((procid + 3000) * i, 2000, i, procid, head, win);
			Delete(3000 * i, head, win);
		} else {
			insertAfter((procid + 3000) * i, 2000, i, procid, head, win);
			Delete(3001 * i, head, win);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	endTime = MPI_Wtime();

	elapsedTime = endTime - startTime;

	MPI_Barrier(MPI_COMM_WORLD);
	if(procid == 0) printList(procid, head, win);
		
	MPI_Barrier(MPI_COMM_WORLD);

	showStat(procid, numproc, testSize, elapsedTime, numproc * testSize);

	for (int i = 0; i < allocNodeCount; i++) {
		MPI_Win_detach(win, allocNodes[i]);
		MPI_Free_mem(allocNodes[i]);
	}

	MPI_Win_free(&win);
	MPI_Finalize();
}

void runTest1(nodePtr head, MPI_Win win, int procid, int numproc, int testSize)
{
	double startTime, endTime, elapsedTime;

	srand(time(0) + procid);

	startTime = MPI_Wtime();

	for (int i = 0; i < testSize; i++) {
		if(rand() % 2 == 0) {
			insertAfter((procid + 3000) * i, 2000, i, procid, head, win);
		} else Delete(i, head, win);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	endTime = MPI_Wtime();

	elapsedTime = endTime - startTime;

	MPI_Barrier(MPI_COMM_WORLD);
	if(procid == 0) printList(procid, head, win);
		
	MPI_Barrier(MPI_COMM_WORLD);

	showStat(procid, numproc, testSize, elapsedTime, numproc * testSize);

	for (int i = 0; i < allocNodeCount; i++) {
		MPI_Win_detach(win, allocNodes[i]);
		MPI_Free_mem(allocNodes[i]);
	}

	MPI_Win_free(&win);
	MPI_Finalize();
}

void test(int argc, char* argv[], int testSize, int type)
{
	int procid, numproc;
	MPI_Win win;
	nodePtr head = { 0 };

	init(argc, argv, &procid, &numproc, &win);
	if (procid == 0) { head.disp = allocElem(-1, -1, 0, win); head.rank = 0;}
	MPI_Bcast(&head.disp, 1, MPI_AINT, 0, MPI_COMM_WORLD);

	srand(time(0) + procid);

	for (int i = 1; i < testSize; i++) {
		if(procid == 0) insertAfter((procid + 1) * i, 2000, -1, procid, head, win);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if(type == 1) runTest1(head, win, procid, numproc, testSize);
	if(type == 2) runTest2(head, win, procid, numproc, testSize);
}

int main(int argc, char** argv)
{
	test(argc, argv, mediumTest, testType);
	return 0;
}