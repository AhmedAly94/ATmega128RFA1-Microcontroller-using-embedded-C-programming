#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


#define FIFO_MAX_SIZE 10

typedef struct
{
    uint8_t data[FIFO_MAX_SIZE];
    uint8_t Ptr;
    uint8_t size;

} fifo_t;

typedef struct
{
    uint8_t data[FIFO_MAX_SIZE];
    uint8_t Ptr;
    uint8_t size;

} lifo_t;

void fifo_init(fifo_t *fifo)
{
    for( int i = 0 ; i< FIFO_MAX_SIZE ; i++)
    {
        fifo->data [i] = 0;
    }

    fifo->Ptr = 0;
    fifo->size = 0;

}

void lifo_init(lifo_t *lifo)
{
    for( int i = 0 ; i< FIFO_MAX_SIZE ; i++)
    {
        lifo->data [i] = 0;
    }

    lifo->Ptr = 0;
    lifo->size =0;

}

bool fifo_isEmpty(fifo_t *fifo)
{
    return (fifo->size == 0);

}

bool lifo_isEmpty(lifo_t *lifo)
{
    return (lifo->size == 0);

}


bool fifo_isFull(fifo_t *fifo)
{
    return (fifo->size == FIFO_MAX_SIZE);

}

bool lifo_isFull(lifo_t *lifo)
{
    return (lifo->size == FIFO_MAX_SIZE);

}

void fifo_pushBack(fifo_t *fifo, uint8_t data)
{
    if (fifo_isFull(fifo)){return;}
    fifo->data[fifo->Ptr]=data;
    fifo->Ptr++;
    fifo->size++;
}

void lifo_push(lifo_t *lifo, uint8_t data)
{
    if (lifo_isFull(lifo)){return;}
    lifo->data[lifo->Ptr]=data;
    lifo->Ptr++;
    lifo->size++;
}



uint8_t fifo_popFront(fifo_t *fifo)
{
    if (fifo_isEmpty(fifo)){return 0;}
    uint8_t val = fifo->data[0];
    fifo->size--;
    for(int i = 0 ; i < (fifo-> size) ; i++)
    {
        fifo->data[i] = fifo->data[i+1];
    }
    fifo->Ptr--;
    return val;
}

/*uint8_t lifo_pop(lifo_t *lifo)
{
    if (lifo_isEmpty(lifo)){return 0;}
    lifo->Ptr--;
    uint8_t val = lifo->data[(lifo->Ptr)];

    lifo->size--;
    return val;
}*/


int main(void)
{
     fifo_t fifo;
     lifo_t lifo;

    fifo_init(&fifo);
    lifo_init(&lifo);

    fifo_pushBack(&fifo, 8);
    fifo_pushBack(&fifo, 2);
    fifo_pushBack(&fifo, 3);
    fifo_pushBack(&fifo, 4);
    fifo_pushBack(&fifo, 5);
    fifo_pushBack(&fifo, 6);
    fifo_pushBack(&fifo, 7);

    lifo_push(&lifo,1);
    lifo_push(&lifo,2);
    lifo_push(&lifo,3);
    lifo_push(&lifo,4);
    lifo_push(&lifo,5);
    lifo_push(&lifo,6);

    printf("Poped Element of queue is : %d \n", fifo_popFront(&fifo));
    printf("Poped Element of queue is : %d \n", fifo_popFront(&fifo));


    printf("Poped Element of stack is : %d \n", lifo_pop(&lifo));
    printf("Poped Element of stack is : %d \n", lifo_pop(&lifo));


}
