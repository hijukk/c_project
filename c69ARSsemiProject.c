#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memcpy 메모리 복사
#include <curl/curl.h>
#include "c60json_c.c"
#include <sqlite3.h>
#include "c69HangeulSort.c"

sqlite3 *db;
sqlite3_stmt *stmt = NULL;
CURL *curl;


char phonechk_post[] = "http://192.168.216.1:8090/semiproject/ars_phonecheck.do";
char phonechk_params[] = "tel=";
char phoneplan_get[] = "http://192.168.216.1:8090/semiproject/ars_phoneplan.do";
char optservice_get[] = "http://192.168.216.1:8090/semiproject/ars_optservice.do";
//아이디 전역변수
char *id;

static size_t read_head_fun(void *ptr, size_t size, size_t nmemb, void *stream){
	char head[2048] = {0};
	memcpy(head,ptr,size*nmemb + 1);
	//서버 옵션 정보
	//printf("%s\n",head); 
	
	return size*nmemb;
}

typedef struct{
	int num;
	char *name;
	char *data;
	char *call;
	char *message;
	int price;
}PhonePlan;

typedef struct{
	char *id;
	char *jdate;
	char *tel;
}MyInfo;

typedef struct{
	char *ser_name;
	int ser_price;
	char *category;
}OptService;



/////function////
MyInfo phoneCheck();
void phonePlan();
void ppTable();
void db_open();

void myppTable(MyInfo mi);
int myppUpdate(PhonePlan *pp);
int myppList();

void optserTable();
void optserList(char *category);

void myoptserTable();
void myoptserList();
int myoptserUpdate(OptService *optser);
int myoptserInsert(OptService *optser);
int myoptserDelete(int ser);

void main_menu();
int count_Hangeul(char *string);
int print_space(int number);

int main(int argc, char **argv)
{	
	db_open();
	
	//pp table create&insert
	ppTable();
	
	//optService table create
	optserTable();
	
	//myoptService table create
	myoptserTable();
	
	//로그인&회원정보
	MyInfo mi;
	mi = phoneCheck();
	
	//mypp table create/////
	myppTable(mi);
	
	//전연변수 아이디 설정 
	id = mi.id;
	
	if(strlen(id)!=0){ //추가해주기 else문 
		main_menu();
		
	}else{
		printf("\n...\n");
		printf("전화번호 확인 후 다시 입력해주세요.\n");
		
	}
		
		
		
	
	
	
	
	
	
	return 0;
}//end main

//메인메뉴////////

void main_menu(){
	while(1){
		putchar('\n');
		printf("*******************\n");
		printf("%s님 안녕하세요!\n",id);
		printf("*******************\n\n");
		int main_num;
		printf("================메뉴=================\n");
		putchar('\n');
		printf("1. 부가서비스신청   2. 부가서비스조회\n");
		printf("3. 요금제 변경      4. 회원정보\n");
		putchar('\n');
		printf("=====================================\n");
		printf(">> 메뉴를 선택해주세요[로그아웃 100]: ");
		scanf("%d",&main_num);
		putchar('\n');
		
		if(main_num == 100){
			break;
		}
		
		if(main_num == 1){
				//printf("부가서비스신청..\n");
				int ser_num;
				printf("===부가서비스신청===\n");
				putchar('\n');
				printf("   1. 유료서비스\n   2. 무료서비스\n   3. 링투유\n");
				putchar('\n');
				printf("====================\n");
				printf(">> 메뉴를 선택해주세요...");
				scanf("%d",&ser_num);
				putchar('\n');
				
				char *category = malloc(sizeof(char)*10); 
				switch(ser_num){
				case 1:
					printf("=================유료서비스=================\n");
					category = "유료";
					optserList(category);
					break;
				case 2:
					printf("=================무료서비스=================\n");
					category = "무료";
					optserList(category);
					break;
				case 3:
					printf("===================링투유===================\n");
					category = "링투유";
					optserList(category);
					break;
				}
		}else if (main_num == 2){
			//printf("부가서비스조회..\n");
			myoptserList();
			
		}else if (main_num == 3){
			//printf("요금제변경..\n");
			phonePlan();
		}else if (main_num == 4){
			//printf("회원정보..\n");
			myppList();
		}
		
		
	}
}





