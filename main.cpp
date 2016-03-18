#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include "lodepng.h"

#ifdef _WIN32
	#include <direct.h>
#else
	#include <unistd.h>
#endif

int TARGET_SIZE = 512;
int SPRTES_PER_ROW = 8;

bool too_many_sprites = false;
bool sprites_different_sizes = false;
bool sprites_not_square = false;

// Copys a single pixel from one PNG to another
void copyPixel( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	std::vector<unsigned char>& src,
	unsigned x,
	unsigned y,
	unsigned srcWidth );

// Adds the sprite to the sprite sheet, can handle sprites of varying dimensions
bool addSprite( std::vector<unsigned char>& dest, const char* filename );

// Returns the string with leading and trailing whitespace removed
std::string trimWhitespace( const std::string& str );

// Prints a pretty error message
void warning( std::string msg );

// Functions for pretty console output
std::string red() {
#ifdef _WIN32
	return "";
#else
	return "\033[31m";
#endif
}
std::string emphasis() {
#ifdef _WIN32
	return "";
#else
	return "\033[36m";
#endif
}
std::string normal() {
#ifdef _WIN32
	return "";
#else
	return "\033[39m";
#endif
}

int main( int argc, char* argv[])
{
	std::string exepath = "";

	// Need to prepend the exe dir or loadPNG will be unable to find the files
	exepath += argv[0];

	for( int i = exepath.length() - 1; i > 0; i-- )
	{
		if( exepath[i] == '/') {
			exepath = exepath.substr(0, i + 1);
			break;
		}
	}

	// The output PNG image
	std::vector<unsigned char> output;
	output.resize( TARGET_SIZE * TARGET_SIZE * 4 );
	for( int i = 0; i < output.size(); i++ ) output[i] = 0;

	std::string outputName = "";
	unsigned destX = 0;
	unsigned destY = 0;

	std::ifstream spritelist(exepath + "spritelist.txt");
	if( spritelist )
	{
		while( spritelist )
		{
			std::string line;
			std::getline( spritelist, line );

			if( !line.empty() )
			{
				// Ignore lines starting with #
				line = trimWhitespace( line );
				if( line[0] == '#' ) continue;
				if( outputName.empty() ) {
					outputName = exepath + line;
					std::cout << "Creating spritesheet " << outputName << std::endl;
					continue;
				}

				// Stack as many sprites horizontaly as possible
				// then move to the next row, the height of the tallest sprite in the first row

				std::cout << "\tAdding " << emphasis() << line << normal() << " \t";
				line = exepath + line;

				addSprite( output, line.c_str() );
			}
		}

		std::cout << "Encoding spritesheet...";
		lodepng::encode( outputName, output, TARGET_SIZE, TARGET_SIZE );
		std::cout << "\tdone!" << std::endl;
		std::cout << "Sprite sheet size " << emphasis() << TARGET_SIZE << normal() << " pixels square" << std::endl;

		// Report any errors or warnings
		if( sprites_different_sizes )
			warning("Sprites are of different sizes!");
		if( sprites_not_square )
			warning("Sprites are not all square!");
		if( too_many_sprites )				
			warning("There were too many sprites for the sprite sheet, some will have been overwritten!");
	}
	else
	{
		std::cout << "Creating default spritelist.txt" << std::endl;
		std::ofstream defaultSpriteList(exepath + "spritelist.txt");

		std::string defaultContents = 
		"# Lines starting with a '#' are ignored\n"
		"# The first line shoud be the name of the spriteSheet you want to make\n"
		"# WARNING! If this is the same name as a file that already exists it will be overwriten!\n"
		"# Then list the filenames of all the prites you want in the spritesheet\n"
		"# NOTE! Don't forget the \'.png\' on the end!\n"
		"mySpriteSheet.png\n"
		"sprite001.png\n"
		"sprite002.png";

		defaultSpriteList << defaultContents;
	}

	std::cout << emphasis() << "Enter 'q' to exit..." << normal() << std::endl;
	char c;
	std::cin >> c;
	return 0;
}

std::string trimWhitespace( const std::string& str )
{
	const size_t strBegin = str.find_first_not_of(" \t");
    if (strBegin == std::string::npos)
        return ""; // no content

    const size_t strEnd = str.find_last_not_of(" \t");
    const size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

bool addSprite( std::vector<unsigned char>& dest, const char* filename )
{
	static unsigned tallest_sprite = 0; // the height in pixels of the tallest sprite on each row
	static unsigned last_sprite_width = 0; // The width in pixels of the previous sprite
	static unsigned next_x = 0;	// Where to write the next spite to
	static unsigned next_y = 0;

	std::vector<unsigned char> sprite;
	unsigned sprite_width, sprite_height;

	unsigned error = lodepng::decode( sprite, sprite_width, sprite_height, filename );
	// If there was an error return without doing anything
	if( error ) {
		warning(std::string("Error loading ") + std::string(filename) );
		return false;
	}

	// Print the dimensions of the sprite
	std::cout << "Width: " << std::setw(4) << sprite_width << " Height: " <<  std::setw(4) << sprite_height << std::endl;

	// Set to the height of the first sprite
	if( tallest_sprite == 0 ) tallest_sprite = sprite_height;

	// Print a warning if sprites are not all the same size
	if( last_sprite_width != 0 && last_sprite_width != sprite_width )
		sprites_different_sizes = true;

	if( sprite_width != sprite_height )
		sprites_not_square = true;

	// Check if the new sprite is taller
	if( sprite_height > tallest_sprite ) tallest_sprite = sprite_height;

	// Check if the sprite is going to overflow
	if( next_x + sprite_width > TARGET_SIZE ) {
		next_x = 0;
		next_y += tallest_sprite;
		tallest_sprite = sprite_height;
	}
	if( next_y + sprite_height > TARGET_SIZE ) {
		next_y = 0;
		too_many_sprites = true;
	}

	// Copy the sprite to the destination vector
	for( int y = 0; y < sprite_height; y++ )
	for( int x = 0; x < sprite_width; x++ )
	{
		// Skip the corners
		if( (x == 0 				&& y == 0				) ||
			(x == sprite_width - 1	&& y == 0				) || 
			(x == 0 				&& y == sprite_height - 1) ||
			(x == sprite_width - 1	&& y == sprite_height - 1) ) {
			continue;
		}

		copyPixel( dest, next_x + x, next_y + y, TARGET_SIZE, sprite, x, y, sprite_width );
	}

	next_x += sprite_width;
	last_sprite_width = sprite_width;
	sprite.clear();
	return true;
}

void copyPixel( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	std::vector<unsigned char>& src,
	unsigned srcX,
	unsigned srcY,
	unsigned srcWidth )
{
	srcWidth *= 4;
	destWidth *= 4;
	destX *= 4;
	srcX *= 4;

	dest[destY * destWidth + destX    ] = src[ srcY * srcWidth + srcX    ];
	dest[destY * destWidth + destX + 1] = src[ srcY * srcWidth + srcX + 1];
	dest[destY * destWidth + destX + 2] = src[ srcY * srcWidth + srcX + 2];
	dest[destY * destWidth + destX + 3] = src[ srcY * srcWidth + srcX + 3];
}

void warning( std::string msg )
{
	std::cout << red() << "WARNING: " << normal() << msg << std::endl;
}