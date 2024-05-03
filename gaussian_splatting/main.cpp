#include <iostream>

#include "engine.h"

int main(int argc, char* argv[])
{
	//if (argc != 2) {
	//	return 0;
	//}

	//Application app(argv[1]);
	//app.Run();
	Engine e;
	e.Init();
	e.Run();
	return 0;
}
