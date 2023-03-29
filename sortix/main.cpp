#include "Sortix.h"
#include <iostream>

int main(int argc, char* argv[]) {
	sortix::Sort sort = sortix::Sort(argc, argv);
	if (!sort.isCongigOk()) return 1;
	sort.sort();
	return 0;
}