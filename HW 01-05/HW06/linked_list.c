#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Node{
	int value;
	struct Node* next;
}Node;

Node*head = NULL;
Node*tail = NULL;
int mysize = 0;
void printlist(){
	Node* temp=head;
	printf("head is %d\n",(head!=NULL)?head->value : -1);
	printf("tail is %d\n",(tail!= NULL)?tail->value :-1);

	while(temp!=NULL){
		printf("%d ", temp->value);
		temp = temp->next;
	}
	printf("\n");
}

_Bool push(int entry){
	//printlist();

	if (entry<0)
		return 0;

	Node* new = (Node*) malloc(sizeof(Node));
	if (new==NULL)
		return 0;

	new-> value = entry;
	new->next = NULL;
	if (tail !=NULL){
		Node* temp = tail;
		temp->next = new;
	}
	tail= new;
	if (head==NULL)
		head=new;

	mysize++;
	//printlist();
	return 1;
}
int pop(){
	if (mysize!=0){
		Node* temp= head;
		head = head->next;
		int ret = temp->value;
		free(temp);
		mysize--;
		if (mysize==0)
			tail = NULL;

		return ret;
	}
	else return -1;
}
_Bool insert(int entry){
	//printlist();
	
	Node* new = (Node*)malloc(sizeof(Node));
	if (new==NULL)
		return 0;

	new->value = entry;
	new->next=NULL;

	if (head== NULL){
		head = new;
		tail = new;
		mysize++;
		return 1;
	}
	else if (head->value <= entry){
		new->next = head;
		head=new;
		mysize++;
		return 1;
	}

	Node* temp = head;
	while (temp->next !=NULL && (temp->next)->value > entry){
	 		temp = temp->next;
	
	}
	if (temp->next==NULL){
		temp->next=new;
		tail= new;
	}
	else{
		new->next = temp->next;
		temp->next = new;
	}
	mysize++;
	return 1;
}
int getEntry(int idx){
	if (idx< 0 || idx>=mysize){
		return -1;
	}
	else{
		int counter=0;
		Node* temp = head;

		while (counter!=idx){
			counter++;
			temp=temp->next;
		}
		return temp->value;
	}
}
int size(){
	return mysize;
}
void clear(){
	Node* temp = head;
	while (temp!=NULL){
		Node* rem = temp;
		temp = temp->next;
		free(rem);
	}
	head= NULL;
	tail=NULL;
	mysize=0;
}



_Bool erase (int entry){
		
	_Bool sucess = 0;

	//if head is null, nothing is to be erased	
	if (head == NULL){
		return 0;
	}
	//secures the validity of head
	while (head->value ==entry){ //if head is target
		sucess =1;
		Node*rem =head;
		head = head->next;
		free(rem);
		mysize--;

		if (head==NULL){
			tail = NULL;
			return 1;
		}
	}
	//secures the validity of tail
	Node* temp = head;
	tail= head;
	//browse other elements	
	while (temp->next!=NULL){
		if ((temp->next)->value==entry){
			sucess= 1;
			Node* rem = temp->next;
			temp->next = (temp->next)->next;
			free(rem);
			mysize--;
		}
		else{
			temp = temp->next;
			tail=temp;
		}
		
	}
	return sucess;
}










	




		
