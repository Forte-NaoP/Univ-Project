import random

def generate_random_input_file(filename="input.txt", num_processes=5):
    # 랜덤 범위 설정
    arrival_time_range = (0, 20)  # 도착 시간 범위
    cycles_range = (2, 5)         # 주기 수 범위
    burst_time_range = (5, 50)    # CPU/IO 버스트 시간 범위

    with open(filename, "w") as f:
        f.write(f"{num_processes}\n")  # 첫 줄: 프로세스 개수

        for pid in range(1, num_processes + 1):
            init_queue = random.randint(0, 3)  # 초기 큐 (0~3 사이의 랜덤 값)
            arrival_time = random.randint(*arrival_time_range)
            num_cycles = random.randint(*cycles_range)

            burst_sequence = []
            for _ in range(num_cycles * 2 - 1):  # CPU/IO 버스트 시간 시퀀스 생성 (CPU-IO 반복)
                burst_sequence.append(random.randint(*burst_time_range))

            # 프로세스 정보 기록
            burst_str = " ".join(map(str, burst_sequence))
            f.write(f"{pid} {init_queue} {arrival_time} {num_cycles} {burst_str}\n")

    print(f"File '{filename}' has been generated with random data.")

# 랜덤 입력 파일 생성 실행
generate_random_input_file()
