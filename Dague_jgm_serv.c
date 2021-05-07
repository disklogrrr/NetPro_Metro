/*
*	2021 KNU CSE EEC312 Network Programming 
*   Daegu Metropolitan Transit Memorizing Game 
*	
* 	File		Daegu_jgm_serv.c 
*	
*	Date		05 / 07 / 2021 
*
*	Authors 	2017116965 Jun Geon Min
*				2017116965 Jun Geon Min
*				2017116965 Jun Geon Min
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define BUF_SIZE 100
#define MAX_CLNT 3
#define true 1
#define false 0

// function
void Draw_Tube();
void *handle_clnt(void *arg);	   // read -> send_mssg ( thread )
void send_msg(char *msg, int len); // erver send msg to client
void error_handling(char *msg);
void start_game();			  // First run
void *change_line(void *arg); // change the quetison ( Subway Line ) -> thread
void download_api(int line);  // api download from data.go.kr ( ROK PUBLIC DATA PORTAL )
int check_answer(char *msg);  // check user is collect or not

//Global variable
int clnt_cnt = 0;		  // Count client ( MAX 3 )
int clnt_socks[MAX_CLNT]; // client socket array
pthread_mutex_t mutx;	  // synchronized by mutex
int round_cnt = 1;		  // start at ROUDN 1 ( line 1 ) , 10 collect -> Next Round , Line 1 -> Line 2 -> Line 3 -> Line 1 ...
int answer_cnt = 0;		  // Cnt Answer

//메인
int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if (argc != 2)
	{
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);			 // init mutex
	serv_sock = socket(PF_INET, SOCK_STREAM, 0); // init socket ( TCP-Socket )

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1) // bind
		error_handling("bind() error");
	if (listen(serv_sock, 5) == -1) // listen
		error_handling("listen() error");

	pthread_create(&t_id, NULL, change_line, NULL); //change_line thread
	pthread_detach(t_id);

	while (1)
	{
		// user max = 3
		if (clnt_cnt <= 2)
		{
			clnt_adr_sz = sizeof(clnt_adr);
			clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

			pthread_mutex_lock(&mutx);
			clnt_socks[clnt_cnt++] = clnt_sock;
			pthread_mutex_unlock(&mutx);

			pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
			pthread_detach(t_id);
			printf("\nConnected client IP: %s ", inet_ntoa(clnt_adr.sin_addr));
			printf("\nRemain user : %d", 3-clnt_cnt);

			printf("\n\n");
			start_game(); // 3 client -> start game
		}
	}

	close(serv_sock);
	return 0;
}

void start_game()
{

	if (clnt_cnt == 3)
	{
		Draw_Tube();
		sleep(5);
		char msgLoading[100] = "\n\n\tNow Loading ...........\n\n\n";
		send_msg(msgLoading, 100);
		sleep(5);
	

		char msg1[100] = "\tThree users are logged in.\n\n\n\tMatching Completed. \n\n";
		send_msg(msg1, 100);
		sleep(3);
		char msg2[50] = "\n\tThe game starts in 3 seconds.\n\n";
		send_msg(msg2, 50);
		sleep(1);
		char msg2_1[50] = "\n\t3 seconds...\n\n";
		send_msg(msg2_1, 50);
		sleep(1);
		char msg2_2[50] = "\n\t\t2 seconds..\n\n";
		send_msg(msg2_2, 50);
		sleep(1);
		char msg2_3[50] = "\n\t\t\t1 seconds.\n\n";
		send_msg(msg2_3, 50);
		sleep(1);
		char msg2_4[50] = "\n\n\n";
		send_msg(msg2_4, 50);
		sleep(1);

		char msg3_1[100] = "***************************************************************************\n";
		send_msg(msg3_1, 100);
		usleep(500);
		char msg3_2[100] = "**		    	Daegu  Metropolitan Transit 			 **";
		send_msg(msg3_2, 100);
		usleep(500);
		char msg3_3[100] = "\n**                                                                       **";
		send_msg(msg3_3, 100);
		usleep(500);
		char msg3_4[100] = "\n**		    	Memorizing game			                 **";
		send_msg(msg3_4, 100);
		usleep(500);
		char msg3_5[100] = "\n**                                                                       **";
		send_msg(msg3_5, 100);
		usleep(500);
		char msg3_6[100] = "\n**		    	By Open API from data.go.kr	       	         **";
		send_msg(msg3_6, 100);
		usleep(500);
		char msg3_7[100] = "\n**                                                                       **";
		send_msg(msg3_7, 100);
		usleep(500);
		char msg3_8[100] = "\n**		    	In ROK PUBLIC DATA PORTAL 			 **";
		send_msg(msg3_8, 100);
		usleep(500);
		char msg3_9[100] = "\n***************************************************************************\n\n";
		send_msg(msg3_9, 100);
		usleep(500);

		sleep(2);

		char msg4[5000] = "[ GAME RULES ]  \n1. This program requires 3 users to start the game.\n2. If you guys get 10 right answer -the names of 10 stations- , the game will move on to the next round\n3. Rounds are repeated in the order Line 1->2->3, and 1.\n4. This program is designed for memorizing subway stations.\n5. Have fun playing games and memorize subway stations! Let's get started. (quit to Q or q) \n\n";
		send_msg(msg4, 5000);
        sleep(5);
		download_api(2);
		char msg5[100] = "***********************This round is [ LINE 1 ]**********************\nSTART ! \n\n";
		send_msg(msg5, 100);
		sleep(1);
	}
}

//change round(question)
void *change_line(void *arg)
{

	char *msg;

	int download_line3 = false;
	int download_line2 = false;
	int download_line1 = false;

	while (1)
	{ // for thread

		if (answer_cnt == 10)
		{

			char msg1[100] = "\nSTOPPPPPPPPPPPPP ! You guys Got 10 Right Answer ! \n";
			send_msg(msg1, 100);
			sleep(1);
			round_cnt++;
			answer_cnt = 0; // for next round
            
            char msg1_2[100] = "Get Ready for next Round ....\n";
            send_msg(msg1_2,100);
            sleep(4);
			//Line1 -> Line2 -> Line 3 -> Line 1 ..
			if (round_cnt % 3 == 0)
			{

				if (download_line3 == false)
				{ //api download once
					download_api(3);
					download_line3 = true;
				}

				msg = "\n************************This round is [ LINE 3 ]**********************\nSTART ! \n\n";
				send_msg(msg, 100);
				sleep(1);
			}
			else if (round_cnt % 3 == 1)
			{
				/* 처음 시작할 때 1호선 문제를 다운받고 시작하므로.
				if(download_line2 == false){
					download_api(2);
					download_line2 = true;
				}
				*/

				msg = "\n************************This round is [ LINE 1 ]**********************\nSTART ! \n\n";
				send_msg(msg, 100);
				sleep(1);
			}
			else if (round_cnt % 3 == 2)
			{

				if (download_line1 == false)
				{
					download_api(1);
					download_line1 = true;
				}

				msg = "\n**********************This round is [ LINE 2 ]**********************\nSTART ! \n\n";
				send_msg(msg, 100);
				sleep(1);
			}

		} 

	} 
}

