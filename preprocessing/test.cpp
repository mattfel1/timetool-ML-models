#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
typedef std::vector<std::string> stringvec;
void read_directory(const std::string& name, stringvec& v)
{
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    while ((dp = readdir(dirp)) != NULL) {
        v.push_back(dp->d_name);
    }
    closedir(dirp);
}