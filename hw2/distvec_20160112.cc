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

// 모든 라우터의 테이블 구하는 함수
// finding all the router's table
void distvec(vector< vector< vector<int> > > &route_tbl, vector< vector<int> > ntwrk, int N, int curr_node) {
    //route_tbl[curr][0: idx, 1: next, 2: cost]

    //distvec
    queue <int> q;
    q.push(curr_node);
    
    while (q.size() != 0) {
        int tmp = q.front();// tmp = currently changed node
        q.pop();
        for (int i = 0; i < N; i++) { // i = neighbor of tmp         
            int cost_to_this_node = ntwrk[tmp][i];
            if (cost_to_this_node != 0 && cost_to_this_node != -999) {// for only neighbor of tmp
                bool change = false;
                for (int j = 0; j < N; j++) { // j = neighbor of i
                    if (route_tbl[tmp][j][2] + cost_to_this_node < route_tbl[i][j][2]) {
                        change = true;
                        route_tbl[i][j][1] = tmp;
                        route_tbl[i][j][2] = route_tbl[tmp][j][2] + cost_to_this_node;
                    }
                    else if (route_tbl[tmp][j][2] + cost_to_this_node == route_tbl[i][j][2] && route_tbl[i][j][1]> tmp) {
						change = true;
						route_tbl[i][j][1] = tmp;
						route_tbl[i][j][2] = route_tbl[tmp][j][2] + cost_to_this_node;
					}
                    if(route_tbl[i][j][1] == tmp && route_tbl[tmp][j][2] + cost_to_this_node  > route_tbl[i][j][2]){
                        change = true;
                        if(ntwrk[i][j] != -999){
                            route_tbl[i][j][1] = j;
                            route_tbl[i][j][2] = ntwrk[i][j]; 
                        }
                        else{
                            route_tbl[i][j][1] = route_tbl[i][j][2] = INF;
                        }
                    }
                }
                if (change) q.push(i); //only if there is change at router table, send to neighbor
            }
        }
    }
}

int main(int argc, char* argv[]){
    // checking number of arguments
    if(argc != 4){
        fprintf(stderr, "usage: distvec topologyfile messagesfile changesfile\n");
		exit(1);
    }
    

    //topology
    FILE* f; 
    if ((f = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr,"Error: open input file.");
        exit(1);
    }
    int N;
    fscanf(f, "%d", &N);

    vector< vector<int> > ntwrk;
    ntwrk.resize(N, vector<int>(N, -999));
    for (int i = 0; i < N; i++) ntwrk[i][i] = 0;

   
    int node, dest, cost;
    while (fscanf(f, "%d %d %d", &node, &dest, &cost) != EOF) {
        ntwrk[node][dest]  = ntwrk[dest][node]= cost;
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

    //라우팅 테이블 초기화
    //initializing routing table
    vector< vector< vector<int> > > route_tbl;
    route_tbl.resize(N, vector< vector<int> >(N, vector<int>(3, INF)));
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < N; i++) {
            route_tbl[j][i][0] = i;
            if (ntwrk[j][i]!=-999) {
                route_tbl[j][i][1] = i;
                route_tbl[j][i][2] = ntwrk[j][i];
            }
        }
    }
    
    //라우팅 프로토콜
    //routing protocols
    f =  fopen("output_dv.txt", "w");
    for(int i = 0; i<N;i++) distvec(route_tbl, ntwrk, N, i);
    while(true){
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
        //change in network
        if(chngs.size() == 0){
            fclose(f);
            printf("Complete. Output file written to output_dv.txt.\n");
            break;
        }
        else{
            vector<int>tmp = chngs.front();
            chngs.pop();
            ntwrk[tmp[0]][tmp[1]] = ntwrk[tmp[1]][tmp[0]] = tmp[2];// 네트워크 수정
            if(tmp[2] == -999) {
                for(int i = 0; i<N;i++){
                    if( route_tbl[tmp[0]][i][1] == tmp[1]) route_tbl[tmp[0]][i][2] = INF;
                    if( route_tbl[tmp[1]][i][1] == tmp[0]) route_tbl[tmp[1]][i][2] = INF;
                }
            }
            else route_tbl[tmp[0]][tmp[1]][2] = route_tbl[tmp[1]][tmp[0]][2] = tmp[2]; // 라우팅테이블 수정
            distvec(route_tbl, ntwrk, N, tmp[1]);
            distvec(route_tbl, ntwrk, N, tmp[0]);
            
        }
    }

    return 0;
}