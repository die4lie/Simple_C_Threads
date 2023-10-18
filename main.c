#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREAD_CNT 10

int mutexChecker(pthread_mutex_t *mtx) {
  switch (pthread_mutex_trylock(mtx)) {
  case 0: /* if we got the lock, unlock and return 1 (true) */
    pthread_mutex_unlock(mtx);
    return 1;
  case EBUSY: /* return 0 (false) if the mutex was locked */
    return 0;
  }
  return 1;
}

void *function_1(void *signal) {
  printf("Thread starting!\n");
  pthread_mutex_t *mx = signal;
  while (!mutexChecker(mx)) {
    /*sleep(10);
    printf("Thread 1 running\n");*/
  }
  printf("Thread terminating!\n");
}

void *function_2(void *signal) {
  pthread_mutex_t *mx = signal;
  while (!mutexChecker(mx)) {
    sleep(10);
    printf("Thread 2\n");
  }
}

int main() {
  printf("Start APP!\n");
  pthread_mutex_t mutex_list[THREAD_CNT];

  pthread_t thread_list[THREAD_CNT];
  for (int a = 0; a < THREAD_CNT; a++) {
    pthread_mutex_init(&mutex_list[a], NULL);
  }

  int thread_flag = 0;
  int thread_num = 0;
  int exit_flag = 0;
  int cmd = 0;
  int thread_status[THREAD_CNT] = {0};
  int thread_id = -1;
  char cmd_buf[50];

  printf("run thread:   start <id>\n");
  printf("stop thread:  stop <id>\n");
  printf("list threads: list\r\n");
  printf("exit program: exit\r\n");

  while (1) {

    if (thread_id == -1 && cmd == 0) {
      memset(&cmd_buf[0], 0x00, 50);
      fgets(&cmd_buf[0], 50, stdin);
      printf("entered: %s\r\n", &cmd_buf[0]);
      cmd = 1;
    }


    //check for commands
    if (strncmp(&cmd_buf[0], "start ", 6) == 0 && cmd == 1) {

      // search which thread to stop
      char *pt = &cmd_buf[6];
      int val = atoi(pt);
      // can be error, check if it's error or if it's number
      if (val == 0) {
        if (cmd_buf[6] != '0')
          val = -1;
      }

      if (val >= 0 && val < THREAD_CNT) {
        if (thread_status[val] == 0) {
          thread_id = val;
        }
      }
      cmd = 0;
      memset(&cmd_buf[0], 0x00, 50);
      printf("[START]value:%d , thread_id:%d thread_status[%d]:%d \r\n", val,
             thread_id, thread_id, thread_status[thread_id]);
    } else if (strncmp(&cmd_buf[0], "stop ", 5) == 0 && cmd == 1) {
      // else ignore command and clear cmd buffer
      // search wich thread to stop
      char *pt = &cmd_buf[5];
      int val = atoi(pt);

      // can be error, check if it's error or if it's number
      if (val == 0) {
        if (cmd_buf[5] != '0')
          val = -1;
      }

      if (val >= 0 && val < THREAD_CNT) {
        if (thread_status[val] == 1)
          thread_id = val;
      }
      cmd = 0;
      memset(&cmd_buf[0], 0x00, 50);
      printf("[STOP]value:%d , thread_id:%d thread_status[%d]:%d \r\n", val,
             thread_id, thread_id, thread_status[thread_id]);
    } else if (strncmp(&cmd_buf[0], "list", 4) == 0 && cmd == 1) {
      cmd = 0;
      memset(&cmd_buf[0], 0x00, 50);
      for (int z = 0; z < THREAD_CNT; z++) {
        if (thread_status[z] == 1) {
          printf("THREAD_%d:active\r\n", z);
        } else {
          printf("THREAD_%d:not active\r\n", z);
        }
      }

    } else if (strncmp(&cmd_buf[0], "exit", 4) == 0 && cmd == 1) {
      printf("Exit command!\r\n");
      cmd = 0;
      memset(&cmd_buf[0], 0x00, 50);
      exit_flag = 1;
    } else if (cmd == 1) {
      printf("Uknown command!\r\n");
      cmd = 0;
      memset(&cmd_buf[0], 0x00, 50);
    }


    // thread start mechanism
    if (thread_id >= 0 && thread_status[thread_id] == 0) {
      pthread_mutex_lock(&mutex_list[thread_id]);
      pthread_create(&thread_list[thread_id], NULL, function_1,
                     &mutex_list[thread_id]);
      thread_status[thread_id] = 1;
      thread_id = -1;
    }

    // termination mechanism
    if (thread_id >= 0 && thread_status[thread_id] == 1) {
      // terminate
      thread_status[thread_id] = 0;
      pthread_mutex_unlock(&mutex_list[thread_id]);
      pthread_join(thread_list[thread_id], NULL);
      thread_id = -1;
    }

    //exit mechanism
    if (exit_flag == 1){
      //terminate all active threads
      for (int j = 0; j < THREAD_CNT; j++) {
        if(thread_status[j] == 1){
          thread_status[j] = 0;
          printf("Thread %d is active, terminating\r\n", j);
          pthread_mutex_unlock(&mutex_list[j]);
          pthread_join(thread_list[j], NULL);
        }
      }
      break;
    }
  }

  exit(0);
  return 0;
}