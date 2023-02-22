#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>

#define XOR(a,b) ( a == b ? '0' : '1')
#define N 1000

using namespace std;

unsigned int err_num = 0, code_num = 0;

string dec_to_bin(char data) {
	string result="00000000";
	int ascii = (int)data, cmp = 128;
    bool neg = false;
    if(ascii <0){
        ascii = abs(ascii);
        neg = true;
    }
	for(int i = 0; i < 8; i++){       
        if(ascii&cmp) result[i]='1';
        else result[i]='0';
        cmp>>=1;
    }  
    
    if(neg){       
        for(int i = 0; i<8;i++){
            if(result[i]=='0')result[i] = '1';
            else result[i] = '0';
        }       
        for(int i = 7; i >= 0; i--){
            if(result[i]=='1') result[i]='0';
            else {
                result[i]='1';
                break;
            }
        }       
    }
	return result;
}

char bin_to_dec(string data) {
	int result = 0;
	
	for(int i = data.size()-1, tmp = 1; i >= 0; i--, tmp*=2) {
		if ( data[i] == '1') result += tmp;
	}
	return (char)result;
}

string decode_CRC(string code, string gntr, int d_size) {
	string remain = code;
    int gntr_size = gntr.size();
	code_num++;
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
    int j = remain.size();
	for(int i = 0; i < j;i++) 
		if (remain[i] == '1') {
			err_num++;
			break;
		}
	return code.substr(0,d_size);
}

int main(int argc, char* argv[]){
     //인자 수 check
    if(argc != 6){
        fprintf(stderr, "usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
		exit(1);
    }

    string generator = argv[4];
    //dataword size 
    int dataword_size = (int)(argv[5][0] - '0');
    if (dataword_size != 4 && dataword_size != 8){
        fprintf(stderr, "dataword size must be 4 or 8.\n");
		exit(1);
    }

    // input file
    FILE *fin, *fout, *fresult; 
    if ((fin = fopen(argv[1], "rb")) == NULL) {
        fprintf(stderr,"input file open error.\n");
        exit(1);
    }
    
    // ouput file
    if ((fout = fopen(argv[2], "wb")) == NULL) {
        fprintf(stderr,"output file open error.\n");
        exit(1);
    }

    // result file
    if ((fresult = fopen(argv[3], "w")) == NULL) {
        fprintf(stderr,"result file open error.\n");
        exit(1);
    }

    // 파일 디코딩 및 출력
    char* buffer = (char*) malloc (sizeof(char));
    fread(buffer,1,1,fin);
    int padding = (int)buffer[0];
    unsigned int codeword_size = dataword_size + generator.size() - 1; 
    string codeword = "";
    
    while (fread(buffer,1,1,fin) != 0) {       
        codeword += dec_to_bin(buffer[0]);          
    }
    codeword = codeword.substr(padding);
    while(codeword.size() > 0){
        string dataword ="";
        if(dataword_size == 4){
            dataword = decode_CRC(codeword.substr(0,codeword_size),generator,dataword_size) 
                     + decode_CRC(codeword.substr(codeword_size,codeword_size),generator,dataword_size);
            codeword = codeword.substr(codeword_size*2);
        }
        else{
            dataword = decode_CRC(codeword.substr(0,codeword_size),generator,dataword_size); 
            codeword = codeword.substr(codeword_size);
        }
        
        buffer[0] = bin_to_dec(dataword);
        fwrite(buffer,1,1,fout);
        //printf("%c",buffer[0]);
        //printf("%s\n",codeword.c_str());
    } 
    free(buffer);
    fprintf(fresult, "%d %d", code_num, err_num);

    fclose(fin);
    fclose(fout);
    fclose(fresult);


    return 0;
}