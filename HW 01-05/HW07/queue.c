#include "queue.h" 
/* Queue structure which holds all necessary data */

queue_t* create_queue(int cap){
	queue_t* queue = malloc(sizeof(queue_t)); 
	queue->data = (void**)malloc(sizeof(void*)*cap);
	queue->allocated=cap;
	queue->head=0;
	queue->size=0;
	return queue;
}
 

void delete_queue(queue_t *queue){
	free(queue->data);
	free(queue);
} 
void* get_from_queue(queue_t *queue, int idx){
	//printf("%d %d %d\n", queue->size, queue->allocated, queue->head);
	//printallelements(queue);	
	if (idx < 0 || idx>= queue->size) 
		return NULL;
	else return queue->data[(queue->head+idx >=queue->allocated)? idx-(queue->allocated-queue->head): queue->head+idx];

	
}


void printallelements(queue_t *queue){
	printf("Printing all elements");
	for (int i=0; i<queue->size; i++){
		printf("%d ", *((int*)get_from_queue(queue,i)));

	}
	printf("\n\n");
}
 
bool push_to_queue(queue_t *queue, void *data){

	//printallelements(queue);
	//printf("%d %d %d\n", queue->size, queue->allocated, queue->head);
	//if (s < queue->capacity){
		if (queue->size == queue-> allocated){
			//queue->allocated = (2*queue->size <=queue->capacity)? 2*queue->size : queue->capacity;	
			queue->allocated= 2*queue->allocated;
			queue->data = (void**)realloc(queue->data,sizeof(void*)* queue->allocated);
			//a=queue->allocated;
			//printf("allocated %lu\n", sizeof(queue->data));

		}

		//printf("%d", (h+s >=a)? s-(a-h): h+s);
		queue->data[(queue->head+queue->size >=queue->allocated)? queue->size-(queue->allocated-queue->head): queue->head+queue->size]=data;
		queue->size+=1;
		return true;
	//else return false;
		
}
 
void* pop_from_queue(queue_t *queue){
	if (queue->size != 0){
		void* ret = queue->data[queue->head];
		//printallelements(queue);
		//printf("%d %d %d\n", queue->size, queue->allocated, queue->head);
		//printf("pop %d\n", *((int*)ret));
		if (queue->head +1 !=queue->allocated)
			queue->head+=1;
		else	
			queue->head=0;	
		queue->size-=1;
		//printf("queue head %d\n",queue->head);

		//if the size is less than third of allocated space
		if (queue-> size < queue->allocated/3 && queue->size!=0){
			void** newdata = (void**)malloc(sizeof(void*)* queue->size);
			for (int i= 0; i < queue->size;i++){
				newdata[i]=get_from_queue(queue,i);
			}
			free(queue->data);
			queue->data = newdata;
			queue->head=0;
			queue->allocated = queue-> size;
		}
		return ret;
	}
	return NULL;
}
int get_queue_size(queue_t *queue){
	return queue->size;
}

