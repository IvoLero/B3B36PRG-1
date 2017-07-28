#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
	void* data;
	struct Node* next; 
}Node; 
typedef struct Queue{ 
	Node*head; 
	Node*tail; 
	int size;
}Queue;

int (*comparefunction)(const void*, const void*);
void (*clearfunction)(void*);


void printallelements(void* queue){
	Queue* q =  queue;
	int counter =0;
	Node* temp = q->head;
	printf("Printing all elements:\n");
	if (q->size==0)
		printf("No elements");
	else{
		printf("Head is %d Tail is %d\n", *((int*)q->head->data), *((int*) q->tail->data));
		while (counter != q->size){
			printf("%d ", *((int*)temp->data));
			temp=temp->next;
			counter++;
		}
	}
	printf("\n");
		
	
}

void* create(){
	Queue* q =  (Queue*) malloc(sizeof(Queue));
	q->size=0;
	q->head= NULL;
	q->tail = NULL;
	return (void*)q;
	
		
} _Bool push (void*queue, void *entry){ Queue* q = queue;

	Node* new = (Node*) malloc(sizeof(Node));
	new->data = entry;
	new->next = NULL;

	if (q->tail != NULL){
		Node* temp = q->tail;
		temp->next = new;
	}
	q->tail = new; q-> size++;
	
	if (q->head==NULL)
		q->head=new;

	//printallelements(queue);		
	return 1;
}	
		


//save the pointer from main program for comparing elements
void setCompare(void* queue, int (*compare)(const void*, const void*)){
	comparefunction = compare;
}
void setClear(void* queue, void (*clear)(void*)){
	clearfunction=clear;
}
void clear(void *queue){
	Queue* q = (Queue*) queue;
	Node* temp = q->head;

	while (temp !=NULL){
		Node* rem = temp;
		temp =temp->next;
		clearfunction(rem->data);
		free(rem);
	}
	q->head = NULL;
	q->tail=NULL;
	q->size=0;
	
}
 

 
/*
 * Pop an entry from the head of the queue
 * return: the stored pointer to the entry or NULL if the queue is empty
 */
void* pop(void *queue){
	Queue* q = (Queue*)queue;
	
	if (q->size!=0){
		//temp variable and free
		Node* temp= q->head;
		q->head = q->head->next;
		void* ret = temp->data;
		free(temp);
		q->size--;
		//control popping the tail
		if (q->size==0)
			q->tail = NULL;
		

		//printallelements(queue);
		return ret;
	}
	else return NULL;
	
}
 
/*
 * Insert the given entry to the queue in the InsertSort style using 
 * the set compare function (by the setCompare() ). If such a function
 * is not set or an error occurs during the insertion it returns false.
 *
 * Since push and insert functions can be combined, it cannot be 
 * guaranteed, the internal structure of the queue is always sorted.
 *
 * The expected behaviour is that insert proceeds from the head of
 * the queue to the tail in such a way that it is insert before the entry
 * with the lower value, i.e., it becomes a new head if its value is the
 * new maximum or a new tail if its value is a new minimum of the values
 * in the queue.
 *
 * return: true on success; false otherwise
 */
_Bool insert(void *queue, void *entry){
	Queue* q = (Queue*)queue;
	Node* new = (Node*)malloc(sizeof(Node));
	new->data= entry;
	new->next=NULL;
		//printf("%d", comparefunction(q->head->data,entry)); 
	if (q->head== NULL){
		q->head = new;
		q->tail = new;
		q->size++;
		return 1;
	}
	else if (comparefunction (q->head->data,entry) <=0){
		new->next = q->head;
		q->head=new;
		q->size++;
	//	printallelements(queue);
		return 1;
	}

	Node* temp = q->head;

	while (temp->next !=NULL && comparefunction(temp->next->data,entry)>=1){
			//printf("%d", comparefunction(temp->next->data,entry));
	 		temp = temp->next;
	
	}
	if (temp->next==NULL){
		temp->next=new;
		q->tail= new;
	}
	else{
		new->next = temp->next;
		temp->next = new;
	}
	q->size++;
//	printallelements(queue);
	return 1;
}

/*
 * Erase all entries with the value entry, if such exists
 * return: true on success; false to indicate no such value has been removed
 */
_Bool erase(void *queue, void *entry){
	Queue* q = (Queue*) queue;		
	_Bool sucess = 0;

	
	if (q->head == NULL){
		return 0;
	}
	while (comparefunction(q->head->data,entry)==0){
		sucess =1;
		Node* rem = q->head;
		q->head = q->head->next;
		clearfunction(rem->data);
		free(rem);
		q->size--;

		if (q->head==NULL){
			q->tail=NULL;
			return 1;
		}
	}	
	Node* temp = q->head;
	q->tail= q->head;
	

	while (temp->next!=NULL){
		//printf("%d", *( (int*) temp->data));
		if (comparefunction(temp->next->data,entry)==0){
			sucess= 1;
			Node* rem = temp->next;
			temp->next = (temp->next)->next;
			clearfunction(rem->data);
			free(rem);
			q->size--;
			//printf("%d %d", *((int*)temp->data), *((int*) temp->next->data));

		}
		else{
			temp = temp->next;
			q->tail=temp;
		}
		
	}
			//printallelements(queue);
	return sucess;

} 
void* getEntry(const void *queue, int idx){
	Queue* q = (Queue*) queue;
	if (idx< 0 || idx >= q->size){
		return NULL;
	}
	else{
		int counter=0;
		Node* temp = q->head;

		while (counter!=idx){
			counter++;
			temp=temp->next;
		}
		return temp->data;
	}
}

 
/*
 * return: the number of stored items in the queue
 */
int size(const void *queue){
	return ((Queue*)queue)->size;
}


