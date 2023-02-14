// Student: Yihao Zhong
// Net ID: yz7654

/* Reference List:
* student that I talked:
* Ruofan Li, netID: rl4126. We do not see each other's code at any times. But we had conversations on general code design, how to use a struct
*                           how to use array for taskQueue, resultQueue and the fwrite. We talked about how to store the first and last digits 
*                            of the result when stitching so we do not need to go through the whole list. We implemented the above ideas by ourselves.
* readlJasong (on discord). He posted some code piece of his own on the discord common channel. I looked into the code and examined them (help him debug).
*                           During the examination, I found some of his code shared similar idea as mine. Mainly the consumer and submit task part. This
*                           could be he is also referencing bullet point 4 below. I read his code so I need to reference him.
 * 
 * Credit to Charlie Xiang, ion, drawingourmoments, Michael Lin (ON DISCORD). We do not see each other's code at any times.
 *                       but in the common group chat, their ideas, comments, explanation are being inspiring for me to come up with my general code.
 1. Book: Operating Systems: Three Easy Pieces, Chapter 25 - 31
 *          https://pages.cs.wisc.edu/~remzi/OSTEP/Chinese/27.pdf etc.
 * The lab inspiration, Prof TY's reference: https://github.com/remzi-arpacidusseau/ostep-projects  (such a classical class for WISC U)
 * 2. File handling: https://www.thegeekstuff.com/2012/07/c-file-handling/
 * 3. mmap: https://stackoverflow.com/questions/30385635/why-use-mmap-over-fread
 *          https://developer.aliyun.com/article/11242
 *          https://man7.org/linux/man-pages/man2/mmap.2.html
 *          https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/MMap.html
 *          https://stackoverflow.com/questions/34674809/mmap-from-file-in-loop
 * 4. Thread Pool code template (approved by prof): https://www.youtube.com/watch?v=_n2hE2gyPxU and its series video: 'Unix Threads in C' 29 videos in total
                    https://code-vault.net/lesson/w1h356t5vg:1610029047572 (thread pool 1)
                    https://code-vault.net/lesson/j62v2novkv:1609958966824 (thread pool 2)
                    https://code-vault.net/course/6q6s9eerd0:1609007479575/lesson/18ec1942c2da46840693efe9b51eabf6 ( mutex)
                    https://code-vault.net/course/6q6s9eerd0:1609007479575/lesson/18ec1942c2da46840693efe9b51fb458 ( thread)
 * 5. getopt(): https://man7.org/linux/man-pages/man3/getopt.3.html
 *              https://www.geeksforgeeks.org/getopt-function-in-c-to-parse-command-line-arguments/
 *              https://stackoverflow.com/questions/18079340/using-getopt-in-c-with-non-option-arguments
 *              https://www.nhooo.com/note/qa0seh.html
 * 
 * 6. struct type, workflow template (approved by prof):
 *              https://codereview.stackexchange.com/questions/274363/simple-parallel-compression-using-run-length-encoding-in-c
 * 7. atoi(): https://www.educative.io/answers/how-to-convert-a-string-to-an-integer-in-c
 * 8. fopen(): https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
 * 9. faster compression code template (only the idea, mine is O(n) in space and time): https://leetcode.com/problems/string-compression/solutions/2646085/c-faster-solution/
 *         https://leetcode.com/problems/string-compression/discussion/ 
 *
 * 10. unsigned_int: https://www.geeksforgeeks.org/unsigned-char-in-c-with-examples/
 * 11. https://stackoverflow.com/questions/20196121/passing-struct-to-pthread-as-an-argument 
 * 12. https://valgrind.org/docs/manual/hg-manual.html 
 * 13. code template for consumer: https://github.com/Saggarwal9/Parallel-ZIP/blob/master/pzip.c 
                        and also for consumer: https://code-vault.net/lesson/j62v2novkv:1609958966824 (thread pool 2)
                        and also for consumer: PPT from Prof TY, page 287 - 291
 * 14. passing argument to thread: https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/ThreadArgs.html 
 * 15. clone: https://man7.org/linux/man-pages/man2/clone.2.html 
 * 16. makefile: https://earthly.dev/blog/make-flags/ 
 * 17. thread initializer: https://stackoverflow.com/questions/14320041/pthread-mutex-initializer-vs-pthread-mutex-init-mutex-param 
 * 18. valgrind manual: https://valgrind.org/docs/manual/hg-manual.html 
 *  */


