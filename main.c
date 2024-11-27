#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <ctype.h>




int count = 0;
int processNumber = 1;
int P1Variables = 0;
int P2Variables = 0;
int P3Variables = 0;
int numberOfInstructionsP1 = 0;
int numberOfInstructionsP2 = 0;
int numberOfInstructionsP3 = 0;
int quantum;



typedef struct {
    char* Name;
    char* Value;
}MemoryWord;

typedef struct {
    MemoryWord* MEMORY;
}Memory;

typedef struct {
    int ID;
    char* State;
    int Priority;
    int PC;
    int minBound;
    int maxBound;
}PCB;


typedef struct Node {
    PCB data;
    struct Node *next;
} Node;

// Queue structure containing pointers to the front and rear nodes
typedef struct {
    Node *front;
    Node *rear;
    char *name;
} Queue;
Queue FirstReadyQueue;
Queue SecondReadyQueue;
Queue ThirdReadyQueue;
Queue FourthReadyQueue;


// Function to initialize the queue
void initQueue(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
}

// Function to check if the queue is empty
int isEmpty(Queue *q) {
    return q->front == NULL;
}

int contains(Queue *q, PCB value) {
    Node *current = q->front;
    while (current != NULL) {
        if (current->data.ID == value.ID) {
            return 1; // Element found
        }
        current = current->next;
    }
    return 0; // Element not found
}

// Function to enqueue an element at the end of the queue
void enqueue(Queue *q, PCB value) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    newNode->data = value;
    newNode->next = NULL;

    if (isEmpty(q)) {
        q->front = newNode;
    } else {
        q->rear->next = newNode;
    }
    q->rear = newNode;
    printf("Enqueued Process %d in %s\n", value.ID, q->name);
}

// Function to dequeue an element from the front of the queue
PCB dequeue(Queue *q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
    }

    Node *temp = q->front;
    PCB value = temp->data;
    q->front = q->front->next;

    // If the queue becomes empty, update the rear pointer to NULL
    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(temp);
    return value;
}

PCB dequeuePriority(Queue *q) {
    Node *current = q->front;
    Node *highestPriorityNode = current;
    PCB highestPriorityPCB = current->data;

    while (current!= NULL) {
        if (current->data.Priority < highestPriorityNode->data.Priority) {
            highestPriorityNode = current;
            highestPriorityPCB = current->data;
        }
        current = current->next;
    }

    // Remove the highest priority PCB from the queue
    Node *temp = highestPriorityNode;
    if (highestPriorityNode == q->front) {
        q->front = highestPriorityNode->next;
    } else {
        Node *prev = q->front;
        while (prev->next!= highestPriorityNode) {
            prev = prev->next;
        }
        prev->next = highestPriorityNode->next;
    }
    free(temp);

    return highestPriorityPCB;
}

// Function to free the queue
void freeQueue(Queue *q) {
    while (!isEmpty(q)) {
        dequeue(q);
    }
}

typedef struct{
    int arrivalTime;
    PCB pcb;
}process;

typedef struct {
    int value;
    Queue* queue;
    Queue* ready;
    int ownerID;
} Mutex;

Mutex MutexInput;
Mutex MutexFile;
Mutex MutexOutput;


void semWaitB(Mutex m, PCB pcb) {
    if (m.value == 1){
        m.ownerID = pcb.ID;
        m.value = 0;
    }else {
    /* place this process in s.queue */
        pcb.State = "Blocked";
        enqueue(m.queue, pcb);
    }
}



void semSignalB(Mutex m,PCB pcb) {
    if(m.ownerID == pcb.ID){
        if (isEmpty(m.queue)){
            m.value = 1;
        }else {
        /* remove a process P from s.queue and place it on ready list*/
            PCB process = dequeuePriority(m.queue);
            process.State = "Ready";
            if(pcb.Priority == 1){
            enqueue(&FirstReadyQueue, process);
            }else{
                if(pcb.Priority == 2){
                    enqueue(&SecondReadyQueue, process);
                }else{
                    if (pcb.Priority == 3)
                    {
                        enqueue(&ThirdReadyQueue, process);
                    }else{
                        enqueue(&FourthReadyQueue, process);
                    }
                }
            }
            m.ownerID = process.ID;
        }
    }
}



