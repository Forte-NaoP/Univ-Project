# **OpenGL API**

OpenGL 파이프라인의 각 단계에서 데이터를 설정하거나 렌더링을 제어하는 역할을 하는 API 정리

---

## **1. 데이터 관리 및 설정 관련 API**

### **`glGenBuffers`**

- **역할**: 하나 이상의 버퍼 객체를 생성하고 고유한 ID를 할당.
- **사용 예**:

  ```cpp
  glGenBuffers(1, &vertex_buffer);
  ```

  - 새로운 버퍼 객체를 생성하고 `vertex_buffer`에 ID를 저장.

---

### **`glBindBuffer`**

- **함수 정의**

    ```cpp
    void glBindBuffer(GLenum target, GLuint buffer);
    ```

  - **`target`**: 버퍼 객체가 어떤 용도로 사용될지 지정하는 **버퍼 타입**.
  - **`buffer`**: 바인딩할 버퍼 객체의 ID. (`glGenBuffers`로 생성된 ID).

- **역할**: 생성된 버퍼 객체를 특정 버퍼 타입(GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER 등)과 바인딩.
- **사용 예**:

  ```cpp
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  ```

  - `vertex_buffer`를 현재 활성화된 **GL_ARRAY_BUFFER**로 설정.
  
- **주요 버퍼 타입 (`target`)**
  - **`GL_ARRAY_BUFFER`**
    - **역할**: 정점 데이터(Vertex Data)를 저장.
    - **사용 데이터**:
      - 정점 위치(Position), 색상(Color), 텍스처 좌표(Texture Coordinates), 법선 벡터(Normal) 등.
    - **적용 대상**:
      - **버텍스 셰이더**에서 입력으로 사용되는 데이터.


  - **`GL_ELEMENT_ARRAY_BUFFER`**
    - **역할**: 인덱스 데이터(Index Data)를 저장.
    - **사용 데이터**:
      - 삼각형의 정점 인덱스 배열.
      - 정점 데이터를 재사용하여 메모리 절약 및 렌더링 효율성 향상.
    - **적용 대상**:
      - **`glDrawElements`** 및 **`glDrawElementsInstanced`** 호출에서 사용.

  - **`GL_UNIFORM_BUFFER`**
    - **역할**: 유니폼 데이터(Uniform Data)를 저장.
    - **사용 데이터**:
      - 여러 셰이더에서 공유하는 전역 데이터(예: 변환 행렬, 조명 데이터 등).
    - **적용 대상**:
      - **유니폼 블록(Uniform Block)** 으로 선언된 데이터를 전달.

  - **(4) `GL_TEXTURE_BUFFER`**
    - **역할**: 텍스처 데이터(Texture Data)를 저장.
    - **사용 데이터**:
      - 텍스처로 사용될 1D 배열 데이터를 저장.
    - **적용 대상**:
      - 텍스처 버퍼를 사용할 때.

---

### **`glBufferData`**

- **역할**: 바인딩된 버퍼 객체에 데이터를 업로드.
- **사용 예**:

  ```cpp
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);
  ```

  - `vertex_list`의 데이터를 GPU로 복사.
  - `GL_STATIC_DRAW`: 데이터가 자주 변경되지 않음을 명시.

---

### **`glGetAttribLocation`**

- **역할**: 셰이더 프로그램에서 특정 속성(attribute)의 위치를 가져옴.
- **사용 예**:

  ```cpp
  GLuint loc = glGetAttribLocation(program, "position");
  ```

  - 셰이더 프로그램(`program`)에서 `"position"` 속성의 위치를 반환.

---

### **`glEnableVertexAttribArray`**

- **역할**: 특정 속성을 활성화하여 GPU가 데이터를 사용할 수 있도록 설정.
- **사용 예**:

  ```cpp
  glEnableVertexAttribArray(loc);
  ```

  - 속성(`loc`)을 활성화.

---

### **`glVertexAttribPointer`**

- **역할**: 셰이더 속성에 데이터의 레이아웃(구조)을 정의.
- **사용 예**:

  ```cpp
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)0);
  ```

  - 셰이더 속성 `loc`에 대해:
    - `3`: 데이터 개수(예: `vec3`).
    - `GL_FLOAT`: 데이터 타입.
    - `GL_FALSE`: 정규화 여부.
    - `sizeof(vertex)`: 정점 데이터의 크기(Stride).
    - `(GLvoid*)0`: 데이터의 시작 위치(Offset).