//chechk answer
int check_answer(char *msg)
{

	FILE *fp;
	char buffer[300];
	char *fname;

	if (round_cnt % 3 == 0)
	{
		fname = "Line3";
	}
	else if (round_cnt % 3 == 1)
	{
		fname = "Line1";
	}
	else if (round_cnt % 3 == 2)
	{
		fname = "Line2";
	}

	char *ptr = strtok(msg, " "); //1차: " " 공백 문자를 기준으로 문자열 자름
	ptr = strtok(NULL, "\n");	  //2차: "\n" 엔터키에 NULL을 넣고 자름

	msg = ptr;

	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "[WARNING] %s Can't open the file \n", fname);
		exit(1);
	}

	while (fgets(buffer, 300, fp))
	{

		if (strstr(buffer, msg))
		{				 //Find str
			return true; //if find, return true
			break;
		}
	}

	fclose(fp);

	return false;
}

//download line status by open api ( user wget )
void download_api(int line)
{

	if (line == 3)
	{
		system("wget -O Line3 \"http://api.data.go.kr/openapi/tn_pubr_public_ctyrlroad_route_api?serviceKey=iYQjcRrPSKRq1%2B9uB33oHleRIEqCm7mZGdRUAW%2FdmJ0yd2RhpoU7jq4gHOZPaD%2Bwc8Z5nv4DCG%2FaVnsih%2Fmp2g%3D%3D&pageNo=1&type=json&routeNo=S2703\"");
	}
	else if (line == 2)
	{

		system("wget -O Line1 \"http://api.data.go.kr/openapi/tn_pubr_public_ctyrlroad_route_api?serviceKey=iYQjcRrPSKRq1%2B9uB33oHleRIEqCm7mZGdRUAW%2FdmJ0yd2RhpoU7jq4gHOZPaD%2Bwc8Z5nv4DCG%2FaVnsih%2Fmp2g%3D%3D&pageNo=1&type=json&routeNo=S2701\"");
	}
	else if (line == 1)
	{

		system("wget -O Line2 \"http://api.data.go.kr/openapi/tn_pubr_public_ctyrlroad_route_api?serviceKey=iYQjcRrPSKRq1%2B9uB33oHleRIEqCm7mZGdRUAW%2FdmJ0yd2RhpoU7jq4gHOZPaD%2Bwc8Z5nv4DCG%2FaVnsih%2Fmp2g%3D%3D&pageNo=1&type=json&routeNo=S2702\"");

	} //end else if
}

