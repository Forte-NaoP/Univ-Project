# swsh - 미니 셸 제작

## 1. 소개

`swsh`는 리눅스 환경에서 사용할 간단한 셸로, 사용자의 입력 해석, 명령어 실행 및 파일, 프로세스, 시그널, IPC와 관련된 기능을 포함한다.

## 2. 명세

### 2.1. 기능 요구사항

1. **사용자 입력 처리**:
   - 사용자가 입력한 문자열을 해석하고 명령을 실행한다.
   - 입력 문자열은 최대 200바이트 이내이다.
2. **입출력 리다이렉션 및 파이프라인**:
   - `<`: 입력 리다이렉션 - 파일의 내용을 표준 입력으로 연결.
   - `>`: 출력 리다이렉션 - 표준 출력을 파일에 저장.
   - `>>`: 출력 덧붙이기 - 기존 파일에 출력 내용을 추가.
   - `|`: 앞 프로세스의 표준 출력을 뒤 프로세스의 표준 입력으로 연결

### 2.2. 입력 문자열 구조

```txt
input       ::= commands
                commands | input
commands    ::= command 
                command < [filename]
                command > [filename]
                command >> [filename]
                command < [filename] > [filename]
command     ::= cmd_type1 [options/arguments]
                cmd_type2 [options/arguments]
                cmd_type3 [arguments]
                cmd_type4
cmd_type1   ::= {ls, man, grep, sort, awk, bc}
                path
path        ::= {pathname with leading "./"}
cmd_type2   ::= {head, tail, cat, cp}
cmd_type3   ::= {mv, rm, cd}
cmd_type4   ::= {pwd , exit}
```

1. **파일명** (`filename`):
   - 파일명은 사용자가 제공하는 파일 이름.
2. **옵션/인자** (`options/arguments`):
   - 명령어에 따라 추가로 제공되는 옵션과 인자.
3. **키워드 구분**:
   - 모든 키워드는 공백으로 구분된다.  
   예를 들어, `command < filename`에서 `command`와 `<` 사이, `<`와 `filename` 사이에는 반드시 공백이 존재한다.

사용자는 200바이트 이내의 문자열을 입력하며, 표의 input과 매칭되는 패턴의 문자열이면 유효한 입력이다.
추가로 `path`에서는 절대경로와 `~`는 사용되지 않는다.
입력이 유효하지 않은 경우 다음 에러 메세지를 **표준 에러**에 출력한다.

```txt
swsh: Command not found
```

### 2.3. 명령어 종류

명령어는 크게 4가지 타입으로 구분된다.

| 명령어 타입 | 명령어 | 설명 |
|-------------|---------|-------|
| **cmd_type1** | `ls`, `man`, `grep`, `sort`, `awk`, `bc`, `path` | 외부 프로그램을 `fork` 및 `exec`로 실행 |
| **cmd_type2** | `head`, `tail`, `cat`, `cp` | 직접 구현 또는 외부 실행 가능 |
| **cmd_type3** | `mv`, `rm`, `cd` | 반드시 직접 구현 (인자 존재) |
| **cmd_type4** | `pwd`, `exit` | 반드시 직접 구현 (인자 없음) |

### 2.4. 명령어 처리 과정

1. **명령어 개수 파악**:
2. **입출력 리다이렉션 처리**:
   - 리다이렉션 사용 여부 확인 및 파일 존재 여부 검증.
3. **명령어 종류 분석**:
4. **프로세스 생성 및 종료 대기**:

### 2.5. 입출력 리다이렉션 세부사항

1. **입력 리다이렉션** (`<`):
   - 파일이 존재할 경우 무조건 읽기 권한이 있는 정상적인 파일.
   - 파일이 없을 경우 에러 메시지 출력: `swsh: No such file`.
2. **출력 리다이렉션** (`>`, `>>`):
   - 파일이 무조건 존재하지 않거나 쓰기 권한이 존재함.
3. **파이프라인과의 조합**:
   - 입력 리다이렉션은 첫 번째 명령어에만 존재한다.
   - 출력 리다이렉션은 마지막 명령어에만 존재한다.
   - B와 C에서 리다이렉션을 검사할 필요가 없음

    ```txt
    A < file1 | B | C | D > file2
    ```

### 2.6. 프로세스 및 시그널 관리

1. **프로세스 그룹**:
   - 자식 프로세스는 새로운 프로세스 그룹에 속하며, 그룹 ID는 프로세스 ID와 동일.
   - 파이프로 인해 두 개 이상의 프로세스를 생성할 경우 제일 먼저 생성된 프로세스의 그룹에 속한다.
2. **좀비 프로세스 방지**:
   - `waitpid` 시스템 콜을 사용해 자식 프로세스를 정리.
3. **시그널 처리**:
   - `SIGINT`와 `SIGTSTP`를 수신해도 `swsh`는 종료되지 않는다.
   - 자식 프로세스가 `SIGTSTP`로 중단되었을 경우, 해당 그룹에 `SIGKILL`을 보내 모든 프로세스를 강제 종료한다.

## 3. 명령어 구현 세부사항

### 3.1. 지원 명령어

| 명령어 | 기능 | 추가 옵션 | 비고 |
|--------|-------|-----------|------|
| `head [option] file` | 파일의 상위 10줄 출력 | `-n K`: 상위 K줄 출력 | 파일은 항상 존재 |
| `tail [option] file`| 파일의 하위 10줄 출력 | `-n K`: 하위 K줄 출력 | `head`와 동일 |
| `cat file` | 파일 내용을 표준 출력으로 출력 | - | `head`와 동일 |
| `cp file1 file2` | file1의 복사본 file2 생성 | - | file1은 항상 존재하고, file2는 항상 덮어쓸 수 있음 |
| `mv file1 file2` | 파일 이름 변경 | - | - |
| `rm file` | 파일 삭제 | - | - |
| `cd dir` | 현재 디렉토리를 지정된 디렉토리로 변경 | - | - |
| `pwd` | 현재 디렉토리를 출력 | - | - |
| `exit [NUM]` | `swsh` 종료 | `NUM`: 종료 코드 지정 | 표준 에러로 `exit` 출력, `NUM` 기본 값은 `0` |

### 3.2. 에러 처리

- `mv, rm, cd` 명령어 수행 중 에러 발생 시 아래 메시지를 조합하여 표준에러로 출력한다:
  - `EACCESS`: Permission denied
  - `EISDIR`: Is a directory
  - `ENOENT`: No such file or directory
  - `ENOTDIR`: Not a directory
  - `EPERM`: Permission denied
  - 기타: `Error occurred: <ERRNO>`

  ```txt
  e.g)
  mv: Permission denied
  cd: Not a directory
  ```