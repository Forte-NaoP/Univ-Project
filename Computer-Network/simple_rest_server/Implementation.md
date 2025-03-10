# Implementation

HTTP 모듈(Flask 등)을 사용하지 않고, Python의 기본 socket 모듈과 스레딩(threading) 등을 이용하여 간단한 RESTful API를 구현한다. 전체적으로 사용자 정보 관리와 고정 메시지 응답, 그리고 Preflight 요청(CORS 처리) 등을 지원하며, 각 HTTP 메서드(GET, POST, PUT, DELETE, OPTIONS)에 대해 별도의 핸들러를 구현하였다.

---

## 1. main.py

- **역할:**  
  서버의 엔트리 포인트로, 소켓을 생성하고 클라이언트 연결을 기다리며 각 연결마다 새로운 스레드를 생성하여 요청을 처리함.

- **구현:**  
  - **소켓 생성 및 바인딩:**  
    - `HOST`와 `PORT`를 설정하고, 소켓 옵션 `SO_REUSEADDR`를 설정하여 주소 재사용을 허용함.
  - **신호 처리:**  
    - SIGINT (Ctrl+C) 시, 서버 소켓을 닫고 프로그램 종료를 수행.
  - **클라이언트 처리:**  
    - 무한 루프 내에서 클라이언트의 연결을 accept하고, 각 클라이언트에 대해 `RequestHandler` 인스턴스를 생성한 후, 별도의 스레드로 `handler.run()`을 실행하여 비동기적으로 요청을 처리하도록 함.

---

## 2. request_handler.py

- **역할:**  
  클라이언트로부터 수신한 HTTP 패킷을 파싱하고, 적절한 HTTP 메서드 핸들러(GET, POST, OPTIONS, PUT, DELETE 등)를 호출하여 응답을 생성함.

- **핵심 내용:**  
  - **공유 변수와 동기화:**  
    - `USER`: 사용자 정보를 저장하는 전역 딕셔너리(서버 실행 동안만 유지).
    - `LOCK`: 여러 스레드에서 사용자 정보를 안전하게 접근하기 위한 락.
  - **패킷 파서:**  
    - `packet_parser()` 함수는 수신한 HTTP 패킷을 헤더와 바디로 분리한다. 헤더 부분은 각 줄을 ":" 기준으로 키/값으로 파싱하여 딕셔너리로 변환한다.
  - **RequestHandler 클래스:**  
    - 생성자에서 각 HTTP 메서드(GET, POST, OPTIONS, PUT, DELETE)에 대해 해당하는 모듈의 `handler` 함수를 매핑한다.
    - `run()` 메서드에서는 소켓으로부터 데이터를 읽어와 패킷을 파싱하고, 요청된 HTTP 메서드에 해당하는 핸들러를 호출한 후, 생성된 응답 패킷을 클라이언트에게 전송한다.
    - 만약 지원하지 않는 메서드가 요청되면, `NotAllowed` 핸들러를 호출하여 405 응답을 반환한다.

---

## 3. response.py

- **역할:**  
  HTTP 응답 메시지를 생성하는 유틸리티 함수들을 제공한다.

- **핵심 내용:**  
  - **상태 코드 매핑:**  
    - `STATUS_CODE` 딕셔너리로 각 상태 코드(200, 201, 400, 404 등)를 HTTP/1.1 상태 메시지와 매핑함.
  - **헤더 생성:**  
    - `create_header()` 함수는 HTTP 버전, 상태 코드, 서버 정보, 필수 헤더(Access-Control-Allow-Origin, Date 등)를 포함하는 응답 헤더를 생성한다.
  - **전체 응답 생성:**  
    - `create_response()` 함수는 지정된 코드와 (선택적) 응답 본문, 그리고 Preflight 요청일 경우 필요한 CORS 관련 옵션(Access-Control-Allow-Methods, Access-Control-Allow-Headers 등)을 처리하여 최종 응답 패킷을 구성한 후 바이트 문자열로 반환한다.

---

## 4. method_handler.py

- **역할:**  
  모든 HTTP 메서드 핸들러가 상속받아야 하는 추상 기본 클래스(ABC)를 정의함.

- **핵심 내용:**  
  - `Method` 클래스는 `handler`라는 클래스 메서드를 정의하고 있으며, 구체적인 메서드(GET, POST, PUT, DELETE, OPTIONS 등)는 이를 오버라이드하여 구현한다.
  - 이를 통해 모든 메서드 핸들러가 동일한 인터페이스를 갖도록 강제하였음.

