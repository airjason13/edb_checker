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
char *cpu_id = "1234567890";
char eth_mac_addr[32];
#elif __aarch64__
char *cpu_id = "";
char eth_mac_addr[32];
#endif

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
	return mac;
}

char* get_board_cpu_id(void){
	char *line = NULL;
	size_t read_len = 0;
	ssize_t read;
	FILE *fp = fopen("/sys/firmware/devicetree/base/serial-number", "r");
	while ((read = getline(&line, &read_len, fp)) != -1) {
                printf("Retrieved line of length %zu:\n", read);
                printf("%s\n", line);
                printf("strlen of line :%ld\n", strlen(line));
	}
	return line;
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
	char *ip = NULL;
	size_t read_len = 0;
	ssize_t read;
	char wget_cmd[256];
	FILE *fp;
	bool ip_pass = false;
	bool mac_pass = false;
	int ret = -1;
#ifdef __x86_64__
	printf("x86\n");
#elif __aarch64__
	cpu_id = get_board_cpu_id();
	printf("self cpu id : %s\n", cpu_id);
	ret = get_eth_mac_addr(eth_mac_addr);
	printf("eth mac addr : %s\n", eth_mac_addr);
	return 0;
#else
	printf("unknow platform!\n");
#endif		

	/* Get id_data from server, once we parse the jason file, we delete it asap.*/
	FILE *server_ip_list_file = fopen("/tmp/.edb_server.dat", "r");
	if(server_ip_list_file == NULL){
		printf("no server ip !\n");
		return -1;
	}
	while ((read = getline(&line, &read_len, server_ip_list_file)) != -1) {
  	     	printf("Retrieved line of length %zu:\n", read);
        	printf("%s\n", line);
        	printf("strlen of line :%ld\n", strlen(line));
		line[strlen(line) - 1] = 0;	
		
		sprintf(wget_cmd, "wget -qO- http://%s:8080/id_data.json > /tmp/.id_data.json", line);
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
	const cJSON *name = NULL;
	const cJSON *check_item = NULL;
	const cJSON *check_items= NULL;
	char buffer[65535];
	int len = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);
	system("rm /tmp/.id_data.json");

	cJSON *json = cJSON_Parse(buffer);
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
			printf("cpu_id : %s\n", cpu_id->valuestring);
			printf("mac_id : %s\n", mac_id->valuestring);
		}else{
			printf("format error~!!\n");
		}
	}
	printf("parse ok!\n");
	cJSON_Delete(json);
	return 0;
}
