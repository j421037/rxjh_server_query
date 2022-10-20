#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/bufferevent_struct.h>

#define RXJH_SERVER_IP "113.196.245.139"
#define RXJH_SERVER_PORT 13503

const char code01[] =  {  
	0xd0, 0x80, 0x0d, 0x00, 0x07, 0x00, 0x00, 0x00, 
	0x07, 0x00, 0x77, 0x68, 0x68, 0x31, 0x30, 0x30, 
	0x39
};

const char code02[] = {
	0x00, 0x80, 0x2f, 0x00, 0x07, 0x00, 0x57, 0x48, 
	0x48, 0x31, 0x30, 0x30, 0x39, 0x10, 0x00, 0x06, 
	0x49, 0xe8, 0x6a, 0xf3, 0xd8, 0xf5, 0xc2, 0x08, 
	0xdb, 0x99, 0x4d, 0x96, 0x6d, 0xa9, 0xbd, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00
};

const char code03[] = {
	0x4c, 0x80, 0x01, 0x00, 0x02
};

const char code04[] = {
	0x48, 0x80, 0x0b, 0x00, 0x09, 0x00, 0x5c, 0x61, 
	0x6e, 0x67, 0x31, 0x32, 0x33, 0x34, 0x35
};

const char code05[] = {
	0x16, 0x80, 0x00, 0x00
};

/* global vals */
struct event_base* _g_base = NULL;
char _g_list[10][32] = { 0 };

//log function
static void vlog(const char*, const char*, int, const char*, const char*, ...);
// service run
#define LOG_PRINTF(level, arg...) vlog(__FILE__, __FUNCTION__, __LINE__, level, ##arg)
static void client_run();
// on connect created..
static void on_client_connected(struct bufferevent*, void*);
// on connect event change 
static void client_event_cb(struct bufferevent*, short, void*);
// 
static void client_read_cb(struct bufferevent*, void*);

static void api_service_fn(struct evhttp_request*, void*);
// run server service
static void server_run();

static void interval_fn(evutil_socket_t, short, void *);

void on_client_connected(struct bufferevent* bev, void* ctx) {
	LOG_PRINTF("INFO", "Connected..");
	bufferevent_write(bev, code01, sizeof(code01));
}

/* implement function */
void client_event_cb(struct bufferevent* bev, short events, void* ctx)
{
	if ( events & BEV_EVENT_CONNECTED ) {
        int i = 1;
        evutil_socket_t fd = bufferevent_getfd(bev);
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &i, sizeof(i));
        on_client_connected(bev, ctx);
        return;
    }
    else if ( events & BEV_EVENT_ERROR ) {
        LOG_PRINTF("ERROR", "connect server failed");
        bufferevent_free(bev);
    }
}

