#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <sys/time.h>

#define MIN_KEY 0
#define MAX_KEY 65535

#define MULTIPLIER 279470273
#define MODULUS 4294967291U
#define DIVISOR ((double) 4294967291U)

#define GET_TIME(now) { \
   struct timeval t; \
   gettimeofday(&t, NULL); \
   now = t.tv_sec + t.tv_usec/1000000.0; \
}



struct list_node_s {
   int    data;
   struct list_node_s* next;
};

struct      list_node_s* head = NULL;
int         thread_count;

int         nInsertion = 100;
int         mOperations = 1000;
double      mInsertPercent = 50;
double      mMemberPercent = 25;
double      mDeletePercent = 25;

unsigned    seed = 1;
int         member_total=0;
int         insert_total=0;
int         delete_total=0;

pthread_mutex_t mutex;
pthread_mutex_t count_operation_onelist_mutex;

pthread_rwlock_t    rwlock;
pthread_mutex_t     count_operation_rwlock_mutex;



int  Insert(int value);
void Print();
int  Member(int value);
int  Delete(int value);
void Free_list();
int  Is_empty();
int  Get_Number();
unsigned randomInteger(unsigned* a_p);
double randomDouble(unsigned* a_p);
void Usage(char* prog_name);
void makeOperationZero();

void runSerialVersionList();
void runOneMutexPerList();
void runReadWriteLockList();
void populate();

void* Thread_work_One_Mutex(void* rank);
void* Thread_work_RW_Lock(void* rank);

int main(int argc, char **argv) {

  if(argc==2){

      thread_count = Get_Number(argv[1]);
    }
    else{
      printf("Number of threads required\n");
      //return 0;

    }

    if (argc != 2) Usage(argv[0]);
    thread_count = strtol(argv[1],NULL,10);

    runOneMutexPerList();
    runSerialVersionList();

    runReadWriteLockList();


   return 0;
}

void runSerialVersionList(){


  int i, val;
  double operation_type;

  double start, finish;
  int member= 0;//mOperations * (mMemberPercent/100);
  int insert= 0;//mOperations * (mInsertPercent/100);
  int delete= 0;//mOperations * (mDeletePercent/100);

  populate();
  makeOperationZero();

  GET_TIME(start);
  for (i = 0; i < mOperations; i++) {
      operation_type = randomDouble(&seed);
      val = randomInteger(&seed) % MAX_KEY;

      if (operation_type < mMemberPercent) {

         Member(val);
         member++;

      } else if (operation_type < mMemberPercent + mInsertPercent) {

         Insert(val);
        insert++;

      } else { /* delete */

         Delete(val);
        delete++;

      }
   }  /* for */
    GET_TIME(finish);
    member_total = member;
    insert_total = insert;
    delete_total = delete;

    printf("Elapsed time = %e seconds\n", finish - start);
    printf("Total ops = %d\n", mOperations);
    printf("member ops = %d\n", member_total);
    printf("insert ops = %d\n", insert_total);
    printf("delete ops = %d\n", delete_total);




}
void runOneMutexPerList(){
  int i;
  pthread_t* thread_handles;
  double start, finish;

  populate();

#  ifdef OUTPUT
   printf("Before starting threads, list = \n");
   Print();
   printf("\n");
#  endif

  makeOperationZero();

   thread_handles = malloc(thread_count*sizeof(pthread_t));
   pthread_mutex_init(&mutex, NULL);
   pthread_mutex_init(&count_operation_onelist_mutex, NULL);

   GET_TIME(start);
   for (i = 0; i < thread_count; i++)
      pthread_create(&thread_handles[i], NULL, Thread_work_One_Mutex, (void*) i);

   for (i = 0; i < thread_count; i++)
      pthread_join(thread_handles[i], NULL);
   GET_TIME(finish);
   printf("Elapsed time = %e seconds\n", finish - start);
   printf("Total ops = %d\n", mOperations);
   printf("member ops = %d\n", member_total);
   printf("insert ops = %d\n", insert_total);
   printf("delete ops = %d\n", delete_total);

#  ifdef OUTPUT
   printf("After threads terminate, list = \n");
   Print();
   printf("\n");
#  endif

   Free_list();
   pthread_mutex_destroy(&mutex);
   pthread_mutex_destroy(&count_operation_onelist_mutex);
   free(thread_handles);

}
void runReadWriteLockList(){
  int i;
  pthread_t* thread_handles;
  double start, finish;
  populate();

#  ifdef OUTPUT
  printf("Before starting threads, list = \n");
  Print();
  printf("\n");
#  endif

  makeOperationZero();

    thread_handles = malloc(thread_count*sizeof(pthread_t));
  pthread_mutex_init(&count_operation_rwlock_mutex, NULL);
  pthread_rwlock_init(&rwlock, NULL);

  GET_TIME(start);
  for (i = 0; i < thread_count; i++)
     pthread_create(&thread_handles[i], NULL, Thread_work_RW_Lock, (void*) i);

  for (i = 0; i < thread_count; i++)
     pthread_join(thread_handles[i], NULL);
  GET_TIME(finish);
  printf("Elapsed time = %e seconds\n", finish - start);
  printf("Total ops = %d\n", mOperations);
  printf("member ops = %d\n", member_total);
  printf("insert ops = %d\n", insert_total);
  printf("delete ops = %d\n", delete_total);

#  ifdef OUTPUT
  printf("After threads terminate, list = \n");
  Print();
  printf("\n");
#  endif

  Free_list();
  pthread_rwlock_destroy(&rwlock);
  pthread_mutex_destroy(&count_operation_rwlock_mutex);
  free(thread_handles);

}

