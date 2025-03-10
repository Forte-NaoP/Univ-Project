# HTTP 패킷 구조

---

## 요청(Request) 패킷 구조
- **Request-Line:**  
  첫 번째 줄에 HTTP 메서드, 요청 URL, 프로토콜 버전이 공백으로 구분됨.  
  예:  
  ```
  GET /hi HTTP/1.1
  ```
- **Header Fields:**  
  요청에 대한 추가 정보를 제공하는 헤더들이 있으며, 각 헤더는 "키: 값" 형식으로 작성된다.  
  예:  
  ```
  Host: localhost:1398
  User-Agent: CustomClient/1.0
  Accept: application/json
  ```
- **빈 줄(CRLF):**  
  헤더와 본문(body) 사이에 반드시 빈 줄(즉, CRLF 두 번)이 들어간다.
- **본문(Body):**  
  POST나 PUT 요청 시 전달하는 데이터가 포함된다. (GET 요청은 보통 본문이 없음)

---

## 응답(Response) 패킷 구조
- **Status-Line:**  
  첫 번째 줄에 HTTP 버전, 상태 코드, 상태 메시지가 공백으로 구분되어 나타남.  
  예:  
  ```
  HTTP/1.1 200 OK
  ```
- **Header Fields:**  
  응답에 대한 메타 정보(예: Content-Type, Content-Length, Date, Access-Control-Allow-Origin 등)를 포함한다.
- **빈 줄(CRLF):**  
  헤더와 본문 사이에 빈 줄이 들어감.
- **본문(Body):**  
  요청 결과에 대한 데이터(예: JSON 형식의 문자열)가 포함된다.
