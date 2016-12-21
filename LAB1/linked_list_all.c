#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
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



struct node {
   int    data;
   struct node* next;
};

struct      node* head = NULL;
int         thread_count;
int         samples = 10 ;
int         nInsertion = 1000;
int         mOperations = 10000;
double      mInsertPercent = 25;
double      mMemberPercent = 50;
double      mDeletePercent = 25;

unsigned    seed = 1;
int         member_total=0;
int         insert_total=0;
int         delete_total=0;
int         thread_counts[3] = {1,2,4};

pthread_mutex_t   mutex;
pthread_mutex_t   count_operation_onelist_mutex;

pthread_rwlock_t    rwlock;
pthread_mutex_t     count_operation_rwlock_mutex;



int  insert(int value);
void print();
int  member(int value);
int  delete(int value);
void free_list();
int  is_empty();

unsigned randomInteger(unsigned* a_p);
double randomDouble(unsigned* a_p);

void makeOperationZero();

double runSerialVersionList();
double runOneMutexPerList();
double runReadWriteLockList();

double Results[3][3][6];


void showResults();
void populate();
void calculateTime(short Case, short row, short col);
void calculateTimeSingle();
void* Thread_work_One_Mutex(void* rank);
void* Thread_work_RW_Lock(void* rank);

int main(int argc, char **argv) {

    if (argc == 2){
      samples = strtol(argv[1],NULL,10);

    }
    else if(argc ==1){
      printf("Enter nInsertion: ");
      scanf("%d",&nInsertion);

      printf("Enter mOperations: ");
      scanf("%d",&mOperations);

      printf("Enter mMember Fraction(E.g:0.5): ");
      scanf("%lf",&mMemberPercent);

      printf("Enter mInsert Fraction(E.g:0.5): ");
      scanf("%lf",&mInsertPercent);

      printf("Enter mDelete Fraction(E.g:0.5): ");
      scanf("%lf",&mDeletePercent);

      printf("Enter thread count: ");
      scanf("%d",&thread_count);

      calculateTimeSingle();
      exit(0);
    }
    else{
      fprintf(stderr, "usage: %s <samples> to run predefined values\n", argv[0]);
//      fprintf(stderr, "usage: %s <threads> <n> <m> <mMember> <mInsert> to run specified values (mDelete will be caluclated)\n", argv[0]);

      exit(0);
    }





    double cases[3][3] = {
        {99, 0.5,0.5},
        {90,5,5},
        {50, 25, 25}
    };

    short Case;
    short thread;
    double start, finish;

    printf("Running....\n");
    GET_TIME(start);

    for(Case = 1; Case<=3; Case++){

      mMemberPercent = cases[Case-1][0];
      mInsertPercent =  cases[Case-1][1];
      mDeletePercent =  cases[Case-1][2];

        for(thread =0; thread<3; thread++){
          thread_count = thread_counts[thread];
            calculateTime(Case-1, 2*thread, 2*thread+1);

        }
    }

    GET_TIME(finish);

    printf("Total Time Elapsed %f\n\n", (finish-start) );
    showResults();
   return 0;
}

void showResults(){
int i,j,k;
  for(i=0; i<3; i++){


    if(i==0){
      printf("Case 1    n = 1,000 and m = 10,000, mMember = 0.99, mInsert = 0.005, mDelete = 0.005\n");
    }
    else if(i==1){
      printf("Case 2    n = 1,000 and m = 10,000, mMember = 0.90, mInsert = 0.05, mDelete = 0.05\n");
    }
    else if(i==2){
      printf("Case 3    n = 1,000 and m = 10,000, mMember = 0.50, mInsert = 0.25, mDelete = 0.25\n");
    }



    printf("\t\tAverage(%d)\tSD (%d)\t\tAverage(%d)\tSD (%d)\t\tAverage(%d)\tSD (%d)\n",thread_counts[0],thread_counts[0],thread_counts[1],thread_counts[1],thread_counts[2],thread_counts[2] );

      for(j=0; j<3; j++){
        if(j==0){
          printf("Serial\t\t");
        }
        if(j==1){
          printf("Mutex\t\t");
        }
        if(j==2){
          printf("RW Lock\t\t");
        }
          for(k=0; k<6; k++){

            if(j==0 && k>1){
              printf("-\t\t-\t\t-\t\t-");
              break;
            }
              printf("%f\t", Results[i][j][k]);

          }
          printf("\n");
      }
      printf("\n\n");

  }
}