/////서비스해지//////
int myoptserDelete(int ser){
	//printf("myoptserDelete : %d\n", ser);
	
	/////서비스명 알아내기////
	char ser_name[20];
	
	const char *sql_select_one =
			"select r.ser_name from (select row_number() over() as rnum, * from myoptservice order by rnum asc) r where r.rnum = ?;";
	sqlite3_prepare_v2(db, sql_select_one, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,ser); 
	
	while(sqlite3_step(stmt) == SQLITE_ROW){
		strcpy(ser_name, (char*)sqlite3_column_text(stmt,0));
	}	
	
	
	/////행번호로 삭제////
	const char *sql_delete =
			"delete from myoptservice where ser_name in (select r.ser_name from (select row_number() over() as rnum, * from myoptservice order by rnum asc) r where r.rnum = ?);";
	sqlite3_prepare_v2(db, sql_delete, -1, &stmt, NULL);
	
	//rs.set()
	sqlite3_bind_int(stmt,1,ser);
	
	int result_delete = sqlite3_step(stmt);
	//printf("result_delete: %d\n", result_delete);
	
	
	if(result_delete == SQLITE_DONE){
		printf("%s 서비스 해지완료 되었습니다!\n", ser_name);
		
	}
	return 0;
}



////myoptserviceList///
void myoptserList(){
	////printf("myoptserList...\n");
	//행번호와 함께 출력 
	const char *sql_select_all =
			"select row_number() over() as rnum, * from myoptservice order by rnum asc;";
	sqlite3_prepare_v2(db, sql_select_all, -1, &stmt, NULL);
	printf("=================부가서비스조회==================\n\n");
	
	printf("%6s | %-23s | %-8s | %-10s |\n", "번호", "서비스","월정액","카테고리");
	printf("%4c | %-20c | %-6c | %-8c |\n", '-', '-', '-', '-');
	while(sqlite3_step(stmt) == SQLITE_ROW){
		
		printf("%4d |", (int)sqlite3_column_int(stmt,0));
		printf("%-21.21s",(char*)sqlite3_column_text(stmt,1));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,1)));
		printf("|");
		printf("%6d원",(int)sqlite3_column_int(stmt,2));
		printf("|");
		printf("%9.9s",(char*)sqlite3_column_text(stmt,3));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,3)));
		printf("|\n");
	}
	printf("\n=================================================\n");
	
	int ser;
	printf("해지 원하는 서비스 번호를 입력하세요 [메인메뉴 0]: ");
	scanf("%d",&ser);
	if(ser == 0){
		return 0;
	}
	
	myoptserDelete(ser);
	
}

////myoptserInsert///
int myoptserInsert(OptService *optser){
	//printf("myoptserInsert...\n");
	//printf("%s %d %s\n", optser->ser_name, optser->ser_price, optser->category);
	
	const char *sql_insert =
		"insert into myoptservice(ser_name,ser_price,category) values(?,?,?);";
	sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);

	sqlite3_bind_text(stmt,1,optser->ser_name,-1,SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt,2,optser->ser_price);
	sqlite3_bind_text(stmt,3,optser->category,-1,SQLITE_TRANSIENT);
	
	
	int result_insert = sqlite3_step(stmt);
	
	if(result_insert == SQLITE_DONE){
		printf("%s 신청완료 되었습니다.\n",optser->ser_name);
	}else if (result_insert == 19){
		printf("%s 는 이미 신청된 서비스 입니다.\n",optser->ser_name);
	}
	
	return 0;
}

////myoptserviceUpdate///
int myoptserUpdate(OptService *optser){
	//printf("myoptserUpdate...\n");
	//printf("%s %d %s\n", optser->ser_name, optser->ser_price, optser->category);
	const char *sql_select_one =
			"select * from myoptservice where category = ?;";
	sqlite3_prepare_v2(db, sql_select_one, -1, &stmt, NULL);
	sqlite3_bind_text(stmt,1,optser->category,-1,SQLITE_TRANSIENT);
	
	//읽어올행이 있으면 update
	if(sqlite3_step(stmt) == SQLITE_ROW){
		const char *sql_update =
			"update myoptservice set ser_name=?, ser_price=? where category=?;";
		sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
		
		//rs.set()
		sqlite3_bind_text(stmt,1,optser->ser_name,-1,SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt,2,optser->ser_price);
		sqlite3_bind_text(stmt,3,optser->category,-1,SQLITE_TRANSIENT);
		
		int result_update = sqlite3_step(stmt); 
		
		if(result_update == SQLITE_DONE){
			printf("링투유 - %s로 변경 완료 되었습니다.\n", optser->ser_name);
		}
	}else{
		const char *sql_insert =
			"insert into myoptservice(ser_name,ser_price,category) values(?,?,?);";
		sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
	
		sqlite3_bind_text(stmt,1,optser->ser_name,-1,SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt,2,optser->ser_price);
		sqlite3_bind_text(stmt,3,optser->category,-1,SQLITE_TRANSIENT);
		
		
		int result_insert = sqlite3_step(stmt);
		
		if(result_insert == SQLITE_DONE){
			printf("링투유 - %s 신청완료 되었습니다.\n",optser->ser_name);
		}else if (result_insert == 19){
			printf("링투유 - %s 는 이미 신청된 서비스 입니다.\n",optser->ser_name);
		}
	}
	
	return 0;
}




