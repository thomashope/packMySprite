#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "lodepng.h"

#ifdef _WIN32
	#include <direct.h>
#else
	#include <unistd.h>
#endif

const int TARGET_WIDTH = 2048;
const int TARGET_HEIGHT = 2048;
const int SPRTES_PER_ROW = 8;

// Copys a single pixel from one PNG to another
void copyPixel( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	std::vector<unsigned char>& src,
	unsigned x,
	unsigned y,
	unsigned srcWidth );

// Copys an entire image to the destinatin at the specified positon
bool copySprite( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	const char* filename );

std::string trimWhitespace( std::string& str );

int main( int argc, char* argv[])
{
	std::string exepath = "";

	// If you double click the app in OSX the currend working directory is set
	// to the users home, but argv[0] points to the exe dir.
	// Need to prepend the exe dir or loadPNG will be unable to find the files
	#ifdef __APPLE__
		exepath += argv[0];

		for( int i = exepath.length() - 1; i > 0; i-- )
		{
			if( exepath[i] == '/') {
				exepath = exepath.substr(0, i + 1);
				break;
			}
		}
	#endif


	// The output PNG image
	std::vector<unsigned char> output;
	output.resize( TARGET_WIDTH * TARGET_HEIGHT * 4 );
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

				std::cout << "\tRead '" << line << "', writing to position " << destX << " " << destY << std::endl;

				// Prepend the path to the exe
				line = exepath + line;

				if( copySprite( output, destX, destY, TARGET_WIDTH, line.c_str() ) )
				{
					destX++;
					
					// Loop arond when passing the end of the row
					if( destX >= SPRTES_PER_ROW )
					{
						destX = 0;
						destY ++;
					}
					// Reset when running out of space on the image
					if( destY >= SPRTES_PER_ROW )
					{
						destX = destY = 0;
					}

				} else {
					std::cout << "\tError reading '" << line << "'! skiping..." << std::endl;
				}

			}
		}

		std::cout << "Encoding spritesheet...";
		lodepng::encode( outputName, output, TARGET_WIDTH, TARGET_HEIGHT );
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

	std::cout << "Enter 'q' to exit..." << std::endl;
	char c;
	std::cin >> c;
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

bool copySprite( std::vector<unsigned char>& dest,
	unsigned destX,
	unsigned destY,
	unsigned destWidth,
	const char* filename )
{
	std::vector<unsigned char> sprite;
	unsigned spriteWidth, spriteHeight;

	unsigned error = lodepng::decode( sprite, spriteWidth, spriteHeight, filename );
	if( error ) return false;

	destX *= spriteWidth;
	destY *= spriteHeight;

	for( int y = 0; y < spriteHeight; y++ )
	for( int x = 0; x < spriteWidth; x++ )
	{
		// Skip the corners
		if( (x == 0 				&& y == 0				) ||
			(x == spriteWidth - 1	&& y == 0				) || 
			(x == 0 				&& y == spriteHeight - 1) ||
			(x == spriteWidth - 1	&& y == spriteHeight - 1) ) {
			continue;
		}

		copyPixel( dest, destX + x, destY + y, destWidth, sprite, x, y, spriteWidth );
	}

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