#pragma once

#include "Sequence.h"

#include <vector>
#include <memory>

namespace btllib
{
	class SeqReader;
}

namespace Stash
{
	class ScopedFastaReader
	{
	public:
		static void loadAllReads( const char* path, std::vector< std::unique_ptr< Read > >& reads, uint64_t minLength = 0 );
		static void loadAllSequences( const char* path, std::vector< std::unique_ptr< Sequence > >& sequences, uint64_t minLength = 0 );

		ScopedFastaReader();
		~ScopedFastaReader();

		bool open( const char* path );
		void close();

		uint32_t loadReads( uint32_t numberToRead, std::vector< std::unique_ptr< Read > >& reads, uint64_t minLength = 0 );
		uint32_t loadSequences( uint32_t numberToRead, std::vector< std::unique_ptr< Sequence > >& sequences, uint64_t minLength = 0 );

	private:
		std::unique_ptr<btllib::SeqReader> m_reader;
	};
}

