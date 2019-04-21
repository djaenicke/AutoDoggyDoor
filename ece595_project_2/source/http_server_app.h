#ifndef HTTP_SERVER_APP_H_
#define HTTP_SERVER_APP_H_

#include "mdns.h"

extern void HTTP_Server_Socket_Init(void);
extern void http_srv_txt(struct mdns_service *service, void *txt_userdata);

#endif /* HTTP_SERVER_APP_H_ */