/* My Notes:
 *  1.  # -o: to specify the executable name
 *      # -O: to turn on optimization
 *
 *
 * */
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <unistd.h>


//
//void singleRLE(char* chunk){
//
//}

typedef struct Task{
    int taskId;
    char* file_addr;

    int size_of_this_Task;

}Task;
Task taskQueue[250000];
typedef struct result{
    int resultId;
    char* result_c;
    unsigned char* result_count;
    char start;
    char end;
    unsigned char start_count;
    unsigned char end_count;
    int result_index;
}Result;



Result**  resultQueue;
int total_num_tasks = 0;
int curr_task_count = 0;
int finishSubmit = 0;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
//pthread_cond_t not_fill = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexQ = PTHREAD_MUTEX_INITIALIZER;

// compression is basically the leetcode like string compression.
void compress(Task* task){
    int resultIndex = 0;
    int j = 0;
    char tempResult_c[250000];
    unsigned char tempResult_count[250000];
    Result* result_for_this_task = malloc(sizeof(Result));
    result_for_this_task->resultId = task->taskId;
    while (j<task->size_of_this_Task){
        int runner = j +1;
        while((runner < task->size_of_this_Task) && (*((task->file_addr)+j) == *((task->file_addr)+runner))){
            runner ++;
            //printf("runnner is %d: ", runner);
        }

        tempResult_c[resultIndex] = *((task->file_addr)+j);
        //fwrite( &tempResult[resultIndex], sizeof(tempResult[resultIndex]), 1, stdout);
        unsigned char temp = (runner -j);
        //printf(" temp is %c, ", temp);
        tempResult_count[resultIndex] = temp;
        //fwrite( &tempResult[resultIndex], sizeof(tempResult[resultIndex]), 1, stdout);

        if (resultIndex == 0){
            //printf("the start count is %d\n", tempResult_count[resultIndex]);
            result_for_this_task->start = tempResult_c[resultIndex];
            result_for_this_task->start_count = tempResult_count[resultIndex];
        }

        resultIndex++;
        j = runner;
    }
    //printf("the end count is %d\n", tempResult_count[resultIndex-1]);
    result_for_this_task->end = tempResult_c[resultIndex-1];
    result_for_this_task->end_count = tempResult_count[resultIndex-1];
    //printf(" tempresult is %s", tempResult);
    result_for_this_task->result_c = malloc(sizeof (char)* resultIndex);
    result_for_this_task->result_count = malloc(sizeof(unsigned char)* resultIndex);
    result_for_this_task->result_index = resultIndex;
    for(int i = 0; i<resultIndex; i++){
        result_for_this_task->result_c[i] = tempResult_c[i];
        result_for_this_task->result_count[i] = tempResult_count[i];
    }
    resultQueue[task->taskId] = result_for_this_task;
    //printf("Result is %s", task->result);
}
int outQueue = 0;
void* consumer(){
    while(1){
        
        pthread_mutex_lock(&mutexQ);
        Task newTask;
        if ((finishSubmit == 1) && (curr_task_count <=0)){
            pthread_mutex_unlock(&mutexQ);
            return NULL;
        }
        while((curr_task_count <= 0) && (finishSubmit == 0)){
            //pthread_cond_signal(&not_fill);
            pthread_cond_wait(&not_empty, &mutexQ);
        }
        if ((finishSubmit == 1) && (curr_task_count <=0)){
            pthread_mutex_unlock(&mutexQ);
            return NULL;
        }
        newTask = taskQueue[outQueue];
        // for(int i = 0; i< curr_task_count; i++){
        //     taskQueue[i] = taskQueue[i+1];
        // }
        outQueue ++;
        curr_task_count --; 
        pthread_mutex_unlock(&mutexQ);
        compress(&newTask);
        
        
       
    }
    return NULL;
}

// void submit(Task submit_task){
    
// }