void interpretProgram(const char *filename, Memory m) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file '%s'\n", filename);
        exit(1);
    }

    int a, b; 

    char line[256];
    PCB currentPCB;
    currentPCB.ID = processNumber;
    processNumber +=1;
    currentPCB.State = "Ready";
    currentPCB.Priority = 1;
    currentPCB.PC = 0;
    currentPCB.minBound = count;
    m.MEMORY[count].Name = "ProcessID";
    m.MEMORY[count].Value = "" + currentPCB.ID;
    count += 1;
    m.MEMORY[count].Name = "State";
    m.MEMORY[count].Value = currentPCB.State;
    count += 1;
    m.MEMORY[count].Name = "Priority";
    m.MEMORY[count].Value = "" + currentPCB.Priority;
    count += 1;
    m.MEMORY[count].Name = "PC";
    m.MEMORY[count].Value = "" + currentPCB.PC;
    count += 1;
    m.MEMORY[count].Name = "minBound";
    m.MEMORY[count].Value = "" + currentPCB.minBound;
    count += 1;
    int temp = count;
    count+=1;
    if(currentPCB.ID == 1){
        P1Variables = count;
    }else{
        if (currentPCB.ID == 2)
        {
            P2Variables = count;
        }else{
            P3Variables = count;
        }
        
    }
    
    m.MEMORY[count].Name = "a";
    m.MEMORY[count].Value = "";
    count += 1;
    m.MEMORY[count].Name = "b";
    m.MEMORY[count].Value = "";
    count += 1;
    m.MEMORY[count].Name = "c";
    m.MEMORY[count].Value = "";
    int instructions = 1;
    while (fgets(line, sizeof(line), file) != NULL) {
        if(line[0] != '\n'){
            char instructionName[20];
            sprintf(instructionName, "Instruction%d", instructions);
            m.MEMORY[count].Name = malloc(strlen(instructionName) + 1);
            strcpy(m.MEMORY[count].Name, instructionName);
            instructions += 1;
            m.MEMORY[count].Value = strdup(line);
            //printf("instruction is :%s IN MEM[%d] \n", m.MEMORY[count].Value, count);
            if (m.MEMORY[count].Value[strlen(m.MEMORY[count].Value) - 1] == '\n') {
                    m.MEMORY[count].Value[strlen(m.MEMORY[count].Value) - 1] = '\0';  // Remove trailing newline
            }      
        }
        instructions += 1;
        count += 1;
    }


    currentPCB.maxBound = count;
    m.MEMORY[temp].Name = "maxBound";
    m.MEMORY[temp].Value = "" + currentPCB.maxBound;
    enqueue(&FirstReadyQueue,currentPCB);

    if(currentPCB.ID == 1){
        numberOfInstructionsP1 = instructions;
    }else{
        if (currentPCB.ID == 2)
        {
            numberOfInstructionsP2 = instructions;
        }else{
            numberOfInstructionsP3 = instructions;
        }
        
    }
    

    fclose(file);

}

int arrange [3];


int* InitializeProcesses(){
    int* arrivalTime;
    for(int i = 1; i<=3; i++){
        printf("Enter the arrival time of Program %d: ", i);
        scanf("%d", &arrivalTime[i-1]);
    }
    if(arrivalTime[0] <= arrivalTime [1] && arrivalTime [0] <= arrivalTime[2]){
        arrange[0] = 1;
        if(arrivalTime[1] <= arrivalTime [2]){
            arrange[1] = 2;
            arrange[2] = 3;
        }else{
            arrange[1] = 3;
            arrange[2] = 2;
        }
    }else{
        if(arrivalTime[1] <= arrivalTime[2]){
            arrange[0] = 2;
            if (arrivalTime[2] < arrivalTime[0])
            {
                arrange[1] = 3;
                arrange[2] = 1;
            }else{
                arrange[1] = 1;
                arrange[2] = 3;
            }
            
        }else{
            arrange[0] = 3;
            if (arrivalTime[1] < arrivalTime[0])
            {
                arrange[1] = 2;
                arrange[2] = 1;
            }else{
                arrange[1] = 1;
                arrange[2] = 2;
            }
        }
    }


    return arrivalTime;
}