////myoptservice Table/////
void myoptserTable(){

	const char *sql_create_table = 
		"create table if not exists myoptservice (ser_name text unique, ser_price integer, category text);";
	sqlite3_prepare_v2(db, sql_create_table, -1, &stmt, NULL);
	
	int result_create_tab = sqlite3_step(stmt); 
	
}



/////////optserList///////
void optserList(char *category){
	const char *sql_selectAll =
			"select * from optservice where category = ?;";
	
	sqlite3_prepare_v2(db, sql_selectAll, -1, &stmt, NULL);
	sqlite3_bind_text(stmt,1,category,-1,SQLITE_TRANSIENT);
	
	
	printf("%-24s | %-9s | %-11s |\n", "서비스","월정액","카테고리");
	printf("%-21c | %-6c | %-8c |\n", '-', '-', '-');
	while(sqlite3_step(stmt) == SQLITE_ROW){
		printf("%-21.21s",(char*)sqlite3_column_text(stmt,0));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,0)));
		printf("|");
		printf("%6d원",(int)sqlite3_column_int(stmt,1));
		printf("|");
		printf("%9.9s",(char*)sqlite3_column_text(stmt,2));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,2)));
		printf("|\n");
	
	}
	printf("============================================\n");
	char service2[40] = "%";
	char service[30];
	printf(">>서비스 검색 [뒤로 0]: ");
	scanf("%s",service);
	strcat(service2, service);
	strcat(service2, "%");
	
	putchar('\n');
	
	if(strcmp(service, "0") == 0){
		return 0;
	}
	
	const char *sql_searchList=
			"select row_number() over() as rnum, * from (select * from optservice where ser_name like ? and category = ?);";
	sqlite3_prepare_v2(db, sql_searchList, -1, &stmt, NULL);
	sqlite3_bind_text(stmt,1,service2,-1,SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt,2,category,-1,SQLITE_TRANSIENT);
	
	
	printf("================================================\n");	
	printf("%6s | %-23s | %-8s | %-10s |\n", "번호", "서비스","월정액","카테고리");
	printf("%4c | %-20c | %-6c | %-8c |\n", '-', '-', '-', '-');
	while(sqlite3_step(stmt) == SQLITE_ROW){
		OptService optser;
		
		int rnum = (int)sqlite3_column_int(stmt,0);
		optser.ser_name = (char*)sqlite3_column_text(stmt,1);
		optser.ser_price = (int)sqlite3_column_int(stmt,2);
		optser.category = (char*)sqlite3_column_text(stmt,3);
		
		////printf("%d %s %d원 %s\n", rnum, optser.ser_name, optser.ser_price, optser.category);
		
		printf("%4d |", rnum);
		printf("%-21.21s",optser.ser_name);
		print_space(count_Hangeul(optser.ser_name));
		printf("|");
		printf("%6d원",optser.ser_price);
		printf("|");
		printf("%9.9s",optser.category);
		print_space(count_Hangeul(optser.category));
		printf("|\n");
		
	}
	printf("================================================\n");
	
	int ser_menu;
	printf("원하시는 서비스 번호를 입력하세요: [메인메뉴 0] : ");
	scanf("%d", &ser_menu);
	printf("\n\n...\n");
	
	if(ser_menu == 0){
		return 0;
	} else{
		const char *sql_selectOne=
			"select * from optservice where ser_name = (select r.ser_name from (select row_number() over() as rnum, * from (select * from optservice where ser_name like ? and category = ?)) r  where r.rnum = ?);";
		sqlite3_prepare_v2(db, sql_selectOne, -1, &stmt, NULL);
		sqlite3_bind_text(stmt,1,service2,-1,SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt,2,category,-1,SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt,3,ser_menu);
		
		//읽어들일 행이 있다면 루프돌아라 
		while(sqlite3_step(stmt) == SQLITE_ROW){
			OptService optser;
			OptService *pt_optser = &optser;
			optser.ser_name = (char*)sqlite3_column_text(stmt,0);
			optser.ser_price = (int)sqlite3_column_int(stmt,1);
			optser.category = (char*)sqlite3_column_text(stmt,2);
			
			//printf("%s %d원 %s\n", optser.ser_name, optser.ser_price, optser.category);
			if(strcmp(optser.category,"링투유") == 0){
				myoptserUpdate(pt_optser);
			
			}else{
				myoptserInsert(pt_optser);
			}
		}
	}
	
}


