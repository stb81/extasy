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
 
#ifndef INCLUDE_GUI_FILECHOOSER_H
#define INCLUDE_GUI_FILECHOOSER_H

#include <vector>
#include <string>

namespace GUI {
	
class FileChooser {
protected:
	struct File {
		std::string	name;
		int			type;
		
		bool operator<(const File& rhs) const
		{
			if (type<0 && rhs.type>=0) return true;
			if (type>=0 && rhs.type<0) return false;
			
			return name < rhs.name;
		}
	};
	
	std::string			directory;
	std::vector<File>	files;
	
	FileChooser();
	virtual ~FileChooser();
	
	void scan_directory();
};

}

#endif
