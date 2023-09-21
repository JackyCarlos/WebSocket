#include "ws.h"

int
main(int argc, char *argv[])
{
	if (ws_server() == -1)
		return -1;

	wsConnection *wsc = accept_ws_connection();


	return 0; 
}
