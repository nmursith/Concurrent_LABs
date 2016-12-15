#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

struct node **root;
int threadCount =4;
int maxValue = 65535;
int n =1000;
int m = 10000;
double mMember=0.99;
double mInsert=0.005;
double mDelete=0.005;
pthread_rwlock_t rwlock;
pthread_mutex_t lock;
int distance;
int samples = 25;

struct node {
  int value;
  struct node *next;
};

int insert(int value,struct node **root){
    struct node* curr = *root;
    struct node* prev = NULL;
    struct node* temp;

    while(curr!=NULL && curr-> value < value){
        prev=curr;
        curr=curr->next;
    }

    if(curr==NULL || curr->value > value){
        temp = malloc(sizeof(struct node));
        temp->value = value;
        temp->next = curr;

        if(prev==NULL){
            *root=temp;
        }
        else{
            prev->next=temp;
        }
        return 1;
    }else{
        return 0;
    }

}


int count(struct node **root){
    struct node* curr = *root;
    struct node* prev = NULL;
    struct node* temp;
    int i=0;
    while(curr!=NULL){
        prev=curr;
        curr=curr->next;
        i++;
    }
    return i;



}


int generateList(int n, struct node **root,int maxNumber){
  int result = 0;
  *root = malloc( sizeof(struct node) );
  (*root)->value = rand() % maxNumber+1;

  for (int i = 0; i < n; ++i)
  {
    result = insert(rand() % maxNumber+1,root);
    if (result == 0)
    {
      i--;
    }
  }
  return result;
}

int member(int value, struct node **root){
  struct node* curr = *root;

  while(curr!=NULL){
      if (curr->value == value)
      {
        return 1;
      }
      curr = curr->next;
  }
   return 0; //item not found

}

int delete(int value, struct node **root){
  struct node* curr = *root;
  struct node* prev = NULL;

  while(curr!=NULL){
    if (curr->value == value)
    {
      if (prev!=NULL && curr->next!=NULL )
      {
        prev->next = curr->next;
      }else if(curr->next!=NULL){
        *root = curr->next;
      }else{
        *root = NULL;
      }
      return 1;
    }
    prev = curr;
    curr = curr->next;
  }
  return 0;
}

double calculateSD(double timespent[])
{
    double sum = 0.0, mean, standardDeviation = 0.0;

    int i;

    for(i=0; i<10; ++i)
    {
        sum += timespent[i];
    }

    mean = sum/10;

    for(i=0; i<10; ++i)
        standardDeviation += pow(timespent[i] - mean, 2);

    return sqrt(standardDeviation/10);
}

double calculateSum(double timespent[]){
    double sum = 0.0;
    for (int i = 0; i < samples; ++i)
    {
      sum += timespent[i];
    }

    return sum;
}

void operations(){
    int memberOperationCounter = 0;
    int insertOperationCounter = 0;
    int deleteOperationCounter = 0;
    int count=0;

    while(memberOperationCounter<mMember || insertOperationCounter<mInsert || deleteOperationCounter<mDelete){
          count++;
          if (memberOperationCounter<mMember)
          {
            memberOperationCounter++;
            member(rand() % maxValue+1,root);
          }

          if (insertOperationCounter<mInsert)
          {
            insertOperationCounter++;
            insert(rand() % maxValue+1,root);
          }

          if (deleteOperationCounter<mDelete)
          {
            deleteOperationCounter++;
            delete(rand() % maxValue+1,root);
          }
    }
}


void *operationsMutex(void* rank){
    int memberOperationCounter = 0;
    int insertOperationCounter = 0;
    int deleteOperationCounter = 0;
    int max_mMember = mMember / threadCount;
    int max_mInsert = mInsert / threadCount;
    int max_mDelete = mDelete / threadCount;
    int count=0;

    while(memberOperationCounter<max_mMember || insertOperationCounter<max_mInsert || deleteOperationCounter<max_mDelete){
          count++;
          if (memberOperationCounter<max_mMember)
          {
            memberOperationCounter++;
            pthread_mutex_lock(&lock);
            member(rand() % maxValue+1,root);
            pthread_mutex_unlock(&lock);
          }

          if (insertOperationCounter<max_mInsert)
          {
            insertOperationCounter++;
            pthread_mutex_lock(&lock);
            insert(rand() % maxValue+1,root);
            pthread_mutex_unlock(&lock);
          }

          if (deleteOperationCounter<max_mDelete)
          {
            deleteOperationCounter++;
            pthread_mutex_lock(&lock);
            delete(rand() % maxValue+1,root);
            pthread_mutex_unlock(&lock);
          }
    }
    return NULL;
}

