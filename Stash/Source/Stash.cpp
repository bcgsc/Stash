#include "Stash/Stash.h"

#include "Stash/FastaReader.h"
#include "btllib/nthash.hpp"
#include "Stash/Sequence.h"

#include <omp.h>
#include <fstream>
#include <iostream>
#include <cinttypes>

FILE* s_outStream = stdout;

#define STASH_LOG_INFO( message ) fprintf( s_outStream, "Stash> " message "\n" )
#define STASH_LOG_ERROR( message ) fprintf( s_outStream, "Stash> Error: " message "\n" )
#define STASH_LOG_INFO_PARAMS( message, ... ) fprintf( s_outStream, "Stash> " message "\n", __VA_ARGS__ )
#define STASH_LOG_ERROR_PARAMS( message, ... ) fprintf( s_outStream, "Stash> Error: " message "\n", __VA_ARGS__ )

namespace Stash
{
    Stash::Stash( uint32_t logRows, const std::vector<std::string>& spacedSeeds )
        : m_memory( nullptr )
        , m_rows( 1ull << logRows )
        , m_spacedSeedLength( 0 )
        , m_rawSeeds( spacedSeeds )
    {
        initialize();

	// Initialize the Stash memory.
        m_memory = new uint64_t[ m_rows ];
        memset( m_memory, 0x0, m_rows * sizeof( uint64_t ) );
    }

    void Stash::initialize()
    {
        if ( m_rawSeeds.size() != Consts::SPACED_SEED_COUNT )
        {
            STASH_LOG_ERROR_PARAMS( "There should be exactly %d spaced seeds.", Consts::SPACED_SEED_COUNT );
            exit( -1 );
        }

        m_lastRow = m_rows - 1;

	// Set up the spaced seeds for ntHash.
        m_spacedSeedLength = static_cast< uint32_t >( m_rawSeeds[ 0 ].size() );
        m_ntSeeds = btllib::parse_seeds( m_rawSeeds );
    }

    Stash::Stash( const char* stashPath )
    {
        FILE* file = fopen( stashPath, "rb" );
        if ( file == nullptr ){
            STASH_LOG_ERROR_PARAMS( "Cannot open Stash file: %s", stashPath );
            exit( -1 );
        }

        int legacy;
        fread( &legacy, sizeof( int ), 1, file );
        
        fread( &m_spacedSeedLength, sizeof( int ), 1, file );

        int spacedSeedCount;
        fread( &spacedSeedCount, sizeof( int ), 1, file );
        
        if ( spacedSeedCount != Consts::SPACED_SEED_COUNT )
        {
            STASH_LOG_ERROR_PARAMS( "Invalid Stash. There should be exactly %d spaced seeds.", Consts::SPACED_SEED_COUNT );
            exit( -1 );
        }

        char seed[ 200 ];
        for ( uint32_t i = 0; i < Consts::SPACED_SEED_COUNT; i++ ){
            fread( seed, sizeof( char ), m_spacedSeedLength, file );
            seed[ m_spacedSeedLength ] = 0;
            m_rawSeeds.emplace_back( seed );
        }

        fread( &m_rows, sizeof( uint64_t ), 1, file );

        int t1, t2;
        fread( &t1, sizeof( int ), 1, file );
        fread( &t2, sizeof( int ), 1, file );

        if ( t1 != Consts::T1 || t2 != Consts::T2 )
        {
            STASH_LOG_ERROR( "Invalid Stash. Invalid T1 or T2 parameters." );
            exit( -1 );
        }

        initialize();

        m_memory = new uint64_t[ m_rows ];
        fread( m_memory, sizeof( uint64_t ), m_rows, file );
        
        fclose( file );
    }

