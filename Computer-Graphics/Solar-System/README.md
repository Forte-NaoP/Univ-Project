# 구현

---

## **데이터 초기화**

행성과 링의 데이터를 초기화하는 작업은 `user_init()` 함수에서 이루어집니다. 이 함수는 프로그램이 시작될 때 호출되어 필요한 데이터와 그래픽 리소스를 준비합니다.

### **1. 초기 위치 설정**

- **`init_pos` 초기화**:

  ```cpp
  for (int i = 0; i < NUM_SPHERE; i++) {
      init_pos[i] = (float)rand() / (float)RAND_MAX * 2 * PI;
  }
  ```

  - 각 행성의 초기 공전 위치를 랜덤 값으로 설정.
  - 초기 위치는 [0, 2π] 사이의 값으로, 행성이 시뮬레이션 시작 시 서로 다른 위치에 배치되도록 보장.

### **2. OpenGL 초기화**

- **텍스처 로드**:

  ```cpp
  glGenTextures(19, textures);
  for (uint i = 0; i < 19; i++) {
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      int width, height, comp = 3;
      unsigned char* pimage0 = stbi_load(meshes[i], &width, &height, &comp, 3);
      // 텍스처 데이터를 GPU에 업로드
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);
      glGenerateMipmap(GL_TEXTURE_2D);
      // 텍스처 파라미터 설정
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
  ```

  - 각 행성에 대한 텍스처 이미지(`meshes` 배열)를 읽어들여 OpenGL 텍스처로 설정.

