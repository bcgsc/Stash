#include "Stash/Sequence.h"

#include <cstring>
#include "CityHash/city.h"

namespace Stash
{
    Sequence::Sequence( const std::string& id, const char* sequence, uint64_t length )
        : m_id( id )
        , m_sequence( nullptr )
        , m_length( length )
    {
        m_sequence = new char[ length + 1 ];
        memcpy( m_sequence, sequence, length );
        m_sequence[ length ] = 0;
    }

    Sequence::~Sequence()
    {
        if ( m_sequence )
            delete[]( m_sequence );
    }

    SequenceView::SequenceView( const std::string& id, const char* sequence, uint64_t length )
        : m_id( id )
        , m_sequence( sequence )
        , m_length( length )
    {
    }

    Read::Read( const std::string& id, const char* sequence, uint64_t length )
        : Sequence( id, sequence, length )
	, m_hash1( CityHash::CityHash64( id.c_str(), id.size() ) )
        , m_hash2( CityHash::CityHash64( ( id + "{" ).c_str(), id.size() + 1 ) )
    {
    }
}
