#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <iostream>
#include <fcntl.h>
 
struct MemoryStruct {
  char *memory;
  size_t size;
};

/* struct HelloCommand {
	int waiting;
	int the_flag;
	int interesting;
//	char * command;
	char command[50];
	char * data;
} currentHelloCommand; */
 
static void *myrealloc(void *ptr, size_t size);

/* Malloc function from the curl example, ensures that we use the right malloc */
static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */ 
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}
 
static size_t
fileCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
 
  mem->memory = (char *) myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  
  int handle = open("/dev/s3simple", O_RDWR);
  write(handle, mem->memory, realsize);
  if(handle > 0) close(handle);
  
  //printf("Request was performed.");
  
  return realsize;
}

static size_t
dirCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
 
  mem->memory = (char *) myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  
  int handle = open("/dev/s3simple", O_RDWR);
  write(handle, mem->memory, realsize);
  if(handle > 0) close(handle);
  
  //printf("Request was performed.");
  
  return realsize;
}

void performRequest(std::string filename ) {
	CURL *curl_handle;
 
  struct MemoryStruct chunk;
 
  chunk.memory = NULL; 
  chunk.size = 0;
 
  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();
 
 	std::string base_uri = "http://tylor.s3.amazonaws.com/";
 	if(filename == "dir") {
 		curl_easy_setopt(curl_handle, CURLOPT_URL, base_uri.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, dirCallback);
 	}
 	else {
		std::string url = base_uri + filename;
		curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, fileCallback);
 	}
 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
  curl_easy_perform(curl_handle);
  curl_easy_cleanup(curl_handle);
 
  if(chunk.memory)
    free(chunk.memory);
 
  curl_global_cleanup();
}

int checkForRequest(void) {
	char buf[200]; // need a better solution, might now always be big enough.
	size_t nbytes;
	ssize_t bytes_read;
  int handle = open("/dev/s3simple", O_RDONLY);
  
  nbytes = sizeof(buf);
  read(handle, buf, nbytes);
  
  // *** Code here is an attempt to rebuild a struct for better command handling.
  //struct HelloCommand * newCommand = (HelloCommand *)buf;
  
  //char cool[50];
  //strcpy(cool, newCommand->command);
  //printf("Work: %s\n", cool);
  
  /*std::cout << "Next might be something" << std::endl;
  std::cout << newCommand->command << std::endl;
  std::cout <<ps newCommand->interesting << std::endl;
    std::cout << newCommand->waiting << std::endl;
  printf("Num: %d\n", newCommand->waiting);
	printf("Hmm: %d\n", newCommand->interesting);
  //std::cout << cool << std::endl;
  std::cout << "After the something" << std::endl;*/
  //std::cout << buf << std::endl;
	//if(1) {
	//return 1;
	//}
  
  if(handle > 0) close(handle);
  
	// Check if we have a request.
  if(strcmp(buf, "0") != 0) {
  	//std::cout << buf << std::endl; // print request for debugging.
  	performRequest(buf);
  }
  
	return atoi(buf);
}
 
int main(int argc, char **argv)
{
	while(1) {
		checkForRequest();
  	sleep(0.1);
 }
  return 0;
}
