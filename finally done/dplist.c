#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									         \
		do {											         \
			fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	 \
			fprintf(stderr,__VA_ARGS__);								 \
			fflush(stderr);                                                                          \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition,err_code)\
	do {						            \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");    \
            assert(!(condition));                                    \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t * prev, * next;
    void * element;
};

struct dplist {
    dplist_node_t * head;
    void * (*element_copy)(void * src_element);
    void (*element_free)(void ** element);
    int (*element_compare)(void * x, void * y);
};
dplist_t * sort( dplist_t * list);
dplist_t * freenode(  dplist_t *  list, dplist_node_t * node, bool  free_element);
dplist_t * copy( dplist_t * list,  dplist_node_t* cp, void * element,bool  insert_copy);
dplist_t * dpl_create (// callback functions
        void * (*element_copy)(void * src_element),
        void (*element_free)(void ** element),
        int (*element_compare)(void * x, void * y)
)
{
    dplist_t * list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t ** list, bool free_element)
{
    DPLIST_ERR_HANDLER(*list==NULL,DPLIST_INVALID_ERROR);
    if((*list)->head!=NULL){
    while((*list)->head->next!=NULL){
        dpl_remove_at_index(  *list, 0,   free_element);
    }}
    free((*list)->head);
    free((*list));
    *list=NULL;
}
dplist_t * dpl_insert_at_reference( dplist_t * list, void * element, dplist_node_t * reference, bool insert_copy )
{

    DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
if(list->head==NULL){
        dplist_node_t* header=malloc(sizeof(dplist_node_t));
        list->head=header;
        header->next=NULL;
    }
    if(dpl_get_index_of_reference(  list,   reference )==-1&&dpl_size(list)!=0) return list;
    dplist_node_t* cp=malloc(sizeof(dplist_node_t));
    cp->next=NULL;
    cp->prev=NULL;
   

    if(reference==NULL){
        if(list->head->next!=NULL){
 dplist_node_t * last=dpl_get_last_reference(  list );
            last->next=cp;
            cp->prev=last;}
        else {list->head->next=cp;};

    }
    else if(reference->prev==NULL){
        list->head->next=cp;
        reference->prev=cp;
        cp->next=reference;
    }
    else{
        cp->next=reference;
        cp->prev=reference->prev;
        cp->next->prev=cp;
        cp->prev->next=cp;

    }
    return  copy(list,cp,element,insert_copy);

}







dplist_t * dpl_insert_at_index(dplist_t * list, void * element, int index, bool insert_copy)
{
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
    
    return dpl_insert_at_reference(   list,   element,index>=dpl_size(list)?NULL:dpl_get_reference_at_index(   list,   index ),   insert_copy );
}

dplist_t * dpl_remove_at_index( dplist_t * list, int index, bool free_element)
{
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);



    dplist_node_t* node=dpl_get_reference_at_index(list,index);
    if(node==NULL)return list;
    return freenode(list,node,free_element);


}

int dpl_size( dplist_t * list )
{
     
    int n=0;
    if(list==NULL)return 0;
    if(list->head==NULL)return 0;
    dplist_node_t * dummy=list->head;
    if(dummy==NULL)return 0;
    while(dummy->next!=NULL){
        dummy=dummy->next;
        n++;
    }
    return n;
}
dplist_node_t * dpl_get_reference_at_index( dplist_t * list, int index )
{
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
    if(dpl_size(list)==0)return NULL;
    dplist_node_t * node=list->head->next;
    for(int count=0;index>0&&node->next!=NULL;count++)
    {
        if(count==index)return node;
        node=node->next;
    }
    return node;
}

void * dpl_get_element_at_index( dplist_t * list, int index )
{
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
    dplist_node_t * re =dpl_get_reference_at_index( list,index );
    if(re==NULL)return (void *)0;
    return (void *)re->element;
}

int dpl_get_index_of_element( dplist_t * list, void * element )
{
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
    int size=dpl_size(list);
    if(size<=0)return -1;
    for(int i=0;i<=size-1;i++){
        if(list->element_compare( dpl_get_reference_at_index(  list,i)->element,element  )==0)return i;
    }
    return -1;
}

// HERE STARTS THE EXTRA SET OF OPERATORS //

// ---- list navigation operators ----//

dplist_node_t * dpl_get_first_reference( dplist_t * list )
{

    return dpl_get_reference_at_index(  list,0 );
}

