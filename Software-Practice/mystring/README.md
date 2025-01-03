# 문자열 라이브러리 구현

## **1. 개요**

- C 표준 라이브러리의 `<string.h>` 및 기타 문자열 관련 함수들을 직접 구현.
- 앞으로의 과제를 위해 문자열 처리와 변환 함수들을 준비.

---

## **2. 구현 함수**

### **2.1 숫자 변환 (Conversions)**

#### 문자열 → 숫자

- `int atoi2(const char *str)`:
  - 문자열 `str`을 `int`로 변환.
  - `int`는 32비트 정수로 간주.
- `long atol2(const char *str)`:
  - 문자열 `str`을 `long`으로 변환.
  - `long`은 64비트 정수로 간주.

#### 숫자 → 문자열

- `char *int2str(char *dest, int num)`:
  - 정수 `num`을 문자열로 변환 후 `dest`에 저장.
  - `dest`가 `NULL`일 경우, 동적으로 메모리를 할당하여 반환.
  - 메모리 할당 실패 시 `NULL` 반환.

---

### **2.2 문자열 조작 (String Manipulation)**

- `char *strcpy(char *dst, const char *src)`:
  - 문자열 `src`를 `dst`로 복사.
- `char *strncpy(char *dst, const char *src, size_t count)`:
  - `src`에서 최대 `count` 바이트를 `dst`로 복사.
- `char *strcat(char *dst, const char *src)`:
  - `dst` 문자열의 끝에 `src`를 붙임.
- `char *strncat(char *dst, const char *src, size_t count)`:
  - `dst` 문자열의 끝에 최대 `count` 바이트만큼 `src`를 붙임.
- `char *strdup(const char *str)`:
  - 문자열 `str`을 복사하여 동적 메모리에 저장하고 해당 포인터 반환.

---

### **2.3 문자열 검사 (String Examination)**

- `size_t strlen(const char *str)`:
  - 문자열 `str`의 길이를 반환.
- `int strcmp(const char *lhs, const char *rhs)`:
  - 두 문자열 `lhs`와 `rhs`를 비교.
- `int strncmp(const char *lhs, const char *rhs, size_t count)`:
  - `lhs`와 `rhs`를 최대 `count` 바이트만큼 비교.
- `char *strchr(const char *str, int ch)`:
  - `str`에서 문자 `ch`가 처음 나타나는 위치를 반환.
- `char *strrchr(const char *str, int ch)`:
  - `str`에서 문자 `ch`가 마지막으로 나타나는 위치를 반환.
- `char *strpbrk(const char *str, const char *accept)`:
  - `str`에서 `accept` 문자열 중 하나라도 처음 등장하는 위치를 반환.
- `char *strstr(const char *str, const char *substr)`:
  - `str`에서 부분 문자열 `substr`이 처음 나타나는 위치를 반환.
- `char *strtok(char *str, const char *delim)`:
  - `str`을 `delim`으로 토큰화.
- `char *strtok_r(char *str, const char *delim, char **saveptr)`:
  - `strtok`와 동일하지만, 내부 상태 대신 `saveptr`에 저장.

---

### **2.4 메모리 조작 (Memory Manipulation)**

- `void *memcpy(void *dest, const void *src, size_t n)`:
  - `src`에서 `dest`로 `n` 바이트 복사.
- `void *memset(void *dest, int ch, size_t count)`:
  - `dest`에서 `count` 바이트를 `ch`로 설정.

---

## **4. 구현 제한 사항**

- C 표준 라이브러리 및 기타 외부 라이브러리 함수 사용 금지.
- **허용된 함수**:
  - `malloc`, `calloc`, `free`를 포함한 일부 메모리 관리 함수.
- 필요한 경우 라이브러리 함수를 직접 구현 가능.
- 동적으로 할당된 모든 자원은 프로그램 종료 전에 반드시 해제.

---
