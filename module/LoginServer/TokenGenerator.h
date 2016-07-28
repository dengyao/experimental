#ifndef __TOKEN_GENERATOR_H__
#define __TOKEN_GENERATOR_H__

#include <random>
#include <numeric>
#include <cstdint>

class TokenGenerator
{
public:
	TokenGenerator()
		: distribution_(0, std::numeric_limits<uint32_t>::max())
	{
	}

	uint64_t operator() (uint32_t user_id) const
	{
		uint64_t token = distribution_(generator_);
		token <<= sizeof(uint32_t) * 8;
		token += user_id;
		return token;
	}

private:
	mutable std::default_random_engine generator_;
	mutable std::uniform_int_distribution<uint32_t> distribution_;
};

#endif