- **정점 데이터 초기화**:

  ```cpp
  update_circle_vertices(NUM_TESS);
  update_vertex_buffer(NUM_TESS);
  update_ring_vertices(NUM_TESS);
  update_ring_vertex_buffer(NUM_TESS);
  ```

  - 행성과 링을 구성하는 정점 데이터를 생성하고, 이를 GPU 버퍼에 업로드.
  - 행성은 구체로, 링은 원형의 평면으로 정의.

  - **`update_circle_vertices(NUM_TESS)`**
    - **구체의 정점 데이터 생성**

    ```cpp
        for (int i = 0; i <= NUM_TESS; i++) {
            float theta = PI * i / NUM_TESS; // 위도
            for (int j = 0; j <= NUM_TESS; j++) {
                float p = PI * 2.0f / float(NUM_TESS) * float(j); // 경도
                float x = sin(theta) * cos(p); float y = sin(theta) * sin(p); float z = cos(theta);
                vertex_list.push_back({ vec3(x, y, z), vec3(x, y, z), vec2((p / (2 * PI)), (1.0f - (theta / PI))) });
                if (i == 0 || i == NUM_TESS) break;
            }
        }
    ```

    - [구면좌표계 설명](https://ko.wikipedia.org/wiki/%EA%B5%AC%EB%A9%B4%EC%A2%8C%ED%91%9C%EA%B3%84)
    - 위도(`theta`)와 경도(`p`)를 이용하여 구체 표면의 정점 좌표 `(x, y, z)`를 계산.
    - 정점의 위치와 법선 벡터(normal)는 동일하게 `(x, y, z)`로 설정 (구의 중심으로부터의 방향).
    - 텍스처 좌표(`texcoord`):
    - `u = p / (2π)`: 경도에 따른 수평 텍스처 좌표.
    - `v = 1 - theta / π`: 위도에 따른 수직 텍스처 좌표.
    - 극지방(위도 `theta = 0` 또는 `theta = π`)의 정점은 반복되지 않도록 처리.

    ```cpp
    if (i == 0 || i == NUM_TESS) break;
    ```

  - **2. `update_vertex_buffer(NUM_TESS)`**
    - **구체의 정점 데이터를 GPU로 전송**
    - `update_circle_vertices()`에서 생성된 정점 데이터를 GPU에 업로드하여 렌더링에 사용.

    - **이전 버퍼 삭제**:

        ```cpp
        if (vertex_buffer) glDeleteBuffers(1, &vertex_buffer); vertex_buffer = 0;
        if (index_buffer) glDeleteBuffers(1, &index_buffer); index_buffer = 0;
        ```

      - 이전에 사용했던 정점 및 인덱스 버퍼를 삭제하여 메모리 낭비를 방지.

    - **인덱스 배열 생성**:
      - 삼각형으로 구를 렌더링하기 위해 **인덱스 리스트**를 생성.

      ```cpp
        index_list.clear();
        for (uint i = 0; i < N; i++) {
            index_list.push_back(0);
            index_list.push_back(i + 1);
            index_list.push_back(i + 2);
        }
        uint k;
        for (k = 1; k < (N + 1) * (N - 2); k += (N + 1)) {
            for (uint j = k; j < k + N; j++) {
                index_list.push_back(j);
                index_list.push_back(j + N + 1);
                index_list.push_back(j + N + 2);

                index_list.push_back(j);
                index_list.push_back(j + N + 2);
                index_list.push_back(j + 1);
            }
        }
        for (uint i = k; i < k + N; i++) {
            index_list.push_back(i);
            index_list.push_back(k + 1 + N);
            index_list.push_back(i + 1);
        }
      ```

    - 삼각형을 구성하는 정점의 인덱스를 정의:
      - 구의 각 위도와 경도 사이의 사각형을 두 개의 삼각형으로 분할.
      - 각 삼각형은 세 개의 정점 인덱스로 정의.

    - **GPU에 버퍼 업로드**:

      ```cpp
      glGenBuffers(1, &vertex_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

      glGenBuffers(1, &index_buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*index_list.size(), &index_list[0], GL_STATIC_DRAW);
      ```

      - 생성된 정점(`vertex_list`)과 인덱스(`index_list`) 데이터를 GPU로 전송.

  - **3. `update_ring_vertices(NUM_TESS)`**
    - **링(고리)의 정점 데이터 생성**
      - 링은 평면에 그려짐.

    - **링의 정점 계산**:

    ```cpp
    ring_vertex_list.clear();
    for (uint i = 0; i <= N; i++) {
        float theta = 2.0f * PI * i / float(N); // 원형 각도
        float x = cos(theta); float y = sin(theta);
        ring_vertex_list.push_back({ vec3(0, 0, 0), vec3(x, y, 0), vec2(0, 0) });   // 내부 원
        ring_vertex_list.push_back({ vec3(x, y, 0), vec3(x, y, 0), vec2(1, 0) });   // 외부 원
    }
    ```

    - 링의 정점은 내부 원과 외부 원의 좌표로 구성.
    - `vec3(x, y, 0)`은 각도를 기준으로 계산된 정점의 위치.

  - **4. `update_ring_vertex_buffer(NUM_TESS)`**
    - **링 정점 데이터를 GPU로 전송**   

    - **앞면(정면) 정의**:

    ```cpp
    ring_index_list.push_back(2 * i);
    ring_index_list.push_back(2 * i + 1);
    ring_index_list.push_back(2 * (i + 1));

    ring_index_list.push_back(2 * (i + 1));
    ring_index_list.push_back(2 * i + 1);
    ring_index_list.push_back(2 * (i + 1) + 1);
    ```

    - 여기서 정의된 삼각형은 링의 앞면(시계 방향)을 그림.

    - **뒷면 정의**:

    ```cpp
    ring_index_list.push_back(2 * i);
    ring_index_list.push_back(2 * (i + 1));
    ring_index_list.push_back(2 * i + 1);

    ring_index_list.push_back(2 * (i + 1));
    ring_index_list.push_back(2 * (i + 1) + 1);
    ring_index_list.push_back(2 * i + 1);
    ```

    - 여기서는 앞면 삼각형과 동일한 영역을 반대 방향(반시계 방향)으로 정의하여 뒷면을 그림.

    - OpenGL 기본 설정에서는 삼각형의 **정점 연결 순서**(시계 방향 또는 반시계 방향)에 따라  
    앞면과 뒷면을 구분하기 때문에 양면 모두 렌더링하려면 두 방향의 삼각형을 각각 정의해야 한다.

---

### **3. 카메라와 조명 설정**

- **카메라 초기화**:

    ```cpp
    cam.eye = vec3(2000, 0, 0); // 초기 카메라 위치
    cam.at = vec3(0, 0, 0);     // 카메라가 바라보는 지점
    cam.up = vec3(0, 0, 1);     // 위쪽 방향
    ```

  - 카메라는 태양계 전체를 관찰할 수 있도록 배치.

- **조명 설정**:

    ```cpp
    light.position = vec4(0.0f, 0.0f, 0.0f, 1.0f); // 태양을 조명 위치로 설정
    light.ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
    light.diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
    light.specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ```

  - 태양을 광원의 위치로 설정하며, 환경광, 난반사, 정반사를 구성.

---

## **데이터 업데이트**

업데이트 작업은 매 프레임마다 호출되는 `update()` 함수에서 수행됨.  
이 함수는 시간(`t`)에 따라 행성의 위치와 회전 상태를 동적으로 계산한다.

### **1. 시간 계산**

- **시간 관리**:

    ```cpp
    if (!isPause) t = float(glfwGetTime()) - bt;
    else bt = float(glfwGetTime()) - t;
    ```

  - `glfwGetTime()`을 사용하여 시뮬레이션이 시작된 이후의 경과 시간을 얻음.
  - `isPause` 변수로 애니메이션을 일시 정지하거나 재개 가능.

### **2. 공전 위치 계산**

- **공전 속도**:

  ```cpp
  speed = t * 100 / radius[i];
  ```

  - 공전 속도는 행성의 크기(`radius[i]`)에 반비례하여 계산.
  - 이는 행성의 반지름이 클수록 공전 속도가 느려짐을 의미.

- **위치 변환**:

  ```cpp
  mat4 trans_mat = {
      cosf(speed), -sinf(speed), 0, 0,
      sinf(speed),  cosf(speed), 0, 0,
      0,            0,           1, 0,
      0,            0,           0, 1
  };
  vec3 temp_pos = vec3(0, 1, 0); 
  temp_pos = mat3(trans_mat) * temp_pos;
  temp_pos *= rot_rad[i];
  ```

  - 행성의 공전 운동은 **원형 회전 행렬(`trans_mat`)** 을 사용해 계산.
  - 공전 반지름(`rot_rad[i]`)을 곱하여 행성이 중심체로부터의 위치를 결정.

### **3. 자전 운동 계산**

- **회전 행렬**:

  ```cpp
  mat4 rot_mat = {
      cosf(speed), -sinf(speed), 0, 0,
      sinf(speed),  cosf(speed), 0, 0,
      0,            0,           1, 0,
      0,            0,           0, 1
  };
  rot_mat = mat4::rotate(vec3(1, 0, 0), axis[i]) * rot_mat;
  ```

  - 자전 운동은 **행성의 자전축(`axis[i]`)을 기준으로 한 회전**으로 계산.
  - 행성의 회전 상태는 행성 표면에 텍스처를 적용할 때 사용.

### **4. 링 데이터 업데이트**

- **링 위치 계산**:

  ```cpp
  float speed = t * 10 / radius[i + 6];
  vec3 temp_pos = vec3(offset[(i + 6) * 3 + 0], offset[(i + 6) * 3 + 1], offset[(i + 6) * 3 + 2]);
  memcpy(ring_offset + i * 3, (float*)temp_pos, 3 * sizeof(float));
  ```

  - 링의 위치는 해당 행성의 공전 위치와 동기화.
  - 링의 회전 상태는 별도로 계산하여 회전 효과를 부여.

### **5. 셰이더에 데이터 전달**

- **행성 데이터 업데이트**:

  ```cpp
  glUniform1fv(glGetUniformLocation(program, "radius"), NUM_SPHERE, radius);
  glUniformMatrix4fv(glGetUniformLocation(program, "rotate_mat"), NUM_SPHERE, GL_TRUE, rotate_mat);
  glUniform3fv(glGetUniformLocation(program, "location"), NUM_SPHERE, offset);
  ```

  - 각 행성의 크기(`radius`), 위치(`offset`), 회전 상태(`rotate_mat`)를 GLSL 셰이더로 전달.

- **링 데이터 업데이트**:

  ```cpp
  glUniform1fv(glGetUniformLocation(program, "radius"), 4, ring_radius);
  glUniformMatrix4fv(glGetUniformLocation(program, "rotate_mat"), 2, GL_TRUE, rotate_mat);
  glUniform3fv(glGetUniformLocation(program, "location"), 2, ring_offset);
  ```

---

## **카메라 조작**

1. **카메라 회전**: 카메라가 특정 축을 중심으로 회전.
2. **줌(확대/축소)**: 카메라와 대상 간의 거리를 조절.
3. **패닝(panning)**: 카메라의 위치를 평행 이동.

### **1. `mouse` 함수: 마우스 입력 처리**

사용자가 버튼을 클릭하거나 놓을 때 호출.

```cpp
if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT) {
    dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
    vec2 npos = vec2(float(pos.x) / float(window_size.x - 1), float(pos.y) / float(window_size.y - 1));
    prev = npos;                 // 마우스 이전 위치 저장
    prev_at = cam.at;            // 카메라가 바라보는 대상 저장
    prev_eye = cam.eye;          // 카메라 위치 저장
    prev_pan = panning;          // 이전 패닝 변환 저장
    prev_view = cam.view_matrix; // 이전 뷰 행렬 저장

    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) key_ctrl = true;
        if (button == GLFW_MOUSE_BUTTON_RIGHT) key_shift = true;
        tb.begin(cam.view_matrix, npos.x, npos.y); // 트랙볼 초기화
        prev_mouse = npos;
    } else if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) key_ctrl = false;
        if (button == GLFW_MOUSE_BUTTON_RIGHT) key_shift = false;
        tb.end(); // 트랙볼 추적 종료
    }
}
```

---

### **2. `motion` 함수: 마우스 이동 처리**

```cpp
if(!tb.is_tracking()) return;
vec2 npos = vec2( float(x)/float(window_size.x-1), float(y)/float(window_size.y-1) );
if (key_shift) { // 줌
    float rate = (npos.y - prev_mouse.y);
    if (rate >= 0.99f) rate = 0.99f;
    cam.eye = (1 - rate) * prev_eye + rate * cam.at; // 카메라와 대상 간 거리 조정
    cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
}
else if (key_ctrl) {
    // 평행 이동 변환
    panning = mat4::translate((npos.x - prev.x) * 300, (prev.y - npos.y) * 300, 0);
    cam.view_matrix = panning * prev_view;
    cam.eye = ((mat3)cam.view_matrix).inverse() * -vec3(cam.view_matrix.at(3), cam.view_matrix.at(7), cam.view_matrix.at(11));
    cam.at = (cam.eye - prev_eye) + prev_at; // 대상 위치 갱신
}
else {
    // 트랙볼 업데이트
    cam.view_matrix = tb.update(npos.x, npos.y, cam.at);
    cam.eye = ((mat3)cam.view_matrix).inverse() * -vec3(cam.view_matrix.at(3), cam.view_matrix.at(7), cam.view_matrix.at(11));
    vec3 u = vec3(cam.view_matrix._11, cam.view_matrix._12, cam.view_matrix._13);
    vec3 n = vec3(cam.view_matrix._31, cam.view_matrix._32, cam.view_matrix._33);
    cam.up = n.cross(u); // 새로운 상단 벡터 계산
}
```

### **기능**

1. **줌**:
   - 마우스 이동의 **Y축 변화량**으로 줌 비율(`rate`)을 계산.
   - 카메라의 위치 `cam.eye`를 조정하여 대상 `cam.at`과의 거리를 변경.

2. **패닝 모드**:
   - 마우스 이동의 X, Y 변화량으로 카메라를 평행 이동.
   - 이동 행렬 `panning`을 생성하여 카메라의 뷰 행렬에 적용.
   - 카메라 위치(`cam.eye`)와 대상(`cam.at`)을 갱신.

3. **회전 모드**:
   - 트랙볼(`trackball`)을 사용해 카메라 회전 구현.
   - 트랙볼은 마우스 이동에 따라 카메라의 뷰 행렬을 회전 변환.
   - 새로운 뷰 행렬(`cam.view_matrix`)과 카메라 위치(`cam.eye`), 상단 벡터(`cam.up`)를 계산.

---