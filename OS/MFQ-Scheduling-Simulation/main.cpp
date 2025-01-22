#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <queue>
#include <cstring>
#include <list>
#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>
#include <type_traits>

using namespace std;

int32_t cpu_clock = 0;

struct Process {
    pid_t pid;
    int32_t priority;
    int32_t used_time;
    int32_t arrival_time; 
    int32_t num_cycles;
    int32_t waiting_time; 
    int32_t finished_time;
    int32_t turnaround_time;
    int32_t total_cpu_time;
    deque<int32_t> io_burst;
    deque<int32_t> cpu_burst;

    Process() : pid(0), priority(0), used_time(0), arrival_time(0), num_cycles(0), 
        finished_time(0), total_cpu_time(0), waiting_time(0), turnaround_time(0) {}

    bool operator<(const Process &p) const {
        return cpu_burst.empty() || (!p.cpu_burst.empty() && cpu_burst.front() > p.cpu_burst.front());
    }
};

void add_process(Process p);
void print_queue_status();

deque<Process> waitq;
vector<Process> finishedq;
deque<Process> global_io_q;

class SchedulerBase {
public:
    virtual void push(const Process &process) = 0;
    virtual Process pop() = 0;
    virtual Process &front() = 0;
    virtual bool empty() const = 0;

//     void set_next(SchedulerBase *next) { this->next = next; }
//     void set_prev(SchedulerBase *prev) { this->prev = prev; }

//     SchedulerBase *get_next() const { return next; }
//     SchedulerBase *get_prev() const { return prev; }

// protected:
//     SchedulerBase *prev = nullptr;
//     SchedulerBase *next = nullptr;
};

void push(std::deque<Process>& q, const Process& process) {
    q.push_back(process);
}

void push(std::priority_queue<Process>& q, const Process& process) {
    q.push(process);
}

// pop 함수 오버로딩
Process pop(std::deque<Process>& q) {
    Process front = q.front();
    q.pop_front();
    return front;
}

Process pop(std::priority_queue<Process>& q) {
    Process top = q.top();
    q.pop();
    return top;
}

// front 함수 오버로딩
Process& front(std::deque<Process>& q) {
    return q.front();
}

Process& front(std::priority_queue<Process>& q) {
    return const_cast<Process&>(q.top());
}

// empty 함수 오버로딩
bool empty(const std::deque<Process>& q) {
    return q.empty();
}

bool empty(const std::priority_queue<Process>& q) {
    return q.empty();
}

template <typename Container>
class Scheduler : public SchedulerBase {
public:
    explicit Scheduler(int32_t time_quantum = -1) : time_quantum(time_quantum) {}

    void push(const Process &process) override {
        ::push(active_q, process);
    }

    Process pop() override {
        return ::pop(active_q);
    }

    Process& front() override {
        return ::front(active_q);
    }

    bool empty() const override { 
        return ::empty(active_q);
    }

    // void demote(Process &process) {
    //     if (next != nullptr) {
    //         next->push(process);
    //     } else {
    //         push(process);
    //     }
    // }

    // void promote(Process &process) {
    //     if (prev != nullptr) {
    //         prev->push(process);
    //     } else {
    //         push(process);
    //     }
    // }

    // void after_run() {
    //     while (!promote_q.empty()) {
    //         Process &p = promote_q.front();
    //         promote(p);
    //         promote_q.pop_front();
    //     }
    //     while (!demote_q.empty()) {
    //         Process &p = demote_q.front();
    //         demote(p);
    //         demote_q.pop_front();
    //     }
    // }
    
