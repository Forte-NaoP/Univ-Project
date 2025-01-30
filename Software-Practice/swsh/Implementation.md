# 미니쉘 (swsh) 동작 과정

## 1. 초기 설정

### 1.1 환경 변수 설정

- `custom_binary = "./commands/bin:"`을 기존 `PATH` 환경 변수와 합쳐서 새로운 `PATH`를 설정한다.
- 이를 통해 사용자가 실행 파일을 `./commands/bin/` 디렉토리에서 실행할 수 있도록 함.

### 1.2 시그널 핸들링

- `SIGINT` (Ctrl+C)와 `SIGTSTP` (Ctrl+Z)를 무시하도록 설정.
- `SIGCHLD` 시그널을 처리하여 종료된 자식 프로세스를 정리하는 핸들러 `chld_handler` 등록.

## 2. 입력 및 파싱

### 2.1 사용자 입력 처리

- `read(0, input, 256)`을 이용해 최대 256바이트까지 입력을 읽음.
- 입력을 `split_commands(input, &cmd_cnt)`로 분리하여 명령어 목록을 생성.

### 2.2 명령어 파싱 (`argparse.c`)

- `split_commands` 함수:
  - `|` 기호를 기준으로 명령어를 여러 개로 분리함.
  - `parse_redirection`을 사용해 `>`, `<`, `>>` 등 입출력 리다이렉션을 처리.
  - `parse_command`를 통해 명령어와 인자를 파싱하여 `COMMAND` 구조체에 저장.

## 3. 명령 실행

### 3.1 파이프 설정

- 명령어 개수(`cmd_cnt`)를 확인하고 필요하면 `pipe(pipe_fds)`를 생성.
- `prev_pipe_fd`를 이용하여 이전 명령의 출력을 현재 명령의 입력으로 연결.

### 3.2 명령 실행 과정

- 각 명령을 `fork()`하여 자식 프로세스를 생성한 후 실행.
- `setup_redirection`을 이용해 입력/출력 리다이렉션을 처리.
- `execvp(cmd->cmd, cmd->argv)`를 호출하여 실행 파일을 실행.
- 파이프의 읽기/쓰기 파일 디스크립터를 적절히 닫음.
- `waitpid(-1, &status, 0)`를 이용해 자식 프로세스가 종료될 때까지 대기.

### 3.3 내부 명령 실행 (`inline.c`)

- `cd`, `pwd`, `exit`는 별도로 `fork()` 없이 실행.
  - `cd`: `chdir(path)`를 호출하여 현재 디렉토리를 변경.
  - `pwd`: `getcwd()`를 호출하여 현재 디렉토리를 출력.
  - `exit`: `exit(status)`를 호출하여 쉘 종료.

## 4. 정리 및 종료

- 실행이 끝난 후 `restore_redirection`을 호출하여 원래의 표준 입력/출력 상태 복구.
- `COMMAND_free`를 이용해 동적 할당된 메모리를 해제.
- `free(cmds)`를 호출하여 `COMMAND` 목록을 해제.
- 다음 입력을 기다리며 루프를 반복.
