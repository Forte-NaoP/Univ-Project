# RDT on UDP

---

## 1. 목표

- UDP 소켓 프로그래밍을 활용하여 **신뢰할 수 있는 데이터 전송 (RDT) 프로토콜** 개발을 목표로 한다.
- 단계별로 구현하며, 다음 네 가지 버전을 차례대로 개발한다.
  - **RDT 1.0**
  - **RDT 2.2**
  - **RDT 3.0**
  - **RDT 3.0 + 파이프라이닝**

---

## 2. 구현 세부 정보

상위 버전의 RDT는 이전 버전을 기반으로 구축된다. 따라서 각 버전을 순서대로 구현한다.

### 2.1 공통 기능

- **기본 절차**
  - **일대일 상황**만 가정하며 다중 사용자 사례는 고려하지 않는다.
  - **[Receiver]**  
    - 소켓을 열고 포트 `10090`에 바인딩하여 Sender의 전송을 기다린다.
  - **[Sender]**  
    - Receiver의 IP와 포트 번호로 소켓을 생성하고, 사용자 입력으로 지정된 파일을 전송한다.
    - 파일 전송 완료 후 연결을 종료한다.
  - **[Receiver]**  
    - 파일 전송이 완료되면 연결을 종료한다.

- **매개변수**
  - **[Sender]**: 실행 시 다음 4가지 매개변수를 받는다.
  `<receiver's IP address> <window size> <source file name> <log file name>`
  - **[Receiver]**: 실행 시 다음 2가지 매개변수를 받는다.
  `<result file name> <log file name>`
  - 매개변수가 충분히 제공되지 않으면 프로그램을 종료한다.

- **패킷**
  - 각 RDT 버전에 맞게 **사용자 지정 패킷 헤더**를 설계한다.
  - 페이로드의 크기가 너무 커서 헤더 공간이 부족해지지 않도록 주의한다.
  - **비트 오류**와 **패킷 손실**은 **Sender 측**에서만 발생한다고 가정한다.  

- **로그 파일**
  - Sender와 Receiver 모두 로그 파일을 작성해야 한다.
  - 제공되는 `logHandler.py` 또는 `logHandler_fct.py` 중 하나를 사용하여 구현하도록 한다.

- **라이브러리 사용**
  - **비표준 라이브러리**는 사용 금지.
  - HTTP 직접 처리 등 과제 해결에 직접적으로 영향을 주는 라이브러리도 사용 금지.
  - 허용 모듈 예시: `sys`, `os`, `threading`, `time`, `socket`

---

### 2.2 각 RDT 프로토콜 버전

#### RDT 1.0

- **가정:** 신뢰할 수 있는 채널
- **기능:** Receiver가 Sender로부터 지정된 파일을 받음
- **특징:**
  - 헤더는 고려하지 않는다
  - `window size`는 **1**로 고정

---

#### RDT 2.2

- **요구사항:** RDT 1.0의 모든 기능 포함
- **가정:** 기본 채널에서 비트 오류가 발생할 수 있음
- **기능:**
  - UDP 방식의 Check Sum 계산을 통해 패킷 손상 여부 확인
  - **NAK-free 프로토콜**을 구현하여 정상 패킷과 손상된 패킷 모두 올바르게 처리
- **특징:** `window size`는 **1**로 고정

---

#### RDT 3.0

- **요구사항:** RDT 2.2의 모든 기능 포함
- **가정:** 기본 채널에서 **패킷 손실**이 발생할 수 있음
- **기능:**
  - Sender가 0.01초 동안 ACK를 수신하지 못하면 해당 패킷을 손실로 간주하고 재전송
  - Sender와 Receiver 모두 중복된 패킷을 처리해야 함
- **특징:** `window size`는 **1**로 고정

---

#### RDT 3.0 + 파이프라이닝

- **요구사항:** RDT 3.0의 모든 기능 포함
- **가정:** RDT 3.0에 **파이프라이닝**을 적용
- **기능:**
  - Sender는 ACK를 기다리지 않고 `window size`에 맞게 여러 패킷을 전송 가능  
    → `window size`는 Sender가 전송할 수 있는 **미확인 패킷의 최대 개수**
  - **Selective Repeat** 전략을 사용하여 패킷 손실 복구 수행  
  - Receiver도 window size에 맞춘 버퍼를 사용하며, Sender로부터 입력받은 window size를 공유하여 동기화를 맞춤
    - 단, 이 소켓 통신은 신뢰할 수 있다고 가정 (비트 오류와 패킷 손실 없음)
  - **테스트:** window size는 최소 **5**에서 최대 **10** 사이의 값으로 시험한다. **Selective Response 딜레마**에 빠지지 않는 **최소 Sequence Number Size**를 사용하도록 한다.
  - **가정:** Sender가 window 크기만큼 패킷을 전송하기 전에 ACK가 도착하지 않을 정도로 RTT(왕복 시간)가 항상 충분히 큼

---

### 3.3 요약 표

