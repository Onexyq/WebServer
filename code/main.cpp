
#include<unistd.h>
#include "server/webserver.h"


int main(int argc, char **argv) {

	//daemon(1, 0);

	WebServer server(80, 60000, false);

	server.Start();

}