    bool Stash::save( const char* outputPath )
    {
        FILE* file = fopen( outputPath, "wb" );
        if ( file == nullptr ){
            STASH_LOG_ERROR_PARAMS( "Cannot open Stash file: %s", outputPath );
            return false;
        }

        int legacy = 0;
        fwrite( &legacy, sizeof( int ), 1, file );
        
        fwrite( &m_spacedSeedLength, sizeof( int ), 1, file );
        
        int spacedSeedCount = Consts::SPACED_SEED_COUNT;
        fwrite( &spacedSeedCount, sizeof( int ), 1, file );
        for ( int i = 0; i < spacedSeedCount; i++ )
            fwrite( m_rawSeeds[ i ].c_str(), sizeof( char ), m_spacedSeedLength, file );

        fwrite( &m_rows, sizeof( uint64_t ), 1, file );
        
        int t1 = Consts::T1;
        int t2 = Consts::T2;
        fwrite( &t1, sizeof( int ), 1, file );
        fwrite( &t2, sizeof( int ), 1, file );
        
        fwrite( m_memory, sizeof( uint64_t ), m_rows, file );

        fclose( file );

        STASH_LOG_INFO_PARAMS( "Successfully saved Stash: %s", outputPath );
        return true;
    }

    Stash::~Stash()
    {
        if ( m_memory )
            delete[]( m_memory );
    }

    struct ThreadData_Fill
    {
        uint8_t readIdTiles[ Consts::READ_ID_TILES * 2 ];
        uint8_t enoughPadding[ 512 - Consts::READ_ID_TILES * 2 ];
    };

    void Stash::fill( std::vector< std::unique_ptr< Read > >& reads, const uint32_t threads )
    {
	STASH_LOG_INFO_PARAMS( "Running Fill with %d threads.", threads );

        omp_set_num_threads( ( int32_t ) threads );

	// Each thread has its own local data.
        std::vector< ThreadData_Fill > threadData;
        threadData.resize( threads );

	// Split reads between threads.
	int64_t readsCount = ( int64_t ) reads.size();
#pragma omp parallel for schedule( dynamic )
        for ( int64_t i = 0; i < readsCount; ++i )
        {
            Read* read = reads[ i ].get();

            if ( i % 10000 == 0 )
            {
#pragma omp critical
                STASH_LOG_INFO_PARAMS( "Processing read %" PRId64 ".", i );
            }

	    // Ignore tiny reads.
            if ( read->m_length < m_spacedSeedLength )
                continue;

            uint8_t* readIdTiles = threadData[ omp_get_thread_num() ].readIdTiles;

            uint64_t hash1 = read->m_hash1;
            uint64_t hash2 = read->m_hash2;

	    // Create read ID hash tiles.
            for ( uint32_t tileIndex = 0; tileIndex < Consts::READ_ID_TILES * 2; )
            {
                readIdTiles[ tileIndex++ ] = hash1 & Consts::MAX_T1;
                readIdTiles[ tileIndex++ ] = hash2 & Consts::MAX_T2;

                hash1 >>= Consts::T1;
                hash2 >>= Consts::T2;
            }

	    // Roll over the sequence and perform insertions.
            btllib::SeedNtHash nt{ read->m_sequence, read->m_length, m_ntSeeds, 1, m_spacedSeedLength };
            while ( nt.roll() )
            {
                const uint64_t* hashes = nt.hashes();
                const uint64_t* last = hashes + 4;
                uint64_t xors = hashes[ 0 ] ^ hashes[ 1 ] ^ hashes[ 2 ] ^ hashes[ 3 ];

                while ( hashes < last )
                {
		    // Update the Stash tile.
                    uint64_t tileIndex = ( ( xors ^ *hashes ) & 7 ) << 1;

                    uint64_t row = *hashes++ & m_lastRow;
                    uint8_t column = readIdTiles[ tileIndex ];

                    uint64_t number = m_memory[ row ];

                    // Do not overwrite if non-zero.
                    uint64_t isolatedBits = ( Consts::MAX_T2 << ( column >> 2 ) ) & number;
                    if ( isolatedBits )
                        continue;

                    number |= ( uint64_t ) readIdTiles[ tileIndex + 1 ] << ( column >> 2 );
                    m_memory[ row ] = number;
                }
            }
        }
    }

