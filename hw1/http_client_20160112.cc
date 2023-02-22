//http client
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>

#define MAXDATASIZE 2000 // max number of bytes we can get at once

using namespace std;

int main(int argc, char* argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, * servinfo;
	int rv;
	char s[INET_ADDRSTRLEN]; // Ip address

	char hostname[MAXDATASIZE];
	char port[MAXDATASIZE] = "80";
	char path_to_file[MAXDATASIZE] = "/";

	
	// 인자 몇개인지, 변수 "http://"로 시작하는지 check
	if (argc != 2 || strncmp(argv[1], "http://", 7) != 0) {
		fprintf(stderr, "usage: http_client http://hostname[:port][/path/to/file]\n");
		exit(1);
	}

	//parsing
	int flag = 0;
	int idx = 0;
	for (int i = 7; argv[1][i] != '\0'; i++) {
		if (argv[1][i] == '/' && flag != 2 ){
			flag = 2;
			idx = 0;
			continue;
		}
		if( argv[1][i] == ':') {
			flag = 1;
			idx = 0;
			continue;
		}
		if (flag == 0) {
			hostname[idx++] = argv[1][i];
		}
		else if (flag == 1) {
			port[idx++] = argv[1][i];
		}
		else {
			path_to_file[++idx] = argv[1][i];
		}
	}


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// TCP 연결 요청
	if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
		// IP address 얻지 못했을 때
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	//creat socket
	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        perror("client:socket");
        return 2;
    }

	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		// 서버와 연결 실패
		close(sockfd);
		perror("connect");
		exit(1);
	}

	// IP address 저장
	inet_ntop(servinfo->ai_family, &((struct sockaddr_in*)servinfo->ai_addr)->sin_addr, s, sizeof s);

	strcpy(buf, "GET ");
	strcat(buf, path_to_file);
	strcat(buf, " HTTP/1.1 \r\n");
	if (send(sockfd, buf, strlen(buf), 0) == -1) {
		perror("send");
	}

	strcpy(buf, "Host: ");
	strcat(buf, hostname);
	strcat(buf, ":");
	strcat(buf, port);
	strcat(buf, "\r\n\r\n");
    if (send(sockfd, buf, strlen(buf), 0) == -1) {
		perror("send");
	}
	
	int recv_len;
	// http response 저장
	recv_len = recv(sockfd, buf, MAXDATASIZE, 0);

	// 첫번째 줄 출력
	for(int i=0;  ; i++){
		if(buf[i] == 13&& buf[i+1] == 10){
			printf("\n");
			break;
		}
		printf("%c", buf[i]);
	}
	
	// out 파일
	//ofstream out("20160112.txt");
	ofstream out("20160112.out");

	// content_length 찾기 + data 출력
	int content_len = 0;
	for(int i=0; i<recv_len;i++ ){
		char * tmp = buf+i;
		if(strncmp(tmp, "ontent-Length",13)== 0 ||strncmp(tmp, "ontent-length",13)== 0 ){
			while(buf[i]>'9'|| buf[i]<'0') i++;
			while(buf[i]<='9'&& buf[i]>='0'){
				content_len *=10;
				content_len += buf[i] - '0';
				i++;
			}
		}

		if(strncmp(tmp, "\r\n\r\n",4)== 0){
			for(int j = i+4; j<recv_len;j++) out<< buf[j];
			break;
		}
	}

	// data 너무 클 경우
	/*
	while( (recv_len = recv(sockfd, buf, MAXDATASIZE, MSG_DONTWAIT)) > 0) { 
		for(int i = 0; i<recv_len;i++) out<< buf[i];
	}
	*/
	int roop_flag = content_len - recv_len;
	while(roop_flag >= 0){
		//printf("roop : %d , recv len : %d\n", roop_flag, recv_len);
		recv_len = recv(sockfd, buf, MAXDATASIZE, 0);
		for(int i = 0; i<recv_len;i++) out<< buf[i];
		roop_flag -= recv_len;
	}


	
	if(content_len == 0) printf("Content-Length not specified.\n");
	else printf("%d bytes written to 20160112.out\n", content_len);

	freeaddrinfo(servinfo); // all done with this structure

	close(sockfd);
	return 0;
}
