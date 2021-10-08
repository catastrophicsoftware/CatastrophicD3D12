#pragma once
#include "pch.h"

class DescriptorPoolFullException : public std::runtime_error
{
public:
	DescriptorPoolFullException(const char* msg) throw();

	virtual const char* what() const throw();
};