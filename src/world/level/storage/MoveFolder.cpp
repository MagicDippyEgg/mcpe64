#include "MoveFolder.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdio>
#endif

void moveFolder(const std::string& src, const std::string& dst) {
#ifdef _WIN32
	MoveFileA(src.c_str(), dst.c_str());
#else
	::rename(src.c_str(), dst.c_str());
#endif
}