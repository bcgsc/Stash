#pragma once

#include "Sequence.h"
#include <btllib/nthash.hpp>

#define STASH_VERSION "1.2.0"

namespace Stash
{
	struct WindowParameters;
	struct CutParameters;

	namespace Consts
	{
		constexpr uint64_t T1 = 4;
		constexpr uint64_t T2 = 4;
		constexpr uint32_t READ_ID_TILES = 8;
		constexpr uint32_t SPACED_SEED_COUNT = 4;
		constexpr uint64_t MAX_T1 = (1 << T1) - 1;
		constexpr uint64_t MAX_T2 = (1 << T2) - 1;
		constexpr uint32_t B1 = T1 * READ_ID_TILES;
		constexpr uint32_t B2 = T2 * READ_ID_TILES;
	}

	class Stash
	{
	public:
		Stash( uint32_t logRows, const std::vector< std::string >& spacedSeeds );
		Stash( const char* stashPath );
		~Stash();

		bool fill( const char* readsPath, const uint32_t threads );
		void fill( std::vector< std::unique_ptr< Read > >& reads, const uint32_t threads );
		bool cut( const char* assemblyPath, const char* outputPath, const WindowParameters& windowParameters, const CutParameters& cutParameters, const uint32_t threads ) const;

		bool save( const char* outputPath );

	private:
		void initialize();

	private:
		uint64_t* m_memory;
		
		uint64_t m_rows;
		uint64_t m_lastRow;

		uint32_t m_spacedSeedLength;
		std::vector<btllib::hashing_internals::SpacedSeed> m_ntSeeds;
		std::vector<std::string> m_rawSeeds;
	};

	struct WindowParameters
	{
		uint32_t numberOfFrames;
		uint32_t stride;
		uint32_t delta;

		uint32_t GetWindowSize( uint32_t spacedSeedLength ) const { return ( numberOfFrames - 1 ) * stride + spacedSeedLength; }
	};

	struct CutParameters
	{
		uint32_t cutThreshold;
		uint32_t maxPoolingRadius;
		uint32_t minCutDistance;
	};
}