//run on-line by thread
void *handle_clnt(void *arg)
{

	int clnt_sock = *((int *)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];
	char *msg1;
	char *cur_line;

	if (round_cnt % 3 == 0)
	{
		cur_line = "Line3";
	}
	else if (round_cnt % 3 == 1)
	{
		cur_line = "Line1";
	}
	else if (round_cnt % 3 == 2)
	{
		cur_line = "Line2";
	}

	//
	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
	{ // while for thread
		send_msg(msg, str_len);
		sleep(1);

		//answer check
		if (check_answer(msg) == true)
		{
			msg1 = "Correct............>_<\n";
			send_msg(msg1, 60);
			answer_cnt++;
		}
		else
		{
			msg1 = "Wrong.............-_-\n";
			send_msg(msg1, 60);
		}
	}

	//
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) // del unconnected client
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);

	return NULL;
}

//call in thread
void send_msg(char *msg, int len) // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
// drwa subway
void Draw_Tube()
{
	char T0[200] = "\n\n\n\n";
	send_msg(T0,200);
	usleep(200);
	char T0_1[200] = "####################################################################\n";
	send_msg(T0_1,200);
	usleep(200);
	char T1[200] = "#                                                                  #\n";
	send_msg(T1,200);
	usleep(200);
	char T2[200] = "#--------------  2021 KNU CSE EECS312 Network Programming  ,,,,,...#\n";
	send_msg(T2,200);
	usleep(200);
	char T3[200] = "#----------------------------  2017116965 전근민  ,,,,,,,..........#\n";
	send_msg(T3,200);
	usleep(200);
	char T4[200] = "#-------------  2017116965 전근민  ---------,,,,,,,,,,,,,..........#\n";
	send_msg(T4,200);
	usleep(200);
	char T5[200] = "#----------------------------  2017116965 전근민  ,,,,,,,..........#\n";
	send_msg(T5,200);
	usleep(200);
	char T6[200] = "#-------  Daegu Metropolitan Transit Memorizing Game  ,.,..........#\n";
	send_msg(T6,200);
	usleep(200);
	char T7[200] = "#                                                                  #\n";
	send_msg(T7,200);
	usleep(200);
	char T8[200] = "#-----------------------------,,,,,,,,,,,,,,,,,,,,,,...............#\n";
	send_msg(T8,200);
	usleep(200);
	char T9[200] = "#-------------------~~~:~~~---,,,,,,,,,,,,,,,,,,,,,,...............#\n";
	send_msg(T9,200);
	usleep(200);
	char T10[200] = "#----------------~;*$$$$$$=~-,,-:,,,,,,,,,,,,,,,,,,,...............#\n";
	send_msg(T10,200);
	usleep(200);
	char T11[200] = "#---------------;=$####$$=~-,.-!=;;;:-,,,,,,,,,,,,,,,,,............#\n";
	send_msg(T11,200);
	usleep(200);
	char T12[200] = "#--------------=$##$$$$$$~--,.,!********:..,,,,,,,,,,,,............#\n";
	send_msg(T12,200);
	usleep(200);
	char T13[200] = "#-------------=###$$$$$$~:-,. ,!!!****=*===$,,-,,,,,,,,,,,.........#\n";
	send_msg(T13,200);
	usleep(200);
	char T14[200] = "#-----------~=###$$#$$$:~-,...-!!!!!******=*=*=:~:~,,,,,,,,,,,.,...#\n";
	send_msg(T14,200);
	usleep(200);
	char T15[200] = "#----------~=####$$#$#*~~-,...!!!!!!!!!!*=*=*****=*;;;:,,,,,,,,,...#\n";
	send_msg(T15,200);
	usleep(200);
	char T16[200] = "#----------$#######@##!:--,,.,=!!!!!!**$=*$$*********=$**=.,,,,,,,,#\n";
	send_msg(T16,200);
	usleep(200);
	char T17[200] = "#---------$=!;::;:::::~~~-,,,,-!!!!!!!**$$**==********=***=$=~,,,,,#\n";
	send_msg(T17,200);
	usleep(200);
	char T18[200] = "#--~------!!;;;;;;*==!:~~-,,,,,,,,,,,:!;;:!;!;:!=********===*=:----#\n";
	send_msg(T18,200);
	usleep(200);
	char T19[200] = "#~~~~~~~~~!!!!;;;;!!!;::~~-,,,,,,,,,,,,,-,,,,,,-:~~~~~;!~:=$==*----#\n";
	send_msg(T19,200);
	usleep(200);
	char T20[200] = "#~~~~~~~~~***!!!;;;;;;;;:~--,,,,,,,,,-~~~~,,,,,,,,,,,,:;,,-----~~~~#\n";
	send_msg(T20,200);
	usleep(200);
	char T21[200] = "#~~~~~~~~~;****!!!!;;;;*;:~--,,,,,,,,,,,,,,,,,,,,,,,,,:;,,,,,,,~~~~#\n";
	send_msg(T21,200);
	usleep(200);
	char T22[200] = "#~~~~~~~~~~!*==***!!!!;;;;::~--,,,,,,,,,,,-,,,,,,,,,,,:;,,,,,~,:;!*#\n";
	send_msg(T22,200);
	usleep(200);
	char T23[200] = "#~~~~~~~~~~~:*$$==***!!!;;;;:~--,,,,,,,,,,-,,,,,,,,,,-:;,,,,,-;*!*=#\n";
	send_msg(T23,200);
	usleep(200);
	char T24[200] = "#~~~~~~~~~~~~~~*##$$==**!!;;;::~-,,,,,,-~,~,,,,,,,,---~:------===*:#\n";
	send_msg(T24,200);
	usleep(200);
	char T25[200] = "#~~~;***=$$=$$$#$=$####$==*!!;;:~--,,,,,-,,,-------------~--:!~~~~~#\n";
	send_msg(T25,200);
	usleep(200);
	char T26[200] = "#$$$$$$$$$$$$#$###==#@##@@@@#**;:~-------------------~;;:!!~~~~~~~~#\n";
	send_msg(T26,200);
	usleep(200);
	char T27[200] = "#$===$$#$$$===*==$$#$=*=====$$$;;:---------------:!!!;;;~~~~~~~~~~~#\n";
	send_msg(T27,200);
	usleep(200);
	char T28[200] = "#=*!!!!!!!**==*!!!!***!!!**=*=$=;:~----------~!===!~~~~~~~~~~~~~~~~#\n";
	send_msg(T28,200);
	usleep(200);
	char T29[200] = "#*!!!!*!;;;;*****!!!!!!!!!**==$=;:~----~~:*=$=**~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T29,200);
	usleep(200);
	char T30[200] = "#!!;;!!!!!!!*******!!!!****=$$#=!;::;;!====!!;~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T30,200);
	usleep(200);
	char T31[200] = "#!!!!!!!!!!********=***=====$$$****=====!:;~~~~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T31,200);
	usleep(200);
	char T32[200] = "#!!!!!!!*!!***=$===========$$$=======*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T32,200);
	usleep(200);
	char T33[200] = "#!!!!!!**==$$===============$===***:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T33,200);
	usleep(200);
	char T34[200] = "#***========$$=====***========!!:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T34,200);
	usleep(200);
	char T35[200] = "#===========$===!!!*;**====!;;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T35,200);
	usleep(200);
	char T36[200] = "#============!!!!;;;;!!**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#\n";
	send_msg(T36,200);
	usleep(200);
	char T0_2[200] = "####################################################################\n";
	send_msg(T0_2,200);
	usleep(200);
	/*
	char T37[200] = "~~~~~~~~~~~~~~~~~~!======*****!!!!!;;;;;;;::~~----,,,,,,,,,,,,,,,~--;,,,,-,,,,,,,,,,,,--!,,,,,-,,,-,-****!*!\n";
	send_msg(T37,200);
	usleep(100);
	char T38[200] = "~~~~~~~~~~~~~~~~~~~:=$$$====****!!!!!;;;;;;;::~----,,,,,,,,,,,,,,~~-,,,,,-,,,,,,,,-,-,,-!,-,,,,,----~*!!**!=\n";
	send_msg(T38,200);
	usleep(100);
	char T39[200] = "~~~~~~~~~~~~~~~~~~~~~;=$$$$$===****!!!!=;;;;;;:~~---,,,,,,,,,,,,,,---,,,,,,,,,,,,--,,--~;-----------!**=====\n";
	send_msg(T39,200);
	usleep(100);
	char T40[200] = "~~~~~~~~~~~~~~~~~~~~~~~;*####$$====***!!!!;;;;;::~---,,,,,,,,,,,,,-~-,,,,,,,,,,,,,,----~;-----------===*!:::\n";
	send_msg(T40,200);
	usleep(100);
	char T41[200] = "~:::~~:::::;!***===$$$=$$==$######$$===**!!!;;;;;::~---,,,,,,,,--,,--,,,-,-------------::-------~::*!;~~~~~~\n";
	send_msg(T41,200);
	usleep(100);
	char T42[200] = "*======$$$$$$$$=$$$##$$$$$$=**=$@@@@@@@##$=*!!;;;;::~---,,,,-,-~--------~--------,-----*----~!*;;:~~~~~~~~~~\n";
	send_msg(T42,200);
	usleep(100);
	char T43[200] = "$$$$$$$$$$$$$$$$$$$##$$$$#$##@@@@@@@@@@@@@@@@@#*;;;;:~--------------------------------~=:;*=**~~~~~~~~~~~~~~\n";
	send_msg(T43,200);
	usleep(100);
	char T44[200] = "$$$$$$$$$$$$$$$$$$$############$$$$$$$#####$$$##=;;;:~-----------------------------~!*=*=::~~~~~~~~~~~~~~~~~\n";
	send_msg(T44,200);
	usleep(100);
	char T45[200] = "=====$$$$$$$$#$$$$==$$==**===========*==*=*====$$!;;:~~---------:-------------~:;*==**!-~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T45,200);
	usleep(100);
	char T46[200] = "$=====*****!!!!!!********!!!!!!!!!!!!!*=***====$#*;;;:~-------~----------~:;**====;:~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T46,200);
	usleep(100);
	char T47[200] = "!!!!;;!!!!!!*!!!!*****!!!!!!!!!!!!!!!!!****==*=$$#!;;:~-------------~~;;*$====**~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T47,200);
	usleep(100);
	char T48[200] = "!**!!!!!!;;;;;;!;!!!*******!!!!!!!!!!;!!***====$$#=;;:~---------~:;**=$====*::~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T48,200);
	usleep(100);
	char T49[200] = "*!!;;;;;;!!!!!!!!!*!*****!*!!!!!!!!;;!!!**====$###$;;;~~---~:;;*========*;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T49,200);
	usleep(100);
	char T50[200] = "!!!!!!;!!!!!!!!!!!*********!**!!!!!;;;*======$$#$*!;;;:::!*==$$===*==!::~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T50,200);
	usleep(100);
	char T51[200] = "!!!!!!!!!!!!**!!!**!*******!!!**!!=$$========$$$$$==**===========*!:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T51,200);
	usleep(100);
	char T52[200] = "!!!!!*!!!!!!!!!!!*********================$$$$$$===============;:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T52,200);
	usleep(100);
	char T53[200] = ";!!!!!!!!!!!!!!!!!****=$$$$=======*======$$$$$$============*!:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T53,200);
	usleep(100);
	char T54[200] = "!!!!!!!!;!!*!!*====$$$$$====================$$=====**===*::~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T54,200);
	usleep(100);
	char T55[200] = "!!!!!!!!!!*=======$$==$====================$======***!!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T55,200);
	usleep(100);
	char T56[200] = "!!!**===============$==$=======***====*===========!:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T56,200);
	usleep(100);
	char T57[200] = "=$=================$$$=====**!*!**==*==========*;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T57,200);
	usleep(100);
	char T58[200] = "$===============$$$======*!!!;;!;!***=======*::~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T58,200);
	usleep(100);
	char T59[200] = "============$=========*!!;!;;!;;;;!**===**;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T59,200);
	usleep(100);
	char T60[200] = "=================***!!!!!;!!;;;;;;!!!=!:~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
	send_msg(T60,200);
	usleep(100);
	*/
}