    bool Stash::fill( const char* readsPath, const uint32_t threads )
    {
	STASH_LOG_INFO_PARAMS( "Running Fill with %d threads.", threads );

        ScopedFastaReader reader{};
        if ( !reader.open( readsPath ) )
        {
            STASH_LOG_ERROR_PARAMS( "Failed to open reads file: %s", readsPath );
            return false;
        }

        omp_set_num_threads( ( int32_t ) threads );

        std::vector< ThreadData_Fill > threadData;
        threadData.resize( threads );

        std::vector< std::unique_ptr< Read > > reads;

        uint32_t batchSize = 20000;
        uint32_t totalReadsProcessed = 0;

        while ( true )
        {
            uint32_t readCount = reader.loadReads( batchSize, reads, m_spacedSeedLength );

		int64_t readsCount = ( int64_t ) reads.size();
#pragma omp parallel for schedule( dynamic )
            for ( int64_t i = 0; i < readsCount; ++i )
            {
                Read* read = reads[ i ].get();

                uint8_t* readIdTiles = threadData[ omp_get_thread_num() ].readIdTiles;

                uint64_t hash1 = read->m_hash1;
                uint64_t hash2 = read->m_hash2;

                for ( uint32_t tileIndex = 0; tileIndex < Consts::READ_ID_TILES * 2; )
                {
                    readIdTiles[ tileIndex++ ] = hash1 & Consts::MAX_T1;
                    readIdTiles[ tileIndex++ ] = hash2 & Consts::MAX_T2;

                    hash1 >>= Consts::T1;
                    hash2 >>= Consts::T2;
                }

                btllib::SeedNtHash nt{ read->m_sequence, read->m_length, m_ntSeeds, 1, m_spacedSeedLength };
                while ( nt.roll() )
                {
                    const uint64_t* hashes = nt.hashes();
                    const uint64_t* last = hashes + 4;
                    uint64_t xors = hashes[ 0 ] ^ hashes[ 1 ] ^ hashes[ 2 ] ^ hashes[ 3 ];

                    while ( hashes < last )
                    {
                        uint64_t tileIndex = ( ( xors ^ *hashes ) & 7 ) << 1;

                        uint64_t row = *hashes++ & m_lastRow;
                        uint8_t column = readIdTiles[ tileIndex ];

                        uint64_t number = m_memory[ row ];

                        // Do not overwrite if non-zero.
                        uint64_t isolatedBits = ( Consts::MAX_T2 << ( column >> 2 ) ) & number;
                        if ( isolatedBits )
                            continue;

                        number |= ( uint64_t ) readIdTiles[ tileIndex + 1 ] << ( column >> 2 );
                        m_memory[ row ] = number;
                    }
                }
            }

            totalReadsProcessed += readCount;
            STASH_LOG_INFO_PARAMS( "Total Processed Reads: %" PRId32, totalReadsProcessed );

            if ( readCount != batchSize )
                break;

            reads.clear();
        }

        reader.close();

        return true;
    }

    struct ThreadData_Cut
    {
        std::vector< SequenceView > outputAssembly;
        uint64_t* frames;
        uint64_t* copySource;
        char header[ 2000 ];
        uint8_t enoughPadding[ 512 ];
    };

