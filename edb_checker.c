#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

#ifdef __x86_64__
char self_cpu_id[64];
char self_eth_mac_addr[32];
#elif __aarch64__
char self_cpu_id[64];
char self_eth_mac_addr[32];
#endif


int i_current_version_year = 0;
int i_current_version_month = 0;
int i_current_version_day = 0;
int i_current_version_major = 0;
int i_current_version_minor = 0;

char c_current_version[64] = {0};
 
#define EDB_VERSION_FILE_URI "./CONST.py" //for x86 debug

char edb_version_file_uri[256];


int get_current_edb_version(char *edb_ver_file_uri, char *cur_ver){
	printf("%s\n", edb_ver_file_uri);
	FILE *fp;
	char buffer[2048];
	bool b_got_year = false;
	bool b_got_month = false;
	bool b_got_day = false;
	bool b_got_major = false;
	bool b_got_minor = false;

	//fp = fopen("/home/eduarts/geany_code/EDB22NTA0/CONST.py", "r");
	fp = fopen(edb_version_file_uri, "r");
	if(fp == NULL){
		printf("version file missing!\n");
		return -1;
	}else{
		printf("ready to parse version file!\n");	
	}
	while(fscanf(fp, "%[^\n] ", buffer) != EOF){
		printf("%s\n", buffer);
		if(strstr(buffer, "VER_MAJOR")){
			printf("got ver major!\n");
			sscanf(buffer, "VER_MAJOR = %d\n", &i_current_version_major);
			b_got_major = true;
			printf("current version major : %d\n", i_current_version_major);
		}else if(strstr(buffer, "VER_MINOR")){
			printf("got ver minor!\n");
			sscanf(buffer, "VER_MINOR = %d\n", &i_current_version_minor);
			b_got_minor = true;
			printf("current version minor : %d\n", i_current_version_minor);
		}else if(strstr(buffer, "VER_YEAR")){
			printf("got ver year!\n");
			sscanf(buffer, "VER_YEAR = %d\n", &i_current_version_year);
			b_got_year = true;
			printf("current version year : %d\n", i_current_version_year);
		}else if(strstr(buffer, "VER_MONTH")){
			sscanf(buffer, "VER_MONTH = %d\n", &i_current_version_month);
			printf("current version month : %d\n", i_current_version_month);
			b_got_month = true;
			printf("got ver month!\n");
		}else if(strstr(buffer, "VER_DAY")){
			printf("got ver day!\n");
			sscanf(buffer, "VER_DAY = %d\n", &i_current_version_day);
			b_got_day = true;
			printf("current version day : %d\n", i_current_version_day);
		}
		if(b_got_year && b_got_month && b_got_day && b_got_major && b_got_minor){
			printf("got all version params!\n");
			break;
		}	
	}
	if(b_got_year && b_got_month && b_got_day && b_got_major && b_got_minor){
	}else{
		return -1;
	}
	sprintf(cur_ver, "Eudarts_%d_%02d_%02d_%02d_%02d.json", i_current_version_year, i_current_version_month, i_current_version_day, i_current_version_major, i_current_version_minor);
	printf("cur_ver : %s\n", cur_ver);

	fclose(fp);
	return 0;
}


int get_fname_from_uri(char *uri, char *fname){
	const char *del = "/";
	printf("uri : %s", uri);
	char *s = strtok(uri, del);
	printf("s : %s\n", s);
	while(s != NULL) {
		s = strtok(NULL, del);
		if(s != NULL){
			memcpy(fname, s, strlen(s));
		}
	}
	return 0;
}

int get_eth_mac_addr(char* mac){
	int fd;
	struct ifreq ifr;
	char *iface = "eth0";
	unsigned char *mac_tmp;
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFHWADDR, &ifr);

	close(fd);
	mac_tmp = (unsigned char *)ifr.ifr_hwaddr.sa_data;
	sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac_tmp[0], mac_tmp[1], mac_tmp[2], mac_tmp[3], mac_tmp[4], mac_tmp[5]);
	printf("mac : %s\n", mac);
	return 0;
}

int get_board_cpu_id(char *id){
	char *line = NULL;
	size_t read_len = 0;
	ssize_t read;
	FILE *fp = fopen("/sys/firmware/devicetree/base/serial-number", "r");
	while ((read = getline(&line, &read_len, fp)) != -1) {
                printf("Retrieved line of length %zu:\n", read);
                printf("%s\n", line);
                printf("strlen of line :%ld\n", strlen(line));
	}
	memcpy(id, line, strlen(line));
	return 0;
}

