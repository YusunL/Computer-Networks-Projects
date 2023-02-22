#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <queue>

using namespace std;

#define num 256
#define INF 99999

typedef struct msg_ {
    int src;
    int dest;
    char data[num];
}msg;

struct node_pq {
    int idx;//현재 노드 idx
    int cst;//현재 드는 cost값
    int nxt;//next node
    int prt;// parent
};

struct cmp {
    bool operator()(node_pq& a, node_pq& b) {
        if( a.cst != b.cst) return a.cst > b.cst;
        else return a.idx > b.idx;
    }
};

// 한 라우터의 테이블 구하는 함수 - 다익스트라
// using Dijkstra algorithm to get a table of router
void linkstate(vector< vector<int> > &route_tbl, vector< vector<int> > ntwrk, int curr_node, int N) {
    //테이블 초기화
    // initializing table
    for (int i = 0; i < N; i++) route_tbl[i][0] = route_tbl[i][3] = i;
    route_tbl[curr_node][1] = curr_node;
    route_tbl[curr_node][2] = 0;
    //route_tbl[curr][0: idx, 1: next, 2: cost ,3: parent]

    //다익스트라
    //Dijkstra
    priority_queue < node_pq, vector<node_pq>, cmp > pq;
    node_pq pq_tmp;
    pq_tmp.idx = curr_node;
    pq_tmp.cst = 0;
    pq_tmp.nxt = curr_node;
    pq_tmp.prt = curr_node;
    pq.push(pq_tmp);
    while (pq.size() != 0) {
        pq_tmp = pq.top();
        pq.pop();
        if (route_tbl[pq_tmp.idx][2] >= pq_tmp.cst) {   
            if(route_tbl[pq_tmp.idx][2] > pq_tmp.cst || route_tbl[pq_tmp.idx][3] > pq_tmp.prt){
                route_tbl[pq_tmp.idx][1] = pq_tmp.nxt;
                route_tbl[pq_tmp.idx][2] = pq_tmp.cst;
                route_tbl[pq_tmp.idx][3] = pq_tmp.prt;
            }
            for (int i = 0; i < N; i++) {
                if (ntwrk[pq_tmp.idx][i] != 0 && ntwrk[pq_tmp.idx][i] != -999) {
                    node_pq adj_node;
                    adj_node.idx = i;
                    adj_node.cst = pq_tmp.cst + ntwrk[pq_tmp.idx][i];
                    adj_node.prt = pq_tmp.idx;
                    if (pq_tmp.idx == curr_node) {
                        // only getting neighbor
                        adj_node.nxt = i;
                    }
                    else {
                        adj_node.nxt = pq_tmp.nxt;
                    }
                    pq.push(adj_node);
                }
            }
        }
    }
}


int main(int argc, char* argv[]) {
    //checking number of arguments
    if (argc != 4) {
        fprintf(stderr, "usage: linkstate topologyfile messagesfile changesfile\n");
        exit(1);
    }


    //topology
    FILE* f;
    if ((f = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Error: open input file.");
        exit(1);
    }
    int N;
    fscanf(f, "%d", &N);

    vector< vector<int> > ntwrk;
    ntwrk.resize(N, vector<int>(N, -999));
    for (int i = 0; i < N; i++) ntwrk[i][i] = 0;


    int node, dest, cost;
    while (fscanf(f, "%d %d %d", &node, &dest, &cost) != EOF) {
        ntwrk[node][dest] = ntwrk[dest][node] = cost;
    }

    fclose(f);
    
    //messages
    if ((f = fopen(argv[2], "r")) == NULL) {
        fprintf(stderr, "Error: open input file.");
        exit(1);
    }
    
    vector<msg> msgs;
    msg msg_tmp;
    while (fscanf(f, "%d %d %[^\n]s",&msg_tmp.src, &msg_tmp.dest ,msg_tmp.data) != EOF) {
        msgs.push_back(msg_tmp);
    }

    fclose(f);


    //changes
    if ((f = fopen(argv[3], "r")) == NULL) {
        fprintf(stderr, "Error: open input file.");
        exit(1);
    }
   
    queue< vector<int> >chngs;
    while (fscanf(f, "%d %d %d", &node, &dest, &cost) != EOF) {
        vector<int>tmp;
        tmp.push_back(node);
        tmp.push_back(dest);
        tmp.push_back(cost);
        chngs.push(tmp);
    }

    fclose(f);

    // 라우팅 프로토콜
    // routing protocol
    vector< vector< vector<int> > > route_tbl;    
    f =  fopen("output_ls.txt", "w");
    while(true){
        route_tbl.clear();
        route_tbl.resize(N, vector< vector<int> >(N, vector<int>(4, INF)));
        for (int i = 0; i < N; i++) linkstate(route_tbl[i], ntwrk, i, N);
        
        // 라우팅 테이블 출력
        // print routing table
         for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                if(route_tbl[i][j][2] != INF){
                    fprintf(f,"%d %d %d\n",route_tbl[i][j][0],route_tbl[i][j][1],route_tbl[i][j][2]);
                }
            }   
            fprintf(f,"\n");
        }

        //메세지 전송
        //sending msg
        int msgs_num = msgs.size();
        for(int i = 0; i < msgs_num; i++){
            int snd = msgs[i].src, rec = msgs[i].dest;
            fprintf(f,"from %d to %d cost ",snd, rec);
            if(route_tbl[snd][rec][2] == INF){
                fprintf(f,"infinite hops unreachable ");
            }
            else{ 
                fprintf(f,"%d hops ", route_tbl[snd][rec][2] );
                while(snd != rec){
                    fprintf(f,"%d ",snd);
                    snd = route_tbl[snd][rec][1];
                }
            }
            fprintf(f,"message %s\n", msgs[i].data);

        }
        fprintf(f,"\n");

        //네트워크 변동
        // chaing in network
        if(chngs.size() == 0){
            fclose(f);
            printf("Complete. Output file written to output_ls.txt.\n");
            break;
        }
        else{
            vector<int>tmp = chngs.front();
            chngs.pop();
            ntwrk[tmp[0]][tmp[1]] = ntwrk[tmp[1]][tmp[0]] = tmp[2]; 
            // modify network
        }
    }



    return 0;
}