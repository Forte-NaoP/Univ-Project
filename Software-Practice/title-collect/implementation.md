# **구현 정리**

---

## **1. 구성 요소**

### **1.1 TaskQueue**

wget 작업을 다루는 Task Queue

- **헤더:** `task_queue.h`
- **구현:** `task_queue.c`

**함수:**

- `TaskQueue_init`: TaskQueue 초기화.
- `TaskQueue_push`: Task 추가.
- `TaskQueue_pop`: Task를 Queue에서 하나 꺼냄.
- `TaskQueue_destroy`: Queue에서 사용한 자원을 정리.

**Task Structure:**

```c
typedef struct Task {
    char url[MAX_DOMAIN_SIZE];
    char filename[16];
    bool final; // 스레드 종료를 위한 작업 종료 지시자
    struct Task *next;
} Task;
```

---

### **1.2 스레드 풀**

wget 작업을 병렬로 처리하기 위한 스레드 풀

- **헤더:** `task_queue.h`
- **구현:** `task_queue.c`

**함수:**

- `thread_pool_init`: 스레드 풀 초기화.
- `thread_pool_add_task`: Task Queue에 Task 추가.
- `thread_pool_wait`: 모든 Task 끝날 때까지 대기.
- `thread_pool_destroy`: 큐에 종료 Task를 추가해 모든 스레드 종료 후 자원 정리.

---

### **1.3 메모리 풀**

수집한 Domain, Title 쌍을 저장하기 위한 BST 전용 메모리 풀

- **헤더:** `memory_pool.h`
- **구현:** `memory_pool.c`

**함수:**

- `MemoryPool_init`: 메모리 풀 초기화.
- `MemoryPool_alloc`: 메모리 풀에서 자원 할당.
- `MemoryPool_free`: 메모리 풀로 자원 반환.
- `MemoryPool_destroy`: 메모리 풀 정리.

---

### **1.4 Binary Search Tree (BST)**

수집한 Domain, Title 쌍을 이진 탐색 트리 형태로 저장

- **헤더:** `bst.h`
- **구현:** `bst.c`

**함수:**

- `BST_new`: BST 노드 생성.
- `BST_insert`: Domain, Title 쌍 추가.
- `BST_search`: Domain 검색.
- `BST_print_inorder`: 디버깅 용 BST Inorder 출력 함수.

---

### **1.5 Collect**

과제에서 요구하는 기능들 집합

- **헤더:** `collect.h`
- **구현:** `collect.c`

**함수:**

- `wget`: 페이지를 다운로드하고 title을 추출해서 BST에 저장.
- `find_title`: HTML file에서 title 태그 추출.
- `get_domain_name`: URL에서 Domain Name을 추출.
- `concat_string`: 여러 개의 문자열을 연걸.

---

### **1.6 BackupStack**

load 명령 실행 시 남아있던 기존 명령을 스택에 백업함

- **구현:** `main.c`

**함수:**

- `backup`: 남은 명령을 입력으로부터 저장.
- `restore`: 저장된 명령을 입력으로 복원.

---