bool check_need_update(char *least_ver){
	char c_least_year[16] = {0};
	char c_least_month[16] = {0};
	char c_least_day[16] = {0};
	char c_least_major[16] = {0};
	char c_least_minor[16] = {0};
	int i_least_year = 0;
	int i_least_month = 0;
	int i_least_day = 0;
	int i_least_major = 0;
	int i_least_minor = 0;
	char name[64] = {0};
	printf("least_ver : %s\n", least_ver);
	sscanf(least_ver, "%7s_%d_%d_%d_%d_%d.json", name, &i_least_year, &i_least_month, &i_least_day, &i_least_major, &i_least_minor);
	//sscanf(least_ver, "Eduarts_%s_%s_%s_%s_%s.json", &c_least_year, &c_least_month, &c_least_day, &c_least_major, &c_least_minor);
	printf("name : %s\n", name);
	printf("version:  %d %d %d %d %d\n", i_least_year, i_least_month, i_least_day, i_least_major, i_least_minor);
	//printf("version:  %s %s %s %s %s\n", c_least_year, c_least_month, c_least_day, c_least_major, c_least_minor);
	if(i_least_year < i_current_version_year){
		return false;
	}
	if(i_least_month < i_current_version_month){
		return false;
	}
	if(i_least_day < i_current_version_day){
		return false;
	}
	if(i_least_major < i_current_version_major){
		return false;
	}
	if(i_least_minor < i_current_version_minor){
		return false;
	}
	if((i_least_year == i_current_version_year) && (i_least_month == i_current_version_month) && (i_least_day == i_current_version_day) 
		&& (i_least_major == i_current_version_major) && (i_least_minor == i_current_version_minor)){
		return false;	
	}	
	return true;

}

/******************************************************************************************
 *
 * edb_check
 *
 *
 * ***************************************************************************************/