    void run() {
        if (empty()) return;  // 큐가 비어있으면 실행 종료

        Process &p = front();  // 현재 실행할 프로세스를 가져옴

        // CPU 실행
        cpu_clock++;  // 실제 CPU 실행 시 클럭 증가
        p.used_time += 1;
        p.total_cpu_time += 1;
        p.cpu_burst.front() -= 1;
        
        // 프로세스 실행 정보 출력
        cout << "[Time: " << cpu_clock << "] Running Process: " << p.pid 
            << ", Used Time: " << p.used_time 
            << ", Remaining CPU Burst: " << p.cpu_burst.front() 
            << ", In Queue: " << (time_quantum == -1 ? 3 : (time_quantum == 2 ? 0 : (time_quantum == 6 ? 1 : 2)))
            << endl;

        // 1. CPU burst 완료 처리
        if (p.cpu_burst.front() == 0) {
            p.cpu_burst.pop_front();
            if (!p.io_burst.empty()) {
                global_io_q.push_back(p);
                cout << "[Time: " << cpu_clock << "] Process " << p.pid << " moved to I/O queue" << endl;
            } else {
                p.finished_time = cpu_clock;
                p.turnaround_time = p.finished_time - p.arrival_time;
                p.waiting_time = p.turnaround_time - p.total_cpu_time;
                finishedq.push_back(p);
                cout << "[Time: " << cpu_clock << "] Process " << p.pid << " finished execution." << endl;
            }
            pop();  // 작업 완료 후 큐에서 제거
            return;
        }

        // 2. 타임 퀀텀 초과 처리 (Round Robin 큐)
        if (time_quantum > 0 && p.used_time >= time_quantum) {
            p.used_time = 0;  // 타임 퀀텀 리셋
            p.priority += 1;  // 다음 큐로 이동
            add_process(p);
            cout << "[Time: " << cpu_clock << "] Process " << p.pid << " demoted to queue " << p.priority << endl;
            pop();  // 큐에서 제거 후 재배치
            return;
        }

        // 3. SRTN 선점 처리 (Q2)
        if constexpr (std::is_same<Container, std::priority_queue<Process>>::value) {
            cout << "[Time: " << cpu_clock << "] SRTN reordering process " << p.pid << endl;
            add_process(pop());
        }
    }
    
    const Container& get_queue() const {
        return active_q;
    }

private:
    Container active_q; 
    // deque<Process> io_q;
    // deque<Process> promote_q;
    // deque<Process> demote_q;
    int32_t time_quantum;
};

Scheduler<deque<Process>> q_0(2);
Scheduler<deque<Process>> q_1(6);
Scheduler<priority_queue<Process>> q_2(0);
Scheduler<deque<Process>> q_3;

