
#include <unistd.h>
#include "server/webserver.h"

int main(int argc, char **argv)
{

	// daemon(1, 0);

	WebServer server(
		1188, 3, 60000, false,			   // Port 1024-65535, trigMode, timedout, optLinger
		3306, "root", "1234", "webserver", // MySQL configurations
		12, 6, true, 1, 1024  		// connPoolNum, threadNum, openLog, logLevel, logQueSize
	);			   

	server.Start();
}