int main(int argc, char* argv[]){
	char *line = NULL;
	char tmp[1024];
	char server_ip[64];
	size_t read_len = 0;
	ssize_t read;
	char wget_cmd[256];
	FILE *fp;
	//bool ip_pass = false;
	//bool mac_pass = false;
	int ret = -1;
	int len = 0;// for read file
	char buffer[65535]; // for read file
	bool b_machine_valid = false;
	char least_json_fname[64] = {0};
	char local_least_json_uri[128];
	char least_swu_fname[128];
	cJSON *json = NULL; //for parse json file	
	const cJSON *name = NULL;  // for parse json file
	const cJSON *check_item = NULL; //for parse json file
	const cJSON *check_items= NULL; //for parse json file
	const cJSON *least_swu_file_json = NULL; //for parse json file
	const cJSON *least_swu_file_jsons = NULL; //for parse json file
	const cJSON *old_swu_file_jsons = NULL; //for parse json file
	const cJSON *swu_fname_json = NULL; // for parse least json file to get swu file name

#ifdef __x86_64__
	ret = get_current_edb_version(EDB_VERSION_FILE_URI, c_current_version);
	if(ret < 0){
		printf("Cannot got current edb version!\n");
		return -1;
	}

#elif __aarch64__
	fp = popen("/usr/bin/find -L /home/eduarts/geany_code -name CONST.py", "r");
	if(fp == NULL){
		printf("find version file error!\n");
		return -1;
	}

	while (fgets(edb_version_file_uri, sizeof(edb_version_file_uri), fp) != NULL){
		printf("path :%s", edb_version_file_uri);
	}
	pclose(fp);
	edb_version_file_uri[strlen(edb_version_file_uri) - 1] = 0;
	printf("edb_version_file_uri :%s", edb_version_file_uri);
	ret = get_current_edb_version(edb_version_file_uri, c_current_version);
	if(ret < 0){
		printf("Cannot got current edb version!\n");
		return -1;
	}
#endif

#ifdef __x86_64__
	printf("x86\n");
	sprintf(self_cpu_id, "%s", "1234567890");
	sprintf(self_eth_mac_addr, "%s", "11:22:33:44:55:66");
	printf("self_cpu_id : %s\n", self_cpu_id);
	printf("self_eth_mac_addr : %s\n", self_eth_mac_addr);
#elif __aarch64__
	ret = get_board_cpu_id(self_cpu_id);
	printf("self cpu id : %s\n", self_cpu_id);
	ret = get_eth_mac_addr(self_eth_mac_addr);
	printf("self eth mac addr : %s\n", self_eth_mac_addr);
#else
	printf("unknow platform!\n");
#endif			
	/* Get id_data from server, once we parse the jason file, we delete it asap.*/
	FILE *server_ip_list_file = fopen("/etc/.edb_info.dat", "r");
	if(server_ip_list_file == NULL){
		printf("no server ip !\n");
		return -1;
	}
	while ((read = getline(&line, &read_len, server_ip_list_file)) != -1) {
  	     	printf("Retrieved line of length %zu:\n", read);
        	printf("%s\n", line);
        	printf("strlen of line :%ld\n", strlen(line));
		line[strlen(line) - 1] = 0;	
		memcpy(server_ip, line, strlen(line));
		printf("server_ip : %s\n", server_ip);
		sprintf(wget_cmd, "wget -qO- http://%s:8080/id_data.json > /tmp/.id_data.json", server_ip);
		printf("wget_cmd : %s\n", wget_cmd);
		system(wget_cmd);
		fp = fopen("/tmp/.id_data.json", "r");
		if(fp == NULL){
			printf("no file to parser\n");
			system("rm /tmp/.id_data.json");
			return -1;
		}else{
			
			break;
		}
	}
	len = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	system("rm /tmp/.id_data.json");

	json = cJSON_Parse(buffer);
	if(json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			printf("Error : %s\n", error_ptr);
		}
		cJSON_Delete(json);
		return -1;
	}

	name = cJSON_GetObjectItemCaseSensitive(json, "name");
	if(cJSON_IsString(name) && (name->valuestring != NULL)){
		printf("check name : %s\n", name->valuestring);
	}else{
		printf("format error!\n");
		return -1;
	}
	check_items = cJSON_GetObjectItemCaseSensitive(json, "check_items");
	cJSON_ArrayForEach(check_item, check_items){
		cJSON *cpu_id = cJSON_GetObjectItemCaseSensitive(check_item, "cpu_id");
		cJSON *mac_id = cJSON_GetObjectItemCaseSensitive(check_item, "mac_id");
		if(cJSON_IsString(cpu_id) && (cJSON_IsString(mac_id))){
			printf("check cpu_id : %s\n", cpu_id->valuestring);
			printf("check mac_id : %s\n", mac_id->valuestring);
			printf("self cpu_id : %s\n", self_cpu_id);
			printf("self eth_mac_addr : %s\n", self_eth_mac_addr);
			if(strstr(cpu_id->valuestring, self_cpu_id) != NULL){
				if(strstr(mac_id->valuestring, self_eth_mac_addr) != NULL){
					b_machine_valid = true;
					break;
				}
			}

		}else{
			printf("format error~!!\n");
		}
	}
	printf("parse ok!machine_valie : %d\n", b_machine_valid);
	cJSON_Delete(json);
	if(b_machine_valid == false){
		printf("machine is not valid!\n");
		return -1;
	}
	// machine is valid, go to download update information json
	sprintf(wget_cmd, "wget -qO- http://%s:8080/Eudarts_update_data.json > /tmp/.eudarts_update_data.json", server_ip);
	printf("wget_cmd : %s\n", wget_cmd);
	system(wget_cmd);
	fp = fopen("/tmp/.eudarts_update_data.json", "r");
	if(fp == NULL){
		printf("update_data.json missing!\n");
	}else{
		printf("ready to parse update_data.json!\n");	
	}

	len = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	system("rm /tmp/.eudarts_update_data.json");
	json = cJSON_Parse(buffer);
	if(json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			printf("Error : %s\n", error_ptr);
		}
		printf("cannot parse eduarts_update_data.json\n");
		cJSON_Delete(json);
		return -1;
	}
	printf("parse update data ok!\n");

	least_swu_file_jsons = cJSON_GetObjectItemCaseSensitive(json, "least_swu_file_json");
	cJSON_ArrayForEach(least_swu_file_json, least_swu_file_jsons){
		cJSON *file_uri = cJSON_GetObjectItemCaseSensitive(least_swu_file_json, "name");
		cJSON *file_md5 = cJSON_GetObjectItemCaseSensitive(least_swu_file_json, "md5");
		printf("file_uri : %s\n", file_uri->valuestring);	
		printf("file md5 : %s\n", file_md5->valuestring);
		ret = get_fname_from_uri(file_uri->valuestring, least_json_fname);
		printf("least_json_fname : %s\n", least_json_fname);	
	}
	printf("least_json_fname : %s\n", least_json_fname);	
	printf("current_version : %s\n", c_current_version);
	bool b_ret = check_need_update(least_json_fname);
	if(b_ret == false){
		printf("Do not need to update!\n");
		return -1;
	}
	printf("update!\n");

	// get the least json to parse
	sprintf(local_least_json_uri, "/tmp/.%s", least_json_fname);
	printf("local_least_json_uri : %s\n", local_least_json_uri);
	sprintf(wget_cmd, "wget -qO- http://%s:8080/%s > %s", server_ip, least_json_fname, local_least_json_uri);
	printf("wget_cmd : %s\n", wget_cmd);
	system(wget_cmd);

	fp = fopen(local_least_json_uri, "r");
	if(fp == NULL){
		printf("local_least_json missing!\n");
	}else{
		printf("ready to parse local_least_json!\n");	
	}

	len = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	//system("rm /tmp/.eudarts_update_data.json");
	json = cJSON_Parse(buffer);
	if(json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			printf("Error : %s\n", error_ptr);
		}
		printf("cannot parse least_json file\n");
		cJSON_Delete(json);
		return -1;
	}
		
	swu_fname_json = cJSON_GetObjectItemCaseSensitive(json, "swu_file_name");
	memcpy(least_swu_fname, swu_fname_json->valuestring, strlen(swu_fname_json->valuestring));
	printf("swu_fname : %s\n", least_swu_fname);
	printf("check!\n");

	// download the swu and check md5
	sprintf(wget_cmd, "wget -qO- http://%s:8080/%s > /tmp/%s", server_ip, least_swu_fname, least_swu_fname);
	printf("wget_cmd : %s\n", wget_cmd);
	system(wget_cmd);
	return 0;
}
