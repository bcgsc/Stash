#include "CLI11.hpp"
#include "Stash/Stash.h"

int parseCommandLineArguments( int argc, char* argv[] )
{
	std::string dummy;

	CLI::App stashApp{ "Stash Data Structure" };
	stashApp.require_subcommand( 1 );

	stashApp.set_version_flag( "-v,--version", "Stash Version: " STASH_VERSION, "Displays the version of Stash.");
	stashApp.set_help_flag( "-h,--help", "Displays the help menu." );

	std::string readsPath, stashPath, assemblyPath, outputPath;
	uint32_t logRows, threads, numberOfFrames, stride, delta, cutThreshold, maxPoolingRadius, minCutDistance;

	auto stashFillArguments = stashApp.add_subcommand( "fill", "Creates and fills a Stash using the input reads." );
	stashFillArguments->add_option( "-r,--reads", readsPath, "Input Reads (fasta)" )->required();
	stashFillArguments->add_option( "-o,--output", outputPath, "Output Path" )->required();
	stashFillArguments->add_option( "-l,--log_rows", logRows, "Log2 of Number of Rows" )->default_val( 30 );
	stashFillArguments->add_option( "-t,--threads", threads, "Number of Threads" )->default_val( 8 );

	auto stashCutArguments = stashApp.add_subcommand( "cut", "Detects and cuts the misassembled contigs of the input assembly." );
	stashCutArguments->add_option( "-a,--assembly", assemblyPath, "Input Assembly (fasta)" )->required();
	stashCutArguments->add_option( "-s,--stash", stashPath, "Stash Path" )->required();
	stashCutArguments->add_option( "-o,--output", outputPath, "Output Path" )->required();
	stashCutArguments->add_option( "-n,--number_of_frames", numberOfFrames, "Number of Frames" )->group( "Stash Window" )->default_val( 1 );
	stashCutArguments->add_option( "-r,--stride", stride, "Stride" )->group( "Stash Window" )->default_val( 13 );
	stashCutArguments->add_option( "-l,--delta", delta, "Delta" )->group( "Stash Window" )->default_val( 751 );
	stashCutArguments->add_option( "-x,--threshold", cutThreshold, "Cut Threshold" )->group( "Cut Parameters" )->default_val( 11 );
	stashCutArguments->add_option( "-m,--max_pooling_radius", maxPoolingRadius, "Max Pooling Radius" )->group( "Cut Parameters" )->default_val( 1 );
	stashCutArguments->add_option( "-d,--min_cut_distance", minCutDistance, "Min Cut Distance" )->group( "Cut Parameters" )->default_val( 1000 );
	stashCutArguments->add_option( "-t,--threads", threads, "Number of Threads" )->default_val( 8 );

	if ( argc == 1 )
	{
		std::cout << stashApp.help( "", CLI::AppFormatMode::All );
		return 0;
	}

	try {
		stashApp.parse( argc, argv );
	}
	catch ( const CLI::ParseError& e ) {
		int returnValue = stashApp.exit( e );
		std::cout << "\n" << stashApp.help() << std::endl;

		return returnValue;
	}

	if ( stashApp.get_subcommands()[ 0 ] == stashFillArguments )
	{
		std::vector<std::string> seeds = {
			"10111111111111111101",
			"11011111111111111011",
			"11101111111111110111",
			"11110111111111101111",
		};

		Stash::Stash stash{ logRows, seeds };
		stash.fill( readsPath.c_str(), threads );
		stash.save( outputPath.c_str() );
	}
	else
	{
		Stash::Stash stash{ stashPath.c_str() };
		stash.cut( assemblyPath.c_str(), outputPath.c_str(), { numberOfFrames, stride, delta }, { cutThreshold, maxPoolingRadius, minCutDistance }, threads );
	}

	return 0;
}

int main( int argc, char* argv[] )
{
	return parseCommandLineArguments( argc, argv );
}
