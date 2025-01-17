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
#include <string>
#include <sstream>
#include <type_traits>

using namespace std;

int32_t cpu_clock = 0;

typedef struct Process {
    pid_t pid; // process id
    int32_t priority; // process priority
    int32_t used_time; // used time quantum
    int32_t arrivalTime; // arrival time
    int32_t num_cycles; // number of cycles
    int32_t finished_time; // finished time
    deque<int32_t> io_burst; // I/O burst time list
    deque<int32_t> cpu_burst; // CPU burst time list

    bool operator<(const Process &p) const {
        return cpu_burst.front() < p.cpu_burst.front();
    }
} Process;

vector<Process> waitq;
vector<Process> finishedq;

class SchedulerBase {
public:
    virtual void push(const Process &process) = 0;
    virtual Process pop() = 0;
    virtual Process &front() = 0;
    virtual bool empty() const = 0;

    void set_next(SchedulerBase *next) { this->next = next; }
    void set_prev(SchedulerBase *prev) { this->prev = prev; }

    SchedulerBase *get_next() const { return next; }
    SchedulerBase *get_prev() const { return prev; }

protected:
    SchedulerBase *prev = nullptr;
    SchedulerBase *next = nullptr;
};

template <typename Container>
class Scheduler : public SchedulerBase {
public:
    explicit Scheduler(int32_t time_quantum = -1) : time_quantum(time_quantum) {}

    void push(const Process &process) override {
        if constexpr (std::is_same<Container, std::priority_queue<Process>>) {
            active_q.push(process);
        } else if constexpr (std::is_same<Container, std::deque<Process>>) {
            active_q.push_back(process);
        } else {
            static_assert("Unsupported container type");
        }
    }

    Process pop() override {
        if constexpr (std::is_same<Container, std::priority_queue<Process>>) {
            Process top = active_q.top();
            active_q.pop();
            return top;
        } else if constexpr (std::is_same<Container, std::deque<Process>>) {
            Process front = active_q.front();
            active_q.pop_front();
            return front;
        } else {
            static_assert("Unsupported container type");
        }
    }

    Process &front() override {
        if constexpr (std::is_same<Container, std::priority_queue<Process>>) {
            return active_q.top();
        } else if constexpr (std::is_same<Container, std::deque<Process>>) {
            return active_q.front();
        } else {
            static_assert("Unsupported container type");
        }
    }

    bool empty() const override { return active_q.empty(); }

    void preempt(Process &process) {
        if (next != nullptr) {
            next->push(process);
        } else {
            push(process);
        }
    }

    void promote(Process &process) {
        if (prev != nullptr) {
            prev->push(process);
        } else {
            push(process);
        }
    }

    void run() {
        bool is_cpu_remain = !active_q.is_empty();
        bool is_io_remain = !io_q.empty();
        if (is_cpu_remain) {
            Process &cur = active_q.front();
            cur.cpu_burst.front() -= 1;
            cur.used_time += 1;
            if (cur.cpu_burst.front() == 0) { // current CPU burst finished
                cur.cpu_burst.pop_front();
                cur.used_time = 0;
                if (cur.cpu_burst.empty()) { // all CPU burst finished
                    cur.finished_time = cpu_clock;
                    finishedq.push_back(cur);
                } else {
                    io_q.push_back(cur);
                }
                active_q.pop();
            } else if (time_quantum > 0 && time_quantum <= cur.used_time) { // time quantum expired
                cur.used_time = 0;
                io_q.push_back(cur);
            }
        }
        if (is_io_remain) {
            for (auto it = io_q.begin(); it != io_q.end();) {
                Process &cur = *it;
                cur.io_burst.front() -= 1;
                if (cur.io_burst.front() == 0) { // current I/O burst finished
                    cur.io_burst.pop_front();
                    cur.used_time = 0;
                    active_q.push(cur);
                    it = io_q.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

private:
    Container active_q; 
    deque<Process> io_q;
    int32_t time_quantum;
};

Scheduler<deque<Process>> q_0;
Scheduler<deque<Process>> q_1;
Scheduler<priority_queue<Process>> q_2;
Scheduler<deque<Process>> q_3;

void parse_process(const string &name) {
    ifstream ifs(name);
    if (!ifs.is_open()) {
        cerr << "Error: cannot open file " << name << endl;
        exit(1);
    }

    int32_t num_processes;
    ifs >> num_processes;
    waitq.reserve(num_processes);
    for (ssize_t i = 0; i < num_processes; ++i) {
        Process process;
        ifs >> process.pid >> process.priority >> process.arrivalTime >> process.num_cycles;
        for (ssize_t j = 0; j < process.num_cycles; ++j) {
            int32_t burst;
            ifs >> burst;
            process.cpu_burst.push_back(burst);
        }
        for (ssize_t j = 0; j < process.num_cycles - 1; ++j) {
            int32_t burst;
            ifs >> burst;
            process.io_burst.push_back(burst);
        }
        waitq[i] = process;
    }

    ifs.close();
}

void add_process(Process &p) {
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

void run() {
    while (true) {
        if (q_0.empty() && q_1.empty() && q_2.empty() && q_3.empty() && waitq.empty()) {
            break;
        }
        if (waitq.back().arrivalTime == cpu_clock) {
            Process p = waitq.back();
            waitq.pop_back();
            add_process(p);
        }
    }
}

int32_t main(int32_t argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input-file>" << endl;
        exit(1);
    }
    
    q_0.set_next(&q_1); q_0.set_prev(NULL);
    q_1.set_next(&q_2); q_1.set_prev(&q_0);
    q_2.set_next(&q_3); q_2.set_prev(&q_1);
    q_3.set_next(NULL); q_3.set_prev(&q_2);

    return 0;
}