|                   | RDT 1.0 | RDT 2.2 | RDT 3.0 | RDT 3.0 + 파이프라이닝 |
|-------------------|:-------:|:-------:|:-------:|:----------------------:|
| **소켓**        |    O    |    O    |    O    |           O            |
| **헤더**        |    X    |    O    |    O    |           O            |
| **타이머**      |    X    |    X    |    O    |           O            |
| **파이프라이닝**|    X    |    X    |    X    |           O            |

---

## 4. 시나리오

### 전반 사항

- 전송 및 로깅 기능을 위한 코드가 제공된다.
- **PASender.py**  
  - `config.txt`의 값을 편집하여 다양한 비트 오류 및 패킷 손실 상황을 시뮬레이션할 수 있다.
  - Sender 측에서는 이 PASender를 활용하여 손실 환경을 구성한다.
  - Receiver 측은 비트 오류와 패킷 손실이 없는 것으로 가정하고 기본 전송 기능을 사용한다.

- **logHandler.py / logHandler_fct.py**  
  - 전체 패킷 처리 과정을 기록하는 로그 파일을 생성한다.

- **make_test_file.py**  
  - 전송할 테스트 파일을 생성할 수 있는 스크립트

---

### 각 버전별 시나리오

#### RDT 1.0

- **파일 크기:** 약 1KB의 작은 파일 전송
- **조건:** 비트 오류 및 패킷 손실 없음 (loss_rate = 0, corrupt_rate = 0)
- **예시 로그 (Sender 측):**
  ```
  0.000 pkt: 0 | Send DATA
  0.001 pkt: 0 | Send DATA
  File transfer is finished.
  ```
- **특이사항:** RDT 1.0은 Sender 측 로그만 작성해도 된다.

---

#### RDT 2.2

- **파일 크기:** 약 1MB의 중간 크기 파일 전송
- **조건:** 패킷 손상 처리, NAK 방지 필요
- **예시 로그:**
  - **Sender 측:**
    ```
    0.000 pkt: 0 | Send DATA
    0.006 pkt: 0 | Sent Successfully
    0.007 pkt: 1 | Send DATA
    0.014 pkt: 1 | Wrong Sequence Number
    0.015 pkt: 1 | Send DATA Again
    0.021 pkt: 1 | Sent Successfully
    File transfer is finished.
    ```
  - **Receiver 측:**  
    ```
    0.003 ACK: 0 | Send ACK 
    0.010 ACK: 1 | DATA Corrupted 
    0.011 ACK: 0 | Send ACK Again 
    0.018 ACK: 1 | Send ACK 
    
    File transfer is finished. 
    ```

---

#### RDT 3.0

- **파일 크기:** 약 50MB의 대용량 파일 전송
- **조건:** 패킷 드롭(손실) 처리 필요
- **예시 로그:**
  - **Sender 측:**
    ```
    0.000 pkt: 0 | Send DATA 
    0.010 pkt: 0 | TIMEOUT 
    0.012 pkt: 0 | Send DATA Again 
    0.018 pkt: 0 | Sent Successfully 
 
    File transfer is finished.
    ```
  - **Receiver 측:**  
    ```
    0.015 ACK: 0 | Send ACK 
 
    File transfer is finished. 
    ```
---

#### RDT 3.0 + 파이프라이닝

- **파일 크기:** 약 50MB의 대용량 파일 전송
- **조건:** 파이프라이닝 및 Selective Repeat 수행
- **예시 로그 (일부):**
  - **Sender 측:**
    ```
    0.000 pkt: 0 | Send DATA 
    0.001 pkt: 1 | Send DATA 
    0.002 pkt: 2 | Send DATA 
    0.003 pkt: 3 | Send DATA 
    0.004 pkt: 0 | Sent Successfully 
    0.005 pkt: 4 | Send DATA 
    0.005 pkt: 1 | Sent Successfully 
    0.007 pkt: 3 | Wrong Sequence 
    0.007 pkt: 3 | Sent Successfully: Mark 
    0.007 pkt: 5 | Send DATA 
    0.009 pkt: 4 | Wrong Sequence 
    0.009 pkt: 4 | Sent Successfully: Mark 
    0.011 pkt: 5 | Wrong Sequence 
    0.011 pkt: 5 | Sent Successfully: Mark 
    0.012 pkt: 2 | TIMEOUT 
    0.013 pkt: 2 | Send DATA Again 
    0.017 pkt: 2 | Sent Successfully 
    
    File transfer is finished.
    ```
  - **Receiver 측:**
    ```
    0.002 ACK: 0 | Send ACK 
    0.003 ACK: 1 | Send ACK 
    0.005 ACK: 3 | Wrong Sequence: Buffer 
    0.005 ACK: 3 | Send ACK 
    0.007 ACK: 4 | Wrong Sequence: Buffer 
    0.007 ACK: 4 | Send ACK 
    0.009 ACK: 5 | Wrong Sequence: Buffer 
    0.009 ACK: 5 | Send ACK 
    0.015 ACK: 2 | Send ACK 
    
    File transfer is finished.
    ```

---