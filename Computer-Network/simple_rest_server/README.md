# Simple REST Server

## **1. 개요**

- 간단한 REST 서버를 만들어 다음 요청에 대한 처리를 수행한다.
  - 특정 메세지 응답
  - 사용자 정보 관리

---

## **2. 기능 명세**

### 기본 사항

- **서버 주소:** `http://localhost:1398`
- **실행 환경:** Python 3.9 (Windows 10, x86-64 CPU)
- **제한 사항:** HTTP 직접 처리 모듈(HTTP, requests, flask 등) 사용 금지
- **가능 모듈:** sys, os, threading, time, socket, json 등

### 고정 메시지 응답

- **URL:** `/hi`
- **Method:** GET
- **Response:**
  - **Status Code:** `200`
  - **Body:**
    ```json
    {"message": "hi"}
    ```

### 요청 메시지 반환

- **URL:** `/echo`
- **Method:** POST
- **Request Example:**
  ```json
  {"message": "hello"}
  ```
- **Response:**
  - **Status Code:** `200`
  - **Body:** (요청 받은 메시지 그대로 반환)

### 사용자 정보 관리

사용자 정보는 서버 실행 중에만 유지되도록 한다.

- 구성 요소: `id`(고유), `name`, `gender`(소문자)

#### 사용자 정보 조회

- **URL:** `/user?id=<id>`
- **Method:** GET
- **Response:**
  - 등록된 사용자:
    - **Status Code:** `200`
    - **Body:**
      ```json
      {"id":"<id>", "name":"<name>", "gender":"<gender>"}
      ```
  - 등록되지 않은 사용자:
    - **Status Code:** `404`

#### 사용자 정보 등록

- **URL:** `/user`
- **Method:** POST
- **Request Body:** `id`, `name`, `gender` 필수 포함
- **Response:**
  - 유효하고 신규 ID:
    - **Status Code:** `201`
  - 이미 존재하는 ID:
    - **Status Code:** `409`
  - 필수 정보 누락:
    - **Status Code:** `400`

#### 사용자 정보 삭제

- **URL:** `/user/<id>`
- **Method:** DELETE
- **Response:**
  - 등록된 사용자:
    - **Status Code:** `200`
  - 등록되지 않은 사용자:
    - **Status Code:** `404`

#### 사용자 정보 수정

- **URL:** `/user/<id>`
- **Method:** PUT
- **Request Body:**
  - 수정 가능한 정보: `name`만 포함 가능
  - `id`나 `gender`가 포함되거나 `name`이 없으면 무효
- **Response:**
  - 유효하고 유의미한 수정:
    - **Status Code:** `200`
  - 무효한 요청 (필수 항목 누락 등):
    - **Status Code:** `400`
  - 무의미한 요청 (기존 값과 동일한 경우):
    - **Status Code:** `422`
  - 등록되지 않은 사용자:
    - **Status Code:** `404`

### 추가 처리 사항

- **Preflight 요청:** (테스트 7 이후)

  - 별도의 Preflight 처리 필요

- **무효한 URL 요청:**

  - 정의되지 않은 모든 주소:
    - **Status Code:** `404`

### 필수 응답 헤더

- 모든 응답 시 필수 헤더:
  ```
  Access-Control-Allow-Origin: *
  ```