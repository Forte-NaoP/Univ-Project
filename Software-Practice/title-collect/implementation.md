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

## **2. 작업 흐름**

1. 명령어를 읽어들임.
    - 읽지 못했을 경우
        - 현재 입력이 표준 입력이고 남은 명령어가 없을 경우 자원을 정리하고 종료
        - 리다이렉트 됐을 경우
            - 남은 명령어가 없으면 표준 입력으로 복구
            - 남았을 경우 명령어 백업으로부터 복구
    - 읽었을 경우 개행문자 기준으로 토큰화

2. 작업 수행
    - URL의 경우, thread_pool에 작업 추가
    - print, stat의 경우 모든 thread 작업 완료 대기 후 명령 수행
    - load의 경우 load 이후에 남아있는 명령 백업 후 입력 리다이렉션 및 1번으로 이동
    - quit의 경우 자원 정리 후 종료