void *operationsRW(void* rank){
    int memberOperationCounter = 0;
    int insertOperationCounter = 0;
    int deleteOperationCounter = 0;
    int max_mMember = mMember / threadCount;
    int max_mInsert = mInsert / threadCount;
    int max_mDelete = mDelete / threadCount;
    int count=0;

    while(memberOperationCounter<max_mMember || insertOperationCounter<max_mInsert || deleteOperationCounter<max_mDelete){
          count++;
          if (memberOperationCounter<max_mMember)
          {
            memberOperationCounter++;

            pthread_rwlock_rdlock(&rwlock);
            member(rand() % maxValue+1,root);
            pthread_rwlock_unlock(&rwlock);
          }

          if (insertOperationCounter<max_mInsert)
          {
            insertOperationCounter++;

            pthread_rwlock_wrlock(&rwlock);
            insert(rand() % maxValue+1,root);
            pthread_rwlock_unlock(&rwlock);

          }

          if (deleteOperationCounter<max_mDelete)
          {
            deleteOperationCounter++;
            pthread_rwlock_wrlock(&rwlock);
            delete(rand() % maxValue+1,root);
            pthread_rwlock_unlock(&rwlock);
          }
    }
    return NULL;
}

int main()
{

    printf("No of samples: ");
    scanf("%d",&samples);
    printf("Enter n: ");
    scanf("%d",&n);
    printf("Enter m: ");
    scanf("%d",&m);
    printf("Enter member_fraction: ");
    scanf("%lf",&mMember);
    printf("Enter insert_fraction: ");
    scanf("%lf",&mInsert);
    printf("Enter delete_fraction: ");
    scanf("%lf",&mDelete);
    printf("Enter thread count: ");
    scanf("%d",&threadCount);

    srand(time(NULL));
    clock_t begin;
    clock_t end;
    root = malloc( sizeof(struct node) );
    mMember = m * mMember;
    mInsert = m * mInsert;
    mDelete = m * mDelete;
    long thread;
    pthread_t* thread_handles;
    double timespent[samples];

    printf("========================================================================================\n");
    printf("A linked list has been generated with %d elements.\n",n);

    //Serial
    for (int i = 0; i < samples; ++i)
    {
      generateList(n,root,maxValue);
      printf("Number of nodes %d\n", count(root));
      begin = clock();
      operations();
      end = clock();
      timespent[i] = (double)(end - begin) / CLOCKS_PER_SEC;
    }

    printf("========================================================================================\n");
    printf("Serial Programme : Average time spent : %f seconds\n",calculateSum(timespent)/samples );
    printf("Serial Programme : Standard deviation : %f seconds\n",calculateSD(timespent));



    //parallel with mutex
    for (int i = 0; i < samples; ++i)
    {
       generateList(n,root,maxValue);
       printf("Number of nodes %d\n", count(root));
      thread_handles = malloc(threadCount*sizeof(pthread_t));
      if (pthread_mutex_init(&lock, NULL) != 0)
      {
        printf("\n mutex init failed\n");
        return 1;
      }
      begin = clock();
      for (thread = 0; thread < threadCount; ++thread)
      {
        pthread_create(&thread_handles[thread],NULL,operationsMutex,(void*) thread);
      }

      for (thread = 0; thread < threadCount; ++thread)
      {
        pthread_join(thread_handles[thread],NULL);
      }
      end = clock();
      timespent[i] = (double)(end - begin) / CLOCKS_PER_SEC;
      free(thread_handles);
      pthread_mutex_destroy(&lock);
    }

    printf("========================================================================================\n");
    printf("Parallel Programme with mutex : Average time spent : %f seconds\n",calculateSum(timespent)/samples );
    printf("Parallel Programme with mutex : Standard deviation : %f seconds\n",calculateSD(timespent));

    //parallel woth read write lock
    for (int i = 0; i < samples; ++i)
    {
      generateList(n,root,maxValue);
      printf("Number of nodes %d\n", count(root));
      thread_handles = malloc(threadCount*sizeof(pthread_t));
      if (pthread_rwlock_init(&rwlock, NULL) != 0)
      {
        printf("\n mutex init failed\n");
        return 1;
      }
      begin = clock();
      for (thread = 0; thread < threadCount; ++thread)
      {
        pthread_create(&thread_handles[thread],NULL,operationsRW,(void*) thread);
      }

      for (thread = 0; thread < threadCount; ++thread)
      {
        pthread_join(thread_handles[thread],NULL);
      }
      end = clock();
      timespent[i] = (double)(end - begin) / CLOCKS_PER_SEC;
      free(thread_handles);
      pthread_rwlock_destroy(&rwlock);
    }

    printf("========================================================================================\n");
    printf("Parallel Programme with RW lock: Average time spent : %f seconds\n",calculateSum(timespent)/samples );
    printf("Parallel Programme with RW lock: Standard deviation : %f seconds\n",calculateSD(timespent));
    
    return 0;
}
