#include <stdio.h>
#include <stdlib.h>

// Структура — элемент списка (значение, указатель на след. элемент)
typedef struct Node {
    int data;
    struct Node *next;
} Node;

// Функция поиска последнего элемента в списке
Node* getLast(Node *head) {
    if (head == NULL) {
        return NULL;
    }
    while (head->next) {
        head = head->next;
    }
    return head;
}

// Функция добавления нового элемента в конец списка
void pushBack(Node **head, int data) {
    Node *last = getLast(*head);
    Node *tmp = (Node*)malloc(sizeof(Node));
    tmp->data = data;
    tmp->next = NULL;
    if (last == NULL) {
        *head = tmp;
    }
    else {
        last->next = tmp;
    }
}

// Функция-оболочка для визуальной идентификации контекстов в файле групп
void add_A(Node **head, int data)
{
    pushBack(head,data);
}
void add_B(Node **head, int data)
{
    pushBack(head,data);
}
void add_C(Node **head, int data)
{
    pushBack(head,data);
}

int my_main() {
    // Размер списка
    int list_size = 10000;

    Node *A = NULL, *B = NULL, *C = NULL;
    // Последовательное добавление элементов в списки
    for(int i = 0; i < list_size; i++)
    {
        add_A(&A,i);
        add_B(&B,i+1);
        add_C(&C,i+2);
    }

    int sum[list_size];
    for(int i = 0; i < list_size; i++)
    {
        sum[i] = A->data + B->data;
        A = A->next;
        B = B->next;
    }

    int res = 0;
    for(int i = 0; i < list_size; i++)
    {
        res += C->data;
        C = C->next;
    }
    
    // Иногда нужно для исправления ошибки
    // `BOLT-ERROR: new function size (0x??) is larger than maximum allowed size (0x??) for function function_name`
    int a = res;
    for (int i = 0; i < list_size; i++)
    {
        printf("%d ", sum[i]);
        a -= 1;
    }
    printf("\n%d", res);

    return 0;
}

int main() {
    return my_main();
}
