

#include <iostream>

#include "cd_import.h"

int main(int argc, char * argv[])
{
	QApplication app(argc, argv);

	CdImport ci;
	ci.show();

	auto retVal = app.exec();
	std::cout << "Bye now!" << std::endl;
	return retVal;
}