void client_read_cb(struct bufferevent* bev, void* ctx) {
	char *buff = NULL;
	size_t sz = 0, lx = 0, ly = 0;

	int idx, r = 0, i = 0, lst_idx = 0;
    unsigned char line[512] = { 0 };
    unsigned char val = 0;
    char name[512] = { 0 };
	char list[10][32] = { 0 };

	if ( bev == NULL ) {
		LOG_PRINTF("ERROR", "Invalid buffer...");
		goto EXIT;
	}

	sz = evbuffer_get_length(bev->input) + 1;
	buff = (char*) malloc(sz);
	ly = sizeof(list) / sizeof(list[0]);
	lx = sizeof(list[0]);

	if ( buff == NULL ) {
		LOG_PRINTF("ERROR", "Failed to malloc..");
		goto EXIT;
	}

	memset(buff, 0x00, sz);
	if ( evbuffer_copyout(bev->input, buff, sz - 1) <= 0 ) {
		LOG_PRINTF("ERROR", "Copy buff failed...");
		goto EXIT;
	}

//	for ( idx = 0; idx < sz - 1; ++idx ) {
//		printf("%02x", (unsigned char)buff[idx]);
//	}

//	printf("\n");

	if ( (unsigned char)buff[0] == 0xd1 && (unsigned char) buff[1] == 0x80 ) {
		LOG_PRINTF("INFO", "Try to send code02..");
		bufferevent_write(bev, code02, sizeof(code02));
	}

	if ( (unsigned char)buff[0] == 0x01 && (unsigned char) buff[1] == 0x80 ) {
		LOG_PRINTF("INFO", "Try to send code03...");
		bufferevent_write(bev, code03, sizeof(code03));
	}

	if ( (unsigned char)buff[0] == 0x4d && (unsigned char)buff[1] == 0x80 ) {

		LOG_PRINTF("INFO", "Try to send code04...");
		bufferevent_write(bev, code04, sizeof(code04));
	}
	
	if ( (unsigned char)buff[0] == 0x49 && (unsigned char)buff[1] == 0x80 ) {
		/*
			result like: 1780d600010001000400a6dcb44c4f0000000b000a000a00010009005365727665722d3031fa001500020009005365727665722d3032fa001500030009005365727665722d3033fa001500040009005365727665722d3034fa00150005000d005365727665722d303528504b29fa00170006000d005365727665722d303628504b29fa00170007000d005365727665722d303728504b29fa001700080013005365727665722d303820285072656d69756d2932001500090013005365727665722d303920285072656d69756d29320015000000010030fa000000
		*/
		LOG_PRINTF("INFO", "Try to send code04...");
		bufferevent_write(bev, code05, sizeof(code05));
	}
	
	
	if ( (unsigned char)buff[0] == 0x17 && (unsigned char)buff[1] == 0x80 ) {

		LOG_PRINTF("INFO", "Try to close connection");
		bufferevent_free(bev);
//		event_base_loopbreak(_g_base);

		for ( idx = 0; idx < sz - 1; ++idx ) {
			if ( buff[idx] == 0x53 && buff[idx - 1] == 0x00 ) {
				r = 1;
				i = idx;
			}
		
		// 	printf("%02x", buff[idx]);
			if ( r == 1 && buff[idx] == 0x00 ) {
				memset(line, 0x00, sizeof(line));
				memset(name, 0x00, sizeof(name));

				memcpy(line, buff + i, idx - i);
			//	printf("line strlen = %ld, i = %d, idx = %d\n", strlen(line), i, idx);
				memcpy(name, line, idx - i - 1);
				val = line[idx - i - 1];

				memset(line, 0x00, sizeof(line));

				if ( val ==  0xfa ) {
					snprintf(line, sizeof(line), "%s 100%%", name);
				}
				else {
					snprintf(line, sizeof(line), "%s %u%%", name, val);
				}

			//	printf("%\n");
				printf("%s\n", line);
				memset(list[lst_idx], 0x00, lx);
				memcpy(list[lst_idx], line, lx);

				r = 0;
				++lst_idx;
			}
		}

		memset(_g_list, 0x00, sizeof(_g_list));
		memcpy(_g_list, list, sizeof(_g_list));
	}

EXIT:

	if ( buff != NULL ) {
		evbuffer_drain(bev->input, sz - 1);
		free(buff);
	}

	return;
}