void trim(char *str) {
    char *end;

    // Trim leading spaces
    while (isspace((unsigned char)*str)) {
        str++;
    }

    // If the string is all spaces
    if (*str == 0) {
        return;
    }

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Null-terminate the trimmed string
    *(end + 1) = '\0';
}


void Execute(PCB* pcb, Memory* m){
    char* inst = m -> MEMORY[pcb->minBound + pcb->PC + 8].Value;
    if (inst == NULL) {
        printf("Error: Instruction at PC %d is NULL\n", pcb->PC + 8);
        return;
    }

    char *str;
    char *token;
    
    //pcb->State = "Running";

    // Allocate memory and make a copy of the original string
    str = malloc(strlen(inst) + 1);  // +1 for the null terminator
    if (str == NULL) {
        perror("Failed to allocate memory");
        return;
    }
    strcpy(str, inst); // copy the string from inst to the newly allocated memory
    

    // Get the first token
    token = strtok(str," ");
    if(strcasecmp(token, "print") == 0){
        token = strtok(NULL, " ");
        trim(token);
        int start = pcb->minBound + 5;
        if(strcasecmp(m -> MEMORY[start].Name, token) == 0){
            printf("printed: %s = %s \n",token,  m -> MEMORY[start].Value);
        }else{
            start += 1;
            if(strcasecmp(m ->MEMORY[start].Name, token) == 0){
                printf("printed: %s = %s \n",token,  m -> MEMORY[start].Value);
        }else{
            start += 1;
            if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                printf("printed: %s = %s \n",token,  m->MEMORY[start].Value);
            }else{
                printf("Cannot print this variable (Not found)");
            }
        }
        }
    }else{
        if (strcasecmp(token, "assign") == 0)
        {
            token = strtok(NULL, " ");
            int start = pcb->minBound + 5;
            if(strcmp(m->MEMORY[start].Value, "") == 0){
                m->MEMORY[start].Name = malloc(strlen(token) + 1);
                strcpy(m->MEMORY[start].Name, token);
                //printf("NEW MEM is : %d \n", start);
            }
            if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                token = strtok(NULL, " ");
                trim(token);
                
                char temp[50];
                if (strcasecmp(token, "input") == 0)
                {
                    printf("Please enter the input to assign to %s: ",m->MEMORY[start].Name );
                    scanf("%s",temp);
                }else{
                    
                    if (strcasecmp(token, "readFile") == 0) {
                        token = strtok(NULL, " ");
                        int start2 = pcb->minBound + 5;
                        trim(token);
                        FILE *file = NULL;
                        if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                            file = fopen(m->MEMORY[start2].Value, "r");
                        } else {
                            start2 += 1;
                            if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                file = fopen(m->MEMORY[start2].Value, "r");
                            } else {
                                start2 += 1;
                                if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                    file = fopen(m->MEMORY[start2].Value, "r");
                                } else {
                                    printf("Cannot read file (Not found)");
                                    return;
                                }
                            }
                        }

                        if (file != NULL) {
                            char* temp = NULL;
                            size_t tempSize = 0;
                            char buffer[100]; // Adjust the buffer size as needed
                            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                                size_t lineLength = strlen(buffer);
                                char* newTemp = realloc(temp, tempSize + lineLength + 1);
                                if (newTemp == NULL) {
                                    perror("Failed to allocate memory");
                                    free(temp);
                                    fclose(file);
                                    return;
                                }
                                temp = newTemp;
                                memcpy(temp + tempSize, buffer, lineLength);
                                tempSize += lineLength;
                            }

                            // Null-terminate the string
                            char* newTemp = realloc(temp, tempSize + 1);
                            if (newTemp == NULL) {
                                perror("Failed to allocate memory");
                                free(temp);
                                fclose(file);
                                return;
                            }
                            temp = newTemp;
                            temp[tempSize] = '\0';

                            fclose(file);
                        }
                    }

                }
                m->MEMORY[start].Value = malloc(strlen(temp) + 1);
                strcpy(m->MEMORY[start].Value, temp);
                printf("\n%s is assigned %s \n",m->MEMORY[start].Name,  m->MEMORY[start].Value);
            }else{
                start += 1;
                if(strcmp(m->MEMORY[start].Value, "") == 0){
                    m->MEMORY[start].Name = malloc(strlen(token) + 1);
                    strcpy(m->MEMORY[start].Name, token);
                }
                if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                    token = strtok(NULL, " ");
                    trim(token);
                    char temp[50];
                    if (strcasecmp(token, "input") == 0)
                    {
                        printf("Please enter the input to assign to %s: ",m->MEMORY[start].Name );
                        scanf("%s",temp);
                    }else{
                        if (strcasecmp(token, "readFile") == 0) {
                            token = strtok(NULL, " ");
                            int start2 = pcb->minBound + 5;
                            trim(token);
                            FILE *file = NULL;
                            if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                file = fopen(m->MEMORY[start2].Value, "r");
                            } else {
                                start2 += 1;
                                if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                    file = fopen(m->MEMORY[start2].Value, "r");
                                } else {
                                    start2 += 1;
                                    if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                        file = fopen(m->MEMORY[start2].Value, "r");
                                    } else {
                                        printf("Cannot read file (Not found)");
                                        return;
                                    }
                                }
                            }

                            if (file != NULL) {
                                char* temp = NULL;
                                size_t tempSize = 0;
                                char buffer[100]; // Adjust the buffer size as needed
                                while (fgets(buffer, sizeof(buffer), file) != NULL) {
                                    size_t lineLength = strlen(buffer);
                                    char* newTemp = realloc(temp, tempSize + lineLength + 1);
                                    if (newTemp == NULL) {
                                        perror("Failed to allocate memory");
                                        free(temp);
                                        fclose(file);
                                        return;
                                    }
                                    temp = newTemp;
                                    memcpy(temp + tempSize, buffer, lineLength);
                                    tempSize += lineLength;
                                }

                                // Null-terminate the string
                                char* newTemp = realloc(temp, tempSize + 1);
                                if (newTemp == NULL) {
                                    perror("Failed to allocate memory");
                                    free(temp);
                                    fclose(file);
                                    return;
                                }
                                temp = newTemp;
                                temp[tempSize] = '\0';

                                fclose(file);
                            }
                        }

                    }
                    
                    m->MEMORY[start].Value = malloc(strlen(temp) + 1);
                    strcpy(m->MEMORY[start].Value, temp);
                    printf("\n%s is assigned %s\n",m->MEMORY[start].Name,  m->MEMORY[start].Value);
            }else{
                start += 1;
                if(strcmp(m->MEMORY[start].Value, "") == 0){
                    m->MEMORY[start].Name = malloc(strlen(token) + 1);
                    strcpy(m->MEMORY[start].Name, token);
                }
                if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                    token = strtok(NULL, " ");
                    trim(token);
                    char temp[50];
                    if (strcasecmp(token, "input") == 0)
                    {
                        printf("Please enter the input to assign to %s: ",m->MEMORY[start].Name );
                        scanf("%s",temp);
                    }else{
                        if (strcasecmp(token, "readFile") == 0) {
                            token = strtok(NULL, " ");
                            int start2 = pcb->minBound + 5;
                            trim(token);
                            FILE *file = NULL;
                            if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                file = fopen(m->MEMORY[start2].Value, "r");
                            } else {
                                start2 += 1;
                                if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                    file = fopen(m->MEMORY[start2].Value, "r");
                                } else {
                                    start2 += 1;
                                    if (strcmp(m->MEMORY[start2].Name, token) == 0) {
                                        file = fopen(m->MEMORY[start2].Value, "r");
                                    } else {
                                        printf("Cannot read file (Not found)");
                                        return;
                                    }
                                }
                            }

                            if (file != NULL) {
                                char* temp = NULL;
                                size_t tempSize = 0;
                                char buffer[100]; // Adjust the buffer size as needed
                                while (fgets(buffer, sizeof(buffer), file) != NULL) {
                                    size_t lineLength = strlen(buffer);
                                    char* newTemp = realloc(temp, tempSize + lineLength + 1);
                                    if (newTemp == NULL) {
                                        perror("Failed to allocate memory");
                                        free(temp);
                                        fclose(file);
                                        return;
                                    }
                                    temp = newTemp;
                                    memcpy(temp + tempSize, buffer, lineLength);
                                    tempSize += lineLength;
                                }

                                // Null-terminate the string
                                char* newTemp = realloc(temp, tempSize + 1);
                                if (newTemp == NULL) {
                                    perror("Failed to allocate memory");
                                    free(temp);
                                    fclose(file);
                                    return;
                                }
                                temp = newTemp;
                                temp[tempSize] = '\0';

                                fclose(file);
                                
                            }
                        }

                    }
                    
                    m->MEMORY[start].Value = malloc(strlen(temp) + 1);
                    strcpy(m->MEMORY[start].Value, temp);
                    printf("\n%s is assigned %s\n",m->MEMORY[start].Name,  m->MEMORY[start].Value);
                }else{
                    printf("Cannot assign to this variable (Not found)\n");
                    return;
                }
            }
            }
        }else{
            if (strcasecmp(token, "writeFile") == 0)
            {
                token = strtok(NULL, " ");                
                int start = pcb->minBound + 5;
                FILE *file;
                if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                    file = fopen(m->MEMORY[start].Value, "w");
                    printf("Created/Opened file %s",m->MEMORY[start].Value );
                }else{
                    start += 1;
                    if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                        file = fopen(m->MEMORY[start].Value, "w");
                        printf("Created/Opened file %s",m->MEMORY[start].Value );
                    }else{
                        start += 1;
                        if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                            file = fopen(m->MEMORY[start].Value, "w");
                            printf("Created/Opened file %s",m->MEMORY[start].Value );
                        }else{
                            printf("Can't find The variable");
                            return;
                        }
                    }
                }
                
                if (file == NULL) {
                    perror("Failed to open file for writing");
                    return;
                }
                token = strtok(NULL, " ");
                trim(token);
                int start2 = pcb->minBound + 5;
                if(strcasecmp(m->MEMORY[start2].Name, token) == 0){
                    fprintf(file, "%s", m->MEMORY[start2].Value);
                    printf(" and wrote in it %s \n", m->MEMORY[start2].Value);
                }else{
                    start2 += 1;
                    if(strcasecmp(m->MEMORY[start2].Name, token) == 0){
                        fprintf(file, "%s", m->MEMORY[start2].Value);
                        printf(" and wrote in it %s \n",m->MEMORY[start2].Value);

                    }else{
                        start2 += 1;
                        if(strcasecmp(m->MEMORY[start2].Name, token) == 0){
                            fprintf(file, "%s", m->MEMORY[start2].Value);
                            printf(" and wrote in it %s \n", m->MEMORY[start2].Value);

                        }else{
                            printf("Can't find the variable!");
                            return;
                        }
                    }
                }
                fclose(file);
            }else{
                if(strcasecmp(token, "printFromTO") == 0){
                    token = strtok(NULL, " ");
                    int start = pcb->minBound + 5;
                    //printf("new start is %d", start);
                    int start2 = pcb->minBound + 5;
                    int a;
                    int b;
                    //printf("token 1 : %s \n", m->MEMORY[start].Name);
                    if(strcasecmp(m->MEMORY[start].Name, token) == 0){
                        a = atoi(m->MEMORY[start].Value);
                    }else{
                        start += 1;
                        if(strcasecmp(m->MEMORY[start].Name, token)){
                            a = atoi(m->MEMORY[start].Value);
                        }else{
                            start += 1;
                            if(strcasecmp(m->MEMORY[start].Name, token)){
                                a = atoi(m->MEMORY[start].Value);
                            }else{
                                printf("Cannot assign to this variable (Not found)");
                                return;
                            }
                        }
                    }
                    token = strtok(NULL, " ");
                    trim(token);
                    if(strcasecmp(m->MEMORY[start2].Name, token) == 0){
                        b = atoi(m->MEMORY[start2].Value);
                    }else{
                        start2 += 1;
                        if(strcasecmp(m->MEMORY[start2].Name, token) == 0){
                            b = atoi(m->MEMORY[start2].Value);
                        }else{
                            start2 += 1;
                            if(strcasecmp(m->MEMORY[start2].Name, token) == 0){
                                b = atoi(m->MEMORY[start2].Value);
                            }else{
                                printf("Cannot assign to this variable (Not found)");
                                return;
                            }
                        }
                    }
                    for(int i = a; i<b ; i++){
                        printf("%d \n", i);
                    }
                }else{
                    if(strcasecmp(token, "semWait") == 0){
                        token = strtok(NULL, " ");
                        trim(token);
                        if(strcasecmp(token, "userInput") == 0){
                            printf("semWait userInput \n");
                            semWaitB(MutexInput, *pcb);
                        }else{
                            if(strcasecmp(token, "userOutput") == 0){
                                printf("semWait userOutput \n");
                                semWaitB(MutexOutput, *pcb);
                            }else{
                                if (strcasecmp(token, "file") == 0)
                                {
                                    printf("semWait file \n");
                                    semWaitB(MutexFile, *pcb);
                                }
                                
                            }
                        }
                    }else{
                        if(strcasecmp(token, "semSignal") == 0){
                            token = strtok(NULL, " ");
                            trim(token);
                            if(strcasecmp(token, "userInput") == 0){
                                printf("semSignal userInput \n");
                                semSignalB(MutexInput, *pcb);
                            }else{
                                if(strcasecmp(token, "userOutput") == 0){
                                    printf("semSignal userOutput \n");
                                    semSignalB(MutexOutput, *pcb);
                                }else{
                                    if (strcasecmp(token, "file") == 0)
                                    {
                                        printf("semSignal File \n");
                                        semSignalB(MutexFile, *pcb);
                                    }
                                    
                                }
                            }
                        }
                    }
                }
            }
        }
        
        
    }

    pcb->PC += 1;
    if(pcb->maxBound <=( pcb->PC + 8 + pcb->minBound)){
        pcb->State = "Finished";
    }
    if(strcmp(pcb->State, "Running") == 0){
        pcb->State = "Ready";
    }

    // Free the allocated memory
    free(str);

    token = NULL;
    
}


