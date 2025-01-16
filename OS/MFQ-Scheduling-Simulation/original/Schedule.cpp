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

using namespace std;

//process infomation
typedef struct process{
    bool run;   // is doing?
    int pid;    //process id
    int cur_q;  // current queue number
    int prev_q; // queue number before i/o burst
    int arrive; // arrival time
    int cycle;  // total cycle
    deque<int> remain; // remain burst time
}process;

//result of finished process
typedef struct result{
    int pid;    
    int start;  // first response time
    int finish; // time of finishing all job
    int wait;   // waiting time
}result;

//Multi-level queue
deque<process> q_0;
deque<process> q_1;
deque<process> q_2;
deque<process> q_3;

queue<process> waitq;
list<process> ioq;  // list of processes in i/o burst   
list<process> tmp;
list<process> tmp1;
result* results;

int cpu_cycle = 0;
int c0, c1; // time quantum of q_0, q_1
int num_ps;

bool active[4]= {false, };

void setNextQ();    // set working queue 
void do_proc();
void do_io();
void assignQueue(process p);
bool isfinished();
bool cmp(const process& a, const process& b);
void calculateWait();
void logger(bool starting, process p, bool io);

bool start = false;

ofstream ofs;

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("specify input file\n");
        exit(0);
    }
    
    string s;
    ifstream ifs;
    ifs.open(string(argv[1]));
    getline(ifs, s);
    num_ps = stoi(s);

    for(int i=0;  i < num_ps; ++i){
        process p;
        string s;
        getline(ifs, s);
        
        stringstream ss(s);
        ss >> s; p.pid = stoi(s);
        ss >> s; p.cur_q = stoi(s);
        ss >> s; p.arrive = stoi(s);
        ss >> s; p.cycle = stoi(s);
        p.prev_q = p.cur_q;
        p.run = false;
        while(ss >> s) p.remain.push_back(stoi(s));
        results = (result *)malloc(sizeof(result) * num_ps);
        waitq.push(p);
    }
    ifs.close();

    ofs.open("log.txt");
    cpu_cycle = -1;
    while(!isfinished()){
        cpu_cycle++;
        if(!waitq.empty()){
            if(cpu_cycle == waitq.front().arrive){ //arrive new proc
                process p = waitq.front();
                waitq.pop();
                assignQueue(p);
                if(!start){
                    setNextQ();
                    start = true;
                }
                result r;
                r.pid = p.pid;
                r.start = cpu_cycle;
                results[r.pid] = r;
            }
            
        }
        do_proc();
        do_io();
        for(auto p : tmp){
            if(!p.remain.empty())
                ioq.push_back(p);
            else{
                results[p.pid].finish = cpu_cycle;
                logger(false, p, false);
            }
        }
        for(auto p : tmp1){
            if(!p.remain.empty())
                assignQueue(p);
            else{
                results[p.pid].finish = cpu_cycle;
                logger(false, p, true);
            }
        } 
        tmp.clear();
        tmp1.clear();
        calculateWait();   
    }
    double avgtt = 0, avgwt = 0;
    ofs << "pid\tTT\tWT" << endl;
    for(int i = 0; i < num_ps; ++i){
        result r = results[i];
        avgtt += (r.finish - r.start);
        avgwt += r.wait;
        ofs << r.pid << "\t" << r.finish - r.start << "\t" << r.wait << endl;
    }

    ofs << avgtt / num_ps << avgwt / num_ps << endl;
    ofs.close();


}

void setNextQ(){
    if(!q_0.empty()) active[0] = true;
    else if(!q_1.empty()) active[1] = true;
    else if(!q_2.empty()) active[2] = true;
    else active[3] = true;

    if(active[0] && !q_0.empty()) logger(true, q_0.front(), false);
    else if(active[1] && !q_1.empty()) logger(true, q_1.front(), false);
    else if(active[2] && !q_2.empty()) logger(true, q_2.front(), false);
    else if(active[3] && !q_3.empty()) logger(true, q_3.front(), false);
    
}

