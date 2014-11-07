/*   extasy - Extensible Tracker And Synthesizer
 *   Copyright (C) 2014 Stefan T. Boettner
 *
 *   extasy is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   extasy is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with extasy.  If not, see <http://www.gnu.org/licenses/>. */
 
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "filechooser.h"

namespace GUI {

FileChooser::FileChooser()
{
	char* tmp=get_current_dir_name();
	directory=tmp;
	free(tmp);
}

FileChooser::~FileChooser()
{
}

void FileChooser::scan_directory()
{
	files.clear();
	
	DIR* dir=opendir(directory.c_str());
	
	if (!dir) {
		fprintf(stderr, "Error reading directory %s\n", directory.c_str());
		return;
	}
	
	while (struct dirent* entry=readdir(dir)) {
		File file;
		struct stat stbuf;
		
		if (entry->d_name[0]=='.') continue;
		
		std::string fullpath=directory;
		fullpath+='/';
		fullpath+=entry->d_name;
		
		if (stat(fullpath.c_str(), &stbuf)) continue;
		
		if (!S_ISREG(stbuf.st_mode) && !S_ISDIR(stbuf.st_mode)) continue;
		
		file.name=entry->d_name;
		file.type=S_ISDIR(stbuf.st_mode) ? -1 : 0;
		
		files.push_back(file);
	}
	
	closedir(dir);
	
	std::sort(files.begin(), files.end());
}

}
