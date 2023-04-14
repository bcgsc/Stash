#pragma once

#include <string>

namespace Stash
{
	struct Sequence
	{
		Sequence( const std::string& id, const char* sequence, uint64_t length );
		~Sequence();

		std::string m_id;
		char* m_sequence;
		uint64_t m_length;
	};

	struct SequenceView
	{
		SequenceView( const std::string& id, const char* sequence, uint64_t length );

		std::string m_id;
		const char* m_sequence;
		uint64_t m_length;
	};

	struct Read : public Sequence
	{
		Read( const std::string& id, const char* sequence, uint64_t length );

		uint64_t m_hash1;
		uint64_t m_hash2;
	};
}
