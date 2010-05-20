/*
 * $Id: getinmemory.c,v 1.13 2008-09-06 04:28:45 yangtse Exp $
 *
 * Example source code to show how the callback function can be used to
 * download data into a chunk of memory instead of storing it in a file.
 *
 * This exact source code has not been verified to work.
 */ 
 
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
 
static void *myrealloc(void *ptr, size_t size);
 
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
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;
 
  mem->memory = (char *) myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  
  int handle = open("/dev/hello", O_RDWR);
  write(handle, mem->memory, realsize);
  if(handle > 0) close(handle);
  
  //printf("Request was performed.");
  
  return realsize;
}

void performRead(void) {
	CURL *curl_handle;
 
  struct MemoryStruct chunk;
 
  chunk.memory = NULL; /* we expect realloc(NULL, size) to work */ 
  chunk.size = 0;    /* no data at this point */ 
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  /* init the curl session */ 
  curl_handle = curl_easy_init();
 
 
 
  /* specify URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, "http://tylor.s3.amazonaws.com/README_public.txt");
 
  /* send all data to this function  */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
 
  /* we pass our 'chunk' struct to the callback function */ 
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
  /* some servers don't like requests that are made without a user-agent
     field, so we provide one */ 
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
  /* get it! */ 
  curl_easy_perform(curl_handle);
 
  /* cleanup curl stuff */ 
  curl_easy_cleanup(curl_handle);
 
	std::cout << "Request performed." << std::endl;
/*
	int i;
	for (i = 0; i < chunk.size; i++) {
		//std::cout << chunk.memory[i] << handle;
		putchar(chunk.memory[i]);
		//printf("\n");
	}
//	std::cout << std::endl;
	printf("\n");*/

  /*
   * Now, our chunk.memory points to a memory block that is chunk.size
   * bytes big and contains the remote file.
   *
   * Do something nice with it!
   *
   * You should be aware of the fact that at this point we might have an
   * allocated data block, and nothing has yet deallocated that data. So when
   * you're done with it, you should free() it as a nice application.
   */ 
 
  if(chunk.memory)
    free(chunk.memory);
 
  /* we're done with libcurl, so clean it up */ 
  curl_global_cleanup();
}

int checkRequest(void) {
	char buf[20];
	size_t nbytes;
	ssize_t bytes_read;
  int handle = open("/dev/hello", O_RDONLY);
  
  nbytes = sizeof(buf);
  read(handle, buf, nbytes);
  
  if(handle > 0) close(handle);
  
  if(atoi(buf) != 0) {
  	performRead();
  }
  
	return atoi(buf);
}
 
int main(int argc, char **argv)
{
	while(1) {
		checkRequest();
  	sleep(0.1);
 }
  return 0;
}