//////////optsertable//////////
void optserTable(){
	curl = curl_easy_init();
	
	char *phoneplan_file = "c69optservice.json";
	FILE *pp_save_file = fopen(phoneplan_file,"w");
	
	
	if(curl){
		curl_easy_setopt(curl,CURLOPT_URL,optservice_get); // get
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,pp_save_file); 
		curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,read_head_fun);
		
		curl_easy_perform(curl);
	}
	
	fclose(pp_save_file);
	
	char json_pp_text[9000] = {0};
	pp_save_file = fopen(phoneplan_file,"r");
	fread(json_pp_text,1,9000,pp_save_file);
	////printf("%s\n",json_pp_text);
	fclose(pp_save_file);
	
	char *json_pp_str = json_pp_text;
	json_value vos = json_create(json_pp_str);
	////json_print(vos); putchar('\n');
	
	const char *sql_create_table = 
		"create table if not exists optservice (ser_name text primary key, ser_price integer, category text);";
	sqlite3_prepare_v2(db, sql_create_table, -1, &stmt, NULL);
	
	int result_create_tab = sqlite3_step(stmt); 
	
	for(int i = 0; i < json_len(vos); i++){
		json_value vo = json_get(vos, i);
		//json_print(vo); putchar('\n');
		
		char *ser_name = json_get_string(vo,"ser_name");
		int ser_price = json_get_int(vo,"ser_price");
		char *category = json_get_string(vo,"category");
		
		const char *sql_insert = "insert or ignore into optservice (ser_name, ser_price, category) values (?,?,?);";
		
		sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
		
		sqlite3_bind_text(stmt,1,ser_name,-1,SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt,2,ser_price);
		sqlite3_bind_text(stmt,3,category,-1,SQLITE_TRANSIENT);
		
		
		int result_insert = sqlite3_step(stmt);
				
	}
	
	json_free(vos);
	
	
}


////회원정보////
int myppList(){
	printf("-+-+-+-+-+회원정보-+-+-+-+-\n\n");
	
	const char *sql_select_all =
			"select * from myphoneplan;";
	
	sqlite3_prepare_v2(db, sql_select_all, -1, &stmt, NULL);
	
	while(sqlite3_step(stmt) == SQLITE_ROW){
		printf("  %s님의 요금제 정보\n",(char*)sqlite3_column_text(stmt,0));
		printf("  가입일자 : %s\n",(char*)sqlite3_column_text(stmt,1));
		printf("  요금제 : %s\n",(char*)sqlite3_column_text(stmt,2));
		printf("  이번달 청구금액 : %d원\n",(int)sqlite3_column_int(stmt,3));
	}//회원정보 gui
	
	printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
	
	int exit;
	printf(">> 메인메뉴 0: ");
	scanf("%d", &exit);
	if(exit == 0){
		return 0;
	}
	
}

////db_open////
void db_open(){
	int result_open = sqlite3_open("c69ars.db",&db);
	//printf("result_open: %d\n",result_open);
	
	//if(!result_open){
		//printf("db open successfully!!\n");
	//}
}

////요금제업데이트/////
int myppUpdate(PhonePlan *pp){
	////printf("myppUpdate %s %d\n",pp->name, pp->price);
	
	const char *sql_update =
			"update myphoneplan set pp_name=?, pp_price=?;";
	
	sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL);
	
	sqlite3_bind_text(stmt,1,pp->name,-1,SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt,2,pp->price);
	
	int result_update = sqlite3_step(stmt); 
	
	if(result_update ==  SQLITE_DONE){
		printf("요금제 변경이 완료 되었습니다!\n");
	}
	return 0;
	
}


