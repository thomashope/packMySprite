#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "lodepng.h"

const int TARGET_WIDTH = 1024;
const int TARGET_HEIGHT = 1024;

// Copys a single pixel from one PNG to another
void copyPixel( std::vector<unsigned char>& dest,
	unsigned destWidth,
	std::vector<unsigned char>& src,
	unsigned x,
	unsigned y,
	unsigned srcWidth );

// Copys an entire image to the destinatin at the specified positon
void copySprite( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	const char* filename );

std::string trimWhitespace( std::string& str );

int main()
{
	// The output PNG image
	std::vector<unsigned char> combined;
	combined.resize( 1024 * 1024 * 4 );

	std::string outputName = "";
	unsigned destX = 0;
	unsigned destY = 0;

	std::ifstream spritelist("spritelist.txt");
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
					outputName = line;
					std::cout << "Creating spritesheet " << outputName << std::endl;
					continue;
				}

				std::cout << "\tRead '" << line << "', writing to position " << destX << " " << destY << std::endl;;
				copySprite( combined, destX, destY, TARGET_WIDTH, line.c_str() );

				destX++;
				if( destX >= 16 ) {
					destX = 0;
					destY ++;
				}
			}
		}

		std::cout << "Encoding spritesheet...";
		lodepng::encode( outputName, combined, 1024, 1024 );
		std::cout << "\tdone!" << std::endl;
	}
	else
	{
		std::cout << "Creating default spritelist.txt" << std::endl;
		std::ofstream defaultSpriteList("spritelist.txt");

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

	return 0;
}

std::string trimWhitespace( std::string& str )
{
	const size_t strBegin = str.find_first_not_of(" \t");
    if (strBegin == std::string::npos)
        return ""; // no content

    const size_t strEnd = str.find_last_not_of(" \t");
    const size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void copySprite( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	const char* filename )
{
	std::vector<unsigned char> sprite;
	unsigned spriteWidth, spriteHeight;

	lodepng::decode( sprite, spriteWidth, spriteHeight, filename );

	destX *= spriteWidth;
	destY *= spriteHeight;

	for( int y = destY; y < destY + spriteHeight; y++ )
	for( int x = destX; x < destX + spriteWidth; x++ )
	{
		copyPixel( dest, destWidth, sprite, x, y, spriteWidth );
	}

	sprite.clear();
}

void copyPixel( std::vector<unsigned char>& dest,
	unsigned destWidth,
	std::vector<unsigned char>& src,
	unsigned x,
	unsigned y,
	unsigned srcWidth )
{
	srcWidth *= 4;
	destWidth *= 4;
	x *= 4;
	dest[y * destWidth + x    ] = src[ y * srcWidth + x    ];
	dest[y * destWidth + x + 1] = src[ y * srcWidth + x + 1];
	dest[y * destWidth + x + 2] = src[ y * srcWidth + x + 2];
	dest[y * destWidth + x + 3] = src[ y * srcWidth + x + 3];
}		