#include "pch.h"
#include "Exceptions.h"

DescriptorPoolFullException::DescriptorPoolFullException(const char* msg) throw() : std::runtime_error(msg)
{
}

const char* DescriptorPoolFullException::what() const throw()
{
	return this->what();
}