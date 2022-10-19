#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <ctype.h>



struct process {
    char name[10];
    int givenRuntime;
    int currentRuntime;
    int remainingRunTime;
    int cpuCount;
    int ioCount;
    int ioTime;
    int totalIOTime;
    int finishTime;
    int ioBlockTime;
    int remainQuantum;
    int whenItmoved;
    double blockProb;
    struct process* next;
};

struct cpu {
    int status;
    int dispatchCount;
    int idleTime;
    int busyTime;
};

struct io {
    int status;
    int dispatchCount;
    int idleTime;
    int busyTime;
};
struct queue{
    struct process *front;
    struct process *rear;
};

int nproc = 0;

typedef struct process process;
typedef struct cpu CPU;
typedef struct io IO;
typedef struct queue queue;

struct process* newProcess(char *str, char*filename, int num, double prob, int lno);
struct queue* createQueue();
void addToQueue(queue *q, process *p);
struct process * removeFromQueue(queue *q);
void initiate(CPU *c, IO *i);
void run(queue *cpuQ,queue *ioQ, int schedulePolicy);

int main(int argc, char* argv[]){

  if (argc != 3) {
    fprintf(stderr, "Usage: ./prsim [-r | -f] file\n");
    exit(1);
  }
    
    srandom(12345);
    int schedulePolicy = 0;
    char * schPolicy = strdup(argv[1]);
    if (strcmp(argv[1], "-f") != 0 && strcmp(argv[1], "-r") != 0) {
          fprintf(stderr, "Usage: ./prsim [-r | -f] file\n");
          exit(1);
    }
    if(schPolicy[1] == 'r'){
        schedulePolicy = 1;
       /* printf("doing r\n");*/
    }
    if(schPolicy[1] == 'f'){
        schedulePolicy = 2;
        /*printf("doing f\n");*/
    }

    
    char * fileName = strdup(argv[2]);
    FILE* ptr = fopen(argv[2], "r"); 
    if (ptr == NULL) {
        perror("file does not exist\n"); 
        exit(1);
        return 0;
    }
    struct queue *cpuQ = createQueue();
    struct queue *ioQ = createQueue();
    nproc = 0;
    char * str;
    str = malloc(sizeof(char)* 30);
    int num;
    double  prob;
    int i = 0;
    while(fscanf(ptr, "%s %d %lf",str ,&num, &prob) != EOF){
      nproc++;  
      struct process * temp = newProcess(str, argv[2], num, prob, nproc-1);
        addToQueue(cpuQ, temp);
    }
    puts("Processes:\n");
    puts("   name     CPU time  when done  cpu disp  i/o disp  i/o time");
    run(cpuQ,ioQ,schedulePolicy);
    return 0;
}

void initiate(CPU *c, IO *i){
    c->status = 0;
    c->dispatchCount = 0;
    c->idleTime = 0;
    c->busyTime = 0;
    i->status = 0;
    i->dispatchCount = 0;
    i->idleTime = 0;
    i->busyTime = 0;
}

