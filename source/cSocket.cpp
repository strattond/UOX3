#include "uox3.h"
#include "CPacketSend.h"
#include "speech.h"
#include "cRaces.h"
#include "cGuild.h"
#include "commands.h"
#include "combat.h"
#include "classes.h"
#include "Dictionary.h"

#if UOX_PLATFORM != PLATFORM_WIN32
	#include <sys/ioctl.h>
#endif

namespace UOX
{

const UI32 DefaultClientVersion		= calcserial( 2, 0, 0, 0 );
const ClientTypes DefaultClientType = CV_NORMAL;


// These vectors are for dealing with packets that are larger than the buffer size
// While admittedly not thread friendly, the number of times these buffers are used
// should be very small and right now, is an implementation that will increase clieht
// compatability
std::vector< UI08 >	largeBuffer;
std::vector< UI08 >	largePackBuffer;

//	1.0		Abaddon		29th November, 2000
//			Initial implementation
//			Stores almost all information currently separated into different vars
//			Also has logging support, and non-blocking IO support
//			Makes use of a socket_error exception class

#if _DEBUG
const bool LOGDEFAULT = true;
#else
const bool LOGDEFAULT = false;
#endif

void dumpStream( std::ofstream &outStream, const char *strToDump, UI08 num )
{
	outStream << "  ";
	for( UI08 parseBuff = 0; parseBuff < num; ++parseBuff )
	{
		if( strToDump[parseBuff] )
			outStream << strToDump[parseBuff];
		else
			outStream << " ";
	}
	outStream << std::endl;
}

void doPacketLogging( std::ofstream &outStream, size_t buffLen, std::vector< UI08 > myBuffer )
{
	outStream << std::hex;
	char qbuffer[8];
	memset( qbuffer, 0x00, 8 );
	UI08 j = 0;
	for( size_t i = 0; i < buffLen; ++i )
	{
		qbuffer[j++] = myBuffer[i];
		outStream << " 0x" << (myBuffer[i] < 16?"0":"") << (UI16)myBuffer[i];
		if( j > 6 )
		{
			dumpStream( outStream, qbuffer, 8 );
			j = 0;
		}
	}
	if( j < 7 )
		dumpStream( outStream, qbuffer, j );
	outStream << std::endl;
}

void doPacketLogging( std::ofstream &outStream, size_t buffLen, const char *myBuffer )
{
	outStream << std::hex;
	char qbuffer[8];
	memset( qbuffer, 0x00, 8 );
	UI08 j = 0;
	for( size_t i = 0; i < buffLen; ++i )
	{
		qbuffer[j++] = myBuffer[i];
		outStream << " 0x" << (myBuffer[i] < 16?"0":"") << (UI16)myBuffer[i];
		if( j > 6 )
		{
			dumpStream( outStream, qbuffer, 8 );
			j = 0;
		}
	}
	if( j < 7 )
		dumpStream( outStream, qbuffer, j );
	outStream << std::endl;
}

long socket_error::ErrorNumber( void ) const
{
	return errorNum;
}

const char *socket_error::what( void ) const throw()
{
	return runtime_error::what();
}

socket_error::socket_error( void ) : errorNum( -1 ), runtime_error( "" )
{
}

socket_error::socket_error( const std::string& what_arg ) : errorNum( -1 ), runtime_error( what_arg )
{
}

socket_error::socket_error( const long errorNumber ) : errorNum( errorNumber ), runtime_error( "" )
{
}

//o---------------------------------------------------------------------------o
//|   Function    -  size_t cSocket::CliSocket( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns the socket identifier for our socket
//o---------------------------------------------------------------------------o
size_t cSocket::CliSocket( void ) const
{
	return cliSocket;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::CliSocket( size_t newValue )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Assigns a new socket value
//o---------------------------------------------------------------------------o
void cSocket::CliSocket( size_t newValue )
{
	cliSocket = newValue;
	UI32 mode = 1;
	// set the socket to nonblocking
	ioctlsocket( cliSocket, FIONBIO, &mode );
}

//o---------------------------------------------------------------------------o
//|   Function    -  bool cSocket::CryptClient( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns the true if the socket is set to crypt mode
//o---------------------------------------------------------------------------o
bool cSocket::CryptClient( void ) const
{
	return cryptclient;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::CryptClient( bool newValue )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Sets the value of the socket's crypt mode
//o---------------------------------------------------------------------------o
void cSocket::CryptClient( bool newValue )
{
	cryptclient = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  std::string cSocket::XText( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns the socket's xtext buffer
//o---------------------------------------------------------------------------o
std::string cSocket::XText( void )
{
	return xtext;
}

void cSocket::XText( std::string newValue )
{
	xtext = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  cBaseObject *TempObj()
//|   Date        -  October 31, 2003
//|   Programmer  -  giwo
//o---------------------------------------------------------------------------o
//|   Purpose     -  Temporary storage for CChar and CItem objects
//o---------------------------------------------------------------------------o
cBaseObject *cSocket::TempObj( void ) const
{
	return tmpObj;
}
void cSocket::TempObj( cBaseObject *newValue )
{
	tmpObj = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  SI32 TempInt()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  The tempint of the socket
//o---------------------------------------------------------------------------o
SI32 cSocket::TempInt( void ) const
{
	return tempint;
}

void cSocket::TempInt( SI32 newValue )
{
	tempint = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  SI08 ClickZ()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Return's the socket's addz
//o---------------------------------------------------------------------------o
SI08 cSocket::ClickZ( void ) const
{
	return clickz;
}

void cSocket::ClickZ( SI08 newValue )
{
	clickz = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI32 AddID()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  The addid associated with the socket
//o---------------------------------------------------------------------------o
void cSocket::AddID( UI32 newValue )
{
	addid[0] = (UI08)( newValue>>24 );
	addid[1] = (UI08)( newValue>>16 );
	addid[2] = (UI08)( newValue>>8 );
	addid[3] = (UI08)( newValue%256 );
}

UI32 cSocket::AddID( void ) const
{
	return calcserial( addid[0], addid[1], addid[2], addid[3] );
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI08 AddID1()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  The first addid associated with the socket
//o---------------------------------------------------------------------------o
UI08 cSocket::AddID1( void ) const
{
	return addid[0];
}
void cSocket::AddID1( UI08 newValue )
{
	addid[0] = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI08 AddID2()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  The second addid associated with the socket
//o---------------------------------------------------------------------------o
UI08 cSocket::AddID2( void ) const
{
	return addid[1];
}

void cSocket::AddID2( UI08 newValue )
{
	addid[1] = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI08 AddID3()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  The third addid associated with the socket
//o---------------------------------------------------------------------------o
UI08 cSocket::AddID3( void ) const
{
	return addid[2];
}

void cSocket::AddID3( UI08 newValue )
{
	addid[2] = newValue;
}
//o---------------------------------------------------------------------------o
//|   Function    -  UI08 AddID4()
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  The fourth addid associated with the socket
//o---------------------------------------------------------------------------o
UI08 cSocket::AddID4( void ) const
{
	return addid[3];
}

void cSocket::AddID4( UI08 newValue )
{
	addid[3] = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI08 DyeAll( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  DyeAll status of a socket
//o---------------------------------------------------------------------------o
UI08 cSocket::DyeAll( void ) const
{
	return dyeall;
}

void cSocket::DyeAll( UI08 newValue )
{
	dyeall = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::CloseSocket( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Closes the open socket
//o---------------------------------------------------------------------------o
void cSocket::CloseSocket( void )
{
	closesocket( cliSocket );
}

//o---------------------------------------------------------------------------o
//|   Function    -  bool cSocket::FirstPacket( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns true if it's the first packet received
//o---------------------------------------------------------------------------o
bool cSocket::FirstPacket( void ) const
{
	return firstPacket;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::FirstPacket( bool newValue )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Sets whether the socket has received it's first packet yet
//o---------------------------------------------------------------------------o
void cSocket::FirstPacket( bool newValue )
{
	firstPacket = newValue;
}

//o---------------------------------------------------------------------------o
//|   Function    -  SI32 cSocket::IdleTimeout( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns the time point at which the char times out
//o---------------------------------------------------------------------------o
SI32 cSocket::IdleTimeout( void ) const
{
	return idleTimeout;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::IdleTimeout( long newValue )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Sets the time point at which the char times out
//o---------------------------------------------------------------------------o
void cSocket::IdleTimeout( SI32 newValue )
{
	idleTimeout = newValue;
	wasIdleWarned = false;
}

bool cSocket::WasIdleWarned( void ) const
{
	return wasIdleWarned;
}

void cSocket::WasIdleWarned( bool value )
{
	wasIdleWarned = value;
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI08 *cSocket::Buffer( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns a pointer to the buffer of the socket
//o---------------------------------------------------------------------------o
UI08 *cSocket::Buffer( void )
{
	return &buffer[0];
}

//o---------------------------------------------------------------------------o
//|   Function    -  UI08 *cSocket::OutBuffer( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns a pointer to the outgoing buffer of the socket
//o---------------------------------------------------------------------------o
UI08 *cSocket::OutBuffer( void )
{
	return &outbuffer[0];
}

//o---------------------------------------------------------------------------o
//|   Function    -  SI16 cSocket::WalkSequence( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns the point in the walk sequence of the socket
//o---------------------------------------------------------------------------o
SI16 cSocket::WalkSequence( void ) const
{
	return walkSequence;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::WalkSequence( SI16 newValue )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Sets the walk sequence value of the socket
//o---------------------------------------------------------------------------o
void cSocket::WalkSequence( SI16 newValue )
{
	walkSequence = newValue;
}

void cSocket::TriggerWord( UI16 newValue )
{
	triggerWord = newValue;
}
UI16 cSocket::TriggerWord( void ) const
{
	return triggerWord;
}

CChar *					DEFSOCK_CURRCHAROBJ				= NULL;
const SI32				DEFSOCK_IDLETIMEOUT				= -1;
const SI32				DEFSOCK_TEMPINT					= 0;
const UI08				DEFSOCK_DYEALL					= 0;
const SI08				DEFSOCK_CLICKZ					= -1;
const SI16				DEFSOCK_CLICKX					= -1;
const SI16				DEFSOCK_CLICKY					= -1;
const bool				DEFSOCK_NEWCLIENT				= true;
const bool				DEFSOCK_FIRSTPACKET				= true;
const UI08				DEFSOCK_RANGE					= 15;
const bool				DEFSOCK_CRYPTCLIENT				= false;
const SI16				DEFSOCK_WALKSEQUENCE			= -1;
const char				DEFSOCK_CURSPELLTYPE			= 0;
const int				DEFSOCK_OUTLENGTH				= 0;
const int				DEFSOCK_INLENGTH				= 0;
const bool				DEFSOCK_LOGGING					= LOGDEFAULT;
const int				DEFSOCK_POSTACKCOUNT			= 0;
const int				DEFSOCK_POSTCOUNT				= 0;
const PickupLocations	DEFSOCK_PSPOT					= PL_NOWHERE;
const SERIAL			DEFSOCK_PFROM					= INVALIDSERIAL;
const SI16				DEFSOCK_PX						= 0;
const SI16				DEFSOCK_PY						= 0;
const SI08				DEFSOCK_PZ						= 0;
const UnicodeTypes		DEFSOCK_LANG					= UT_ENU;
const ClientTypes		DEFSOCK_CLITYPE					= DefaultClientType;
const UI32				DEFSOCK_CLIENTVERSION			= DefaultClientVersion;
const UI32				DEFSOCK_BYTESSENT				= 0;
const UI32				DEFSOCK_BYTESRECEIVED			= 0;
const bool				DEFSOCK_RECEIVEDVERSION			= false;
cBaseObject *			DEFSOCK_TMPOBJ					= NULL;
const UI16				DEFSOCK_TRIGGERWORD				= 0xFFFF;

cSocket::cSocket( size_t sockNum ) : currCharObj( DEFSOCK_CURRCHAROBJ )/*, actbAccount()*/, idleTimeout( DEFSOCK_IDLETIMEOUT ), 
tempint( DEFSOCK_TEMPINT ), dyeall( DEFSOCK_DYEALL ), clickz( DEFSOCK_CLICKZ ), newClient( DEFSOCK_NEWCLIENT ), firstPacket( DEFSOCK_FIRSTPACKET ), 
range( DEFSOCK_RANGE ), cryptclient( DEFSOCK_CRYPTCLIENT ), cliSocket( sockNum ), walkSequence( DEFSOCK_WALKSEQUENCE ),  clickx( DEFSOCK_CLICKX ), 
currentSpellType( DEFSOCK_CURSPELLTYPE ), outlength( DEFSOCK_OUTLENGTH ), inlength( DEFSOCK_INLENGTH ), logging( DEFSOCK_LOGGING ), clicky( DEFSOCK_CLICKY ), 
postCount( DEFSOCK_POSTCOUNT ), postAckCount( DEFSOCK_POSTACKCOUNT ), pSpot( DEFSOCK_PSPOT ), pFrom( DEFSOCK_PFROM ), pX( DEFSOCK_PX ), pY( DEFSOCK_PY ), 
pZ( DEFSOCK_PZ ), lang( DEFSOCK_LANG ), cliType( DEFSOCK_CLITYPE ), clientVersion( DEFSOCK_CLIENTVERSION ), bytesReceived( DEFSOCK_BYTESRECEIVED ), 
bytesSent( DEFSOCK_BYTESSENT ), receivedVersion( DEFSOCK_RECEIVEDVERSION ), tmpObj( DEFSOCK_TMPOBJ ), triggerWord( DEFSOCK_TRIGGERWORD )
{
	InternalReset();
}

cSocket::~cSocket()
{
	closesocket( cliSocket );
}

void cSocket::InternalReset( void )
{
	memset( buffer, 0, MAXBUFFER );
	memset( outbuffer, 0, MAXBUFFER );
	xtext.reserve( MAXBUFFER );
	ClearAuthor();
	ClearTitle();
	ClearPage();
	addid[0] = addid[1] = addid[2] = addid[3] = 0;
	clientip[0] = clientip[1] = clientip[2] = clientip[3] = 0;
	// set the socket to nonblocking
	UI32 mode = 1;
	ioctlsocket( cliSocket, FIONBIO, &mode );
	for( int mTID = (int)tPC_SKILLDELAY; mTID < (int)tPC_COUNT; ++mTID )
		pcTimers[mTID] = 0;
	actbAccount.wAccountIndex = AB_INVALID_ID;
}

//o---------------------------------------------------------------------------o
//|   Function    -  char cSocket::CurrentSpellType( void )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Returns the current spell type of the socket
//o---------------------------------------------------------------------------o
UI08 cSocket::CurrentSpellType( void ) const
{
	return currentSpellType;
}

//o---------------------------------------------------------------------------o
//|   Function    -  void cSocket::CurrentSpellType( char newValue )
//|   Date        -  November 29th, 2000
//|   Programmer  -  Abaddon
//o---------------------------------------------------------------------------o
//|   Purpose     -  Sets the spell type of the socket
//o---------------------------------------------------------------------------o
void cSocket::CurrentSpellType( UI08 newValue )
{
	currentSpellType = newValue;
}

bool cSocket::FlushBuffer( bool doLog )
{
	if( outlength > 0 )
	{
		UI32 len;
		if( cryptclient )
		{
			UI08 xoutbuffer[MAXBUFFER*2];
			len = Pack( outbuffer, xoutbuffer, outlength );
			send( cliSocket, (char *)xoutbuffer, len, 0 );
		}
		else
			send( cliSocket, (char *)&outbuffer[0], outlength, 0 );
		if( Logging() && doLog )
		{
			SERIAL toPrint;
			if( !ValidateObject( currCharObj ) )
				toPrint = INVALIDSERIAL;
			else
				toPrint = currCharObj->GetSerial();
			std::string logFile = cwmWorldState->ServerData()->Directory( CSDDP_LOGS ) + UString::number( toPrint ) + ".snd";
			std::ofstream logDestination;
			logDestination.open( logFile.c_str(), std::ios::out | std::ios::app );
			if( logDestination.is_open() )
			{
				logDestination << "[SEND]Packet: 0x" << (outbuffer[0] < 10?"0":"") << std::hex << (UI16)outbuffer[0] << "--> Length: " << std::dec << outlength << std::endl;
				doPacketLogging( logDestination, outlength, (char *)outbuffer );
				logDestination.close();
			}
			else
				Console.Error( 1, "Failed to open socket log %s", logFile.c_str() );
			bytesSent += outlength;
		}
		outlength = 0;
		return true;
	}
	else
		return false;
}

bool cSocket::FlushLargeBuffer( bool doLog )
{
	if( outlength > 0 )
	{
		if( cryptclient )
		{
			largePackBuffer.resize( outlength * 2 );
			int len = Pack( &largeBuffer[0], &largePackBuffer[0], outlength );
			send( cliSocket, (char *)&largePackBuffer[0], len, 0 );
		}
		else
			send( cliSocket, (char *)&largeBuffer[0], outlength, 0 );

		if( Logging() && doLog )
		{
			SERIAL toPrint;
			if( !ValidateObject( currCharObj ) )
				toPrint = INVALIDSERIAL;
			else
				toPrint = currCharObj->GetSerial();
			std::string logFile = cwmWorldState->ServerData()->Directory( CSDDP_LOGS ) + UString::number( toPrint ) + ".snd";
			std::ofstream logDestination;
			logDestination.open( logFile.c_str(), std::ios::out | std::ios::app );
			if( logDestination.is_open() )
			{
				logDestination << "[SEND]Packet: 0x" << (outbuffer[0] < 10?"0":"") << std::hex << (UI16)outbuffer[0] << "--> Length: " << std::dec << outlength << std::endl;
				doPacketLogging( logDestination, outlength, largeBuffer );
				logDestination.close();
			}
			else
				Console.Error( 1, "Failed to open socket log %s", logFile.c_str() );
			bytesSent += outlength;
		}
		outlength = 0;
		return true;
	}
	else
		return false;
}

static UI32 bit_table[257][2] =
{
	{0x02, 0x00}, 	{0x05, 0x1F}, 	{0x06, 0x22}, 	{0x07, 0x34}, 	{0x07, 0x75}, 	{0x06, 0x28}, 	{0x06, 0x3B}, 	{0x07, 0x32}, 
	{0x08, 0xE0}, 	{0x08, 0x62}, 	{0x07, 0x56}, 	{0x08, 0x79}, 	{0x09, 0x19D},	{0x08, 0x97}, 	{0x06, 0x2A}, 	{0x07, 0x57}, 
	{0x08, 0x71}, 	{0x08, 0x5B}, 	{0x09, 0x1CC},	{0x08, 0xA7}, 	{0x07, 0x25}, 	{0x07, 0x4F}, 	{0x08, 0x66}, 	{0x08, 0x7D}, 
	{0x09, 0x191},	{0x09, 0x1CE}, 	{0x07, 0x3F}, 	{0x09, 0x90}, 	{0x08, 0x59}, 	{0x08, 0x7B}, 	{0x08, 0x91}, 	{0x08, 0xC6}, 
	{0x06, 0x2D}, 	{0x09, 0x186}, 	{0x08, 0x6F}, 	{0x09, 0x93}, 	{0x0A, 0x1CC},	{0x08, 0x5A}, 	{0x0A, 0x1AE},	{0x0A, 0x1C0}, 
	{0x09, 0x148},	{0x09, 0x14A}, 	{0x09, 0x82}, 	{0x0A, 0x19F}, 	{0x09, 0x171},	{0x09, 0x120}, 	{0x09, 0xE7}, 	{0x0A, 0x1F3}, 
	{0x09, 0x14B},	{0x09, 0x100},	{0x09, 0x190},	{0x06, 0x13}, 	{0x09, 0x161},	{0x09, 0x125},	{0x09, 0x133},	{0x09, 0x195}, 
	{0x09, 0x173},	{0x09, 0x1CA},	{0x09, 0x86}, 	{0x09, 0x1E9}, 	{0x09, 0xDB}, 	{0x09, 0x1EC},	{0x09, 0x8B}, 	{0x09, 0x85}, 
	{0x05, 0x0A}, 	{0x08, 0x96}, 	{0x08, 0x9C}, 	{0x09, 0x1C3}, 	{0x09, 0x19C},	{0x09, 0x8F}, 	{0x09, 0x18F},	{0x09, 0x91}, 
	{0x09, 0x87}, 	{0x09, 0xC6}, 	{0x09, 0x177},	{0x09, 0x89}, 	{0x09, 0xD6}, 	{0x09, 0x8C}, 	{0x09, 0x1EE},	{0x09, 0x1EB}, 
	{0x09, 0x84}, 	{0x09, 0x164}, 	{0x09, 0x175},	{0x09, 0x1CD}, 	{0x08, 0x5E}, 	{0x09, 0x88}, 	{0x09, 0x12B},	{0x09, 0x172}, 
	{0x09, 0x10A},	{0x09, 0x8D}, 	{0x09, 0x13A},	{0x09, 0x11C}, 	{0x0A, 0x1E1},	{0x0A, 0x1E0}, 	{0x09, 0x187},	{0x0A, 0x1DC}, 
	{0x0A, 0x1DF},	{0x07, 0x74}, 	{0x09, 0x19F},	{0x08, 0x8D},	{0x08, 0xE4}, 	{0x07, 0x79}, 	{0x09, 0xEA}, 	{0x09, 0xE1}, 
	{0x08, 0x40}, 	{0x07, 0x41}, 	{0x09, 0x10B},	{0x09, 0xB0}, 	{0x08, 0x6A}, 	{0x08, 0xC1}, 	{0x07, 0x71}, 	{0x07, 0x78}, 
	{0x08, 0xB1}, 	{0x09, 0x14C}, 	{0x07, 0x43}, 	{0x08, 0x76}, 	{0x07, 0x66}, 	{0x07, 0x4D}, 	{0x09, 0x8A}, 	{0x06, 0x2F}, 
	{0x08, 0xC9},	{0x09, 0xCE}, 	{0x09, 0x149},	{0x09, 0x160}, 	{0x0A, 0x1BA}, 	{0x0A, 0x19E}, 	{0x0A, 0x39F}, 	{0x09, 0xE5}, 
	{0x09, 0x194}, 	{0x09, 0x184}, 	{0x09, 0x126}, 	{0x07, 0x30}, 	{0x08, 0x6C}, 	{0x09, 0x121}, 	{0x09, 0x1E8}, 	{0x0A, 0x1C1}, 
	{0x0A, 0x11D}, 	{0x0A, 0x163}, 	{0x0A, 0x385}, 	{0x0A, 0x3DB}, 	{0x0A, 0x17D}, 	{0x0A, 0x106}, 	{0x0A, 0x397}, 	{0x0A, 0x24E}, 
	{0x07, 0x2E}, 	{0x08, 0x98}, 	{0x0A, 0x33C}, 	{0x0A, 0x32E}, 	{0x0A, 0x1E9}, 	{0x09, 0xBF}, 	{0x0A, 0x3DF}, 	{0x0A, 0x1DD}, 
	{0x0A, 0x32D}, 	{0x0A, 0x2ED}, 	{0x0A, 0x30B}, 	{0x0A, 0x107}, 	{0x0A, 0x2E8}, 	{0x0A, 0x3DE}, 	{0x0A, 0x125}, 	{0x0A, 0x1E8}, 
	{0x09, 0xE9}, 	{0x0A, 0x1CD}, 	{0x0A, 0x1B5}, 	{0x09, 0x165}, 	{0x0A, 0x232}, 	{0x0A, 0x2E1}, 	{0x0B, 0x3AE}, 	{0x0B, 0x3C6}, 
	{0x0B, 0x3E2}, 	{0x0A, 0x205}, 	{0x0A, 0x29A}, 	{0x0A, 0x248}, 	{0x0A, 0x2CD}, 	{0x0A, 0x23B}, 	{0x0B, 0x3C5}, 	{0x0A, 0x251}, 
	{0x0A, 0x2E9}, 	{0x0A, 0x252}, 	{0x09, 0x1EA}, 	{0x0B, 0x3A0}, 	{0x0B, 0x391}, 	{0x0A, 0x23C}, 	{0x0B, 0x392}, 	{0x0B, 0x3D5}, 
	{0x0A, 0x233}, 	{0x0A, 0x2CC}, 	{0x0B, 0x390}, 	{0x0A, 0x1BB}, 	{0x0B, 0x3A1}, 	{0x0B, 0x3C4}, 	{0x0A, 0x211}, 	{0x0A, 0x203}, 
	{0x09, 0x12A}, 	{0x0A, 0x231}, 	{0x0B, 0x3E0}, 	{0x0A, 0x29B}, 	{0x0B, 0x3D7}, 	{0x0A, 0x202}, 	{0x0B, 0x3AD}, 	{0x0A, 0x213}, 
	{0x0A, 0x253}, 	{0x0A, 0x32C}, 	{0x0A, 0x23D}, 	{0x0A, 0x23F}, 	{0x0A, 0x32F}, 	{0x0A, 0x11C}, 	{0x0A, 0x384}, 	{0x0A, 0x31C}, 
	{0x0A, 0x17C}, 	{0x0A, 0x30A}, 	{0x0A, 0x2E0}, 	{0x0A, 0x276}, 	{0x0A, 0x250}, 	{0x0B, 0x3E3}, 	{0x0A, 0x396}, 	{0x0A, 0x18F}, 
	{0x0A, 0x204}, 	{0x0A, 0x206}, 	{0x0A, 0x230}, 	{0x0A, 0x265}, 	{0x0A, 0x212}, 	{0x0A, 0x23E}, 	{0x0B, 0x3AC}, 	{0x0B, 0x393}, 
	{0x0B, 0x3E1}, 	{0x0A, 0x1DE}, 	{0x0B, 0x3D6}, 	{0x0A, 0x31D}, 	{0x0B, 0x3E5}, 	{0x0B, 0x3E4}, 	{0x0A, 0x207}, 	{0x0B, 0x3C7}, 
	{0x0A, 0x277}, 	{0x0B, 0x3D4}, 	{0x08, 0xC0},	{0x0A, 0x162}, 	{0x0A, 0x3DA}, 	{0x0A, 0x124}, 	{0x0A, 0x1B4}, 	{0x0A, 0x264}, 
	{0x0A, 0x33D}, 	{0x0A, 0x1D1}, 	{0x0A, 0x1AF}, 	{0x0A, 0x39E}, 	{0x0A, 0x24F}, 	{0x0B, 0x373}, 	{0x0A, 0x249}, 	{0x0B, 0x372}, 
	{0x09, 0x167}, 	{0x0A, 0x210}, 	{0x0A, 0x23A}, 	{0x0A, 0x1B8}, 	{0x0B, 0x3AF}, 	{0x0A, 0x18E}, 	{0x0A, 0x2EC}, 	{0x07, 0x62}, 
	{0x04, 0x0D}
};

UI32 DoPack( UI08 *pIn, UI08 *pOut, int len )
{
	UI32 packedLength	= 0;
	int bitByte			= 0;
	int nrBits;
	UI32 value;

	while( len-- )
	{
		nrBits	= bit_table[*pIn][0];
		value	= bit_table[*pIn++][1];

		while( nrBits-- )
		{
			pOut[packedLength] = static_cast<UI08>((pOut[packedLength] << 1) | (UI08)((value >> nrBits) & 0x1));

			bitByte = (bitByte + 1) & 0x07;
			if( !bitByte )
				++packedLength;
		}
	}

	nrBits	= bit_table[256][0];
	value	= bit_table[256][1];

	while( nrBits-- )
	{
		pOut[packedLength] = static_cast<UI08>((pOut[packedLength] << 1) | (UI08)((value >> nrBits) & 0x1));

		bitByte = (bitByte + 1) & 0x07;
		if( !bitByte )
			++packedLength;
	}

	if( bitByte )
	{
		while( bitByte < 8 )
		{
			pOut[packedLength] <<= 1;
			++bitByte;
		}
		++packedLength;
	}
	return packedLength;
}

UI32 cSocket::Pack( void *pvIn, void *pvOut, int len )
{
	UI08 *pIn = (UI08 *)pvIn;
	UI08 *pOut = (UI08 *)pvOut;

	return DoPack( pIn, pOut, len );
}

void cSocket::Send( const void *point, int length ) // Buffering send function
{
	if( outlength + length > MAXBUFFER )
		FlushBuffer();
	if( outlength > 0 )
		Console.Print( "Fragmented packet! [packet: %i]\n", outbuffer[0] );
		// sometimes we send enormous packets... oh what fun
	if( length > MAXBUFFER )
	{
#ifdef _DEBUG
		Console.Print( "Large packet found [%i]\n", outbuffer[0] );
#endif
		largeBuffer.resize( length );
		memcpy( &largeBuffer[0], point, length );
		outlength = length;
		FlushLargeBuffer();
		return;
	}
	else
		memcpy( &outbuffer[outlength], point, length );

	outlength += length;
}

#if UOX_PLATFORM != PLATFORM_WIN32
int GrabLastError( void )
{
	return errno;
}
#else
int GrabLastError( void )
{
	return WSAGetLastError();
}
#endif

void cSocket::FlushIncoming( void )
{
	int count = 0;
	do
	{
		count = recv( cliSocket, (char *)&buffer[inlength], 1, 0 );
	} while( count > 0 );
}

void cSocket::ReceiveLogging( cPInputBuffer *toLog )
{
	if( Logging() )
	{
		SERIAL toPrint;
		if( !ValidateObject( currCharObj ) )
			toPrint = INVALIDSERIAL;
		else
			toPrint = currCharObj->GetSerial();
		std::string logFile = cwmWorldState->ServerData()->Directory( CSDDP_LOGS ) + UString::number( toPrint ) + ".rcv";
		std::ofstream logDestination;
		logDestination.open( logFile.c_str(), std::ios::out | std::ios::app );
		if( !logDestination.is_open() )
		{
			Console.Error( 1, "Failed to open socket log %s", logFile.c_str() );
			return;
		}
		if( toLog != NULL )
			toLog->Log( logDestination );
		else
		{
			logDestination << "[RECV]Packet: 0x" << std::hex << (buffer[0] < 10?"0":"") << (UI16)buffer[0] << " --> Length: " << std::dec << inlength << std::endl << std::endl;
			doPacketLogging( logDestination, inlength, (char *)buffer );
		}
		logDestination.close();
	}
}
int cSocket::Receive( int x, bool doLog )
{
	int count			= 0;
	UI08 recvAttempts	= 0;
	long curTime		= getclock();
	long nexTime		= curTime;
	do
	{
		count = recv( cliSocket, (char *)&buffer[inlength], x - inlength, 0 );
		if( count > 0 )
		{
			inlength += count;
		}
		else if( count == SOCKET_ERROR )
		{
			int lastError = GrabLastError();
#if UOX_PLATFORM != PLATFORM_WIN32
			if( lastError != EWOULDBLOCK )
#else
			if( lastError != WSAEWOULDBLOCK )
#endif
				throw socket_error( lastError );
			UOXSleep( 20 );
		}
		++recvAttempts;
		nexTime = getclock();
		// You will find the values for the following in the uox.ini file as NETRCVTIMEOUT, and NETRETRYCOUNT respectivly
		if( recvAttempts == cwmWorldState->ServerData()->ServerNetRetryCount() || (nexTime - curTime) > (SI32)(cwmWorldState->ServerData()->ServerNetRcvTimeout() * 1000) )
		{ // looks like we're not going to get it!
			// April 3, 2004 - EviLDeD - If we have some data, then we need to return it. Some of the network logic is looking at count size. this way we can also validate on the calling side so we ask for 4 bytes, but only 3 were sent back, adn let the calling routing handle it, if we call for 4 and get get NOTHING then throw... Just my thoughts - EviLDeD
			if( count <= 0 )
				throw socket_error( "Socket receive error" );
		}
	}
	while( x != 2560 && x != inlength );
	if( doLog )
		ReceiveLogging( NULL );
	bytesReceived += count;
	return count;
}

void cSocket::ClearAuthor( void )
{
	memset( authorbuffer, '~', 32 );
}
void cSocket::ClearTitle( void )
{
	memset( titlebuffer, '~', 62 );
}
void cSocket::ClearPage( void )
{
	memset( pagebuffer, '~', 512 );
}

void cSocket::OutLength( int newValue )
{
	outlength = newValue;
}
void cSocket::InLength( int newValue )
{
	inlength = newValue;
}
int cSocket::OutLength( void ) const
{
	return outlength;
}
int cSocket::InLength( void ) const
{
	return inlength;
}

bool cSocket::Logging( void ) const
{
	return logging;
}

void cSocket::Logging( bool newValue )
{
	logging = newValue;
}

CChar *cSocket::CurrcharObj( void ) const
{
	return currCharObj;
}

void cSocket::CurrcharObj( CChar *newValue )
{
	currCharObj = newValue;
}

//o--------------------------------------------------------------------------o
//|	Function			-	ACCOUNTSBLOCK &cSocket::GetAccount(void)
//|	Date					-	1/17/2003 6:21:59 AM
//|	Developers		-	EviLDeD
//|	Organization	-	UOX3 DevTeam
//|	Status				-	Currently under development
//o--------------------------------------------------------------------------o
//|	Description		-	Return to the calling function this objects accounts 
//|									referance.
//o--------------------------------------------------------------------------o
//| Modifications	-	
//o--------------------------------------------------------------------------o
ACCOUNTSBLOCK &cSocket::GetAccount(void)
{
	return actbAccount;
}

//o--------------------------------------------------------------------------o
//|	Function			-	void cSocket::SetAccount(ACCOUNTSBLOCK &actbBlock)
//|	Date					-	1/17/2003 7:01:23 AM
//|	Developers		-	EviLDeD
//|	Organization	-	UOX3 DevTeam
//|	Status				-	Currently under development
//o--------------------------------------------------------------------------o
//|	Description		-	
//o--------------------------------------------------------------------------o
//| Modifications	-	
//o--------------------------------------------------------------------------o
void cSocket::SetAccount( ACCOUNTSBLOCK& actbBlock )
{
	if( actbBlock.wAccountIndex == AB_INVALID_ID )
	{
		actbAccount.wAccountIndex = AB_INVALID_ID;
		return;
	}
	actbAccount	= actbBlock;
	Accounts->ModAccount( actbAccount.wAccountIndex, AB_ALL, actbBlock );
}

//o---------------------------------------------------------------------------o
//|		Function    -	UI16 cSocket::AcctNo( void )
//|		Date        -	November 29th, 2000
//|		Programmer  -	Abaddon
//|		Modified	-	Maarc, February 3, 2003 - reduced to UI16 to deal with
//|						accounts changes
//o---------------------------------------------------------------------------o
//|		Purpose     -	Returns the ID of the account number socket belongs to
//o---------------------------------------------------------------------------o
UI16 cSocket::AcctNo( void ) const
{
	return actbAccount.wAccountIndex;
}

//o---------------------------------------------------------------------------o
//|		Function    -	void cSocket::AcctNo( UI16 newValue )
//|		Date        -	November 29th, 2000
//|		Programmer  -	Abaddon
//|		Modified	-	Maarc, February 3, 2003 - reduced to UI16 to deal with
//|						accounts changes
//o---------------------------------------------------------------------------o
//|		Purpose     -	Sets the ID of the account number the socket belongs to
//o---------------------------------------------------------------------------o
void cSocket::AcctNo( UI16 newValue )
{
	ACCOUNTSBLOCK actbBlock;
	if( !Accounts->GetAccountByID( newValue, actbBlock ) )
	{
		actbAccount.wAccountIndex	= AB_INVALID_ID;
		return;
	}
	actbAccount	= actbBlock;
	Accounts->ModAccount( actbAccount.wAccountIndex, AB_ALL, actbBlock );
}

UI08 cSocket::ClientIP1( void ) const
{
	return clientip[0];
}
UI08 cSocket::ClientIP2( void ) const
{
	return clientip[1];
}
UI08 cSocket::ClientIP3( void ) const
{
	return clientip[2];
}
UI08 cSocket::ClientIP4( void ) const
{
	return clientip[3];
}

void cSocket::ClientIP1( UI08 newValue )
{
	clientip[0] = newValue;
}
void cSocket::ClientIP2( UI08 newValue )
{
	clientip[1] = newValue;
}
void cSocket::ClientIP3( UI08 newValue )
{
	clientip[2] = newValue;
}
void cSocket::ClientIP4( UI08 newValue )
{
	clientip[3] = newValue;
}

bool cSocket::NewClient( void ) const
{
	return newClient;
}

void cSocket::NewClient( bool newValue )
{
	newClient = newValue;
}

UI32 cSocket::GetDWord( size_t offset )
{
	return calcserial( buffer[offset], buffer[offset+1], buffer[offset+2], buffer[offset+3] );
}

UI16 cSocket::GetWord( size_t offset )
{
	return (UI16)((buffer[offset]<<8) + buffer[offset+1]);
}

UI08 cSocket::GetByte( size_t offset )
{
	return buffer[offset];
}

void cSocket::SetDWord( size_t offset, UI32 newValue )
{
	buffer[offset]   = (UI08)( newValue>>24 );
	buffer[offset+1] = (UI08)( newValue>>16 );
	buffer[offset+2] = (UI08)( newValue>>8 );
	buffer[offset+3] = (UI08)( newValue%256 );
}
void cSocket::SetWord( size_t offset, UI16 newValue ) 
{
	buffer[offset]   = (UI08)( newValue>>8 );
	buffer[offset+1] = (UI08)( newValue%256 );
}
void cSocket::SetByte( size_t offset, UI08 newValue )
{
	buffer[offset] = newValue;
}

void cSocket::ClientIP( UI32 newValue )
{
	clientip[0] = (UI08)( newValue>>24 );
	clientip[1] = (UI08)( newValue>>16 );
	clientip[2] = (UI08)( newValue>>8 );
	clientip[3] = (UI08)( newValue%256 );
}

void cSocket::TargetOK( bool newValue )
{
	targetok = newValue;
}

bool cSocket::TargetOK( void ) const
{
	return targetok;
}

void cSocket::ClickX( SI16 newValue )
{
	clickx = newValue;
}
void cSocket::ClickY( SI16 newValue )
{
	clicky = newValue;
}
SI16 cSocket::ClickX( void ) const
{
	return clickx;
}
SI16 cSocket::ClickY( void ) const
{
	return clicky;
}

char cSocket::PostAcked( int x, int y ) const
{
	return postAcked[x][y];
}
int cSocket::PostCount( void ) const
{
	return postCount;
}
int cSocket::PostAckCount( void ) const
{
	return postAckCount;
}

void cSocket::PostAcked( int x, int y, char newValue )
{
	postAcked[x][y] = newValue;
}
void cSocket::PostCount( int newValue )
{
	postCount = newValue;
}
void cSocket::PostAckCount( int newValue )
{
	postAckCount = newValue;
}

void cSocket::Send( cPBaseBuffer *toSend )
{
	if( toSend == NULL )
		return;

	// If the client cannot receive it validly, abort, abort!
	if( !( ((cPUOXBuffer *)toSend)->ClientCanReceive( this ) ) )
		return;

	UI32 len = 0;
	if( cryptclient )
	{
		len = toSend->Pack();
		send( cliSocket, (char *)toSend->PackedPointer(), len, 0 );
	}
	else
	{
		len = toSend->Length();
		send( cliSocket, (char *)toSend->Pointer(), len, 0 );
	}

	bytesSent += len;

	if( Logging() )
	{
		SERIAL toPrint;
		if( !ValidateObject( currCharObj ) )
			toPrint = INVALIDSERIAL;
		else
			toPrint = currCharObj->GetSerial();
		std::string logFile = cwmWorldState->ServerData()->Directory( CSDDP_LOGS ) + UString::number( toPrint ) + ".snd";
		std::ofstream logDestination;
		logDestination.open( logFile.c_str(), std::ios::out | std::ios::app );
		if( !logDestination.is_open() )
		{
			Console.Error( 1, "Failed to open socket log %s", logFile.c_str() );
			return;
		}
		toSend->Log( logDestination );
		logDestination.close();
	}
}

void cSocket::PickupSpot( PickupLocations newValue )
{
	pSpot = newValue;
}
PickupLocations	cSocket::PickupSpot( void ) const
{
	return pSpot;
}
SERIAL cSocket::PickupSerial( void ) const
{
	return pFrom;
}
void cSocket::PickupSerial( SERIAL pickupSerial )
{
	pFrom = pickupSerial;
}

SI16 cSocket::PickupX( void ) const
{
	return pX;
}

SI16 cSocket::PickupY( void ) const
{
	return pY;
}

SI08 cSocket::PickupZ( void ) const
{
	return pZ;
}

void cSocket::PickupLocation( SI16 x, SI16 y, SI08 z )
{
	pX = x;
	pY = y;
	pZ = z;
}

void cSocket::PickupX( SI16 x )
{
	pX = x;
}

void cSocket::PickupY( SI16 y )
{
	pY = y;
}

void cSocket::PickupZ( SI08 z )
{
	pZ = z;
}

char *cSocket::AuthorBuffer( void )
{
	return authorbuffer;
}
char *cSocket::TitleBuffer( void )
{
	return titlebuffer;
}
char *cSocket::PageBuffer( void )
{
	return pagebuffer;
}
void cSocket::AuthorBuffer( const char *newValue )
{
	strcpy( authorbuffer, newValue );
}
void cSocket::PageBuffer( const char *newValue )
{
	strcpy( pagebuffer, newValue );
}
void cSocket::TitleBuffer( const char *newValue )
{
	strcpy( titlebuffer, newValue );
}


// Helper function to help us packet data into an array
template< class T >
void PackInto( UI08 *toPack, size_t offset, T& value )
{
	size_t sizeVal		= sizeof( T );
	UI08 *recastedPtr	= reinterpret_cast< UI08 * >(&value);
	for( size_t i = 0; i < sizeVal; ++i )
	{
		toPack[offset + i] = recastedPtr[i];
	}
}


// cPBaseBuffer class implementation
size_t cPBaseBuffer::Length( void ) const
{
	return internalBuffer.size();
}

UI32 cPBaseBuffer::PackedLength( void ) const
{
	return packedLength;
}

void cPBaseBuffer::Log( std::ofstream &outStream, bool fullHeader )
{
	if( fullHeader )
		outStream << "[SEND]Packet: 0x" << (internalBuffer[0] < 10?"0":"") << std::hex << (UI16)internalBuffer[0] << "--> Length:" << std::dec << internalBuffer.size() << std::endl;
	doPacketLogging( outStream, internalBuffer.size(), internalBuffer );
}

UI08& cPBaseBuffer::operator [] ( size_t Num )
{
	if( Num > Length() )
		throw std::runtime_error( "out of bounds" );
	return internalBuffer[Num];
}

void cPBaseBuffer::InternalReset( void )
{
	internalBuffer.resize( 0 );
	packedBuffer.resize( 0 );
	isPacked		=	false;
	packedLength	=	0;
	internalBufferOffset = 0xFFFFFFFF;	// we aren't positioned yet
}
cPBaseBuffer::cPBaseBuffer()
{
	InternalReset();
}

cPBaseBuffer::cPBaseBuffer( char *initBuffer, size_t len )
{
	InternalReset();
	internalBuffer.resize( len );
	for( size_t i = 0; i < len; ++i )
		internalBuffer[i] = initBuffer[i];
	internalBufferOffset = len;
}

cPBaseBuffer::cPBaseBuffer( cPBaseBuffer *initBuffer )
{
	InternalReset();
	internalBuffer.resize( initBuffer->Length() );
	for( size_t i = 0; i < initBuffer->Length(); ++i )
		internalBuffer[i] = (*initBuffer)[i];
}

cPBaseBuffer::~cPBaseBuffer()
{
	internalBuffer.resize( 0 );
	packedBuffer.resize( 0 );
}

void cPBaseBuffer::Resize( size_t newLen )
{
	internalBuffer.resize( newLen );
}

const UI08 *cPBaseBuffer::Pointer( void ) const
{
	return (const UI08 *)&internalBuffer[0];
}

const UI08 *cPBaseBuffer::PackedPointer( void ) const
{
	return (const UI08 *)&packedBuffer[0];
}

cPBaseBuffer& cPBaseBuffer::operator<<( const UI08 value )
{
	if( internalBufferOffset == 0xFFFFFFFF )
	{
		internalBuffer.resize( 1 );
		internalBufferOffset = 0;
	}
	else
		internalBuffer.resize( internalBufferOffset + 1 );
	PackInto( &internalBuffer[0], internalBufferOffset, (UI08)value );
	return (*this);
}
cPBaseBuffer& cPBaseBuffer::operator<<( const UI16 value )
{
	if( internalBufferOffset == 0xFFFFFFFF )
	{
		internalBuffer.resize( 2 );
		internalBufferOffset = 0;
	}
	else
		internalBuffer.resize( internalBufferOffset + 2 );
	PackInto( &internalBuffer[0], internalBufferOffset, (UI16)value );
	return (*this);
}
cPBaseBuffer& cPBaseBuffer::operator<<( const UI32 value )
{
	if( internalBufferOffset == 0xFFFFFFFF )
	{
		internalBuffer.resize( 4 );
		internalBufferOffset = 0;
	}
	else
		internalBuffer.resize( internalBufferOffset + 4 );
	PackInto( &internalBuffer[0], internalBufferOffset, (UI32)value );
	return (*this);
}
cPBaseBuffer& cPBaseBuffer::operator<<( const SI08 value )
{
	return ( (*this) << static_cast< UI08 >(value) );
}
cPBaseBuffer& cPBaseBuffer::operator<<( const SI16 value )
{
	return ( (*this) << static_cast< UI16 >(value) );
}
cPBaseBuffer& cPBaseBuffer::operator<<( const SI32 value )
{
	return ( (*this) << static_cast< UI32 >(value) );
}
cPBaseBuffer& cPBaseBuffer::operator<<( const std::string& value )
{
	std::string::const_iterator i = value.begin();
	while( i != value.end() )
	{
		(*this) << static_cast< UI08 >(*i);
		++i;
	}
	return (*this);
}

cPUOXBuffer::cPUOXBuffer() : cPBaseBuffer()
{
}
cPUOXBuffer::cPUOXBuffer( char *initBuffer, size_t len ) : cPBaseBuffer( initBuffer, len )
{
}
cPUOXBuffer::~cPUOXBuffer()
{
}
cPUOXBuffer::cPUOXBuffer( cPBaseBuffer *initBuffer ) : cPBaseBuffer( initBuffer )
{
}
bool cPUOXBuffer::ClientCanReceive( cSocket *mSock )
{
	// Default implementation, all clients can receive all packets
	return true;
}
UI32 cPUOXBuffer::Pack( void )
{
	if( isPacked )
		return packedLength;

	packedBuffer.resize( internalBuffer.size() * 2 );
	size_t len	= Length();
	UI08 *pIn	= (UI08 *)&internalBuffer[0];
	UI08 *pOut	= (UI08 *)&packedBuffer[0];

	isPacked	= true;

	packedLength = DoPack( pIn, pOut, len );
	return packedLength;
}


cPXGMBuffer::cPXGMBuffer() : cPBaseBuffer()
{
}
cPXGMBuffer::cPXGMBuffer( char *initBuffer, size_t len ) : cPBaseBuffer( initBuffer, len )
{
}
cPXGMBuffer::~cPXGMBuffer()
{
}
cPXGMBuffer::cPXGMBuffer( cPBaseBuffer *initBuffer ) : cPBaseBuffer( initBuffer )
{
}
UI32 cPXGMBuffer::Pack( void )
{
	return Length();
}
UI32 cPXGMBuffer::PackedLength( void ) const
{
	return Length();
}
const UI08 *cPXGMBuffer::PackedPointer( void ) const
{
	// we don't pack with xGM
	return Pointer();
}



cPInputBuffer::cPInputBuffer() : tSock( NULL )
{
}
cPInputBuffer::cPInputBuffer( cSocket *input )
{
	SetSocket( input );
}
void cPInputBuffer::Receive( void )
{
}
UI08& cPInputBuffer::operator[] ( size_t num )
{
	return internalBuffer[num];
}
size_t cPInputBuffer::Length( void )
{
	return internalBuffer.size();
}
UI08 *cPInputBuffer::Pointer( void )
{
	return &internalBuffer[0];
}
void cPInputBuffer::Log( std::ofstream &outStream, bool fullHeader )
{
	UI08 *buffer	= tSock->Buffer();
	const UI32 len	= tSock->InLength();
	if( fullHeader )
		outStream << "[RECV]Packet Class Generic: 0x" << std::hex << (UI16)buffer[0] << "--> Length: " << std::dec << len << std::endl;
	doPacketLogging( outStream, len, (char *)buffer );
}

long cPInputBuffer::DWord( size_t offset )
{
	return ( (internalBuffer[offset]<<24) + (internalBuffer[offset+1]<<16) + (internalBuffer[offset+2]<<8) + internalBuffer[offset+3] );
}

SI32 cPInputBuffer::Word( size_t offset )
{
	return ( (internalBuffer[offset]<<8) + internalBuffer[offset+1] );
}

UI08 cPInputBuffer::Byte( size_t offset )
{
	return internalBuffer[offset];
}


bool cPInputBuffer::Handle( void )
{
	return false;
}

void cPInputBuffer::SetSocket( cSocket *toSet )
{
	tSock = toSet;
}

cSocket *cPInputBuffer::GetSocket( void ) const
{
	return tSock;
}

UnicodeTypes cSocket::Language( void ) const
{
	return lang;
}

void cSocket::Language( UnicodeTypes newVal )
{
	lang = newVal;
}

UI32 cSocket::ClientVersion( void ) const
{
	return clientVersion;
}
void cSocket::ClientVersion( UI32 newVer )
{
	clientVersion = newVer;
}
void cSocket::ClientVersion( UI08 major, UI08 minor, UI08 sub, UI08 letter )
{
	ClientVersion( calcserial( major, minor, sub, letter ) );
}

ClientTypes cSocket::ClientType( void ) const
{
	return cliType;
}
void cSocket::ClientType( ClientTypes newVer )
{
	cliType = newVer;
}


UI08 cSocket::ClientVersionMajor( void ) const
{
	return (UI08)(clientVersion>>24);
}

UI08 cSocket::ClientVersionMinor( void ) const
{
	return (UI08)(clientVersion>>16);
}

UI08 cSocket::ClientVersionSub( void ) const
{
	return (UI08)(clientVersion>>8);
}

UI08 cSocket::ClientVersionLetter( void ) const
{
	return (UI08)(clientVersion%256);
}

void cSocket::ClientVersionMajor( UI08 value )
{
	ClientVersion( value, ClientVersionMinor(), ClientVersionSub(), ClientVersionLetter() );
}

void cSocket::ClientVersionMinor( UI08 value )
{
	ClientVersion( ClientVersionMajor(), value, ClientVersionSub(), ClientVersionLetter() );
}

void cSocket::ClientVersionSub( UI08 value )
{
	ClientVersion( ClientVersionMajor(), ClientVersionMinor(), value, ClientVersionLetter() );
}

void cSocket::ClientVersionLetter( UI08 value )
{
	ClientVersion( ClientVersionMajor(), ClientVersionMinor(), ClientVersionSub(), value );
}

void cSocket::Range( UI08 value )
{
	range = value;
}

UI08 cSocket::Range( void ) const
{
	return range;
}

void cSocket::sysmessage( const char *txt, ...) // System message (In lower left corner)
{
	va_list argptr;
	if( txt == NULL )
		return;

	CChar *mChar = CurrcharObj();
	if( !ValidateObject( mChar ) )
		return;

	char msg[512];
	va_start( argptr, txt );
	vsprintf( msg, txt, argptr );
	va_end( argptr );
	
	CSpeechEntry *toAdd = SpeechSys->Add();
	toAdd->Speech( msg );
	toAdd->Font( FNT_NORMAL );
	toAdd->Speaker( INVALIDSERIAL );
	toAdd->SpokenTo( mChar->GetSerial() );
	toAdd->Colour( 0x0040 );
	toAdd->Type( SYSTEM );
	toAdd->At( cwmWorldState->GetUICurrentTime() );
	toAdd->TargType( SPTRG_INDIVIDUAL );
}

void cSocket::sysmessage( SI32 dictEntry, ... )	// System message (based on dictionary entry)
{
	CChar *mChar = CurrcharObj();
	if( !ValidateObject( mChar ) )
		return;

	va_list argptr;
	std::string txt = Dictionary->GetEntry( dictEntry, Language() );
	if( txt.empty() )
		return;
	char msg[512];
	va_start( argptr, dictEntry );
	vsprintf( msg, txt.c_str(), argptr );
	va_end( argptr );

	CSpeechEntry *toAdd = SpeechSys->Add();
	toAdd->Speech( msg );
	toAdd->Font( FNT_NORMAL );
	toAdd->Speaker( INVALIDSERIAL );
	toAdd->SpokenTo( mChar->GetSerial() );
	toAdd->Colour( 0x0040 );
	toAdd->Type( SYSTEM );
	toAdd->At( cwmWorldState->GetUICurrentTime() );
	toAdd->TargType( SPTRG_INDIVIDUAL );
}

//o---------------------------------------------------------------------------o
//|		Function    :	void objMessage( SI32 dictEntry, cBaseObject *getObj, R32 secsFromNow, UI16 Colour, ... )
//|		Date        :	2/11/2003
//|		Programmer  :	Zane
//o---------------------------------------------------------------------------o
//|		Purpose     :	Shows information on items when clicked or guild info (if any) for players
//o---------------------------------------------------------------------------o
void cSocket::objMessage( SI32 dictEntry, cBaseObject *getObj, R32 secsFromNow, UI16 Colour, ... )
{
	if( !ValidateObject( getObj ) )
		return;

	std::string txt = Dictionary->GetEntry( dictEntry, Language() );
	if( txt.empty() )
		return;

	va_list argptr;

	char msg[512];
	va_start( argptr, Colour );
	vsprintf( msg, txt.c_str(), argptr );
	va_end( argptr );
	objMessage( msg, getObj, secsFromNow, Colour );
}

//o---------------------------------------------------------------------------o
//|		Function    :	void objMessage( const char *txt, cBaseObject *getObj, R32 secsFromNow, UI16 Colour )
//|		Date        :	2/11/2003
//|		Programmer  :	Zane
//o---------------------------------------------------------------------------o
//|		Purpose     :	Shows information on items when clicked or guild info (if any) for players
//o---------------------------------------------------------------------------o
void cSocket::objMessage( const char *txt, cBaseObject *getObj, R32 secsFromNow, UI16 Colour )
{
	if( txt == NULL )
		return;
	CChar *mChar		= CurrcharObj();
	CSpeechEntry *toAdd = SpeechSys->Add();
	toAdd->Speech( txt );
	toAdd->Font( FNT_NORMAL );
	toAdd->Speaker( getObj->GetSerial() );
	toAdd->SpokenTo( mChar->GetSerial() );
	toAdd->Type( SYSTEM );
	toAdd->At( BuildTimeValue( secsFromNow ) );
	toAdd->TargType( SPTRG_INDIVIDUAL );

	if( getObj->GetObjType() == OT_ITEM )
	{
		CItem *getItem = static_cast< CItem *>(getObj);
		if( getItem->isCorpse() )
		{
			UI16 targColour;
			CChar *targChar = getItem->GetOwnerObj();
			if( ValidateObject( targChar ) )
				targColour = GetFlagColour( mChar, targChar );
			else
			{
				switch( getItem->GetTempVar( CITV_MOREZ ) )
				{
					case 0x01:	targColour = 0x0026;	break;	//red
					case 0x02:	targColour = 0x03B2;	break;	// gray
					case 0x08:	targColour = 0x0049;	break;	// green
					case 0x10:	targColour = 0x0030;	break;	// orange
					default:
					case 0x04:	targColour = 0x005A;	break;	// blue
				}
			}
			toAdd->Colour( targColour );
		}
	}
	else
		toAdd->Colour( Colour );
	if( toAdd->Colour() == 0 )
		toAdd->Colour( 0x03B2 );
}

void cSocket::ShowCharName( CChar *i, bool showSer ) // Singleclick text for a character
{
	UI08 a1 = i->GetSerial( 1 );
	UI08 a2 = i->GetSerial( 2 );
	UI08 a3 = i->GetSerial( 3 );
	UI08 a4 = i->GetSerial( 4 );
	UString newName = i->GetName();
	CChar *mChar = CurrcharObj();
	if( mChar->GetSingClickSer() || showSer )
		objMessage( 1737, i, 0.0f, 0x03B2, a1, a2, a3, a4 );
	if( !i->IsNpc() )
	{
		if( i->GetSquelched() )
			objMessage( 1736, i );
		if( i->GetCommandLevel() < CNS_CMDLEVEL && i->GetFame() >= 10000 )	// Morollan, only normal players have titles now
		{
			if( i->GetID( 2 ) == 0x91 )
				newName = UString::sprintf( Dictionary->GetEntry( 1740, Language() ).c_str(), newName.c_str() );	// Morrolan, added Lord/Lady to title overhead
			else if( i->GetID( 1 ) == 0x90 )
				newName = UString::sprintf( Dictionary->GetEntry( 1739, Language() ).c_str(), newName.c_str() );
		}
		if( i->GetRace() != 0 && i->GetRace() != 65535 )	// need to check for placeholder race (Abaddon)
		{
			newName += " (";
			newName += Races->Name( i->GetRace() );
			newName += ")";
		}
		if( i->GetTownPriv() == 2 )
			newName = UString::sprintf( Dictionary->GetEntry( 1738, Language() ).c_str(), newName.c_str() );
		if( !isOnline( i ) )
			newName += " (OFF)";
	}
	else
	{
		if( i->IsTamed() && ValidateObject( i->GetOwnerObj() ) && i->GetNPCAiType() != aiPLAYERVENDOR )
			newName += " (tame) ";
	}
	if( i->IsInvulnerable() )
		newName += " (invulnerable)";
	if( i->IsFrozen() )
		newName += " (frozen) ";
	if( i->IsGuarded() )
		newName += " (guarded)";
	if( i->GetKills() > cwmWorldState->ServerData()->RepMaxKills() )
		newName += " [Murderer]";
	if( i->GetGuildNumber() != -1 && !i->IsIncognito() )
		GuildSys->DisplayTitle( this, i );
	
	CSpeechEntry *toAdd = SpeechSys->Add();
	toAdd->Speech( newName );
	toAdd->Font( FNT_NORMAL );
	toAdd->Speaker( i->GetSerial() );
	toAdd->SpokenTo( mChar->GetSerial() );
	toAdd->Colour( GetFlagColour( mChar, i ) );
	toAdd->Type( SYSTEM );
	toAdd->At( cwmWorldState->GetUICurrentTime() );
	toAdd->TargType( SPTRG_INDIVIDUAL );
}

COLOUR cSocket::GetFlagColour( CChar *src, CChar *trg )
{
	GUILDRELATION gRel;
	if( trg->IsIncognito() )
		gRel = GR_UNKNOWN;
	else
		gRel = GuildSys->Compare( src, trg );
	SI08 race = Races->Compare( src, trg );

	COLOUR retVal = 0x0058;	// blue
	if( !trg->IsNpc() && trg->GetCommandLevel() > 0 && ( trg->GetID() == 0x03DB ) )
		retVal = Commands->GetColourByLevel( trg->GetCommandLevel() );//get their command level color if they look like a gm ONLY
	else if( trg->GetKills() > cwmWorldState->ServerData()->RepMaxKills() || trg->IsMurderer() )
		retVal = 0x0026;//Red
	else if( gRel == GR_SAME || gRel == GR_ALLY || race > 0 )
		retVal = 0x0043;//Same guild (Green)
	else if( gRel == GR_WAR || race < 0 )
		retVal = 0x0030;//enemy guild (orange)
	else if( trg->IsCriminal() )
		retVal = 0x03B2;//grey

	return retVal;
}

//o---------------------------------------------------------------------------o
//|	Function	-	void target()
//|	Programmer	-	UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|	Purpose		-	Send targeting cursor to client
//o---------------------------------------------------------------------------o
void cSocket::target( UI08 targType, UI08 targID, const char *txt )
{
	CPTargetCursor toSend;
	toSend.ID( calcserial( 0, 1, targType, targID ) );
	toSend.Type( 1 );
	TargetOK( true );
	sysmessage( txt );
	Send( &toSend );
}

void cSocket::target( UI08 targType, UI08 targID, SI32 dictEntry, ... )
{
	std::string txt = Dictionary->GetEntry( dictEntry, Language() );
	if( txt.empty() )
		return;

	va_list argptr;

	char msg[512];
	va_start( argptr, dictEntry );
	vsprintf( msg, txt.c_str(), argptr );
	va_end( argptr );
	target( targType, targID, msg );
}

void cSocket::mtarget( UI16 itemID, SI32 dictEntry )
{
	std::string txt = Dictionary->GetEntry( dictEntry, Language() );
	if( txt.empty() )
		return;
	CPMultiPlacementView toSend( calcserial( 0, 1, 0, TARGET_BUILDHOUSE ) );
	toSend.MultiModel( itemID );
	toSend.RequestType( 1 );

	sysmessage( txt.c_str() );
	TargetOK( true );
	Send( &toSend );
}

bool cSocket::ReceivedVersion( void ) const
{
	return receivedVersion;
}
void cSocket::ReceivedVersion( bool value )
{
	receivedVersion = value;
}

UI32 cSocket::BytesSent( void ) const
{
	return bytesSent;
}

UI32 cSocket::BytesReceived( void ) const
{
	return bytesReceived;
}

//o---------------------------------------------------------------------------o
//|	Function	-	void statwindow( CChar *i )
//|	Programmer	-	UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|	Purpose		-	Opens the status window
//o---------------------------------------------------------------------------o
void cSocket::statwindow( CChar *i )
{
	if( !ValidateObject( i ) )
		return;

	CPStatWindow toSend( (*i), (*this) );
	
	CChar *mChar = CurrcharObj();
	//Zippy 9/17/01 : fixed bug of your name on your own stat window
	toSend.NameChange( mChar != i && ( mChar->IsGM() || i->GetOwnerObj() == mChar ) );
	toSend.Gold( GetItemAmount( i, 0x0EED ) );
	toSend.AC( Combat->calcDef( i, 0, false ) );
	toSend.Weight( (UI16)(i->GetWeight() / 100) );
	Send( &toSend );
}

//o---------------------------------------------------------------------------o
//|	Function	-	void updateskill( UI08 skillnum )
//|	Programmer	-	UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|	Purpose		-	Update a certain skill
//o---------------------------------------------------------------------------o
void cSocket::updateskill( UI08 skillnum )
{
	CChar *mChar = CurrcharObj();
	CPUpdIndSkill toSend( (*mChar), (UI08)skillnum );
	Send( &toSend );
}

//o---------------------------------------------------------------------------o
//|	Function	-	void openPack( cSocket *s, CItem *i )
//|	Programmer	-	Unknown
//o---------------------------------------------------------------------------o
//|	Purpose		-	Open backpack and display contents
//o---------------------------------------------------------------------------o
void cSocket::openPack( CItem *i )
{
	if( !ValidateObject( i ) )
	{
		Console.Warning( 2, "openPack() was passed an invalid item" );
		return;
	}
	CPDrawContainer contSend = (*i);
	contSend.Model( 0x3C );

	if( i->GetID( 1 ) == 0x3E )            // boats
		contSend.Model( 0x4C );
	else
	{
		switch( Items->getPackType( i ) )
		{
			case PT_COFFIN:
				contSend.Model( 0x09 );
				break;
			case PT_PACK:
			case PT_PACK2:
				contSend.Model( 0x3C );
				break;
			case PT_BAG:
				contSend.Model( 0x3D );
				break;
			case PT_BARREL:
				contSend.Model( 0x3E );
				break;
			case PT_SQBASKET:
				contSend.Model( 0x3F );
				break;
			case PT_RBASKET:
				contSend.Model( 0x41 );
				break;
			case PT_GCHEST:
				contSend.Model( 0x42 );
				break;
			case PT_WBOX:
				contSend.Model( 0x43 );
				break;
			case PT_CRATE:
				contSend.Model( 0x44 );
				break;
			case PT_SHOPPACK:
				contSend.Model( 0x47 );
				break;
			case PT_DRAWER:
				contSend.Model( 0x48 );
				break;
			case PT_WCHEST:
				contSend.Model( 0x49 );
				break;
			case PT_MBOX:
				contSend.Model( 0x4B );
				break;
			case PT_SCHEST:
			case PT_BANK:
				contSend.Model( 0x4A );
				break;
			case PT_BOOKCASE:
				contSend.Model( 0x4D );
				break;
			case PT_FARMOIRE:
				contSend.Model( 0x4E );
				break;
			case PT_WARMOIRE:
				contSend.Model( 0x4F );
				break;
			case PT_DRESSER:
				contSend.Model( 0x51 );
				break;
			case PT_UNKNOWN:
				Console.Warning( 2, "openPack() passed an invalid container type: 0x%X", i->GetSerial() );
				return;
		}
	}   
	Send( &contSend );
	CPItemsInContainer itemsIn( this, i );
	Send( &itemsIn );
}

//o---------------------------------------------------------------------------o
//|	Function	-	void openBank( CChar *i )
//|	Programmer	-	UOX3 DevTeam
//o---------------------------------------------------------------------------o
//|	Purpose		-	Opens players bank box
//o---------------------------------------------------------------------------o
void cSocket::openBank( CChar *i )
{
	CItem *bankBox = NULL;
	char temp[1024];

	bankBox = i->GetItemAtLayer( IL_BANKBOX );
	if( ValidateObject( bankBox ) )
	{
		if( bankBox->GetType() == IT_CONTAINER && bankBox->GetTempVar( CITV_MOREX ) == 1 )
		{
			CPWornItem toWearO = (*bankBox);
			Send( &toWearO );
			openPack( bankBox );
			return;
		}
	}
	
	sprintf( temp, Dictionary->GetEntry( 1283 ).c_str(), i->GetName().c_str() );
	bankBox = Items->CreateItem( NULL, i, 0x09AB, 1, 0, OT_ITEM );
	bankBox->SetName( temp );
	bankBox->SetLayer( IL_BANKBOX );
	bankBox->SetOwner( i );
	if( !bankBox->SetCont( i ) )
		return;
	bankBox->SetTempVar( CITV_MOREX, 1 );
	bankBox->SetType( IT_CONTAINER );
	CPWornItem toWear = (*bankBox);
	Send( &toWear );
	openPack( bankBox );
}

//o---------------------------------------------------------------------------o
//|   Function    -  TIMERVAL Timer()
//|   Date        -  September 25, 2003
//|   Programmer  -  giwo
//o---------------------------------------------------------------------------o
//|   Purpose     -  Temporary Timer Values associated to the character connected
//|						to the socket
//o---------------------------------------------------------------------------o

TIMERVAL cSocket::GetTimer( cS_TID timerID ) const
{
	TIMERVAL rvalue = 0;
	if( timerID != tPC_COUNT )
		rvalue = pcTimers[timerID];
	return rvalue;
}
void cSocket::SetTimer( cS_TID timerID, TIMERVAL value )
{
	if( timerID != tPC_COUNT )
		pcTimers[timerID] = value;
}

//o---------------------------------------------------------------------------o
//|   Function    -  TIMERVAL ClearTimers()
//|   Date        -  September 25, 2003
//|   Programmer  -  giwo
//o---------------------------------------------------------------------------o
//|   Purpose     -  Resets all timers (but mutetime) to 0 used when player logs out
//o---------------------------------------------------------------------------o
void cSocket::ClearTimers( void )
{
	for( int mTID = (int)tPC_SKILLDELAY; mTID < (int)tPC_COUNT; ++mTID )
	{
		if( mTID != tPC_MUTETIME )
			pcTimers[mTID] = 0;
	}
}

//o---------------------------------------------------------------------------o
//|	Function	-	OpenURL( const std::string txt )
//|	Programmer	-	Unknown
//o---------------------------------------------------------------------------o
//|	Purpose		-	Launch a webpage from within the client
//o---------------------------------------------------------------------------o
void cSocket::OpenURL( const std::string txt )
{
	sysmessage( 1210 );
	CPWebLaunch toSend( txt );
	Send( &toSend );
}

}
