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
 
#ifndef INCLUDE_SYNTH_SERIALIZATION_H
#define INCLUDE_SYNTH_SERIALIZATION_H

namespace Synth {

class Mixer;
class Serializer;


template<typename T>
class serialization_tag_t {
	friend class Deserializer;
	
	const char*	name;
	T&			value;
	
public:
	serialization_tag_t(const char* name, T& value):name(name), value(value) {}
	
	void serialize(Serializer& ser) const;
};


template<typename T>
class serialization_tag_ptr_t {
	friend class Deserializer;
	
	const char*	name;
	const T*	value;
	
public:
	serialization_tag_ptr_t(const char* name, const T* value):name(name), value(value) {}
	
	void serialize(Serializer& ser) const;
};


template<typename T>
class serialization_tag_with_default_t {
	friend class Deserializer;
	
	const char*	name;
	T&			value;
	T			default_value;
	
public:
	serialization_tag_with_default_t(const char* name, T& value, T default_value):name(name), value(value), default_value(default_value) {}
};


class Serializer {
	std::vector<unsigned char>	buffer;
	
	template<typename T>
	void store(const std::vector<T>& v)
	{
		store((int) v.size());
		
		for (auto i=v.begin(); i!=v.end(); i++)
			store(*i);
	}
	
	template<typename T>
	void store(const T& v)
	{
		v.serialize(*this);
	}

	void store(int);
	void store(float);
	void store(const char*);
	void store(const std::string&);
	
public:
	Serializer();
	~Serializer();
	
	void write_to_file(const char*) const;
	
	template<typename T>
	Serializer& operator<<(const T* v)
	{
		v->serialize(*this);
		return *this;
	}
	
	template<typename T>
	Serializer& operator<<(T* v)
	{
		v->serialize(*this);
		return *this;
	}
	
	template<typename T>
	Serializer& operator<<(const T& v)
	{
		store(v);
		return *this;
	}
	
	Serializer& operator<<(const char* v)
	{
		store(v);
		return* this;
	}
	
	class Tag {
		Serializer&	ser;
		const char*	name;
		int			offset;
		
	public:
		Tag(Serializer& ser, const char* name);
		~Tag();
	};
};


class Deserializer {
protected:
	Mixer&	mixer;

	char*	buffer;
	int		readptr;
	int		size;
	
	template<typename T>
	void read(std::vector<T>& v)
	{
		int n;
		
		read(n);
		
		v.resize(n);
		
		for (int i=0;i<n;i++)
			read(v[i]);
	}
	
	template<typename T>
	void read(T& v)
	{
		v.deserialize(*this);
	}
	
	void read(int&);
	void read(float&);
	void read(std::string&);
	
	bool seektag(const char*);
	
public:
	class Tag;
	
	Deserializer(Mixer&);
	
	template<typename T>
	Deserializer& operator>>(T& v)
	{
		read(v);
		return *this;
	}
	
	template<typename T>
	Deserializer& operator>>(T*& v)
	{
		v=T::deserialize(*this);
		return *this;
	}
	
	template<typename T>
	Deserializer& operator>>(const serialization_tag_t<T>& tag);

	template<typename T>
	Deserializer& operator>>(const serialization_tag_with_default_t<T>& tag);
	
	Mixer& get_mixer() const
	{
		return mixer;
	}
	
	bool is_valid() const
	{
		return buffer!=nullptr;
	}
};


class Deserializer::Tag:public Deserializer {
public:
	Tag(Deserializer&, const char* tag);
};


class FileDeserializer:public Deserializer {
public:
	FileDeserializer(const char*, Mixer&);
	~FileDeserializer();
};


template<typename T>
serialization_tag_t<T> tag(const char* name, T& value)
{
	return serialization_tag_t<T>(name, value);
}

template<typename T>
serialization_tag_ptr_t<T> tag(const char* name, const T* value)
{
	return serialization_tag_ptr_t<T>(name, value);
}

template<typename T>
serialization_tag_with_default_t<T> tag(const char* name, T& value, T default_value)
{
	return serialization_tag_with_default_t<T>(name, value, default_value);
}

template<typename T>
void serialization_tag_t<T>::serialize(Serializer& ser) const
{
	Serializer::Tag tag(ser, name);
	ser << value;
}

template<typename T>
void serialization_tag_ptr_t<T>::serialize(Serializer& ser) const
{
	Serializer::Tag tag(ser, name);
	ser << value;
}

template<typename T>
Deserializer& Deserializer::operator>>(const serialization_tag_t<T>& tag)
{
	Tag buf(*this, tag.name);
	buf >> tag.value;
	return *this;
}

template<typename T>
Deserializer& Deserializer::operator>>(const serialization_tag_with_default_t<T>& tag)
{
	Tag buf(*this, tag.name);
	
	if (buf.is_valid())
		buf >> tag.value;
	else
		tag.value=tag.default_value;
		
	return *this;
}

}

#endif