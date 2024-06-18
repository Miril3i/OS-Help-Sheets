#include <stdlib.h>
#include <stdio.h>
#include <string.h>     // strerror
#include <errno.h>      // errno access
#include <limits.h>
#include <sys/types.h>  // fork, wait, open, socket, bind
#include <unistd.h>     // fork, pipe, unlink
#include <sys/wait.h>   // wait
#include <stdatomic.h>  // atomics
#include <signal.h>     // sigaction
#include <sys/stat.h>   // open, shm_open, mq_open
#include <fcntl.h>      // open, shm_open, mq_open
#include <sys/mman.h>   // shm_open
#include <pthread.h>
#include <semaphore.h>
#include <mqueue.h>
#include <sched.h>      // sched_yield
#include <sys/socket.h>
#include <netinet/in.h> // ADDR_DEFINES 

/*
 *  NOTE: This file is only used for a fast reference for common
 *        functions and how to (hopefully) correctly use them.
 *        It does not compile!
 */

// argc checking
    if (argc != ARGC_SOLL) {
        printf("WRONG USAGE: ./program <args>\n");
        exit(EXIT_FAILURE);
    }

// strtol + error handling
    const char* string = "1234";
    int x;
    x = strtol(string, NULL, 10);
    if (x == LONG_MAX || x == LONG_MIN) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

// sched_yield + error handling
// to make busy waiting less busy
// mostly useful in a while loop
   if (sched_yield() == -1) {
    perror("sched_yield");
    exit(EXIT_FAILURE);
   }
// custom mutex implementation
atomic_flag isLocked = ATOMIC_FLAG_INIT;
void my_mutex_lock() {
    while (atomic_flag_test_and_set(&isLocked)) {
        if (sched_yield() == -1) {
            perror("sched_yield");
            exit(EXIT_FAILURE);
        }
    }
}

void my_mutex_unlock() {
    atomic_flag_clear(&isLocked);
}

// malloc + error handling
    TYPE* NAME = malloc(N * sizeof(TYPE));
    if (NAME == NULL && ((N * sizeof(TYPE)) != 0)) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    free(NAME); // no return value

// fork + error handling
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // child
        // code to child
        return EXIT_SUCCESS;
    } else {
        // code for parent
    }
    // wait for all children to finish
    while (wait(NULL) > 0) {
    }