int run(int* arrivalTime, Memory m){
    int clkCycle = 0;
    int N = 0;
    while(1){
        printf("Clkcycle %d: \n", clkCycle);
        if(arrivalTime[0] == clkCycle){
            interpretProgram("Program_1.txt",m);
        }
        if(arrivalTime[1] == clkCycle){
            interpretProgram("Program_2.txt", m);
        }
        if(arrivalTime[2] == clkCycle){
            interpretProgram("Program_3.txt", m);
        }
        if(!isEmpty(&FirstReadyQueue)){
            PCB currentPCB = dequeue(&FirstReadyQueue);
            printf("PCB %d is running (Program %d) in %s\n", currentPCB.ID, arrange[currentPCB.ID-1], FirstReadyQueue.name);
            if(strcmp(currentPCB.State,"Ready") == 0){
                Execute(&currentPCB, &m);
                currentPCB.Priority = 2;
                if(strcmp(currentPCB.State,"Finished")==0){
                    printf("PCB %d (Program %d) is finished \n", currentPCB.ID, arrange[currentPCB.ID-1]);
                    N += 1;
                }else{
                    if (strcmp(currentPCB.State, "Ready") == 0){
                        enqueue(&SecondReadyQueue,currentPCB);
                    }
                }
            }
        }else{
           if(!isEmpty(&SecondReadyQueue)){
                PCB currentPCB = dequeue(&SecondReadyQueue);
                printf("PCB %d is running (Program %d) in %s\n", currentPCB.ID, arrange[currentPCB.ID-1], SecondReadyQueue.name);
                if(strcmp(currentPCB.State,"Ready") == 0){
                    for(int i = 0 ; i < 2 ; i++){
                        Execute(&currentPCB, &m);
                        currentPCB.Priority = 3;
                        if(strcmp(currentPCB.State,"Finished")==0){
                            break;
                        }
                    }
                    if(strcmp(currentPCB.State,"Finished")==0){
                        printf("PCB %d (Program %d) is finished \n", currentPCB.ID, arrange[currentPCB.ID-1]);
                        N += 1;
                    }else{
                        if (strcmp(currentPCB.State, "Ready") == 0){
                            enqueue(&ThirdReadyQueue,currentPCB);
                        }
                    }
                }
            }else{
                if(!isEmpty(&ThirdReadyQueue)){
                    PCB currentPCB = dequeue(&ThirdReadyQueue);
                    printf("PCB %d is running (Program %d) in %s \n", currentPCB.ID, arrange[currentPCB.ID-1], ThirdReadyQueue.name);
                    if(strcmp(currentPCB.State,"Ready") == 0){
                        for(int i = 0 ; i < 4 ; i++){
                            Execute(&currentPCB, &m);
                            currentPCB.Priority = 4;
                            if(strcmp(currentPCB.State,"Finished")==0){
                                break;
                            }
                        }
                        if(strcmp(currentPCB.State,"Finished")==0){
                            printf("PCB %d (Program %d) is finished \n", currentPCB.ID, arrange[currentPCB.ID-1]);
                            N += 1;
                        }else{
                            if (strcmp(currentPCB.State, "Ready") == 0){
                                enqueue(&FourthReadyQueue,currentPCB);
                            }
                        }
                    }
                }else{
                    if(!isEmpty(&FourthReadyQueue)){
                        PCB currentPCB = dequeue(&FourthReadyQueue);
                        printf("PCB %d is running (Program %d) in %s\n", currentPCB.ID, arrange[currentPCB.ID-1], FourthReadyQueue.name);
                        if(strcmp(currentPCB.State,"Ready") == 0){
                            for(int i = 0 ; i < 8 ; i++){
                                Execute(&currentPCB, &m);
                                if(strcmp(currentPCB.State,"Finished")==0){
                                    break;
                                }
                            }
                            if(strcmp(currentPCB.State,"Finished")==0){
                                printf("PCB %d (Program %d) is finished \n", currentPCB.ID, arrange[currentPCB.ID-1]);
                                N += 1;
                            }else{
                                if (strcmp(currentPCB.State, "Ready") == 0)
                                {
                                    enqueue(&FourthReadyQueue,currentPCB);
                                }
                            }
                        }
                    }
                }
            } 
        }
        if(N >= 3){
            break;
        }
        
        clkCycle += 1;
    }
    return 1;
}



int main() {
    MutexInput.value = 1;
    MutexFile.value = 1;
    MutexOutput.value = 1;/*
    initQueue(MutexFile.queue);
    initQueue(MutexInput.queue);
    initQueue(MutexOutput.queue);*/
    MemoryWord mw;
    mw.Name = malloc(20*sizeof(char));
    mw.Value = malloc(20*sizeof(char));
    int clkCycle = 1;
    Memory m;
    m.MEMORY = malloc(60 * sizeof(mw));
    FirstReadyQueue.name = "First Queue";
    SecondReadyQueue.name = "Second Queue";
    ThirdReadyQueue.name = "Third Queue";
    FourthReadyQueue.name = "Fourth Queue";


    int* arrivalTimes = InitializeProcesses();
    run(arrivalTimes, m);


    /*
    initializeProcesses();
    runMultilevelFeedbackQueue(Memory m);
    */
    exit(0);
    return 0;

}