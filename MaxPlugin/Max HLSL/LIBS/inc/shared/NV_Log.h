/*********************************************************************NVMH4****
Path:  SDK\LIBS\inc\shared
File:  NV_Log.h

Copyright NVIDIA Corporation 2002
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.



Comments:



******************************************************************************/

#ifndef _NV_LOG_HEADER_GJ
#define _NV_LOG_HEADER_GJ


#include <assert.h>
#include <time.h>
#include <stdio.h>
#pragma warning(disable : 4786)
#include <vector>
#include <string>

#include "shared/NV_Error.h"

//@#include <sys/types.h>
//@#include <sys/timeb.h>
//@#include <string.h>



class NVLog
{
public:

	std::vector< std::string >	m_vLines;

	bool	m_bEcho;		// echo messages via OutputDebugString

	std::string		m_strLastFileName;


	NVLog()
	{
		// allocate 1000 lines to begin with
		m_vLines.reserve( 1000 );
	}
	~NVLog()
	{
		m_vLines.clear();
	}

	void	SaveToFile( const char * filename )
	{
		FILE * fp;
		fp = fopen( filename, "wt" );
		if( fp == NULL )
		{
			OutputDebugString("NVLog couldn't open file!\n");
			assert( false );
			return;
		}

		int i;
		for( i=0; i < m_vLines.size(); i++ )
		{
			fprintf( fp, "%s", m_vLines.at(i).c_str() );
		}

		fclose( fp );
		fp = NULL;

		m_strLastFileName = filename;

		FMsg("Saved log to file [%s]\n", filename );
	}

	void	SaveToUniqueFile( const char * filename )
	{
		// filename should not have any extension
		char buf[4096];
		FILE * fp = NULL;
		int i = 0;

		do
		{
			sprintf( buf, "%s%3.3d.txt", filename, i );
			
//			FMsg("Trying to open %s\n", buf );
			fp = fopen( buf, "rt" );

			if( fp == NULL )
			{
				// file does not exist, so use the name
				break;
			}
			else
			{
//				FMsg("  File [%s] exists!  Trying another...\n", buf );
			}

			i++;

		} while( i < 999 );

		if( i >= 999 )
		{
			FMsg("Could not open a file for writing!\n");
			assert(false);
			return;
		}

		SaveToFile( buf );
	}


	char * AddLine( const char * szFormat, ... )
	{
		static char buffer[2048];

		va_list args;
		va_start( args, szFormat );
		_vsnprintf( buffer, 2048, szFormat, args );
		va_end( args );

		buffer[2048-1] = '\0';			// terminate in case of overflow

		m_vLines.push_back( buffer );

		if( m_bEcho )
		{
			OutputDebugString("(LOG)  ");
			OutputDebugString ( buffer );
		}

		return( buffer );
	};

	void	OutputToDebugString()
	{
		// output entire log to debug string
		int i;
		OutputDebugString("************* Log listing:\n");
		for( i=0; i < m_vLines.size(); i++ )
		{
			OutputDebugString( m_vLines.at(i).c_str() );
		}
		OutputDebugString("************* End of log\n");
	}


	void	AddDateAndTime()
	{
		char buf[4096];
		std::string str;

		_strdate( buf );
		str = buf;
		_strtime( buf );
		str = str + "  " + buf;

		AddLine( str.c_str() );
	}
};





#endif
