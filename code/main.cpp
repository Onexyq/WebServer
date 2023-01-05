
#include <unistd.h>
#include "server/webserver.h"

int main(int argc, char **argv)
{

	// daemon(1, 0);

	WebServer server(
		80, 3, 60000, false,			   // Port, trigMode, timedout, optLinger
		3306, "root", "root", "webserver", // MySQL configurations
		12, 6, true, 1, 1024  		// connPoolNum, threadNum, openLog, logLevel, logQueSize
	);			   

	server.Start();
}