void calculateTime(short Case, short col1, short col2){
  double elapsed[3][samples];
  double average[3] = {0,0,0};
  double variance[3] = {0,0,0};

  int i,j;
  clock_t begin;
  clock_t end;

  for(i=0; i<samples; i++){
    // begin = clock();

    elapsed[0][i] = runSerialVersionList();
    // end = clock();
    // elapsed[0][i] = (double)(end - begin) / CLOCKS_PER_SEC;
    average[0]+=elapsed[0][i];
  }

  for(i=0; i<samples; i++){

    elapsed[1][i] = runOneMutexPerList();
    average[1]+=elapsed[1][i];
  }

  for(i=0; i<samples; i++){
    elapsed[2][i] = runReadWriteLockList();
    average[2]+=elapsed[2][i];
  }


  for(j=0; j<3; j++){
    average[j] = average[j]/samples;
    //printf("%f\t", average[j] );

    Results[Case][j][col1] = average[j];

    for(i=0; i<samples; i++){
      variance[j] += pow((elapsed[j][i] - average[j]), 2);
    }
    variance[j] = variance[j]/samples;
        //printf("%f\t", sqrt(variance[j]));
      Results[Case][j][col2] = sqrt(variance[j]);

      //printf("sample needed %f\n", pow(((100 * Results[Case][j][col2] * 1.96)/(5* Results[Case][j][col1])),2));

  }
//  printf("\n" );
}


void calculateTimeSingle(){
  double elapsed[3];


  int i,j;
  clock_t begin;
  clock_t end;


    // begin = clock();
  printf("\nCase special  threads = %d   n = %d and m = %d, mMember = %f, mInsert = %f, mDelete = %f\n",thread_count,
   nInsertion, mOperations, mMemberPercent, mInsertPercent, mDeletePercent);
    elapsed[0] = runSerialVersionList();
    elapsed[1] = runOneMutexPerList();


    elapsed[2] = runReadWriteLockList();

    printf("Elapese Time for Serial : %f seconds\n", elapsed[0] );
    printf("Elapese Time for OneMutex : %f seconds\n", elapsed[1] );
    printf("Elapese Time for Readwrite Lock : %f seconds\n\n", elapsed[2] );


//  printf("\n" );
}



double runSerialVersionList(){


  int i, val;
  double operation_type;

  double start, finish;
  int members= 0;//mOperations * (mMemberPercent/100);
  int insertions= 0;//mOperations * (mInsertPercent/100);
  int deletions= 0;//mOperations * (mDeletePercent/100);

  populate();
  makeOperationZero();

  GET_TIME(start);
  for (i = 0; i < mOperations; i++) {
      operation_type = randomDouble(&seed);
      val = randomInteger(&seed) % MAX_KEY;

      if (operation_type < mMemberPercent) {

         member(val);
         members++;

      } else if (operation_type < mMemberPercent + mInsertPercent) {

         insert(val);
        insertions++;

      } else { /* delete */

         delete(val);
        deletions++;

      }
   }  /* for */


    GET_TIME(finish);
    member_total = members;
    insert_total = insertions;
    delete_total = deletions;

    free_list();

    //printf("Elapsed time Serial = %e seconds\n", finish - start);
    // printf("Total ops = %d\n", mOperations);
    // printf("member ops = %d\n", member_total);
    // printf("insert ops = %d\n", insert_total);
    // printf("delete ops = %d\n", delete_total);
    double elapsed = finish - start;

    return elapsed;


}
double runOneMutexPerList(){
  long i;
  pthread_t* thread_handles;
  double start, finish;

  populate();

#  ifdef OUTPUT
   printf("Before starting threads, list = \n");
   print();
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

   //printf("Elapsed time OneMutex (threads %d)= %e seconds\n", thread_count, finish - start);
  //  printf("Total ops = %d\n", mOperations);
  //  printf("member ops = %d\n", member_total);
  //  printf("insert ops = %d\n", insert_total);
  //  printf("delete ops = %d\n", delete_total);

#  ifdef OUTPUT
   printf("After threads terminate, list = \n");
   print();
   printf("\n");
#  endif

   free_list();
   pthread_mutex_destroy(&mutex);
   pthread_mutex_destroy(&count_operation_onelist_mutex);
   free(thread_handles);

   double elapsed = finish - start;

   return elapsed;

}
double runReadWriteLockList(){
  long i;
  pthread_t* thread_handles;
  double start, finish;
  populate();

#  ifdef OUTPUT
  printf("Before starting threads, list = \n");
  print();
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

  //printf("Elapsed time ReadWriteLock (threads %d)= %e seconds\n", thread_count, finish - start);
  // printf("Total ops = %d\n", mOperations);
  // printf("member ops = %d\n", member_total);
  // printf("insert ops = %d\n", insert_total);
  // printf("delete ops = %d\n", delete_total);

#  ifdef OUTPUT
  printf("After threads terminate, list = \n");
  print();
  printf("\n");
#  endif

  free_list();
  pthread_rwlock_destroy(&rwlock);
  pthread_mutex_destroy(&count_operation_rwlock_mutex);
  free(thread_handles);
  double elapsed = finish - start;

  return elapsed;
}

