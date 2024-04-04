#include "Stash/FastaReader.h"

#include "btllib/seq_reader.hpp"

namespace Stash
{
	void ScopedFastaReader::loadAllReads( const char* path, std::vector< std::unique_ptr< Read > >& reads, uint64_t minLength )
	{
		ScopedFastaReader reader;
		reader.open( path );
		reader.loadReads( 0xFFFFFFFF, reads, minLength );
		reader.close();
	}

	void ScopedFastaReader::loadAllSequences( const char* path, std::vector< std::unique_ptr< Sequence > >& sequences, uint64_t minLength )
	{
		ScopedFastaReader reader;
		reader.open( path );
		reader.loadSequences( 0xFFFFFFFF, sequences, minLength );
		reader.close();
	}

	ScopedFastaReader::ScopedFastaReader()
		: m_reader( nullptr )
	{
	}

	ScopedFastaReader::~ScopedFastaReader()
	{
		close();
	}

	bool ScopedFastaReader::open( const char* path )
	{
		m_reader = std::make_unique<btllib::SeqReader>( path, btllib::SeqReader::Flag::LONG_MODE );
		return true;
	}

	void ScopedFastaReader::close()
	{
		if ( m_reader )
			m_reader->close();
	}

	uint32_t ScopedFastaReader::loadReads( uint32_t numberToRead, std::vector< std::unique_ptr< Read > >& reads, uint64_t minLength )
	{
		uint32_t count;

		for ( count = 0; count < numberToRead; count++ )
		{
			const auto record = m_reader->read();
			if ( !record )
				break;

			if ( record.seq.size() < minLength )
			{
				count--;
				continue;
			}

			std::unique_ptr< Read > read = std::make_unique< Read >( record.id, record.seq.c_str(), record.seq.size() );
			reads.push_back( std::move( read ) );
		}

		return count;
	}

	uint32_t ScopedFastaReader::loadSequences( uint32_t numberToRead, std::vector< std::unique_ptr< Sequence > >& sequences, uint64_t minLength )
	{
		uint32_t count;

		for ( count = 0; count < numberToRead; count++ )
		{
			const auto record = m_reader->read();
			if ( !record )
				break;

			if ( record.seq.size() < minLength )
			{
				count--;
				continue;
			}

			std::unique_ptr< Sequence > read = std::make_unique< Sequence >( record.id, record.seq.c_str(), record.seq.size() );
			sequences.push_back( std::move( read ) );
		}

		return count;
	}
}