void run(queue *cpuQ,queue *ioQ, int scheduleAlg){
    struct cpu cpuStats;
    struct io ioStats;
    struct process * cpuP = NULL;
    struct process * ioP = NULL;
    initiate(&cpuStats, &ioStats);
    int worldClock = 0;
    /*run till all queue and process pointer to be NULL*/
    while((ioQ->front != NULL) ||(cpuQ->front != NULL)||(cpuP != NULL)||(ioP != NULL)){
        worldClock = worldClock + 1;
        /*CPU part*/
        /*if both cpu process pointer and cpuQ is both empty, increase idle timer*/
        if((cpuQ->front == NULL)&&( cpuP == NULL)){
            cpuStats.idleTime =cpuStats.idleTime+1;
        }
        /*if not, run all the below process*/
        else{
            if((cpuP == NULL)&&(cpuQ->front != NULL)){
                int remainTime;
                /*move process from queue to cpu*/
                cpuP = removeFromQueue(cpuQ);
                // printf("%d %s come in cpu \n", worldClock, cpuP->name);
                cpuP->cpuCount = cpuP->cpuCount +1;
                cpuStats.dispatchCount = cpuStats.dispatchCount +1;
                if(scheduleAlg == 1){
                  if(cpuP->remainingRunTime < 5){
                    cpuP->remainQuantum = cpuP->remainingRunTime;
                    remainTime = cpuP->remainingRunTime;
                  }
                  else{
                    remainTime = 5;
                  }
                }
                else{
                   remainTime = cpuP->remainingRunTime;
                }
                if(remainTime>1){
                    int tempNum = random();
                    float chance = (float)tempNum/RAND_MAX;
                    // printf("bc %d %f \n", tempNum, chance);
                    /* printf("%s block chance: %f\n" ,cpuP->name,chance);*/
                    if(cpuP->blockProb > chance){
                        int num = random();
                        // printf("ib %d %d\n", num,(num%remainTime) +1);
                        cpuP->ioBlockTime = (int)num%remainTime +1;
                        /*cpuP->ioBlockTime = num/remainTime;*/ /*CHANGE MOD*/
                    }
                    else{
                        cpuP->ioBlockTime = -3;
                    }
                }
                cpuP->remainQuantum = remainTime;
            }
            
            /*remaining run time is zero, print out the datas*/
            if(cpuP->remainingRunTime== 0){
                cpuStats.idleTime =cpuStats.idleTime+1;
                cpuP->finishTime =  worldClock;
                printf("%-10s %6d     %6d    %6d    %6d    %6d\n",cpuP->name,(ulong)(uint)cpuP->givenRuntime,
                (ulong)(uint)cpuP->finishTime,(ulong)(uint)cpuP->cpuCount,(ulong)(uint)cpuP->ioCount,
                (ulong)(uint)cpuP->totalIOTime);
                free(cpuP);
                cpuP = NULL;
            }
            /*remaining run time is not zero*/
            else{
                cpuStats.busyTime = cpuStats.busyTime+ 1;
                cpuP->remainingRunTime = cpuP->remainingRunTime -1;
                if(scheduleAlg == 1){
                  cpuP->remainQuantum = cpuP->remainQuantum -1;
                }
                if(cpuP->ioBlockTime != -3){
                    cpuP->ioBlockTime = cpuP->ioBlockTime -1;
                }
                
                if(cpuP->ioBlockTime == 0){
                    cpuP->whenItmoved = worldClock;
                    // printf("%d %s to ioQueue\n",worldClock, cpuP->name);
                    addToQueue(ioQ, cpuP);
                    cpuP = NULL;
                }
                else if((scheduleAlg == 1)&& (cpuP->remainQuantum == 0) && cpuP->ioBlockTime != 0){
                  addToQueue(cpuQ, cpuP);
                  int remainTime;
                  /*move another process from cpuQ queue to cpu*/
                  cpuP = removeFromQueue(cpuQ);
                  // printf("%d %s come in cpu \n", worldClock, cpuP->name);
                  cpuP->cpuCount = cpuP->cpuCount +1;
                  cpuStats.dispatchCount = cpuStats.dispatchCount +1;
                  if(scheduleAlg == 1){
                    if(cpuP->remainingRunTime < 5){
                      cpuP->remainQuantum = cpuP->remainingRunTime;
                      remainTime = cpuP->remainingRunTime;
                    }
                    else{
                      remainTime = 5;
                    }
                  }
                  else{
                     remainTime = cpuP->remainingRunTime;
                  }
                  if(remainTime>1){
                      int tempNum = random();
                      float chance = (float)tempNum/RAND_MAX;
                      // printf("bc %d %f \n", tempNum, chance);
                      /* printf("%s block chance: %f\n" ,cpuP->name,chance);*/
                      if(cpuP->blockProb > chance){
                          int num = random();
                          // printf("ib %d %d\n", num,(num%remainTime) +1);
                          cpuP->ioBlockTime = (int)num%remainTime +1;
                          /*cpuP->ioBlockTime = num/remainTime;*/ /*CHANGE MOD*/
                      }
                      else{
                          cpuP->ioBlockTime = -3;
                      }
                  }
                  cpuP->remainQuantum = remainTime;
                }
            }
        }
        /* io */
        if((ioP == NULL) && (ioQ->front == NULL)){
            ioStats.idleTime = ioStats.idleTime +1;
        }
        else if((ioP == NULL) && (ioQ->front != NULL) && ((ioQ->front)->whenItmoved == worldClock)){
          /*if it just moved from cpu wait 1 turn*/
          (ioQ->front)->whenItmoved = -4;
          ioStats.idleTime = ioStats.idleTime +1;
        }
          
        else{
            ioStats.busyTime = ioStats.busyTime +1;
            if((ioP == NULL) && (ioQ->front != NULL)){
                /* unload io from queue */
                ioP = removeFromQueue(ioQ);
                // printf("%d %s come in io \n",worldClock, ioP->name);
                ioP->ioCount = ioP->ioCount +1;
                ioStats.dispatchCount = ioStats.dispatchCount +1;
                if(ioP->remainingRunTime == 0){
                    ioP->ioTime = 1;
                }
                else{
                    int num = random();
                    // printf("it %d %d\n", num, (num%30) +1);
                    ioP->ioTime = (num%30)+1;
                    // printf("iotime %d\n", ioP->ioTime); 
                }
                ioP->totalIOTime = ioP->totalIOTime + ioP->ioTime;
                /* printf("total io:%s %d\n", ioP->name,ioP->totalIOTime ); */
            }
            ioP->ioTime = ioP->ioTime -1;
            if(ioP->ioTime == 0){
                addToQueue(cpuQ, ioP);
                 // printf("%d %s to cpuQueue \n", worldClock,ioP->name);
                ioP = NULL;
            }
        }
    }
    /*printf("wall clock: %d\n", wolrdClock );
    printf("cpu total busy time: %d  idleTime: %d number of dispatch: %d\n",cpuStats.busyTime,cpuStats.idleTime,cpuStats.dispatchCount);
    printf("io  total busy time: %d  idleTime: %d number of dispatch: %d\n",ioStats.busyTime,ioStats.idleTime,ioStats.dispatchCount);
    */
    puts("\nSystem:");
    printf("The wall clock time at which the simulation finished: %d\n",worldClock);
    puts("\nCPU:");
    printf("Total time spent busy: %d\n",cpuStats.busyTime);
    printf("Total time spent idle: %d\n",cpuStats.idleTime);
    // printf((char)(double)((float)cpuStats.busyTime / (float)wolrdClock),"CPU utilization: %.2f\n");
    printf("CPU utilization: %.2f\n", (double)((float)cpuStats.busyTime / (float)worldClock));

    printf("Number of dispatches: %d\n",cpuStats.dispatchCount);
    printf("Overall throughput: %.2f\n", (double)((float)nproc / (float)worldClock));
    puts("\nI/O device:");
    printf("Total time spent busy: %d\n",ioStats.busyTime);
    printf("Total time spent idle: %d\n",ioStats.idleTime);
    printf("I/O utilization: %.2f\n", (double)((float)ioStats.busyTime / (float)worldClock));
    printf("Number of dispatches: %d\n",ioStats.dispatchCount);
    printf("Overall throughput: %.2f\n", (double)((float)nproc / (float)worldClock));
  return;
}

