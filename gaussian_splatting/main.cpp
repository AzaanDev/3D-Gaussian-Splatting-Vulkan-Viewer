#include <iostream>

#include "application.h"

int main(int argc, char* argv[])
{
	if (argc != 2) {
		return 0;
	}

	Application app(argv[1]);
	app.Run();
	return 0;
}