void parse_process(const string &name) {
    ifstream ifs(name);
    if (!ifs.is_open()) {
        cerr << "Error: cannot open file " << name << endl;
        exit(1);
    }

    int32_t num_processes;
    ifs >> num_processes;
    for (size_t i = 0; i < num_processes; ++i) {
        Process process;
        ifs >> process.pid >> process.priority >> process.arrival_time >> process.num_cycles;
        int32_t cpu_burst, io_burst;
        for (size_t j = 0; j < process.num_cycles - 1; ++j) {
            ifs >> cpu_burst >> io_burst;
            process.cpu_burst.push_back(cpu_burst);
            process.io_burst.push_back(io_burst);
        }
        ifs >> cpu_burst;
        process.cpu_burst.push_back(cpu_burst);
        waitq.push_back(process);
    }
    sort(waitq.begin(), waitq.end(), [](const Process &lhs, const Process &rhs) {
        return lhs.arrival_time < rhs.arrival_time;
    });

    std::cout << "Parsed Process List:\n";
    std::cout << "-----------------------------------------------------\n";
    std::cout << std::setw(5) << "PID" << " | "
              << std::setw(8) << "Priority" << " | "
              << std::setw(7) << "Arrival" << " | "
              << std::setw(6) << "Cycles" << " | "
              << "CPU Bursts   | IO Bursts\n";
    std::cout << "-----------------------------------------------------\n";

    for (const Process &p : waitq) {
        std::cout << std::setw(5) << p.pid << " | "
                  << std::setw(8) << p.priority << " | "
                  << std::setw(7) << p.arrival_time << std::setfill(' ') << " | "
                  << std::setw(6) << p.num_cycles << " | ";

        for (int32_t burst : p.cpu_burst) {
            std::cout << burst << " ";
        }
        std::cout << " | ";

        for (int32_t burst : p.io_burst) {
            std::cout << burst << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "-----------------------------------------------------\n";
}

void add_process(Process p) {
    switch (p.priority) {
        case 0:
            q_0.push(p);
            break;
        case 1:
            q_1.push(p);
            break;
        case 2:
            q_2.push(p);
            break;
        default:
            q_3.push(p);
            break;
    }
}

void process_io_queue() {
    size_t io_size = global_io_q.size();

    for (size_t i = 0; i < io_size; ++i) {
        Process p = global_io_q.front();
        global_io_q.pop_front();

        p.io_burst.front() -= 1;

        if (p.io_burst.front() == 0) {  // I/O 완료
            p.io_burst.pop_front();
            p.priority = max(0, p.priority - 1);  // I/O 후 상위 큐로 복귀
            add_process(p);
            cout << "[Time: " << cpu_clock << "] Process " << p.pid << " finished IO and returned to queue " << p.priority << endl;
        } else {
            global_io_q.push_back(p);  // I/O가 끝나지 않은 프로세스는 다시 큐에 추가
        }
    }
}

void run() {
    while (true) {
        if (q_0.empty() && q_1.empty() && q_2.empty() && q_3.empty() && waitq.empty() && global_io_q.empty()) {
            break;
        }

        while (!waitq.empty() && waitq.front().arrival_time <= cpu_clock) {
            Process p = waitq.front();
            waitq.pop_front();
            add_process(p);
            cout << "[Time: " << cpu_clock << "] Process " << p.pid << " arrived and added to queue " << p.priority << endl;
        }

        if (!q_0.empty()) q_0.run();
        else if (!q_1.empty()) q_1.run();
        else if (!q_2.empty()) q_2.run();
        else if (!q_3.empty()) q_3.run();
        else {}

        // q_0.after_run(); 
        // q_1.after_run();
        // q_2.after_run(); 
        // q_3.after_run();

        process_io_queue();
        print_queue_status();
    }
}

void print_results() {
    cout << "\n[Scheduling Results]\n";
    cout << "---------------------------------------------------\n";
    cout << setw(5) << "PID" << " | "
         << setw(10) << "Arrival" << " | "
         << setw(10) << "Finish" << " | "
         << setw(10) << "Turnaround" << " | "
         << setw(10) << "Waiting\n";
    cout << "---------------------------------------------------\n";

    int total_tt = 0, total_wt = 0;
    for (const Process &p : finishedq) {
        total_tt += p.turnaround_time;
        total_wt += p.waiting_time;

        cout << setw(5) << p.pid << " | "
             << setw(10) << p.arrival_time << " | "
             << setw(10) << p.finished_time << " | "
             << setw(10) << p.turnaround_time << " | "
             << setw(10) << p.waiting_time << "\n";
    }
    cout << "---------------------------------------------------\n";

    cout << "Average Turnaround Time: " << (double)total_tt / finishedq.size() << endl;
    cout << "Average Waiting Time: " << (double)total_wt / finishedq.size() << endl;
}

void print_queue_status() {
    cout << "========================================\n";
    cout << "[Time: " << cpu_clock << "] Queue Status:\n";

    auto print_deque = [](const deque<Process>& q, const string& name) {
        cout << name << " (size: " << q.size() << "): ";
        for (const auto& p : q) {
            cout << "[PID: " << p.pid 
                 << ", CPU Burst: " << (p.cpu_burst.empty() ? 0 : p.cpu_burst.front()) 
                 << ", IO Burst: " << (p.io_burst.empty() ? 0 : p.io_burst.front()) 
                 << ", Used Time: " << p.used_time << "] ";
        }
        cout << endl;
    };

    auto print_priority_queue = [](priority_queue<Process> q, const string& name) {
        cout << name << " (size: " << q.size() << "): ";
        while (!q.empty()) {
            Process p = q.top();
            q.pop();
            cout << "[PID: " << p.pid 
                 << ", CPU Burst: " << (p.cpu_burst.empty() ? 0 : p.cpu_burst.front()) 
                 << ", IO Burst: " << (p.io_burst.empty() ? 0 : p.io_burst.front()) 
                 << ", Used Time: " << p.used_time << "] ";
        }
        cout << endl;
    };

    print_deque(q_0.get_queue(), "Q0 (RR, TQ=2)");
    print_deque(q_1.get_queue(), "Q1 (RR, TQ=6)");
    print_priority_queue(q_2.get_queue(), "Q2 (SRTN)");
    print_deque(q_3.get_queue(), "Q3 (FCFS)");

    print_deque(global_io_q, "Global I/O Queue");
    print_deque(waitq, "Wait Queue");

    cout << "========================================\n";
}

int32_t main(int32_t argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input-file>" << endl;
        exit(1);
    }
    
    parse_process(argv[1]);
    run();
    print_results();
    return 0;
}