////mypptable create&insert////
void myppTable(MyInfo mi){
	////printf("myppTable : %s %s\n",mi.id, mi.jdate);
	
	//db_open();
	
	const char *sql_create_table = 
		"create table if not exists myphoneplan (mi_id text primary key, mi_jdate text, pp_name text, pp_price integer);";
	sqlite3_prepare_v2(db, sql_create_table, -1, &stmt, NULL);
	
	int result_create_tab = sqlite3_step(stmt); 
	////table create
	if(result_create_tab == SQLITE_DONE){
		////insert////
		const char *sql_insert =
				"insert or ignore into myphoneplan (mi_id, mi_jdate) values(?,?);";
		
		sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
		
		sqlite3_bind_text(stmt,1,mi.id,-1,SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt,2,mi.jdate,-1,SQLITE_TRANSIENT);
		
		int result_insert = sqlite3_step(stmt); 
		////table insert
	}
}

////////ppTable create&insert////////
void ppTable(){
	PhonePlan pp;
	//PhonePlan *pt_pp = &pp;
	
	curl = curl_easy_init();
	
	char *phoneplan_file = "c69phoneplan.json";
	FILE *pp_save_file = fopen(phoneplan_file,"w");
	
	
	if(curl){
		curl_easy_setopt(curl,CURLOPT_URL,phoneplan_get); // get
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,pp_save_file); 
		curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,read_head_fun);
		
		curl_easy_perform(curl);
	}
	
	fclose(pp_save_file);
	
	char json_pp_text[4096] = {0};
	pp_save_file = fopen(phoneplan_file,"r");
	fread(json_pp_text,1,4096,pp_save_file);
	////printf("%s\n",json_pp_text);
	fclose(pp_save_file);
	
	char *json_pp_str = json_pp_text;
	json_value vos = json_create(json_pp_str);
	////json_print(vos); putchar('\n');
	

	const char *sql_create_table = 
		"create table if not exists phoneplan (num integer primary key autoincrement, name text unique, data text, call text, message text, price integer);";
	sqlite3_prepare_v2(db, sql_create_table, -1, &stmt, NULL);
	
	int result_create_tab = sqlite3_step(stmt); 
	
	if(result_create_tab==SQLITE_DONE){
		for(int i = 0; i < json_len(vos); i++){
			json_value vo = json_get(vos, i);
			////json_print(vo); putchar('\n');
			////printf("%s - ",json_get_string(vo,"name"));
			////printf("데이터 : %s | ",json_get_string(vo,"data"));
			////printf("전화 : %s | ",json_get_string(vo,"call"));
			////printf("문자 : %s | ",json_get_string(vo,"message"));
			////printf("요금 : %d원\n",json_get_int(vo,"price"));
			
			pp.name = json_get_string(vo,"name");
			pp.data = json_get_string(vo,"data");
			pp.call = json_get_string(vo,"call");
			pp.message = json_get_string(vo,"message");
			pp.price = json_get_int(vo,"price");
			
			const char *sql_insert = "insert into phoneplan (name, data, call, message, price) values (?,?,?,?,?);";
			
			sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL);
			
			sqlite3_bind_text(stmt,1,pp.name,-1,SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt,2,pp.data,-1,SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt,3,pp.call,-1,SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt,4,pp.message,-1,SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt,5,pp.price);
			
			int result_insert = sqlite3_step(stmt);
			//table insert
		}
	}
	json_free(vos);

}

