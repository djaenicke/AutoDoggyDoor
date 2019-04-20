#ifndef SERVER_H_
#define SERVER_H_

#include "mdns.h"

extern void Run_HTTPServer(void);
extern void http_srv_txt(struct mdns_service *service, void *txt_userdata);

#endif /* SERVER_H_ */