struct queue* createQueue(){
    struct queue* q = (struct queue*)malloc(sizeof(struct queue));
    q->front = NULL;
    q->rear = NULL;
    return q;
}

void addToQueue(queue *q, process *p){
    if((q != NULL) && (p != NULL)){
        if(q->front == NULL){
            q->front = p;
            q->rear  = p;
            p->next = NULL;
        }
        else{
        q->rear->next = p;
        q->rear = p;
        p->next = NULL;
        }
        return;
    }
}

struct process * removeFromQueue(queue *q){
    if(q->front == NULL)
        return NULL;
    struct process* temp = q->front;
    q->front = q->front->next;
    if(q->front == NULL)
        q->rear = NULL;
    return temp;
}

struct process * newProcess(char *str, char*filename, int num, double prob, int lno){
    if(strlen(str) > 10){
        perror("process name '%s' larger than 10 char\n");
        exit(1);
    }
    if(num < 1){
        fprintf(stderr,"Malformed line %s(%d)\n",filename,lno);
        exit(1);
    }
    double x = prob;
    int count = -1;
    do{ 
        ++count; 
        x *= 10; 
    } while((int)(x) % 10 != 0 );
    if((count > 2)|| prob >1 || prob <0){
        perror("something wrong with the probability %d");
        exit(1);
    }
    
    struct process * temp = (struct process*)malloc(sizeof(struct process));
    strcpy(temp->name, str);
    temp->givenRuntime = num;
    temp->remainingRunTime = num;
    temp->blockProb = prob;
    temp->ioCount = 0;
    temp->cpuCount = 0;
    temp->totalIOTime = 0;
    temp->ioCount = 0;
    temp->ioTime = 0;
    temp->whenItmoved = 0;
    temp->remainQuantum = 5;
    /* struct process a = { .givenRuntime = num, .remainingRunTime = num, .blockProb = prob, .willBeBlock = willBlock}; */
    /* strcpy(a.name, str); */
    /* printf("%s %d %f\n", a.name, a.givenRuntime, a.blockProb); */
    return temp;
}