////////phonePlan/////////////
void phonePlan(){
	PhonePlan pp;
	PhonePlan *pt_pp = &pp;
	
	
	const char *sql_select_all =
			"select * from phoneplan order by num asc;";
	
	sqlite3_prepare_v2(db, sql_select_all, -1, &stmt, NULL);
	printf("===========================요금제 변경=============================\n\n");
	printf(" no |    상품이름    |  데이터  |   통화   |    문자     |  가격  |\n");
	while(sqlite3_step(stmt) == SQLITE_ROW){
		/////가격 값 이상해짐 바꿔줘야함///회원정보 gui 늘려줄것
		printf("%3d |", (int)sqlite3_column_int(stmt,0));
		printf("%-15.15s",(char*)sqlite3_column_text(stmt,1));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,1)));
		printf("|");
		printf("%-9.9s",(char*)sqlite3_column_text(stmt,2));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,2)));
		printf("|");
		printf("%-9.9s",(char*)sqlite3_column_text(stmt,3));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,3)));
		printf("|");
		printf("%-12.12s",(char*)sqlite3_column_text(stmt,4));
		print_space(count_Hangeul((char*)sqlite3_column_text(stmt,4)));
		printf("|");
		printf("%6d원",(int)sqlite3_column_int(stmt,5));
		printf("|\n");
		
	}//요금제 전체 gui
	printf("\n===================================================================\n");
	///////////요금제선택//////////
	int plan_num;
	printf("원하는 데이터 요금제 번호를 입력하세요 [메인메뉴 0]: ");
	scanf("%d",&plan_num);
	////printf("요금제: %d\n",plan_num);
	if(plan_num == 0){
		return 0;
	}
	
	//////selectOne 요금제 선택////////////
	const char *sql_select_one =
	"select * from phoneplan where num = ?;";

	sqlite3_prepare_v2(db, sql_select_one, -1, &stmt, NULL);
	sqlite3_bind_int(stmt,1,plan_num); 
	
	
	while(sqlite3_step(stmt) == SQLITE_ROW){
		////printf("%d | ",(int)sqlite3_column_int(stmt,0));
		////printf("%s | ",(char*)sqlite3_column_text(stmt,1));
		////printf("%s | ",(char*)sqlite3_column_text(stmt,2));
		////printf("%s | ",(char*)sqlite3_column_text(stmt,3));
		////printf("%s | ",(char*)sqlite3_column_text(stmt,4));
		////printf("%d원\n",(int)sqlite3_column_int(stmt,5));
		
		pp.num = (int)sqlite3_column_int(stmt,0);
		pp.name = (char*)sqlite3_column_text(stmt,1);
		pp.data = (char*)sqlite3_column_text(stmt,2);
		pp.call = (char*)sqlite3_column_text(stmt,3);
		pp.message = (char*)sqlite3_column_text(stmt,4);
		pp.price = (int)sqlite3_column_int(stmt,5);
		////printf("%d %s %s %s %s %d\n", 
			////pp.num, pp.name, pp.data, pp.call, pp.message, pp.price);
		////printf("%d %s %s %s %s %d\n", 
			////pt_pp->num, pt_pp->name, pt_pp->data, pt_pp->call, pt_pp->message, pt_pp->price);
		
	}
	
	
	////요금제 업데이트////
	myppUpdate(pt_pp);
		
}

///////////phoneCheck////////////
MyInfo phoneCheck(){
	MyInfo mi;
	char *params = malloc(sizeof(char)*15);
	printf("핸드폰 번호를 입력하세요[ '-' 포함 ]... ");
	scanf("%15s",params);
	
	strcat(phonechk_params,params);
	//printf("phonechk_params:%s\n",phonechk_params);
	
	curl = curl_easy_init();
	
	char *phonechk_file = "c69phonechk.json";
	FILE *phone_save_file = fopen(phonechk_file,"w");
	
	
	if(curl){
		curl_easy_setopt(curl,CURLOPT_URL,phonechk_post); // post
		curl_easy_setopt(curl,CURLOPT_POSTFIELDS,phonechk_params);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,phone_save_file); 
		curl_easy_setopt(curl,CURLOPT_HEADERFUNCTION,read_head_fun);
		
		curl_easy_perform(curl);
	}
	
	fclose(phone_save_file);
		
	char json_phonechk_text[2048] = {0};
	phone_save_file = fopen(phonechk_file,"r");
	fread(json_phonechk_text,1,2048,phone_save_file);
	//printf("%s\n",json_phonechk_text);
	fclose(phone_save_file);
	
	if(strlen(json_phonechk_text)!=0){
		char *json_phonechk_str = json_phonechk_text;
		json_value vo = json_create(json_phonechk_str);
		//printf("%s ",json_get_string(vo,"id"));
		//printf("%s ",json_get_string(vo,"tel"));
		//putchar('\n');
		
		mi.id = json_get_string(vo,"id");
		mi.jdate = json_get_string(vo,"jdate");
		mi.tel = json_get_string(vo,"tel");
	}else{
		mi.id = "";
		mi.jdate = "";
		mi.tel = "";
	}
	
	return mi;
	
}