dplist_node_t * dpl_get_last_reference( dplist_t * list )
{
    return dpl_get_reference_at_index(  list,dpl_size(   list ) -1);
}

dplist_node_t * dpl_get_next_reference( dplist_t * list, dplist_node_t * reference )
{
    if(dpl_get_index_of_reference(list,reference)==-1)return NULL;
    if(reference->next==NULL)return NULL;
    return reference->next;
}

dplist_node_t * dpl_get_previous_reference( dplist_t * list, dplist_node_t * reference ){
    if(dpl_get_index_of_reference(list,reference)==-1)return NULL;
    if(reference->prev==NULL)return NULL;
    return reference->prev;
}

// ---- search & find operators ----//

void * dpl_get_element_at_reference( dplist_t * list, dplist_node_t * reference )
{
int index=dpl_get_index_of_reference(list,reference);
    if(index==-1)return NULL;
dplist_node_t *ref=dpl_get_reference_at_index(  list,  index );
    return ref->element;
}

dplist_node_t * dpl_get_reference_of_element( dplist_t * list, void * element )
{
    int index=dpl_get_index_of_element(list, element);
    if(index==-1)return NULL;
    return dpl_get_reference_at_index(  list, index);
}

int dpl_get_index_of_reference(dplist_t * list, dplist_node_t *  reference )
{
    //DPLIST_ERR_HANDLER(list==NULL,DPLIST_MEMORY_ERROR);
    int re=0;
    
    if(dpl_size(list)==0)return -1;
    if(reference==NULL) return dpl_size(list)-1;
dplist_node_t * node = list->head->next;
    while(node!=NULL){
        if(reference==node)return re;
        re++;
        if(node->next==NULL)break;
        node=node->next;
    }
    return -1;
}

// ---- extra insert & remove operators ----//



dplist_t * dpl_insert_sorted( dplist_t * list, void * element, bool insert_copy )
{
    dpl_insert_at_reference(  list,  element, NULL,   insert_copy );
    return sort(list);
}

dplist_t * dpl_remove_at_reference( dplist_t * list, dplist_node_t * reference, bool free_element )
{//printf("dpl_remove_at_reference( dplist_t * list, dplist_node_t * reference, bool free_element )\n");
    return freenode(list,reference,free_element);
}

dplist_t * dpl_remove_element( dplist_t * list, void * element, bool free_element )
{
    dplist_node_t * node=dpl_get_reference_of_element( list,  element );
    if(node==NULL)return list;
    return freenode(list,node,free_element);
}
// ---- you can add your extra operators here ----//
dplist_t * freenode(  dplist_t *  list, dplist_node_t * node, bool  free_element){
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
    int index=dpl_get_index_of_reference(   list, node);
    dplist_node_t * tnode ;
    if(index==-1)return list;
    tnode= dpl_get_reference_at_index(  list,   index );
    if(free_element){list->element_free(&(tnode->element));}
    if(dpl_size(list)==0)return list;

    if(tnode->prev==NULL){
        if(tnode->next!=NULL)
        {list->head->next=tnode->next;
            tnode->next->prev=NULL;
        }
        else {list->head->next=NULL;  }
    }
    else if(tnode->next!=NULL){
        tnode->next->prev=tnode->prev;
        tnode->prev->next=tnode->next;
    }
    else tnode->prev->next=NULL;
    free(tnode);
    return list;
}
dplist_t * copy( dplist_t * list,  dplist_node_t* cp, void * element,bool  insert_copy){
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
    if(insert_copy){
        cp->element=list->element_copy(element);
    }
    else{
        cp->element=element;
    }
    return list;
}
dplist_t *   sort( dplist_t * list){
    DPLIST_ERR_HANDLER(list==NULL,DPLIST_INVALID_ERROR);
    int i,j,size=dpl_size(list);
    void * ele,*ele1,*ele2;
    if(size<=1)return list;
    for(i=size-1;i>=1;i--){
        for(j=0;j<=i-1;j++){
            ele1=dpl_get_reference_at_index(  list,j)->element;
            ele2=dpl_get_reference_at_index(  list,j+1)->element;
            if(list->element_compare( ele1,ele2  )==1){
                ele=ele1;
                dpl_get_reference_at_index(  list,j)->element=ele2;
                dpl_get_reference_at_index(  list,j+1)->element=ele;
            }
        }
    }
    return list;
}


