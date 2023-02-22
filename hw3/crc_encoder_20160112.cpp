#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#define XOR(a,b) ( a == b ? '0' : '1')
#define N 1000

using namespace std;

string dec_to_bin(char data) {
	string result = "";
	int ascii = (int)data;
	while( ascii > 0) {
		if (ascii % 2 == 1) result = "1" + result;
		else result = "0" + result;
		ascii >>= 1;
	}
	return result;
}

string encode_CRC(string data, string gntr, int d_size) {
	string remain;
	int gntr_size = gntr.size();
	if (gntr_size == 4)remain = data+ "000";
	else remain = data+ "0000000";

	while (remain.size() >= gntr.size()) {
		if (remain[0] == '0' && gntr[0] == '1') remain = remain.substr(1);
		else {
			string tmp = "";
			for (int i = 0; i < gntr_size; i++) {
				tmp += XOR(remain[i], gntr[i]);
			}
			remain = tmp + remain.substr(gntr_size);
		}
	}

	return data+remain;
}

void str_to_bin(string str, char &bin ){
    bin = 0b00000000;
    //printf("%s\n",str.c_str());
    for(int i = str.size()-1, tmp = 1; i >= 0; i--, tmp*=2) {
		if ( str[i] == '1') bin += tmp;
    }
    //printf("%d\n",(int)bin);
}

int main(int argc, char* argv[]){
     //checking number of arguments
    if(argc != 5){
        fprintf(stderr, "usage: ./crc_encoder input_file output_file generator dataword_size\n");
		exit(1);
    }

    //dataword size 
    int dataword_size = (int)(argv[4][0] - '0');
    if (dataword_size != 4 && dataword_size != 8){
        fprintf(stderr, "dataword size must be 4 or 8.\n");
		exit(1);
    }

    // input file
    FILE *fin, *fout; 
    if ((fin = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr,"input file open error.\n");
        exit(1);
    }
    
    // ouput file
    if ((fout = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr,"output file open error.\n");
        exit(1);
    }

    // 파일 인코딩 및 출력
    // incoding and prining file
    char* buffer = (char*) malloc (sizeof(char));
    string total_code =""; 
    while (fread(buffer,1,1,fin) != 0) {        
        string tmp_code;
        string data_bin = dec_to_bin(buffer[0]);

        while(data_bin.size()!=8) data_bin = '0'+data_bin;
        //printf("%s\n", data_bin.c_str());
        if (dataword_size == 4){
            tmp_code = encode_CRC(data_bin.substr(0,4), argv[3],4);
            tmp_code += encode_CRC(data_bin.substr(4), argv[3],4);           
            //printf("4자리는 %s\n", tmp_code.c_str()); 
        }
        else {//8
            tmp_code = encode_CRC(data_bin, argv[3],8);
            //printf("8자리는 %s\n", tmp_code.c_str()); 
        }
        total_code += tmp_code;
    }
    int padding = 0;
    while(total_code.size() % 8 != 0) {
            total_code = '0'+total_code;
            padding++;
    }
       //printf("%d\n",padding);
    string tmp_padd = dec_to_bin((char)padding);
    while(tmp_padd.size() <8) tmp_padd = '0'+tmp_padd;
        //printf("%s\n", tmp_padd.c_str());
        
    char* ptr, bin;
    ptr = &bin;
    str_to_bin(tmp_padd, bin);
    fwrite(ptr,1,1,fout);

    while(total_code.size() != 0) {
        str_to_bin(total_code.substr(0,8),bin);
        fwrite(ptr,1,1,fout);
        //printf("%s\n", tmp_code.c_str());
        total_code = total_code.substr(8);            
    }
    //fprintf(fout,"\n");      
    fclose(fin);
    fclose(fout);
    free(buffer);


    return 0;
}