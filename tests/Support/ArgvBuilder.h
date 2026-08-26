#pragma once

#include <string>
#include <vector>

namespace tests
{

//! Owns a vector<string> of argv entries and exposes them as argc/argv for
//! CliOptions::parse(). The mutable char* pointers are only ever read by
//! parse(), never written back through.
class ArgvBuilder
{
	//
	// Construction and destruction.
	//
public:
	//! Constructor.
	explicit ArgvBuilder(std::vector<std::string> args)
		: args_{ std::move(args) }
	{
		for (auto& arg : args_)
		{
			pointers_.push_back(arg.data());
		}
	}

	//
	// Public interface.
	//
public:
	//! Argument count, as parse() expects it.
	int argc() const
	{
		return static_cast<int>(pointers_.size());
	}
	//! Argument vector, as parse() expects it.
	char** argv()
	{
		return pointers_.data();
	}

	//
	// Private data members.
	//
private:
	//! Backing storage for the argument strings.
	std::vector<std::string> args_;
	//! Non-owning pointers into args_, in the shape parse() requires.
	std::vector<char*> pointers_;
};

}	 // namespace tests