// signal handling usage + initialization
    atomic_int global_signum = 0;

    void handler(int signum) {
        global_signum = signum;
    }

    struct sigaction sa = {
        .sa_handler = handler,
    };
    if (sigemptyset(&sa.sa_mask) == -1) {
        perror("sigemtyset");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGNAL_TO_LISTEN, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    switch (global_signum) {
        case SIGNAL_TO_LISTEN:
            /* code */
            break;
        
        default:
            break;
    }

// anonymous pipe usage + error handling
// use when when fork() is used and not two 
// different executables
// dup2() may be useful to use STDIN/OUT as 
// IN/OUTput for the pipe
// dup2(pipefd[0], STDIN_FILENO);   // read
// dup2(pipefd2[1], STDOUT_FILENO); // write
    int pipefd[2];
    // [0] read
    // [1] write
    pid_t pid;
    // creating anonymous pipe 
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    // creating different process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // process logic
    if (pid == 0) { // child
        // assuming child wants to write
        close(pipefd[0]);   // close read
        TYPE STUFF_TO_WRITE;
        write(pipefd[1], &STUFF_TO_WRITE, sizeof(STUFF_TO_WRITE_TYPE));
        close(pipefd[1]);   // close write after writing everything
        return EXIT_SUCCESS;
    } else {
        // assuming parent wants to read
        close(pipefd[1]);   // close write
        TYPE STUFF_TO_READ;
        read(pipefd[0], &STUFF_TO_READ, sizeof(STUFF_TO_READ_TYPE));
        close(pipefd[0]);   // close read after reading everything
    }
    // wait for all childs to finish
    while (wait(NULL) > 0) {
    }

// named pipe usage + error handling
// always useable the same way - preferred
// when having to different executables 
// which want to communicate
    pid_t pid;
    char pathname[256] = {0};
    char name[5] = "test"; // could be argv[N] don't forget space for '\0'
    const mode_t permissions = S_IRUSR | S_IWUSR; // maybe needs additional permissions
    int n;
    n = snprintf(pathname, 256, "/tmp/csaz-%s", name);
    if (n < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    } else if ((size_t)n >= sizeof(pathname)) {
        fprintf(stderr, "Buffer exhausted\n");
        exit(EXIT_FAILURE);
    }
    //creating named pipe
    if (mkfifo(pathname, permissions) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    } 
    // simulating different executable 
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    // executable logic
    if (pid == 0) { // simulating different executable which write the fifo
        int fd = open(pathname, O_WRONLY); // open according to needs write/read
        TYPE STUFF_TO_WRITE;
        write(fd, &STUFF_TO_WRITE, sizeof(STUFF_TO_WRITE_TYPE));
        return EXIT_SUCCESS;
    } else {       // -"- reads the fifo
        int fd = open(pathname, O_RDONLY);
        TYPE STUFF_TO_READ;
        read(fd, &STUFF_TO_READ, sizeof(STUFF_TO_READ_TYPE));
    }
    // unlink fifo
    if (unlink(pathname) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }
    // wait for all childs to finish
    while (wait(NULL) > 0) {
    }

// shared memory usage + error handling
// must link with -lrt
// name mustn't include a '/'
// my create function
void* shm_create(const char* name, const size_t size, const int oflag, const mode_t permissions) {
    char shmName[255] = {0};
    int sharedMemFd;
    void* sharedMem;

    int n;
    n = snprintf(shmName, 255, "/tmp-csaz-shared_mem-%s", name);
    if (n < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    } else if ((size_t)n >= sizeof(pathname)) {
        fprintf(stderr, "Buffer exhausted\n");
        exit(EXIT_FAILURE);
    }
    sharedMemFd = shm_open(shmName, oflag, permissions);
    if (sharedMemFd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(sharedMemFd, size) == -1) {
        fprintf(stderr, "Can't allocate shared memory %s - %s\n", shmName, strerror(errno));
        shm_unlink(shmName);

        exit(EXIT_FAILURE);
    }

    sharedMem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemFd, 0);
    if (sharedMem == MAP_FAILED) {
        fprintf(stderr, "Can't map %s - %s\n", shmName, strerror(errno));
        shm_unlink(shmName);

        exit(EXIT_FAILURE);
    }
    close(sharedMemFd);

    return sharedMem;
}
// usage - don't forget munmap and unlink
    const int oflag = O_CREAT | O_RDWR;
    const mode_t permissions = S_IRUSR | S_IWUSR;
    size_t size = sizeof(TYPE) * N;

    void* sharedMem = shm_create("name", size, oflag, permissions);
    TYPE SHARED_MEM = (TYPE*)sharedMem

    munmap(sharedMem, size);
    shm_unlink("name");

// access methods
// if shm_create was called before fork then just casting the
// returned void* to a local variable is sufficent to access it
    int* sharedMemChild = (int*)sharedMem;
// if shm_create was not called before fork or is called in an extern
// file then the memory segment must be opened and mapped again
    int sharedMemFdChild = shm_open("/tmp-csaz-shared_mem-NAME", O_RDWR, permission);
    int* sharedMemChild = (int*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemFdChild, 0);
       
// conditiond variable usage + error handling
    pthread_cond_t condVariable;

    int condInitValue;
    if ((condInitValue = pthread_cond_init(&condVariable, NULL)) != 0) {
        fprintf(stderr, "Error when initializing the condition variable: %s\n", strerror(condInitValue));
        exit(EXIT_FAILURE);
    }

    int condWaitValue;
    if ((condWaitValue = pthread_cond_wait(&condVariable, &LOCKED_MUTEX)) != 0) {
        fprintf(stderr, "Error when waiting for the condition variable: %s\n", strerror(condWaitValue));
        exit(EXIT_FAILURE);
    }

    int condSignalValue;
    if ((condSignalValue = pthread_cond_signal(&condVariable)) != 0) {
        fprintf(stderr, "Error when signaling the condition variable: %s\n", strerror(condSignalValue));
        exit(EXIT_FAILURE);
    }

    int condDestroyValue;
    if ((condDestroyValue = pthread_cond_destroy(&condVariable)) != 0) {
        fprintf(stderr, "Error when destroying the condition variable: %s\n", strerror(condDestroyValue));
        exit(EXIT_FAILURE);
    }

// mutex usage + error handling
    pthread_mutex_t mutex;

    int mutexInitValue = 0;
    if ((mutexInitValue = pthread_mutex_init(&mutex, NULL)) != 0) {
        fprintf(stderr, "Error when initializing the mutex: %s\n", strerror(mutexInitValue));
        exit(EXIT_FAILURE);
    }

    int mutexLockValue = 0;
    if ((mutexLockValue = pthread_mutex_lock(&mutex)) != 0) {
        fprintf(stderr, "Error when locking the mutex: %s\n", strerror(mutexInitValue));
        exit(EXIT_FAILURE);
    }
    
    int mutexUnlockValue = 0;
    if ((mutexUnlockValue = pthread_mutex_unlock(&mutex)) != 0) {
        fprintf(stderr, "Error when unlocking the mutex: %s\n", strerror(mutexLockValue));
        exit(EXIT_FAILURE);
    }

    int mutexDestroyValue = 0;
    if ((mutexDestroyValue = pthread_mutex_destroy(&mutex)) != 0) {
        fprintf(stderr, "Error when destroying the mutex: %s\n", strerror(mutexDestroyValue));
        exit(EXIT_FAILURE);
    }

