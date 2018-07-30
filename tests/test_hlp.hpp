#pragma once
#include <boost/test/test_tools.hpp>
#include <string>
#include <stdexcept>

class CheckMessage
{
public:
	CheckMessage(std::string msg) : msg_{ std::move(msg) } {}
	bool operator()(std::exception const& e) const
	{
//		BOOST_WARN(msg_ == e.what());
		auto res = msg_.compare(e.what()) == 0;
		BOOST_WARN_MESSAGE(res, msg_ << "!=" << e.what());
		return res;
	}
private:
	std::string msg_;
};