---

## 5. delete.py

- **역할:**  
  DELETE HTTP 메서드를 처리하며, 사용자 정보 삭제 요청을 처리함.

- **핵심 내용:**  
  - 경로가 `/user/<id>`인 경우, URL에서 사용자 id를 추출하여, 공유 USER 딕셔너리에서 해당 사용자를 삭제함.
  - 동기화(LOCK)를 사용해 여러 스레드에서 동시에 접근할 때 안전하게 처리.
  - 사용자가 존재하지 않으면 404, 존재하면 삭제 후 200 응답을 반환함.

---

## 6. get.py

- **역할:**  
  GET HTTP 메서드를 처리합니다. 두 가지 주요 엔드포인트 `/hi`와 `/user`에 대해 각각 다른 응답을 반환한다.

- **핵심 내용:**  
  - **`/hi` 엔드포인트:**  
    - 고정 메시지 응답으로, JSON 형식으로 `{"message": "hi"}`를 반환함.
  - **`/user` 엔드포인트:**  
    - URL 쿼리 스트링에서 `id` 값을 추출하여, 해당 사용자가 USER 딕셔너리에 존재하는지 확인.
    - 사용자 정보가 존재하면 JSON 형태로 반환(200), 없으면 404 응답을 반환.

---

## 7. not_allowed.py

- **역할:**  
  지원하지 않는 HTTP 메서드에 대해 405 Method Not Allowed 응답을 생성한다.

- **핵심 내용:**  
  - `NotAllowed` 클래스는 `Method`를 상속받아, 호출 시 `response.create_response(405)`를 반환함.
  - RequestHandler에서 정의되지 않은 메서드가 요청되었을 때 이 핸들러가 호출되도록 함.

---

## 8. options.py

- **역할:**  
  OPTIONS HTTP 메서드를 처리하여, 클라이언트에서 Preflight 요청 시 필요한 CORS 정보를 제공한다.

- **핵심 내용:**  
  - 요청 URL이 `/user`로 시작하는 경우, URL의 세부 경로에 따라 허용되는 메서드와 헤더가 달라집니다.
    - **사용자 목록(컬렉션) 요청**: `/user`인 경우, 허용 메서드로 `GET`, `POST`, `OPTIONS`를 반환한다.
    - **특정 사용자 요청**: `/user/<id>`인 경우, 허용 메서드로 `DELETE`, `PUT`, `OPTIONS`를 반환한다.
  - 응답에 `Access-Control-Allow-Methods`, `Access-Control-Allow-Headers`, `Access-Control-Max-Age` 등의 헤더를 포함시켜 Preflight 요청을 처리함.

---

## 9. post.py

- **역할:**  
  POST HTTP 메서드를 처리한다.
  
- **핵심 내용:**  
  - **`/echo` 엔드포인트:**  
    - 클라이언트가 보낸 본문(body)을 그대로 응답으로 반환한다.
  - **`/user` 엔드포인트 (사용자 등록):**  
    - 요청 본문을 JSON으로 파싱하여 `id`, `name`, `gender` 필드가 모두 존재하는지 확인함.
    - 필수 필드가 누락되면 400 Bad Request 응답을 반환.
    - 이미 동일한 id가 존재하면 409 Conflict 응답 반환.
    - 모든 조건이 충족되면, LOCK을 사용해 USER 딕셔너리에 신규 사용자를 추가하고 201 Created 응답을 반환한다.

---

## 10. put.py

- **역할:**  
  PUT HTTP 메서드를 처리하여, 특정 사용자의 정보를 수정한다.

- **핵심 내용:**  
  - URL에서 `/user/<id>` 형식으로 사용자 id를 추출한다.
  - 요청 본문을 JSON으로 파싱한 후, 수정 가능한 정보가 올바른지 검증함.
    - 수정 가능 정보는 오직 `name`만 허용된다.
    - 만약 JSON 데이터에 `id`나 `gender`가 포함되거나, `name` 필드가 누락된 경우 400 Bad Request를 반환하도록 함.
  - 현재 저장된 이름과 동일한 경우엔 무의미한 수정으로 간주하여 422 Unprocessable Entity 응답을 반환한다.
  - 올바른 요청이면 사용자 정보를 업데이트하고 200 OK 응답을 반환한다.

---