// semaphore usage + error handling
// must link with -pthread
    sem_t semaphore;

    if (sem_init(&semaphore, 0, 0) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if (sem_post(&semaphore) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
    if (sem_wait(&semaphore) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&semaphore) == -1) {
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }

// barrier usage + error handling
    pthread_barrier_t barrier;

    int barrierInitValue = 0;
    if ((barrierInitValue = pthread_barrier_init(&barrier, NULL, COUNT)) != 0) {
        fprintf(stderr, "Error when initializing the barrier: %s\n", strerror(barrierInitValue));
        exit(EXIT_FAILURE);
    }

    int barrierWaitValue = 0;
    if ((barrierWaitValue = pthread_barrier_wait(&barrier)) != 0
         && barrierWaitValue != PTHREAD_BARRIER_SERIAL_THREAD) {
        fprintf(stderr, "Error when waiting for the barrier: %s\n", strerror(barrierWaitValue));
        exit(EXIT_FAILURE);
    } else if (barrierWaitValue == PTHREAD_BARRIER_SERIAL_THREAD) {
        /* Code for a spesific thread to handle something*/
        /* Can be omitted if no handling is required */
    }

    int barrierDestroyValue = 0;
    if ((barrierDestroyValue = pthread_barrier_destroy(&barrier)) != 0) {
        fprintf(stderr, "Error when destroying the barrier: %s\n", strerror(barrierDestroyValue));
        exit(EXIT_FAILURE);
    }

// message queue usage + error handling
// must link with -lrt
// name mustn't include a '/'
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256

void mq_attr_init(struct mq_attr* attr) {
    attr->mq_flags = 0;
    attr->mq_maxmsg = MAX_MESSAGES;
    attr->mq_msgsize = MAX_MSG_SIZE;
    attr->mq_curmsgs = 0;
}

// mq with fork - same mq fd for both processes
    mqd_t queue;
    struct mq_attr attr;
    const int oflag = O_RDWR | O_CREAT | O_EXCL; // use correct oflags
    const mode_t permissions = S_IRUSR | S_IWUSR; 
    char queueName[255] = {0};
    char name[32] = "test";
    char *receive_msg;
    char *send_msg = "Message";

    mq_attr_init(&attr);    

    int n;
    n = snprintf(queueName, 255, "/tmp-csaz-mq-%s", name);
    if (n < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    } else if ((size_t)n >= sizeof(queueName)) {
        fprintf(stderr, "Buffer exhausted\n");
        exit(EXIT_FAILURE);
    }
    queue = mq_open(queueName, oflag, permissions, &attr);
    if (queue == -1) {
        perror("mq_open");
        return EXIT_FAILURE;
    }

    if (fork() == 0) {
        if (mq_send(queue, send_msg, strlen(send_msg)+1, 0) == -1) {
            perror("mq_send");
            mq_close(queue);
            mq_unlink(queueName);

            exit(EXIT_FAILURE);
        }
        printf("message send\n");
    } else {
        receive_msg = malloc(sizeof(char) * attr.mq_msgsize);
        sleep(2);
        if (mq_receive(queue, receive_msg, attr.mq_msgsize, NULL) == -1) {
            perror("mq_receive");
            mq_close(queue);
            mq_unlink(queueName);

            exit(EXIT_FAILURE);
        }
        printf("received: %s\n", receive_msg);
        free(receive_msg);
    }

    while (wait(NULL) > 0) {
    }

    mq_close(queue);
    mq_unlink(queueName);


// mq with fork (simulating two different executables)
// - different mq fd for processes 
// doesn't work
    if (fork() == 0) {
        mqd_t queue_send;
        struct mq_attr attr;
        const int oflag = O_WRONLY | O_CREAT | O_EXCL; // use correct oflags
        const mode_t permissions = S_IRUSR | S_IWUSR; 
        char queueName[255] = {0};
        char name[32] = "test";
        char *send_msg = "Message";

        mq_attr_init(&attr);

        int n;
        n = snprintf(queueName, 255, "/tmp-csaz-mq-%s", name);
        if (n < 0) {
            perror("snprintf");
            exit(EXIT_FAILURE);
        } else if ((size_t)n >= sizeof(queueName)) {
            fprintf(stderr, "Buffer exhausted\n");
            exit(EXIT_FAILURE);
        }
        printf("send: %s\n", queueName);
        queue_send = mq_open(queueName, oflag, permissions, &attr);
        if (queue_send == -1) {
            perror("send: mq_open");
            return EXIT_FAILURE;
        }
        printf("created queue\n");
        if (mq_send(queue_send, send_msg, strlen(send_msg)+1, 0) == -1) {
            perror("mq_send");
            mq_close(queue_send);
            mq_unlink(queueName);

            exit(EXIT_FAILURE);
        }

        printf("message send\n");

        mq_close(queue_send);
        // mq_unlink(queueName);  // make sure to only use this if the queue
                                  // isn't needed in any other way anymore
    } else {

        sleep(2); // making sure child created mq and send messages
        mqd_t queue_receive;
        struct mq_attr attr;
        const int oflag = O_RDONLY; // use correct oflags
        char queueName[255] = {0};
        char name[32] = "test4";
        char *receive_msg;

        int n;
        n = snprintf(queueName, 255, "/tmp-csaz-mq-%s", name);
        if (n < 0) {
            perror("snprintf");
            exit(EXIT_FAILURE);
        } else if ((size_t)n >= sizeof(queueName)) {
            fprintf(stderr, "Buffer exhausted\n");
            exit(EXIT_FAILURE);
        }
        printf("receive: %s\n", queueName);
        queue_receive = mq_open(queueName, oflag);
        if (queue_receive == -1) {
            perror("receive: mq_open");
            return EXIT_FAILURE;
        }

        if (mq_getattr(queue_receive, &attr) == -1) {
            perror("mq_getattr");
            mq_close(queue_receive);
            mq_unlink(queueName);

            exit(EXIT_FAILURE);
        }

        receive_msg = malloc(sizeof(char) * attr.mq_msgsize);
        if (mq_receive(queue_receive, receive_msg, attr.mq_msgsize, NULL) == -1) {
            perror("mq_receive");
            mq_close(queue_receive);
            mq_unlink(queueName);

            exit(EXIT_FAILURE);
        }
        printf("received: %s\n", receive_msg);
        
        free(receive_msg);
        mq_close(queue_receive);
        mq_unlink(queueName);
    }

// pthread usage + error handling
// pthread_cancel(); and pthread_setcanceltype(); might be useful but
// could create leaks
// don't use rand() - not thread_safe;
void* thread_job(void* arg){
    int* data = arg;
    unsigned int randomSeed = pthread_self();

    printf("arg = %d\n", *data);
    *data = rand_r(&randomSeed);
    
    return data;
}  
    pthread_t threadID;
    int arg = 2;
    void* threadReturn;

    int createThreadValue;
    if ((createThreadValue = pthread_create(&threadID, NULL, &thread_job, &arg)) != 0) {
        fprintf(stderr, "Error when creating thread: %s\n", strerror(createThreadValue));
        exit(EXIT_FAILURE);
    }

    int joinThreadValue;
    if ((joinThreadValue = pthread_join(threadID, &threadReturn)) != 0) {
        fprintf(stderr, "Error when joining thread: %s\n", strerror(joinThreadValue));
        exit(EXIT_FAILURE);
    }

    printf("received %d from thread\n", *(int*)threadReturn);

// thread pool - aight imma head out

// socket usage + error handling
// server
    int socketFd;
    int connectionFd;
    int port = 12345; // potentially argv[X] (converted strtol)
    
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socketFd, (const struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(socketFd, N) == -1) { // N = amount of queueable connections 
        perror("listen");
        exit(EXIT_FAILURE);
    }

    connectionFd = accept(socketFd, NULL, NULL);
    if (connectionFd == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    // use read(connectionFd, BUFFER, MSG_SIZE) to read (-1 on error)
    // might need some checking for '\0' or MSG_SIZE to ensure correctness
    // also use memset(BUFFER, 0, MSG_SIZE * sizeof(TYPE)) to avoid artifacts
    // in the buffer

    if (close(socketFd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

// client
    int socketFd;
    int port = 12345; // same as port for server

    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(socketFd, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // use fgets(buffer, MSG_SIZE, stdin);
    // and send(socket_fd, buffer, MSG_SIZE, 0); (-1 on error)
    // to send stuff to the server
    // dprintf(FD, "text %s", BUFFER) may also be useful

    if (close(socketFd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