void do_proc(){
    if(active[0] && !q_0.empty()){
        if(c0 != 2){
            process* pp = &q_0.front();
            if(!pp->run) pp->run = true;
            c0++;
            if(pp->remain.front() == 1){ // job done
                process p = *pp;
                p.prev_q = p.cur_q;
                p.remain.pop_front();
                tmp.push_back(p);
                q_0.pop_front();
                active[0] = false;
                c0 = 0;
                setNextQ();
                
            }else{
                pp->remain.front()--;
            }
        }else{ // time quantum expire
            c0 = 0;
            process p = q_0.front();
            p.cur_q = 1;
            tmp1.push_back(p);
            q_0.pop_front();
            active[0] = false;
            setNextQ();
        }
    }else if(active[1] && !q_1.empty()){
        if(c1 != 6){
            process* pp = &q_1.front();
            if(!pp->run) pp->run = true;
            c1++;
            if(pp->remain.front() == 1){ //job done
                process p = *pp;
                p.prev_q = p.cur_q;
                p.remain.pop_front();
                tmp.push_back(p);
                q_1.pop_front();
                active[1] = false;
                c1 = 0;
                setNextQ();
            
            }else{
                pp->remain.front()--;
            }
        }else{
            c1 = 0;
            process p = q_1.front();
            p.cur_q = 2;
            tmp1.push_back(p);
            q_1.pop_front();
            active[1] = false;
            setNextQ();
        }
    }else if(active[2] && !q_2.empty()){
        process* pp = &q_2.front();
        if(!pp->run) pp->run = true;
        if(pp->remain.front() == 1){
            process p = *pp;
            p.prev_q = p.cur_q;
            p.remain.pop_front();
            tmp.push_back(p);
            q_2.pop_front();
            active[2] = false;
            setNextQ();
            
        }else{
            pp->remain.front()--;
        }
    }else if(active[3] && !q_3.empty()){
        process* pp = &q_3.front();
        if(!pp->run) pp->run = true;
        if(pp->remain.front() == 1){
            process p = *pp;
            p.prev_q = p.cur_q;
            p.remain.pop_front();
            tmp.push_back(p);
            q_3.pop_front();
            active[3] = false;
            setNextQ();
            
        }else{
            pp->remain.front()--;
        }
    }
}

void do_io(){
    
    for(auto it = ioq.begin(); it != ioq.end(); ++it){
        (*it).remain.front()--;
        if((*it).remain.front() == 0){
            process p = (*it);
            it = ioq.erase(it);
            it = --it;
            p.remain.pop_front();
            p.cur_q = max(p.prev_q - 1, 0);
            tmp1.push_back(p);
        }
    }
    
}

void assignQueue(process p){
    int num_q = p.cur_q;
    if(num_q == 0 || num_q == 1){
        num_q == 0 ? q_0.push_back(p) : q_1.push_back(p);
    }else if(num_q == 2){
        if(q_2.empty()){
            q_2.push_back(p);
            return;
        }
        process cur = q_2.front();
        if(cur.remain.front() > p.remain.front()){
            q_2.pop_front();
            q_2.push_front(p);
            cur.cur_q = 3;
            q_3.push_back(cur);
        }
        else{
            q_2.push_back(p);
            sort(q_2.begin()+1, q_2.end(), cmp);
        }
        
    }else{
        q_3.push_back(p);
    }
}

bool isfinished(){
    bool finished = true;
    finished = (finished && q_0.empty() && q_1.empty() && q_2.empty() && q_3.empty());
    finished = (finished && waitq.empty() && ioq.empty());
    return finished;
}

bool cmp(const process& a, const process& b){
    return a.remain.front() < b.remain.front();
}

void calculateWait(){
    bool chk = false;
    for(process p : q_0){
        if(!chk && active[0]) chk = true;
        else results[p.pid].wait++;
    }
    for(process p : q_1){
        if(!chk && active[1]) chk = true;
        else results[p.pid].wait++;
    }
    for(process p : q_2){
        if(!chk && active[2]) chk = true;
        else results[p.pid].wait++; 
    }
    for(process p : q_3){
        if(!chk && active[3]) chk = true;
        else results[p.pid].wait++;
    }
    for(process p : ioq){
        results[p.pid].wait++;       
    }
}

void logger(bool starting, process p, bool io){
    if(starting){
        if(!p.run)
            ofs << cpu_cycle << " : " << p.pid << " start" << endl;
    }
    else{
        if(io)
            ofs << cpu_cycle << " : " << p.pid << " finish in i/o" << endl;
        else
            ofs << cpu_cycle << " : " << p.pid << " finish in cpu" << endl;
    }
}
