//
// Created by baymin on 19-8-16.
//
#ifdef WIN32 
	#include "draw.h"
#else
	#include "draw.h"
	#include <curl/curl.h>

int draw(char *postUrl, char *postVal) {
	CURL *curl;
	CURLcode res;
	FILE* fptr;
	struct curl_slist *http_header = NULL;

	//    if ((fptr = fopen(FILENAME,"w")) == NULL)
	//    {
	//        fprintf(stderr,"fopen file error:%s\n",FILENAME);
	//        return -1;
	//    }

	curl = curl_easy_init();
	if (!curl)
	{
		fprintf(stderr, "curl init failed\n");
		return -1;
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8"));
	curl_easy_setopt(curl, CURLOPT_URL, postUrl); //url地址
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postVal); //post参数
														 //    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data); //对返回的数据进行操作的函数地址
														 //    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fptr); //这是write_data的第四个参数值
	curl_easy_setopt(curl, CURLOPT_POST, 1); //设置问非0表示本次操作为post
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0); //打印调试信息
	curl_easy_setopt(curl, CURLOPT_HEADER, 0); //将响应头信息和相应体一起传给write_data
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0); //设置为非0,响应头信息location
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/Users/zhu/CProjects/curlposttest.cookie");
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		return -1;
	}
	curl_easy_cleanup(curl);
}

#endif


//#define POSTFIELDS "[{\"x\": 22233.00, \"y\": 3434444.33, \"win_id\": \"窗体名称->就是当前容器的id+该图标的含义\", \"title\": \"窗体显示的名称\"}]"
//#define FILENAME   "curlposttest.log"