void client_run() {
	int fd = 0;
	struct sockaddr_in addr;
	struct bufferevent *bev = NULL;

	memset(&addr, 0x00, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(RXJH_SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr(RXJH_SERVER_IP);
	
	// _g_base = event_base_new();
	if ( _g_base == NULL ) goto EXIT;

	bev = bufferevent_socket_new(_g_base, -1, BEV_OPT_CLOSE_ON_FREE);
	if ( bev == NULL )  {
		LOG_PRINTF("ERROR", "Failed create bufferevent...");
		goto EXIT;
	}

	bufferevent_setcb(bev, client_read_cb, NULL, client_event_cb, NULL);
	bufferevent_enable(bev, EV_READ | EV_WRITE);
	
	if ( bufferevent_socket_connect(
		bev, (struct sockaddr*)&addr, sizeof(addr)
		) < 0 ) {
	
		LOG_PRINTF("ERROR", "Connect Failed..");
		goto EXIT;
	}

	LOG_PRINTF("INFO", "client is running...");
	//bufferevent_setcb(bev, client_read_cb, NULL, client_event_cb, NULL);
	// bufferevent_enable(bev, EV_READ | EV_WRITE);
	//event_base_dispatch(_g_base);
	
EXIT:
	//if ( _g_base != NULL ) {
	//	event_base_free(_g_base);
	// }
	return;
}

void server_run() {
	struct timeval timeout;
	struct event* ev = NULL;
	struct evhttp *http = NULL;
	struct evhttp_bound_socket *handle = NULL;

	_g_base = event_base_new();
	if ( _g_base == NULL ) {
		goto EXIT;
	}

	/* create http server */
	http = evhttp_new(_g_base);
	if ( http == NULL ) {
		LOG_PRINTF("Error", "Failed to create http server...");
		goto EXIT;
	}

	/* set http api callback */
	evhttp_set_cb(http, "/api/rxjh", api_service_fn, NULL);

	/* add event loop */
	ev = event_new(_g_base, -1, EV_PERSIST, interval_fn, 0);
	if ( ev != NULL ) {
		timeout.tv_sec = 5; //interlval 5 seconds
		timeout.tv_usec = 0;
		event_add(ev, &timeout);
	}

	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", 10086);
	if ( handle == NULL ) {
		LOG_PRINTF("Error", "http server port bind failed...");
		goto EXIT;
	}

	LOG_PRINTF("INFO", "http server start on 0.0.0.0:10086...");
	event_base_dispatch(_g_base);

EXIT:

	if ( http != NULL ) {
		evhttp_free(http);
	}
	
	if ( _g_base != NULL ) {
		event_base_free(_g_base);
	}
}

void interval_fn(evutil_socket_t evfd, short what, void* ctx) {
	client_run();
}

void api_service_fn(struct evhttp_request* request, void* ctx) {
	int idx;
	char item[40] = { 0 };
	char ptr[1024] = { 0 }; // json array string
	struct evbuffer *evb = NULL;
	
	LOG_PRINTF("INFO", "try to reply..");
	evb = evhttp_request_get_output_buffer(request);
	if ( evb == NULL ) {
		goto EXIT;
	}

	for ( idx = 0; idx < sizeof(_g_list) / sizeof(_g_list[0]); ++idx ) {
		memset(item, 0x00, sizeof(item));

		if ( strlen(_g_list[idx]) > 0 ) {
			snprintf(item, sizeof(item), "\"%s\",", _g_list[idx]);
			strncat(ptr, item, strlen(item));
		}
	}

	/* erase "," for jsons string */
	if ( ptr[strlen(ptr) - 1] == ',' ) {
		ptr[strlen(ptr) - 1] = '\0';
	}

	evhttp_add_header(evhttp_request_get_output_headers(request),
		"Content-Type", "application/json"
	);
	evbuffer_add_printf(evb, "{ \"code\": 0, \"data\": [%s]}", ptr);
	evhttp_send_reply(request, 200, "Ok", evb);

EXIT:
	LOG_PRINTF("INFO", "request done...");
	

}

static void vlog(const char* file, const char* func, int line, const char* level, const char* fmt, ...)
{
    time_t timer;
    struct tm *ptm = NULL;
    char buffer[4096] = { 0 };

    /* check level */
    if ( level == NULL ) {
        printf("+-[Error]: invalid arguments by %s\n", __FUNCTION__);
        return;
    }

    /* parse format */
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    /* init timer */
    time(&timer);
    ptm = localtime(&timer);

    if ( strcasecmp("debug", level) == 0 ) {
        printf("%d-%02d-%02d %02d:%02d:%02d %s <%s - %s - %d>:   %s\n",
               ptm->tm_year + 1900,
               ptm->tm_mon + 1,
               ptm->tm_mday,
               ptm->tm_hour,
               ptm->tm_min,
               ptm->tm_sec,
               level,
               file,
               func,
               line,
               buffer
        );
    } else {
        printf("%d-%02d-%02d %02d:%02d:%02d %s <%s - %d>:   %s\n",
               ptm->tm_year + 1900,
               ptm->tm_mon + 1,
               ptm->tm_mday,
               ptm->tm_hour,
               ptm->tm_min,
               ptm->tm_sec,
               level,
               func,
               line,
               buffer
        );
    }

    fflush(stdout);
}

int main(int argc, char* argv[]) {
	server_run();
	return 0;
}