---

### **`glGetUniformLocation`**

- **역할**: 셰이더 프로그램에서 특정 유니폼 변수의 위치를 가져옴.
- **사용 예**:

  ```cpp
  GLint uloc = glGetUniformLocation(program, "radius");
  ```

  - `"radius"` 유니폼 변수의 위치를 반환.

---

### **`glUniform*`**

- **역할**: 특정 유니폼 변수에 값을 설정.
- **사용 예**:

  ```cpp
  glUniform1fv(uloc, NUM_CIRCLES, radius);
  ```

  - `uloc` 위치에 `radius` 데이터를 전달.

---

## **2. 셰이더 관련 API**

### **`glCreateProgram`**

- **역할**: 새로운 셰이더 프로그램 객체를 생성.
- **사용 예**:

  ```cpp
  GLuint program = glCreateProgram();
  ```

  - 셰이더 프로그램 ID를 생성.

---

### **`glUseProgram`**

- **역할**: 특정 셰이더 프로그램을 활성화.
- **사용 예**:

  ```cpp
  glUseProgram(program);
  ```

  - 현재 렌더링에 사용할 셰이더 프로그램을 설정.

---

## **3. 렌더링 관련 API**

### **`glClear`**

- **역할**: 컬러 버퍼, 깊이 버퍼 등을 초기화.
- **사용 예**:

  ```cpp
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ```

  - 컬러 버퍼와 깊이 버퍼를 초기화.

---

### **`glDrawElementsInstanced`**

- **역할**: 인덱스 버퍼를 기반으로 다수의 인스턴스를 렌더링.
- **사용 예**:

  ```cpp
  glDrawElementsInstanced(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr, 21);
  ```

  - `GL_TRIANGLES`: 삼각형 렌더링.
  - `index_list.size()`: 인덱스 데이터 개수.
  - `GL_UNSIGNED_INT`: 인덱스 데이터 타입.
  - `21`: 인스턴스의 개수.

---

### **`glDrawArrays`**

- **역할**: 정점 데이터를 순서대로 읽어와 렌더링.
- **사용 예**:

  ```cpp
  glDrawArrays(GL_TRIANGLES, 0, NUM_TESS * 3);
  ```

  - `GL_TRIANGLES`: 삼각형 렌더링.
  - `0`: 첫 번째 정점부터 시작.
  - `NUM_TESS * 3`: 정점 개수.

---

### **`glfwSwapBuffers`**

- **역할**: 더블 버퍼링 방식에서 현재 렌더링된 버퍼를 화면에 출력.
- **사용 예**:

  ```cpp
  glfwSwapBuffers(window);
  ```

---

## **4. 윈도우 및 이벤트 관련 API**

### **`glfwSetWindowSizeCallback`**

- **역할**: 윈도우 크기 변경 시 호출되는 콜백 함수를 설정.
- **사용 예**:

  ```cpp
  glfwSetWindowSizeCallback(window, reshape);
  ```

---

### **`glfwPollEvents`**

- **역할**: 이벤트 큐에서 대기 중인 이벤트를 처리.
- **사용 예**:

  ```cpp
  glfwPollEvents();
  ```

---

## **요약**

| API                         | 역할                                         | 관련 작업               |
|-----------------------------|----------------------------------------------|-------------------------|
| `glGenBuffers` / `glBindBuffer` | 버퍼 생성 및 바인딩                          | GPU 메모리 관리         |
| `glBufferData`              | 버퍼에 데이터 업로드                         | 정점, 인덱스 데이터 전송|
| `glGetAttribLocation`       | 셰이더 속성 위치 가져오기                     | 속성 바인딩             |
| `glEnableVertexAttribArray` | 속성 활성화                                  | 속성 사용 설정          |
| `glVertexAttribPointer`     | 속성과 데이터 연결                           | 셰이더 데이터 매핑      |
| `glGetUniformLocation`      | 유니폼 변수 위치 가져오기                    | 셰이더 변수 설정        |
| `glUniform*`                | 유니폼 값 전달                               | 값 설정                 |
| `glDrawElementsInstanced`   | 인스턴스 렌더링                              | 다중 객체 렌더링        |
| `glDrawArrays`              | 정점 순차 렌더링                             | 단일 객체 렌더링        |
| `glClear`                   | 화면 초기화                                  | 화면 준비               |
| `glfwSwapBuffers`           | 렌더링된 화면 출력                          | 더블 버퍼링             |
