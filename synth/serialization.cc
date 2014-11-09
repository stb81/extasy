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
 
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include "serialization.h"

namespace Synth {

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

void Serializer::write_to_file(const char* filename) const
{
	FILE* file=fopen(filename, "w");
	fwrite(&buffer[0], 1, buffer.size(), file);
	fclose(file);
}

void Serializer::store(int v)
{
	unsigned char* ptr=reinterpret_cast<unsigned char*>(&v);
	
	for (int i=0;i<sizeof(v);i++)
		buffer.push_back(ptr[i]);
}

void Serializer::store(unsigned int v)
{
	unsigned char* ptr=reinterpret_cast<unsigned char*>(&v);
	
	for (int i=0;i<sizeof(v);i++)
		buffer.push_back(ptr[i]);
}

void Serializer::store(float v)
{
	unsigned char* ptr=reinterpret_cast<unsigned char*>(&v);
	
	for (int i=0;i<sizeof(v);i++)
		buffer.push_back(ptr[i]);
}

void Serializer::store(const char* v)
{
	int i=0;
	
	while (v[i])
		buffer.push_back(v[i++]);

	for (i&=3; i<4; i++)
		buffer.push_back(0);
}

void Serializer::store(const std::string& v)
{
	for (auto i=v.begin();i!=v.end();i++)
		buffer.push_back(*i);

	for (int i=v.size()&3;i<4;i++)
		buffer.push_back(0);
}

void Serializer::store_nullptr()
{
	store("null");
	store(0);
}

void Serializer::putc(unsigned char c)
{
	buffer.push_back(c);
}

void Serializer::align()
{
	while (buffer.size()&3)
		buffer.push_back(0);
}


Serializer::Tag::Tag(Serializer& ser, const char* name):ser(ser), name(name)
{
	ser << name;
	
	offset=ser.buffer.size();
	
	ser << 0;
}

Serializer::Tag::~Tag()
{
	ser.align();
	
	int length=ser.buffer.size() - offset - sizeof(int);
	*reinterpret_cast<int*>(&ser.buffer[offset])=length;
}


Deserializer::Deserializer(Mixer& mixer):mixer(mixer)
{
}

void Deserializer::read(int& val)
{
	val=*reinterpret_cast<int*>(buffer+readptr);
	readptr+=sizeof(int);
}

void Deserializer::read(unsigned int& val)
{
	val=*reinterpret_cast<unsigned int*>(buffer+readptr);
	readptr+=sizeof(unsigned int);
}

void Deserializer::read(float& val)
{
	val=*reinterpret_cast<float*>(buffer+readptr);
	readptr+=sizeof(float);
}

void Deserializer::read(std::string& str)
{
	str.clear();
	
	while (buffer[readptr])
		str.push_back(buffer[readptr++]);
		
	readptr+=3;
	readptr&=-4;
}

bool Deserializer::seektag(const char* tag)
{
	int ptr=readptr;
	
	while (ptr<size) {
		char* thistag=buffer+ptr;
		
		bool match=strcmp(buffer+ptr, tag)==0;
		while (buffer[ptr++]);
		
		ptr+=3;
		ptr&=-4;
		
		if (match) {
			readptr=ptr;
			return true;
		}

		printf("Skipping tag '%s'\n", thistag);
		
		ptr+=*reinterpret_cast<int*>(buffer+ptr) + sizeof(int);
	}
	
	return false;
}

unsigned char Deserializer::getc()
{
	return buffer[readptr++];
}

void Deserializer::align()
{
	readptr=(readptr+3) & -4;
}

Deserializer::Tag::Tag(Deserializer& parent, const char* tag):Deserializer(parent.mixer)
{
	if (parent.seektag(tag)) {
		parent.read(size);
		buffer=parent.buffer + parent.readptr;
		parent.readptr+=size;
		
		module=parent.module;
	}
	else {
		buffer=nullptr;
		size=0;
		
		module=nullptr;
		
		printf("Tag '%s' not found\n", tag);
	}
	
	readptr=0;
}


FileDeserializer::FileDeserializer(const char* filename, Mixer& mixer):Deserializer(mixer)
{
	FILE* file=fopen(filename, "r");
	
	fseek(file, 0, SEEK_END);
	size=ftell(file);
	fseek(file, 0, SEEK_SET);
	
	buffer=new char[size];
	fread(buffer, 1, size, file);
	
	fclose(file);
	
	readptr=0;
}

FileDeserializer::~FileDeserializer()
{
	delete[] buffer;
}

}