void populate(){
  long i;
  int key, success, attempts;

  i = attempts = 0;
  while ( i < nInsertion && attempts < 2*nInsertion ) {
     key = randomInteger(&seed) % MAX_KEY;
     success = Insert(key);
     attempts++;
     if (success) i++;

  }
  printf("Inserted %ld keys in empty list\n", i);

}

void Usage(char* prog_name) {
   fprintf(stderr, "usage: %s <thread_count>\n", prog_name);
   exit(0);
}
void Print() {
   struct list_node_s* curr_p;

   printf("list = ");

   curr_p = head;
   while (curr_p != (struct list_node_s*) NULL) {
      printf("%d ", curr_p->data);
      curr_p = curr_p->next;
   }
   printf("\n");
}


int  Member(int value) {
   struct list_node_s* curr_p;

   curr_p = head;
   while (curr_p != NULL && curr_p->data < value)
      curr_p = curr_p->next;

   if (curr_p == NULL || curr_p->data > value) {

      return 0;
   } else {

      return 1;
   }
}  /* Member */


int Insert(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   struct list_node_s* temp;
   int rv = 1;

   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }

   if (curr == NULL || curr->data > value) {
      temp = malloc(sizeof(struct list_node_s));
      temp->data = value;
      temp->next = curr;
      if (pred == NULL)
         head = temp;
      else
         pred->next = temp;
   } else { /* value in list */
      rv = 0;
   }

   return rv;
}

int Delete(int value) {
   struct list_node_s* curr_p = head;
   struct list_node_s* pred_p = NULL;

   /* Find value */
   while (curr_p != NULL && curr_p->data < value) {
      pred_p = curr_p;
      curr_p = curr_p->next;
   }

   if (curr_p != NULL && curr_p->data == value) {
      if (pred_p == NULL) { /* first element in list */
         head = curr_p->next;

//         printf("Freeing %d\n", value);

         free(curr_p);
      } else {
         pred_p->next = curr_p->next;

         //printf("Freeing %d\n", value);

         free(curr_p);
      }
      return 1;
   } else {

      return 0;
   }
}
void Free_list() {
   struct list_node_s* curr_p;
   struct list_node_s* succ_p;

   if (Is_empty()) return;
   curr_p = head;
   succ_p = curr_p->next;
   while (succ_p != NULL) {

      free(curr_p);
      curr_p = succ_p;
      succ_p = curr_p->next;
   }


   free(curr_p);
   head = NULL;
}


int  Is_empty() {
   if (head == NULL)
      return 1;
   else
      return 0;
}



int Get_Number(char *arg){
  char *p;
  int num;

  errno = 0;
  long conv = strtol(arg, &p, 10);

  // Check for errors: e.g., the string does not represent an integer
  // or the integer is larger than int
  if (errno != 0 || *p != '\0' || conv > INT_MAX) {
      // Put here the handling of the error, like exiting the program with
      // an error message
  } else {
      // No error
      num = conv;

  }
  return num;
}

void* Thread_work_One_Mutex(void* rank) {
   long current_rank = (long) rank;
   int i, val;
   double operation_type;
   unsigned seed = current_rank + 1;
   int member=0, insert=0, delete=0;
   int ops_per_thread = mOperations/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      operation_type = randomDouble(&seed);
      val = randomInteger(&seed) % MAX_KEY;
      if (operation_type < mMemberPercent) {
         pthread_mutex_lock(&mutex);
         Member(val);
         pthread_mutex_unlock(&mutex);
         member++;
      } else if (operation_type < mMemberPercent + mInsertPercent) {
         pthread_mutex_lock(&mutex);
         Insert(val);
         pthread_mutex_unlock(&mutex);
         insert++;
      } else { /* delete */
         pthread_mutex_lock(&mutex);
         Delete(val);
         pthread_mutex_unlock(&mutex);
         delete++;
      }
   }  /* for */

   pthread_mutex_lock(&count_operation_onelist_mutex);
   member_total += member;
   insert_total += insert;
   delete_total += delete;
   pthread_mutex_unlock(&count_operation_onelist_mutex);

   return NULL;
}



void* Thread_work_RW_Lock(void* rank) {
  long current_rank = (long) rank;
  int i, val;
  double operation_type;
  unsigned seed = current_rank + 1;
  int member=0, insert=0, delete=0;
  int ops_per_thread = mOperations/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      operation_type = randomDouble(&seed);
      val = randomInteger(&seed) % MAX_KEY;
      if (operation_type < mMemberPercent) {
         pthread_rwlock_rdlock(&rwlock);
         Member(val);
         pthread_rwlock_unlock(&rwlock);
         member++;
      } else if (operation_type < mMemberPercent + mInsertPercent) {
         pthread_rwlock_wrlock(&rwlock);
         Insert(val);
         pthread_rwlock_unlock(&rwlock);
         insert++;
      } else { /* delete */
         pthread_rwlock_wrlock(&rwlock);
         Delete(val);
         pthread_rwlock_unlock(&rwlock);
         delete++;
      }
   }  /* for */

   pthread_mutex_lock(&count_operation_rwlock_mutex);
   member_total += member;
   insert_total += insert;
   delete_total += delete;
   pthread_mutex_unlock(&count_operation_rwlock_mutex);

   return NULL;
}

void makeOperationZero(){
  member_total = 0;
  insert_total = 0;
  delete_total = 0;
}

unsigned randomInteger(unsigned* seed_p) {

   long long z = *seed_p;
   z *= MULTIPLIER;
   z %= MODULUS;
   *seed_p = z;

   return *seed_p;
}


double randomDouble(unsigned* seed_p) {
   unsigned x = randomInteger(seed_p);
   double y = (x/DIVISOR) *100;
   return y;
}