    bool Stash::cut( const char* assemblyPath, const char* outputPath, const WindowParameters& windowParameters, const CutParameters& cutParameters, const uint32_t threads ) const
    {
	STASH_LOG_INFO_PARAMS( "Running Cut with %d threads.", threads );

        ScopedFastaReader reader{};
        if ( !reader.open( assemblyPath ) )
        {
            STASH_LOG_ERROR_PARAMS( "Failed to open assembly file: %s", assemblyPath );
            return false;
        }

        std::ofstream outputFile;
        outputFile.open( outputPath );
        if ( !outputFile.is_open() ){
            STASH_LOG_ERROR_PARAMS( "Failed to open output file: %s", outputPath );
            return false;
        }

        omp_set_num_threads( ( int32_t ) threads );

	// Each thread has its own private data.
        std::vector< ThreadData_Cut > threadData;
        threadData.resize( threads );

        uint32_t batchSize = 0xFFFFFFFF; // TODO: Since we're not writing to file after each batch, we need to have a single batch.

        uint32_t chunkSize = 10000;
        uint32_t windowSize = windowParameters.GetWindowSize( m_spacedSeedLength );
        uint32_t shift = windowSize + windowParameters.delta / 2;
        uint32_t distance = windowSize + windowParameters.delta;
        uint64_t minContigLength = ( uint64_t ) ( distance + windowSize + 2 * cutParameters.maxPoolingRadius );
        uint32_t lastValidHashOffset = 2 * windowParameters.stride * ( windowParameters.numberOfFrames - 1 ) + windowParameters.delta;
        uint32_t copySize = lastValidHashOffset * Consts::SPACED_SEED_COUNT * sizeof( uint64_t );

	// Set up intermediate memory for each thread.
        for ( uint32_t i = 0; i < threads; i++ )
        {
            ThreadData_Cut& data = threadData[ i ];
            data.frames = new uint64_t[ Consts::SPACED_SEED_COUNT * chunkSize ];
            data.copySource = data.frames + ( chunkSize - lastValidHashOffset ) * Consts::SPACED_SEED_COUNT;
        }

        std::vector< std::unique_ptr< Sequence > > sequences;

        uint32_t totalSequencesProcessed = 0;

        while ( true )
        {
	    // Read sequences in batches.
            uint32_t readCount = reader.loadSequences( batchSize, sequences );

	    // Schedule the batch between threads.
            int sequencesCount = ( int64_t ) sequences.size();
#pragma omp parallel for schedule( dynamic )
            for ( int64_t i = 0; i < sequencesCount; ++i )
            {
                Sequence* sequence = sequences[ i ].get();

                ThreadData_Cut& threadExclusiveData = threadData[ omp_get_thread_num() ];

		// Ignore short sequences.
                if ( sequence->m_length < minContigLength )
                {
                    threadExclusiveData.outputAssembly.emplace_back( sequence->m_id, sequence->m_sequence, sequence->m_length );
                    continue;
                }

                uint64_t* frames = threadExclusiveData.frames;

                uint64_t maxHashes = sequence->m_length - m_spacedSeedLength + 1;
                uint64_t matchesLength = maxHashes - lastValidHashOffset;

                uint8_t* signal = new uint8_t[ matchesLength ];

                uint64_t hashCounter = 0;
                uint64_t currentBatchCounter = 0;
                uint8_t matchSet[ 16 * 16 ];

		// Generate the matches signal.
                btllib::SeedNtHash nt{ sequence->m_sequence, sequence->m_length, m_ntSeeds, 1, m_spacedSeedLength };
                while ( 1 ){
                    while ( hashCounter < maxHashes && currentBatchCounter < chunkSize ){
                        if ( nt.roll() )
                        {
                            uint64_t index = currentBatchCounter << 2;
                            for ( int i = 0; i < 4; i++ )
                                frames[ index++ ] = m_memory[ nt.hashes()[ i ] & m_lastRow ];
                        }
                        currentBatchCounter++;
                        hashCounter++;
                    }

                    for ( uint64_t position = hashCounter - currentBatchCounter; position < hashCounter - lastValidHashOffset; position++ ){
                        // Count number of matches between two windows.
			int maxMatches = 0;
                        uint64_t frameIndex = position - ( hashCounter - currentBatchCounter );
                        uint64_t lastFrame1 = frameIndex + windowParameters.numberOfFrames * windowParameters.stride;
                        uint64_t lastFrame2 = lastFrame1 + distance;
                        for ( uint64_t index1 = frameIndex; index1 < lastFrame1; index1 += windowParameters.stride ){
                            for ( uint64_t index2 = frameIndex + distance; index2 < lastFrame2; index2 += windowParameters.stride ){
                                memset( matchSet, 0x0, 256 );
                                for ( int row = 0; row < 4; ){
                                    uint64_t frame = frames[ ( index1 << 2 ) + row++ ];

                                    matchSet[ ( ( frame & 15 ) << 4 ) ] = 1;

                                    for ( int column = 1; column < 15; ){
                                        matchSet[ ( frame & 240 ) + column++ ] = 1;
                                        frame >>= 4;
                                    }

                                    matchSet[ ( frame & 240 ) + 15 ] = 1;
                                }

                                memset( matchSet, 0x0, 16 );

                                int matches = 0;
                                for ( int row = 0; row < 4; )
                                {
                                    uint64_t frame = frames[ ( index2 << 2 ) + row++ ];

                                    int pair = ( frame & 15 ) << 4;
                                    matches += matchSet[ pair ];
                                    matchSet[ pair ] = 0;

                                    for ( int column = 1; column < 15; )
                                    {
                                        pair = ( frame & 240 ) + column++;
                                        matches += matchSet[ pair ];
                                        matchSet[ pair ] = 0;
                                        frame >>= 4;
                                    }

                                    pair = ( frame & 240 ) + 15;
                                    matches += matchSet[ pair ];
                                    matchSet[ pair ] = 0;
                                }

                                if ( matches > maxMatches )
                                    maxMatches = matches;
                            }
                        }

                        signal[ position ] = maxMatches;
                    }

                    if ( hashCounter == maxHashes )
                        break;

                    memcpy( frames, threadExclusiveData.copySource, copySize );
                    currentBatchCounter = lastValidHashOffset;
                }

                uint64_t start = 0;
                uint64_t lastCutPosition = 0;
                uint64_t chainStart = 0;

		// Perform cutting over the matches signal.
                for ( uint64_t position = cutParameters.maxPoolingRadius + 1; position < matchesLength - cutParameters.maxPoolingRadius - 1; position++ )
                {
                    uint32_t max = 0;
                    for ( uint64_t j = position - cutParameters.maxPoolingRadius; j < position + cutParameters.maxPoolingRadius; j++ ){
                        if ( signal[ j ] > max )
                            max = signal[ j ];
                    }

                    if ( max < cutParameters.cutThreshold ){
                        if ( position - lastCutPosition >= cutParameters.minCutDistance || lastCutPosition == 0 ) {
                            if ( chainStart != 0 )
                            {
                                uint64_t end = ( chainStart + lastCutPosition ) / 2 + shift;

                                sprintf( threadExclusiveData.header, "%s:%" PRIu64 "-%" PRIu64, sequence->m_id.c_str(), start, end );
                                threadExclusiveData.outputAssembly.emplace_back( threadExclusiveData.header, sequence->m_sequence + start, end - start );

                                start = end;
                            }
                            chainStart = position;
                        }
                        lastCutPosition = position;
                    }
                }

		// Write the output.
                if ( chainStart != 0 ){
                    uint64_t end = ( chainStart + lastCutPosition ) / 2 + shift;

                    sprintf( threadExclusiveData.header, "%s:%" PRIu64 "-%" PRIu64, sequence->m_id.c_str(), start, end );
                    threadExclusiveData.outputAssembly.emplace_back( threadExclusiveData.header, sequence->m_sequence + start, end - start );

                    start = end;
                }

                if ( start )
                    sprintf( threadExclusiveData.header, "%s:%" PRIu64 "-%" PRIu64, sequence->m_id.c_str(), start, sequence->m_length );
                else
                    sprintf( threadExclusiveData.header, "%s", sequence->m_id.c_str() );

                threadExclusiveData.outputAssembly.emplace_back( threadExclusiveData.header, sequence->m_sequence + start, sequence->m_length - start );

                delete[]( signal );
            }

            totalSequencesProcessed += readCount;
            STASH_LOG_INFO_PARAMS( "Total Processed Sequences: %" PRId32, totalSequencesProcessed );

            if ( readCount != batchSize )
                break;

            sequences.clear();
        }

        reader.close();

        bool first = true;
        for ( auto& threadExclusiveData : threadData )
        {
            for ( size_t i = 0; i < threadExclusiveData.outputAssembly.size(); i++ ){
                if ( first )
                {
                    first = false;
                    outputFile << ">" << threadExclusiveData.outputAssembly[ i ].m_id.c_str() << "\n";
                }
                else
                    outputFile << "\n>" << threadExclusiveData.outputAssembly[ i ].m_id.c_str() << "\n";

                outputFile.write( threadExclusiveData.outputAssembly[ i ].m_sequence, threadExclusiveData.outputAssembly[ i ].m_length );
            }
        }

        outputFile << "\n";
        outputFile.close();

        return true;
    }
}