int main(int argc, char * argv[]) {
    //char * line = NULL;
    //size_t len = 0;
    //char current;
    //unsigned char count = 0;
    //bool started = false;
    int number_of_worker_thread = 1;


    //mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    int opt ;
    // getopt sets the optind variable to indicate the position of the next argument.
    while ((opt = getopt(argc, argv, "j:")) != -1){
        switch (opt){
            case 'j':
                number_of_worker_thread = atoi(optarg);
                //printf("here is %s\n",optarg);
                //printf("here is  j;\n");
                break;
            default:
                break;
        }
    }

    //if (argv[optind] == NULL){
    //  printf("Null");
    //}
    // printf("J IS %d.\n", number_of_worker_thread);
    //taskQueue = malloc(sizeof(Task) *100000);
    resultQueue = malloc(sizeof(Result) *250000);

    
    /*

    pthread section

    */

    
    pthread_t th_pol[number_of_worker_thread];
    //pthread_t main_th
    
    for(int i = 0; i<number_of_worker_thread; i++){
        pthread_create(&th_pol[i], NULL, &consumer, NULL); 
    }
    

        char *addr;
        struct stat sb;
        // man page code template :
         
        for(; optind < argc; optind++){
            //printf("optind is %d. \n", optind);
            int fp = open(argv[optind], O_RDONLY);
            if (fstat(fp, &sb) == -1)           /* To obtain file size */
                continue;
            int substract_size_of_file = sb.st_size;
            int temp_ceil = sb.st_size;
            int num_of_tasks_in_this_file = 0;
            while (temp_ceil>0){
                temp_ceil -=4096;
                num_of_tasks_in_this_file ++;
        }
        // ceil(), math.h does not work here.......
        //num_of_tasks_in_this_file = ceil((double)(sb.st_size) /4096);
        //printf("Number of this task is %d.\n", num_of_tasks_in_this_file);
        addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fp, 0);
        // mmap need to be inside the for loop
        for (int i = 0; i<num_of_tasks_in_this_file; i++) {
            Task *temp_task = malloc(sizeof(Task));
            // the last chunk of that file
            if (substract_size_of_file< 4096){
                temp_task->size_of_this_Task = substract_size_of_file;
            }
            // other chunk of that file are all 4096
            else{
                temp_task->size_of_this_Task = 4096;
                substract_size_of_file -= 4096;
            }
            temp_task->taskId = total_num_tasks;
            // only store the addr index, not the actual address
            temp_task->file_addr = addr + 4096*i;

            // put into the task queue
            //submit(*temp_task);
            pthread_mutex_lock(&mutexQ);
            //pthread_cond_signal(&not_fill);
            taskQueue[total_num_tasks] = *temp_task;
            //total_num_tasks ++;
            curr_task_count ++;
            total_num_tasks++;
            pthread_cond_signal(&not_empty);
            pthread_mutex_unlock(&mutexQ);
            
            //printf("the num is : %d", total_num_tasks);
            
        }
        //printf("Total num of tasks are %d.\n",total_num_tasks);
        
    }
    pthread_mutex_lock(&mutexQ);
    finishSubmit = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&mutexQ);
    


    
    // for (int i = 0; i<total_num_tasks; i++){
    //     compress(taskQueue[i]);
    //     //printf("%d", i);

    // }


    /*

    pthread section

    */

  
    for(int i = 0; i<number_of_worker_thread; i++){
        pthread_join(th_pol[i], NULL);
    }
    
    pthread_mutex_destroy(&mutexQ);
    pthread_cond_destroy(&not_empty);

    //printf("here is the debug");
    for (int i = 0; i<total_num_tasks; i++){
        for(int j = 0; j < (resultQueue[i]->result_index)-1; j++){
            fwrite( &resultQueue[i]->result_c[j], sizeof(char), 1, stdout);
            fwrite( &resultQueue[i]->result_count[j], sizeof(unsigned char), 1, stdout);
        }
        //printf("Our i is %d", i);
        if (i < (total_num_tasks-1)){
            //printf("%d", i);

            if((resultQueue[i]->end) == (resultQueue[i+1]->start)){

                resultQueue[i+1]->result_count[0] = resultQueue[i]->end_count + resultQueue[i+1]->start_count;
                //printf("the new start is %d\n", resultQueue[i+1]->result_count[0]);
            }
            else{

                fwrite( &resultQueue[i]->result_c[(resultQueue[i]->result_index)-1], sizeof(char), 1, stdout);
                fwrite( &resultQueue[i]->result_count[(resultQueue[i]->result_index)-1], sizeof(unsigned char), 1, stdout);

            }
        }
        else{

            fwrite( &resultQueue[i]->result_c[(resultQueue[i]->result_index)-1], sizeof(char), 1, stdout);
            fwrite( &resultQueue[i]->result_count[(resultQueue[i]->result_index)-1], sizeof(unsigned char), 1, stdout);

        }
    }



    // mmap create a 4096 for each file
//    for (int i =0; i<= 4098; i++){
//        printf("%c", *(addr+i));
//    }
    // this print first file, and 4 char in the second file


//

   //munmap (addr, sb.st_size);
    return 0;
}