void populate(){
  long i;
  int key, success, attempts;

  i = attempts = 0;
  while ( i < nInsertion && attempts < 2*nInsertion ) {
     key = randomInteger(&seed) % MAX_KEY;
     success = insert(key);
     attempts++;
     if (success) i++;

  }
//  printf("Inserted %ld keys in empty list\n", i);

}


void print() {
   struct node* curr_p;

   printf("list = ");

   curr_p = head;
   while (curr_p != (struct node*) NULL) {
      printf("%d ", curr_p->data);
      curr_p = curr_p->next;
   }
   printf("\n");
}


int  member(int value) {
   struct node* curr_p;

   curr_p = head;
   while (curr_p != NULL && curr_p->data < value)
      curr_p = curr_p->next;

   if (curr_p == NULL || curr_p->data > value) {

      return 0;
   } else {

      return 1;
   }
}  /* Member */


int insert(int value) {
   struct node* curr = head;
   struct node* pred = NULL;
   struct node* temp;
   int rv = 1;

   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }

   if (curr == NULL || curr->data > value) {
      temp = malloc(sizeof(struct node));
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

int delete(int value) {
   struct node* curr_p = head;
   struct node* pred_p = NULL;

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
void free_list() {
   struct node* curr_p;
   struct node* succ_p;

   if (is_empty()) return;
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


int  is_empty() {
   if (head == NULL)
      return 1;
   else
      return 0;
}


void* Thread_work_One_Mutex(void* rank) {
   long current_rank = (long) rank;
   int i, val;
   double operation_type;
   unsigned seed = current_rank + 1;
   int members=0, insertions=0, deletions=0;
   int ops_per_thread = mOperations/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      operation_type = randomDouble(&seed);
      val = randomInteger(&seed) % MAX_KEY;
      if (operation_type < mMemberPercent) {
         pthread_mutex_lock(&mutex);
         member(val);
         pthread_mutex_unlock(&mutex);
         members++;
      } else if (operation_type < mMemberPercent + mInsertPercent) {
         pthread_mutex_lock(&mutex);
         insert(val);
         pthread_mutex_unlock(&mutex);
         insertions++;
      } else { /* delete */
         pthread_mutex_lock(&mutex);
         delete(val);
         pthread_mutex_unlock(&mutex);
         deletions++;
      }
   }  /* for */

   pthread_mutex_lock(&count_operation_onelist_mutex);
   member_total += members;
   insert_total += insertions;
   delete_total += deletions;
   pthread_mutex_unlock(&count_operation_onelist_mutex);

   return NULL;
}



void* Thread_work_RW_Lock(void* rank) {
  long current_rank = (long) rank;
  int i, val;
  double operation_type;
  unsigned seed = current_rank + 1;
  int members=0, insertions=0, deletions=0;
  int ops_per_thread = mOperations/thread_count;

   for (i = 0; i < ops_per_thread; i++) {
      operation_type = randomDouble(&seed);
      val = randomInteger(&seed) % MAX_KEY;
      if (operation_type < mMemberPercent) {
         pthread_rwlock_rdlock(&rwlock);
         member(val);
         pthread_rwlock_unlock(&rwlock);
         members++;
      } else if (operation_type < mMemberPercent + mInsertPercent) {
         pthread_rwlock_wrlock(&rwlock);
         insert(val);
         pthread_rwlock_unlock(&rwlock);
         insertions++;
      } else { /* delete */
         pthread_rwlock_wrlock(&rwlock);
         delete(val);
         pthread_rwlock_unlock(&rwlock);
         deletions++;
      }
   }  /* for */

   pthread_mutex_lock(&count_operation_rwlock_mutex);
   member_total += members;
   insert_total += insertions;
   delete_total += deletions;
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
