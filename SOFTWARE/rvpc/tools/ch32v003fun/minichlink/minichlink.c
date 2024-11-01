// The rest of the code, Copyright 2023 Charles Lohr
// Freely licensable under the MIT/x11, NewBSD Licenses, or
// public domain where applicable. 

// TODO: Can we make a unified DMPROG for reading + writing?

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "terminalhelp.h"
#include "minichlink.h"
#include "../ch32v003fun/ch32v003fun.h"

#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#ifndef _SYNCHAPI_H_
void Sleep(uint32_t dwMilliseconds);
#endif
#else
#include <unistd.h>
#endif

static int64_t StringToMemoryAddress( const char * number ) __attribute__((used));
static void StaticUpdatePROGBUFRegs( void * dev ) __attribute__((used));
int DefaultReadBinaryBlob( void * dev, uint32_t address_to_read_from, uint32_t read_size, uint8_t * blob );

void TestFunction(void * v );
struct MiniChlinkFunctions MCF;

void * MiniCHLinkInitAsDLL( struct MiniChlinkFunctions ** MCFO, const init_hints_t* init_hints )
{
	void * dev = 0;
	
	const char * specpgm = init_hints->specific_programmer;
	if( specpgm )
	{
		if( strcmp( specpgm, "linke" ) == 0 )
			dev = TryInit_WCHLinkE();
		else if( strcmp( specpgm, "esp32s2chfun" ) == 0 )
			dev = TryInit_ESP32S2CHFUN();
		else if( strcmp( specpgm, "nchlink" ) == 0 )
			dev = TryInit_NHCLink042();
		else if( strcmp( specpgm, "b003boot" ) == 0 )
			dev = TryInit_B003Fun();
		else if( strcmp( specpgm, "ardulink" ) == 0 )
			dev = TryInit_B003Fun();
	}
	else
	{
		if( (dev = TryInit_WCHLinkE()) )
		{
			fprintf( stderr, "Found WCH Link\n" );
		}
		else if( (dev = TryInit_ESP32S2CHFUN()) )
		{
			fprintf( stderr, "Found ESP32S2 Programmer\n" );
		}
		else if ((dev = TryInit_NHCLink042()))
		{
			fprintf( stderr, "Found NHC-Link042 Programmer\n" );
		}
		else if ((dev = TryInit_B003Fun()))
		{
			fprintf( stderr, "Found B003Fun Bootloader\n" );
		}
		else if ( init_hints->serial_port && (dev = TryInit_Ardulink(init_hints)))
		{
			fprintf( stderr, "Found Ardulink Programmer\n" );
		}
	}

	if( !dev )
	{
		fprintf( stderr, "Error: Could not initialize any supported programmers\n" );
		return 0;
	}

	struct InternalState * iss = calloc( 1, sizeof( struct InternalState ) );
	((struct ProgrammerStructBase*)dev)->internal = iss;
	iss->ram_base = 0x20000000;
	iss->ram_size = 2048;
	iss->sector_size = 64;
	iss->flash_size = 16384;
	iss->target_chip_type = 0;

	SetupAutomaticHighLevelFunctions( dev );

	if( MCFO )
	{
		*MCFO = &MCF;
	}
	return dev;
}

#if !defined( MINICHLINK_AS_LIBRARY ) && !defined( MINICHLINK_IMPORT )
int main( int argc, char ** argv )
{
	int i;

	if( argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h' )
	{
		goto help;
	}
	init_hints_t hints;
	memset(&hints, 0, sizeof(hints));

	// Scan for possible hints.
	for( i = 0; i < argc; i++ )
	{
		char * v = argv[i];
		if( strncmp( v, "-c", 2 ) == 0 )
		{
			i++;
			if( i < argc )
				hints.serial_port = argv[i];
		}
		else if( strncmp( v, "-c", 2 ) == 0 )
		{
			i++;
			if( i < argc )
				hints.specific_programmer = argv[i];
		}
	}

	void * dev = MiniCHLinkInitAsDLL( 0, &hints );
	if( !dev )
	{
		fprintf( stderr, "Error: Could not initialize any supported programmers\n" );
		return -32;
	}

	int status;
	int must_be_end = 0;

	int skip_startup = 
		(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'u' ) |
		(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h' ) |
		(argc > 1 && argv[1][0] == '-' && argv[1][1] == 't' ) |
		(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'f' ) |
		(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'X' );

	if( !skip_startup && MCF.SetupInterface )
	{
		if( MCF.SetupInterface( dev ) < 0 )
		{
			fprintf( stderr, "Could not setup interface.\n" );
			return -33;
		}
		printf( "Interface Setup\n" );
	}

//	TestFunction( dev );

	int iarg = 1;
	const char * lastcommand = 0;
	for( ; iarg < argc; iarg++ )
	{
		char * argchar = argv[iarg];

		lastcommand = argchar;
		if( argchar[0] != '-' )
		{
			fprintf( stderr, "Error: Need prefixing - before commands\n" );
			goto help;
		}
		if( must_be_end )
		{
			fprintf( stderr, "Error: the command '%c' cannot be followed by other commands.\n", must_be_end );
			return -1;
		}
		
keep_going:
		switch( argchar[1] )
		{
			default:
				fprintf( stderr, "Error: Unknown command %c\n", argchar[1] );
			case 'h':
				goto help;
			case '3':
				if( MCF.Control3v3 )
					MCF.Control3v3( dev, 1 );
				else
					goto unimplemented;
				break;
			case '5':
				if( MCF.Control5v )
					MCF.Control5v( dev, 1 );
				else
					goto unimplemented;
				break;
			case 't':
				if( MCF.Control3v3 )
					MCF.Control3v3( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'f':
				if( MCF.Control5v )
					MCF.Control5v( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'C': // For specifying programmer
			case 'c':
				// COM port or programmer argument already parsed previously
				// we still need to skip the next argument
				iarg+=1;
				if( iarg >= argc )
				{
					fprintf( stderr, "-c/C argument required 2 arguments\n" );
					goto unimplemented;
				}
				break;
			case 'u':
				if( MCF.Unbrick )
					MCF.Unbrick( dev );
				else
					goto unimplemented;
				break;
			case 'U':
				// Unlock Bootloader
				if( InternalUnlockBootloader( dev ) )
					goto unimplemented;
				break;
			case 'b':  //reBoot
				if( !MCF.HaltMode || MCF.HaltMode( dev, HALT_MODE_REBOOT ) )
					goto unimplemented;
				break;
			case 'B':  //reBoot into Bootloader
				if( !MCF.HaltMode || MCF.HaltMode( dev, HALT_MODE_GO_TO_BOOTLOADER ) )
					goto unimplemented;
				break;
			case 'e':  //rEsume
				if( !MCF.HaltMode || MCF.HaltMode( dev, HALT_MODE_RESUME ) )
					goto unimplemented;
				break;
			case 'E':  //Erase whole chip.
				if( MCF.HaltMode ) MCF.HaltMode( dev, HALT_MODE_HALT_AND_RESET );
				if( !MCF.Erase || MCF.Erase( dev, 0, 0, 1 ) )
					goto unimplemented;
				break;
			case 'a':
				if( !MCF.HaltMode || MCF.HaltMode( dev, HALT_MODE_HALT_AND_RESET ) )
					goto unimplemented;
				break;
			case 'A':  // Halt without reboot
				if( !MCF.HaltMode || MCF.HaltMode( dev, HALT_MODE_HALT_BUT_NO_RESET ) )
					goto unimplemented;
				break;

			// disable NRST pin (turn it into a GPIO)
			case 'd':  // see "RSTMODE" in datasheet
				if( MCF.HaltMode ) MCF.HaltMode( dev, HALT_MODE_HALT_AND_RESET );
				if( MCF.ConfigureNRSTAsGPIO )
					MCF.ConfigureNRSTAsGPIO( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'D': // see "RSTMODE" in datasheet
				if( MCF.HaltMode ) MCF.HaltMode( dev, HALT_MODE_HALT_AND_RESET );
				if( MCF.ConfigureNRSTAsGPIO )
					MCF.ConfigureNRSTAsGPIO( dev, 1 );
				else
					goto unimplemented;
				break;
			case 'p': 
				if( MCF.HaltMode ) MCF.HaltMode( dev, HALT_MODE_HALT_AND_RESET );
				if( MCF.ConfigureReadProtection )
					MCF.ConfigureReadProtection( dev, 0 );
				else
					goto unimplemented;
				break;
			case 'P':
				if( MCF.HaltMode ) MCF.HaltMode( dev, HALT_MODE_HALT_AND_RESET );
				if( MCF.ConfigureReadProtection )
					MCF.ConfigureReadProtection( dev, 1 );
				else
					goto unimplemented;
				break;
			case 'G':
			case 'T':
			{
				if( !MCF.PollTerminal )
					goto unimplemented;

				if( argchar[1] == 'G' && SetupGDBServer( dev ) )
				{
					printf( "Error: can't start GDB server\n" );
					return -1;
				}
				if( argchar[1] == 'G' )
				{
					printf( "GDBServer Running\n" );
				}
				else if( argchar[1] == 'T' )
				{
					// In case we aren't running already.
					MCF.HaltMode( dev, 2 );
				}

				CaptureKeyboardInput();

				uint32_t appendword = 0;
				do
				{
					uint8_t buffer[256];
					if( !IsGDBServerInShadowHaltState( dev ) )
					{
						// Handle keyboard input.
						if( appendword == 0 )
						{
							int i;
							for( i = 0; i < 3; i++ )
							{
								if( !IsKBHit() ) break;
								appendword |= ReadKBByte() << (i*8+8);
							}
							appendword |= i+4; // Will go into DATA0.
						}
						int r = MCF.PollTerminal( dev, buffer, sizeof( buffer ), appendword, 0 );
						if( r == -1 )
						{
							// Other end ack'd without printf.
							appendword = 0;
						}
						else if( r < 0 )
						{
							fprintf( stderr, "Terminal dead.  code %d\n", r );
							return -32;
						}
						else if( r > 0 )
						{
							fwrite( buffer, r, 1, stdout );
							fflush( stdout );
							// Otherwise it's basically just an ack for appendword.
							appendword = 0;
						}
					}

					if( argchar[1] == 'G' )
					{
						PollGDBServer( dev );
					}
				} while( 1 );

				// Currently unreachable - consider reachable-ing
				if( argchar[1] == 'G' )
					ExitGDBServer( dev );
				break;
			}
			case 's':
			{
				iarg+=2;
				if( iarg >= argc )
				{
					fprintf( stderr, "Debug set commands require 2 parameters, a register and a value.\n" );
					goto unimplemented;
				}

				uint32_t datareg = SimpleReadNumberInt( argv[iarg-1], DMDATA0 );
				uint32_t value = SimpleReadNumberInt( argv[iarg], 0 ); 

				if( MCF.WriteReg32 && MCF.FlushLLCommands )
				{
					MCF.FlushLLCommands( dev );	
					MCF.WriteReg32( dev, datareg, value );
					MCF.FlushLLCommands( dev );
				}
				else
					goto unimplemented;
				break;
			}
			case 'm':
			{
				iarg+=1;
				if( iarg >= argc )
				{
					fprintf( stderr, "Debug get commands require 1 parameter, a register.\n" );
					fprintf( stderr, "One of the following:\n"
						"	DMDATA0        0x04\n"
						"	DMDATA1        0x05\n"
						"	DMCONTROL      0x10\n"
						"	DMSTATUS       0x11\n"
						"	DMHARTINFO     0x12\n"
						"	DMABSTRACTCS   0x16\n"
						"	DMCOMMAND      0x17\n"
						"	DMABSTRACTAUTO 0x18\n"
						"	DMPROGBUF0     0x20\n"
						"	DMPROGBUF1     0x21\n"
						"	DMPROGBUF2     0x22\n"
						"	DMPROGBUF3     0x23\n"
						"	DMPROGBUF4     0x24\n"
						"	DMPROGBUF5     0x25\n"
						"	DMPROGBUF6     0x26\n"
						"	DMPROGBUF7     0x27\n"
						"	DMCPBR       0x7C\n"
						"	DMCFGR       0x7D\n"
						"	DMSHDWCFGR   0x7E\n" );

					goto unimplemented;
				}

				uint32_t datareg = SimpleReadNumberInt( argv[iarg], DMDATA0 );

				if( MCF.ReadReg32 && MCF.FlushLLCommands )
				{
					uint32_t value;
					int ret = MCF.ReadReg32( dev, datareg, &value );
					printf( "REGISTER %02x: %08x, %d\n", datareg, value, ret );
				}
				else
					goto unimplemented;
				break;
			}
			case 'i':
			{
				if( MCF.PrintChipInfo )
					MCF.PrintChipInfo( dev ); 
				else
					goto unimplemented;
				break;
			}
			case 'X':
			{
				iarg++;
				if( iarg >= argc )
				{
					fprintf( stderr, "Vendor command requires an actual command\n" );
					goto unimplemented;
				}
				if( MCF.VendorCommand )
					if( MCF.VendorCommand( dev, argv[iarg++] ) )
						goto unimplemented;
				break;
			}
			case 'r':
			{
				if( MCF.HaltMode ) MCF.HaltMode( dev, HALT_MODE_HALT_BUT_NO_RESET ); //No need to reboot.

				if( argchar[2] != 0 )
				{
					fprintf( stderr, "Error: can't have char after paramter field\n" ); 
					goto help;
				}
				iarg++;
				argchar = 0; // Stop advancing
				if( iarg + 2 >= argc )
				{
					fprintf( stderr, "Error: missing file for -o.\n" ); 
					goto help;
				}
				const char * fname = argv[iarg++];
				uint64_t offset = StringToMemoryAddress( argv[iarg++] );

				uint64_t amount = SimpleReadNumberInt( argv[iarg], -1 );
				if( offset > 0xffffffff || amount > 0xffffffff )
				{
					fprintf( stderr, "Error: memory value request out of range\n" );
					return -9;
				}

				FILE * f = 0;
				int hex = 0;
				if( strcmp( fname, "-" ) == 0 )
					f = stdout;
				else if( strcmp( fname, "+" ) == 0 )
					f = stdout, hex = 1;
				else
					f = fopen( fname, "wb" );
				if( !f )
				{
					fprintf( stderr, "Error: can't open write file \"%s\"\n", fname );
					return -9;
				}
				uint8_t * readbuff = malloc( amount );

				if( MCF.ReadBinaryBlob )
				{
					if( MCF.ReadBinaryBlob( dev, offset, amount, readbuff ) < 0 )
					{
						fprintf( stderr, "Fault reading device\n" );
						return -12;
					}
				}				
				else
				{
					goto unimplemented;
				}

				printf( "Read %d bytes\n", (int)amount );

				if( hex )
				{
					int i;
					for( i = 0; i < amount; i++ )
					{
						if( ( i & 0xf ) == 0 )
						{
							if( i != 0 ) printf( "\n" );
							printf( "%08x: ", (uint32_t)(offset + i) );
						}
						printf( "%02x ", readbuff[i] );
					}
					printf( "\n" );
				}
				else
					fwrite( readbuff, amount, 1, f );

				free( readbuff );

				if( f != stdout ) fclose( f );
				break;
			}
			case 'w':
			{
				struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
				if( argchar[2] != 0 ) goto help;
				iarg++;
				argchar = 0; // Stop advancing
				if( iarg + 1 >= argc ) goto help;

				// Write binary.
				int len = 0;
				uint8_t * image = 0;
				const char * fname = argv[iarg++];

				if( fname[0] == '-' )
				{
					len = strlen( fname + 1 );
					image = (uint8_t*)strdup( fname + 1 );
					status = 1;
				}
				else if( fname[0] == '+' )
				{
					int hl = strlen( fname+1 );
					if( hl & 1 )
					{
						fprintf( stderr, "Error: hex input doesn't align to chars correctly.\n" );
						return -32;
					}
					len = hl/2;
					image = malloc( len );
					int i;
					for( i = 0; i < len; i ++ )
					{
						char c1 = fname[i*2+1];
						char c2 = fname[i*2+2];
						int v1, v2;
						if( c1 >= '0' && c1 <= '9' ) v1 = c1 - '0';
						else if( c1 >= 'a' && c1 <= 'f' ) v1 = c1 - 'a' + 10;
						else if( c1 >= 'A' && c1 <= 'F' ) v1 = c1 - 'A' + 10;
						else
						{
							fprintf( stderr, "Error: Bad hex\n" );
							return -32;
						}

						if( c2 >= '0' && c2 <= '9' ) v2 = c2 - '0';
						else if( c2 >= 'a' && c2 <= 'f' ) v2 = c2 - 'a' + 10;
						else if( c2 >= 'A' && c2 <= 'F' ) v2 = c2 - 'A' + 10;
						else
						{
							fprintf( stderr, "Error: Bad hex\n" );
							return -32;
						}
						image[i] = (v1<<4) | v2;
					}
					status = 1;
				}
				else
				{
					FILE * f = fopen( fname, "rb" );
					if( !f )
					{
						fprintf( stderr, "Error: Could not open %s\n", fname );
						return -55;
					}
					fseek( f, 0, SEEK_END );
					len = ftell( f );
					fseek( f, 0, SEEK_SET );
					image = malloc( len );
					status = fread( image, len, 1, f );
					fclose( f );
				}

				uint64_t offset = StringToMemoryAddress( argv[iarg] );
				if( offset > 0xffffffff )
				{
					fprintf( stderr, "Error: Invalid offset (%s)\n", argv[iarg] );
					exit( -44 );
				}
				if( status != 1 )
				{
					fprintf( stderr, "Error: File I/O Fault.\n" );
					exit( -10 );
				}
				if( len > iss->flash_size )
				{
					fprintf( stderr, "Error: Image for CH32V003 too large (%d)\n", len );
					exit( -9 );
				}

				int is_flash = IsAddressFlash( offset );
				if( MCF.HaltMode ) MCF.HaltMode( dev, is_flash ? HALT_MODE_HALT_AND_RESET : HALT_MODE_HALT_BUT_NO_RESET );

				if( MCF.WriteBinaryBlob )
				{
					if( MCF.WriteBinaryBlob( dev, offset, len, image ) )
					{
						fprintf( stderr, "Error: Fault writing image.\n" );
						return -13;
					}
				}
				else
				{
					goto unimplemented;
				}

				printf( "Image written.\n" );

				free( image );
				break;
			}
			
		}
		if( argchar && argchar[2] != 0 ) { argchar++; goto keep_going; }
	}

	if( MCF.FlushLLCommands )
		MCF.FlushLLCommands( dev );

	if( MCF.Exit )
		MCF.Exit( dev );

	return 0;

help:
	fprintf( stderr, "Usage: minichlink [args]\n" );
	fprintf( stderr, " single-letter args may be combined, i.e. -3r\n" );
	fprintf( stderr, " multi-part args cannot.\n" );
	fprintf( stderr, " -3 Enable 3.3V\n" );
	fprintf( stderr, " -5 Enable 5V\n" );
	fprintf( stderr, " -t Disable 3.3V\n" );
	fprintf( stderr, " -f Disable 5V\n" );
	fprintf( stderr, " -c [serial port for Ardulink, try /dev/ttyACM0 or COM11 etc]\n" );
	fprintf( stderr, " -u Clear all code flash - by power off (also can unbrick)\n" );
	fprintf( stderr, " -E Erase chip\n" );
	fprintf( stderr, " -b Reboot out of Halt\n" );
	fprintf( stderr, " -e Resume from halt\n" );
	fprintf( stderr, " -a Reboot into Halt\n" );
	fprintf( stderr, " -A Go into Halt without reboot\n" );
	fprintf( stderr, " -D Configure NRST as GPIO\n" );
	fprintf( stderr, " -d Configure NRST as NRST\n" );
	fprintf( stderr, " -i Show chip info\n" );
	fprintf( stderr, " -s [debug register] [value]\n" );
	fprintf( stderr, " -m [debug register]\n" );
	fprintf( stderr, " -T Terminal Only\n" );
	fprintf( stderr, " -G Terminal + GDB\n" );
	fprintf( stderr, " -P Enable Read Protection\n" );
	fprintf( stderr, " -p Disable Read Protection\n" );
	fprintf( stderr, " -w [binary image to write] [address, decimal or 0x, try0x08000000]\n" );
	fprintf( stderr, " -r [output binary image] [memory address, decimal or 0x, try 0x08000000] [size, decimal or 0x, try 16384]\n" );
	fprintf( stderr, "   Note: for memory addresses, you can use 'flash' 'launcher' 'bootloader' 'option' 'ram' and say \"ram+0x10\" for instance\n" );
	fprintf( stderr, "   For filename, you can use - for raw (terminal) or + for hex (inline).\n" );
	fprintf( stderr, " -T is a terminal. This MUST be the last argument. Also, will start a gdbserver.\n" );

	return -1;	

unimplemented:
	fprintf( stderr, "Error: Command '%s' unimplemented on this programmer.\n", lastcommand );
	return -1;
}
#endif

#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#define strtoll _strtoi64
#endif

int64_t SimpleReadNumberInt( const char * number, int64_t defaultNumber )
{
	if( !number || !number[0] ) return defaultNumber;
	int radix = 10;
	if( number[0] == '0' )
	{
		char nc = number[1];
		number+=2;
		if( nc == 0 ) return 0;
		else if( nc == 'x' ) radix = 16;
		else if( nc == 'b' ) radix = 2;
		else { number--; radix = 8; }
	}
	char * endptr;
	uint64_t ret = strtoll( number, &endptr, radix );
	if( endptr == number )
	{
		return defaultNumber;
	}
	else
	{
		return ret;
	}
}

static int64_t StringToMemoryAddress( const char * number )
{
	uint32_t base = 0;

	if( strncmp( number, "flash", 5 ) == 0 )       base = 0x08000000, number += 5;
	if( strncmp( number, "launcher", 8 ) == 0 )    base = 0x1FFFF000, number += 8;
	if( strncmp( number, "bootloader", 10 ) == 0 ) base = 0x1FFFF000, number += 10;
	if( strncmp( number, "option", 6 ) == 0 )      base = 0x1FFFF800, number += 6;
	if( strncmp( number, "user", 4 ) == 0 )        base = 0x1FFFF800, number += 4;
	if( strncmp( number, "ram", 3 ) == 0 )         base = 0x20000000, number += 3;

	if( base )
	{
		if( *number != '+' )
			return base;
		number++;
		return base + SimpleReadNumberInt( number, 0 );
	}
	return SimpleReadNumberInt( number, -1 );
}

static int DefaultWaitForFlash( void * dev )
{
	uint32_t rw, timeout = 0;
	do
	{
		rw = 0;
		MCF.ReadWord( dev, (intptr_t)&FLASH->STATR, &rw ); // FLASH_STATR => 0x4002200C
		if( timeout++ > 100 ) return -1;
	} while(rw & 3);  // BSY flag for 003, or WRBSY for other processors.

	if( rw & FLASH_STATR_WRPRTERR )
	{
		fprintf( stderr, "Memory Protection Error\n" );
		return -44;
	}

	return 0;
}

static int DefaultWaitForDoneOp( void * dev, int ignore )
{
	int r;
	uint32_t rrv;

	do
	{
		r = MCF.ReadReg32( dev, DMABSTRACTCS, &rrv );
		if( r ) return r;
	}
	while( rrv & (1<<12) );

	if( (rrv >> 8 ) & 7 )
	{
		if( !ignore )
		{
			const char * errortext = 0;
			switch( (rrv>>8)&7 )
			{
			case 1: errortext = "Command in execution"; break;
			case 2: errortext = "Abstract Command Unsupported"; break;
			case 3: errortext = "Execption executing Abstract Command"; break;
			case 4: errortext = "Processor not halted."; break;
			case 5: errortext = "Bus Error"; break;
			case 6: errortext = "Parity Bit"; break;
			default: errortext = "Other Error"; break;
			}

			uint32_t temp;
			MCF.ReadReg32( dev, DMSTATUS, &temp );
			fprintf( stderr, "Fault writing memory (DMABSTRACTS = %08x) (%s) DMSTATUS: %08x\n", rrv, errortext, temp );
		}
		MCF.WriteReg32( dev, DMABSTRACTCS, 0x00000700 );
		return -9;
	}
	return 0;
}

int DefaultSetupInterface( void * dev )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);

	if( MCF.Control3v3 ) MCF.Control3v3( dev, 1 );
	MCF.DelayUS( dev, 16000 );
	MCF.WriteReg32( dev, DMSHDWCFGR, 0x5aa50000 | (1<<10) ); // Shadow Config Reg
	MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // CFGR (1<<10 == Allow output from slave)
	MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // Bug in silicon?  If coming out of cold boot, and we don't do our little "song and dance" this has to be called.

	// Read back chip status.  This is really basic.
	uint32_t reg = 0;
	int r = MCF.ReadReg32( dev, DMSTATUS, &reg );
	if( r >= 0 )
	{
		// Valid R.
		if( reg == 0x00000000 || reg == 0xffffffff )
		{
			fprintf( stderr, "Error: Setup chip failed. Got code %08x\n", reg );
			return -9;
		}
	}
	else
	{
		fprintf( stderr, "Error: Could not read chip code.\n" );
		return r;
	}

	iss->statetag = STTAG( "STRT" );
	return 0;
}

static void StaticUpdatePROGBUFRegs( void * dev )
{
	uint32_t rr;
	if( MCF.ReadReg32( dev, DMHARTINFO, &rr ) )
	{
		fprintf( stderr, "Error: Could not get hart info.\n" );
		return;
	}
	uint32_t data0offset = 0xe0000000 | ( rr & 0x7ff );
	MCF.WriteReg32( dev, DMDATA0, data0offset );       // DATA0's location in memory.
	MCF.WriteReg32( dev, DMCOMMAND, 0x0023100a );      // Copy data to x10
	MCF.WriteReg32( dev, DMDATA0, data0offset + 4 );   // DATA1's location in memory.
	MCF.WriteReg32( dev, DMCOMMAND, 0x0023100b );      // Copy data to x11
	MCF.WriteReg32( dev, DMDATA0, 0x40022010 );        // FLASH->CTLR
	MCF.WriteReg32( dev, DMCOMMAND, 0x0023100c );      // Copy data to x12
	MCF.WriteReg32( dev, DMDATA0, CR_PAGE_PG|CR_BUF_LOAD);
	MCF.WriteReg32( dev, DMCOMMAND, 0x0023100d );      // Copy data to x13
}

int InternalUnlockBootloader( void * dev )
{
	if( !MCF.WriteWord ) return -99;
	int ret = 0;
	uint32_t OBTKEYR;
	ret |= MCF.WriteWord( dev, 0x40022028, 0x45670123 ); //(FLASH_BOOT_MODEKEYP)
	ret |= MCF.WriteWord( dev, 0x40022028, 0xCDEF89AB ); //(FLASH_BOOT_MODEKEYP)
	ret |= MCF.ReadWord( dev, 0x40022008, &OBTKEYR ); //(FLASH_OBTKEYR)
	if( ret )
	{
		fprintf( stderr, "Error operating with OBTKEYR\n" );
		return -1;
	}
	if( OBTKEYR & (1<<15) )
	{
		fprintf( stderr, "Error: Could not unlock boot section (%08x)\n", OBTKEYR );
	}
	OBTKEYR |= (1<<14); // Configure for boot-to-bootload.
	ret |= MCF.WriteWord( dev, 0x40022008, OBTKEYR );
	ret |= MCF.ReadWord( dev, 0x40022008, &OBTKEYR ); //(FLASH_OBTKEYR)
	printf( "FLASH_OBTKEYR = %08x (%d)\n", OBTKEYR, ret );
	return ret;
}


int InternalIsMemoryErased( struct InternalState * iss, uint32_t address )
{
	if(( address & 0xff000000 ) != 0x08000000 ) return 0;
	int sector = (address & 0xffffff) / iss->sector_size;
	if( sector >= MAX_FLASH_SECTORS )
		return 0;
	else
		return iss->flash_sector_status[sector];
}

void InternalMarkMemoryNotErased( struct InternalState * iss, uint32_t address )
{
	if(( address & 0xff000000 ) != 0x08000000 ) return;
	int sector = (address & 0xffffff) / iss->sector_size;
	if( sector < MAX_FLASH_SECTORS )
		iss->flash_sector_status[sector] = 0;
}

static int DefaultWriteHalfWord( void * dev, uint32_t address_to_write, uint16_t data )
{
	int ret = 0;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	if( MCF.VoidHighLevelState ) MCF.VoidHighLevelState( dev );
	iss->statetag = STTAG( "XXXX" );

	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.

	// Different address, so we don't need to re-write all the program regs.
	// sh x8,0(x9)  // Write to the address.
	MCF.WriteReg32( dev, DMPROGBUF0, 0x00849023 );
	MCF.WriteReg32( dev, DMPROGBUF1, 0x00100073 ); // c.ebreak

	MCF.WriteReg32( dev, DMDATA0, address_to_write );
	MCF.WriteReg32( dev, DMCOMMAND, 0x00231009 ); // Copy data to x9
	MCF.WriteReg32( dev, DMDATA0, data );
	MCF.WriteReg32( dev, DMCOMMAND, 0x00271008 ); // Copy data to x8, and execute program.

	ret |= MCF.WaitForDoneOp( dev, 0 );
	iss->currentstateval = -1;

	if( ret ) fprintf( stderr, "Fault on DefaultWriteHalfWord\n" );
	return ret;
}

static int DefaultReadHalfWord( void * dev, uint32_t address_to_write, uint16_t * data )
{
	int ret = 0;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	if( MCF.VoidHighLevelState ) MCF.VoidHighLevelState( dev );
	iss->statetag = STTAG( "XXXX" );

	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.

	// Different address, so we don't need to re-write all the program regs.
	// lh x8,0(x9)  // Write to the address.
	MCF.WriteReg32( dev, DMPROGBUF0, 0x00049403 ); // lh x8, 0(x9)
	MCF.WriteReg32( dev, DMPROGBUF1, 0x00100073 ); // c.ebreak

	MCF.WriteReg32( dev, DMDATA0, address_to_write );
	MCF.WriteReg32( dev, DMCOMMAND, 0x00231009 ); // Copy data to x9
	MCF.WriteReg32( dev, DMCOMMAND, 0x00241000 ); // Only execute.
	MCF.WriteReg32( dev, DMCOMMAND, 0x00221008 ); // Read x8 into DATA0.

	ret |= MCF.WaitForDoneOp( dev, 0 );
	iss->currentstateval = -1;

	if( ret ) fprintf( stderr, "Fault on DefaultReadHalfWord\n" );

	uint32_t rr;
	ret |= MCF.ReadReg32( dev, DMDATA0, &rr );
	*data = rr;
	return ret;
}



static int DefaultWriteByte( void * dev, uint32_t address_to_write, uint8_t data )
{
	int ret = 0;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	if( MCF.VoidHighLevelState ) MCF.VoidHighLevelState( dev );
	iss->statetag = STTAG( "XXXX" );

	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.

	// Different address, so we don't need to re-write all the program regs.
	// sh x8,0(x9)  // Write to the address.
	MCF.WriteReg32( dev, DMPROGBUF0, 0x00848023 ); // sb x8, 0(x9)
	MCF.WriteReg32( dev, DMPROGBUF1, 0x00100073 ); // c.ebreak

	MCF.WriteReg32( dev, DMDATA0, address_to_write );
	MCF.WriteReg32( dev, DMCOMMAND, 0x00231009 ); // Copy data to x9
	MCF.WriteReg32( dev, DMDATA0, data );
	MCF.WriteReg32( dev, DMCOMMAND, 0x00271008 ); // Copy data to x8, and execute program.

	ret |= MCF.WaitForDoneOp( dev, 0 );
	if( ret ) fprintf( stderr, "Fault on DefaultWriteByte\n" );
	iss->currentstateval = -1;
	return ret;
}

static int DefaultReadByte( void * dev, uint32_t address_to_write, uint8_t * data )
{
	int ret = 0;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	if( MCF.VoidHighLevelState ) MCF.VoidHighLevelState( dev );
	iss->statetag = STTAG( "XXXX" );

	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.

	// Different address, so we don't need to re-write all the program regs.
	// lb x8,0(x9)  // Write to the address.
	MCF.WriteReg32( dev, DMPROGBUF0, 0x00048403 ); // lb x8, 0(x9)
	MCF.WriteReg32( dev, DMPROGBUF1, 0x00100073 ); // c.ebreak

	MCF.WriteReg32( dev, DMDATA0, address_to_write );
	MCF.WriteReg32( dev, DMCOMMAND, 0x00231009 ); // Copy data to x9
	MCF.WriteReg32( dev, DMCOMMAND, 0x00241000 ); // Only execute.
	MCF.WriteReg32( dev, DMCOMMAND, 0x00221008 ); // Read x8 into DATA0.

	ret |= MCF.WaitForDoneOp( dev, 0 );
	if( ret ) fprintf( stderr, "Fault on DefaultReadByte\n" );
	iss->currentstateval = -1;

	uint32_t rr;
	ret |= MCF.ReadReg32( dev, DMDATA0, &rr );
	*data = rr;
	return ret;
}


static int DefaultWriteWord( void * dev, uint32_t address_to_write, uint32_t data )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	int ret = 0;

	int is_flash = IsAddressFlash( address_to_write );

	if( iss->statetag != STTAG( "WRSQ" ) || is_flash != iss->lastwriteflags )
	{
		int did_disable_req = 0;
		if( iss->statetag != STTAG( "WRSQ" ) )
		{
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
			did_disable_req = 1;
			// Different address, so we don't need to re-write all the program regs.
			// c.lw x9,0(x11) // Get the address to write to. 
			// c.sw x8,0(x9)  // Write to the address.
			MCF.WriteReg32( dev, DMPROGBUF0, 0xc0804184 );
			// c.addi x9, 4
			// c.sw x9,0(x11)
			MCF.WriteReg32( dev, DMPROGBUF1, 0xc1840491 );

			if( iss->statetag != STTAG( "RDSQ" ) )
			{
				StaticUpdatePROGBUFRegs( dev );
			}
		}

		if( iss->lastwriteflags != is_flash || iss->statetag != STTAG( "WRSQ" ) )
		{
			// If we are doing flash, we have to ack, otherwise we don't want to ack.
			if( is_flash )
			{
				// After writing to memory, also hit up page load flag.
				// c.sw x13,0(x12) // Acknowledge the page write.
				// c.ebreak
				MCF.WriteReg32( dev, DMPROGBUF2, 0x9002c214 );
			}
			else
			{
				MCF.WriteReg32( dev, DMPROGBUF2, 0x00019002 ); // c.ebreak
			}
		}

		MCF.WriteReg32( dev, DMDATA1, address_to_write );
		MCF.WriteReg32( dev, DMDATA0, data );

		if( did_disable_req )
		{
			MCF.WriteReg32( dev, DMCOMMAND, 0x00271008 ); // Copy data to x8, and execute program.
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 1 ); // Enable Autoexec.
		}
		iss->lastwriteflags = is_flash;

		iss->statetag = STTAG( "WRSQ" );
		iss->currentstateval = address_to_write;

		if( is_flash )
		{
			ret |= MCF.WaitForDoneOp( dev, 0 );
			if( ret ) fprintf( stderr, "Fault on DefaultWriteWord Part 1\n" );
		}
	}
	else
	{
		if( address_to_write != iss->currentstateval )
		{
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 0 ); // Disable Autoexec.
			MCF.WriteReg32( dev, DMDATA1, address_to_write );
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 1 ); // Enable Autoexec.
		}
		MCF.WriteReg32( dev, DMDATA0, data );
		if( is_flash )
		{
			// XXX TODO: This likely can be a very short delay.
			// XXX POSSIBLE OPTIMIZATION REINVESTIGATE.
			ret |= MCF.WaitForDoneOp( dev, 0 );
			if( ret ) fprintf( stderr, "Fault on DefaultWriteWord Part 2\n" );
		}
		else
		{
			ret |= MCF.WaitForDoneOp( dev, 0 );
			if( ret ) fprintf( stderr, "Fault on DefaultWriteWord Part 3\n" );
		}
	}


	iss->currentstateval += 4;

	return ret;
}

int DefaultWriteBinaryBlob( void * dev, uint32_t address_to_write, uint32_t blob_size, uint8_t * blob )
{
	// NOTE IF YOU FIX SOMETHING IN THIS FUNCTION PLEASE ALSO UPDATE THE PROGRAMMERS.
	//  this is only fallback functionality for really realy basic programmers.

	uint32_t rw;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	int sectorsize = iss->sector_size;

	// We can't write into flash when mapped to 0x00000000
	if( address_to_write < 0x01000000 )
		address_to_write |= 0x08000000;

	int is_flash = IsAddressFlash( address_to_write );

	if( blob_size == 0 ) return 0;

	if( is_flash && !iss->flash_unlocked )
	{
		if( ( rw = InternalUnlockFlash( dev, iss ) ) )
			return rw;
	}

	// Special: For user data, need to write to it very carefully.
	if( address_to_write > 0x1ffff7c0 && address_to_write < 0x20000000 )
	{
		if( !MCF.WriteHalfWord )
		{
			fprintf( stderr, "Error: to write this type of memory, half-word-writing is required\n" );
			return -5;
		}

		uint32_t base = address_to_write & 0xffffffc0;
		uint8_t block[64];

		if( base != ((address_to_write+blob_size-1) & 0xffffffc0) )
		{
			fprintf( stderr, "Error: You cannot write across a 64-byte boundary when writing to option bytes\n" );
			return -9;
		}

		MCF.ReadBinaryBlob( dev, base, 64, block );

		uint32_t offset = address_to_write - base;
		memcpy( block + offset, blob, blob_size );

		uint32_t temp;
		MCF.ReadWord( dev, 0x4002200c, &temp );
		if( temp & 0x8000 )
		{
			MCF.WriteWord( dev, 0x40022004, 0x45670123 ); // KEYR
			MCF.WriteWord( dev, 0x40022004, 0xCDEF89AB );
			MCF.WriteWord( dev, 0x40022008, 0x45670123 ); // OBWRE
			MCF.WriteWord( dev, 0x40022008, 0xCDEF89AB );
			MCF.WriteWord( dev, 0x40022028, 0x45670123 ); //(FLASH_BOOT_MODEKEYP)
			MCF.WriteWord( dev, 0x40022028, 0xCDEF89AB ); //(FLASH_BOOT_MODEKEYP)
			MCF.ReadWord( dev, 0x40022010, &temp );
			MCF.ReadWord( dev, 0x4002200c, &temp );
		}

		MCF.ReadWord( dev, 0x4002200c, &temp );
		if( temp & 0x8000 )
		{
			fprintf( stderr, "Error: Critical memory zone is still locked out\n" );
			return -10;
		}

		if( MCF.WaitForFlash ) MCF.WaitForFlash( dev );

		MCF.ReadWord( dev, 0x40022010, &temp );
		if( !(temp & (1<<9)) ) // Check OBWRE
		{
			fprintf( stderr, "Error: Option Byte Unlock Failed\n" );
			return -10;
		}

		// Perform erase.
		MCF.WriteWord( dev, 0x40022010, FLASH_CTLR_OPTER | FLASH_CTLR_OPTWRE );
		MCF.WriteWord( dev, 0x40022010, FLASH_CTLR_OPTER | FLASH_CTLR_OPTWRE | FLASH_CTLR_STRT );

		if( MCF.WaitForFlash ) MCF.WaitForFlash( dev );

		MCF.ReadWord( dev, 0x4002200c, &temp );
		if( temp & 0x10 )
		{
			fprintf( stderr, "WRPTRERR is set.  Write failed\n" );
			return -9;
		}

		int i;
		for( i = 0; i < 8; i++ )
		{
			MCF.WriteWord( dev, 0x40022010, FLASH_CTLR_OPTPG | FLASH_CTLR_OPTWRE );
			MCF.WriteWord( dev, 0x40022010, FLASH_CTLR_OPTPG | FLASH_CTLR_STRT | FLASH_CTLR_OPTWRE );
			MCF.WriteHalfWord( dev, i*2+base, block[i*2+0] | (block[i*2+1]<<8) );

			if( MCF.WaitForFlash ) MCF.WaitForFlash( dev );
			MCF.ReadWord( dev, 0x4002200c, &temp );
			if( temp & 0x10 )
			{
				fprintf( stderr, "WRPTRERR is set.  Write failed\n" );
				return -9;
			}
		}
		if( MCF.WaitForFlash ) MCF.WaitForFlash( dev );
		MCF.WriteWord( dev, 0x40022010, 0 );

		return 0;
	}

	// Regardless of sector size, allow block write to do its thing if it can.
	if( is_flash && MCF.BlockWrite64 && ( address_to_write & 0x3f ) == 0 && ( blob_size & 0x3f ) == 0 )
	{
		int i;
		for( i = 0; i < blob_size; i+= 64 )
		{
			int r = MCF.BlockWrite64( dev, address_to_write + i, blob + i );
			if( r )
			{
				fprintf( stderr, "Error writing block at memory %08x / Error: %d\n", address_to_write, r );
				return r;
			}
		}
		return 0;
	}

	uint8_t tempblock[sectorsize];
	int sblock =  address_to_write / sectorsize;
	int eblock = ( address_to_write + blob_size + (sectorsize-1) ) / sectorsize;
	int b;
	int rsofar = 0;

	for( b = sblock; b < eblock; b++ )
	{
		int offset_in_block = address_to_write - (b * sectorsize);
		if( offset_in_block < 0 ) offset_in_block = 0;
		int end_o_plus_one_in_block = ( address_to_write + blob_size ) - (b*sectorsize);
		if( end_o_plus_one_in_block > sectorsize ) end_o_plus_one_in_block = sectorsize;
		int	base = b * sectorsize;

		if( offset_in_block == 0 && end_o_plus_one_in_block == sectorsize )
		{
			if( MCF.BlockWrite64 )
			{
				int i;
				for( i = 0; i < sectorsize/64; i++ )
				{
					int r = MCF.BlockWrite64( dev, base + i*64, blob + rsofar+i*64 );
					rsofar += 64;
					if( r )
					{
						fprintf( stderr, "Error writing block at memory %08x (error = %d)\n", base, r );
						return r;
					}
				}
			}
			else 					// Block Write not avaialble
			{
				if( is_flash )
				{
					if( !InternalIsMemoryErased( iss, base ) )
						MCF.Erase( dev, base, sectorsize, 0 );
					MCF.WriteWord( dev, 0x40022010, CR_PAGE_PG ); // THIS IS REQUIRED, (intptr_t)&FLASH->CTLR = 0x40022010
					MCF.WriteWord( dev, 0x40022010, CR_BUF_RST | CR_PAGE_PG );  // (intptr_t)&FLASH->CTLR = 0x40022010
				}

				int j;
				for( j = 0; j < sectorsize/4; j++ )
				{
					uint32_t writeword;
					memcpy( &writeword, blob + rsofar, 4 );
					MCF.WriteWord( dev, j*4+base, writeword );
					rsofar += 4;
				}

				if( is_flash )
				{
					MCF.WriteWord( dev, 0x40022014, base );  //0x40022014 -> FLASH->ADDR
					if( MCF.PrepForLongOp ) MCF.PrepForLongOp( dev );  // Give the programmer a headsup this next operation could take a while.
					MCF.WriteWord( dev, 0x40022010, CR_PAGE_PG|CR_STRT_Set ); // 0x40022010 -> FLASH->CTLR
					if( MCF.WaitForFlash ) MCF.WaitForFlash( dev );
					InternalMarkMemoryNotErased( iss, base );
				}
			}
		}
		else
		{
			//Ok, we have to do something wacky.
			if( is_flash )
			{
				MCF.ReadBinaryBlob( dev, base, sectorsize, tempblock );

				// Permute tempblock
				int tocopy = end_o_plus_one_in_block - offset_in_block;
				memcpy( tempblock + offset_in_block, blob + rsofar, tocopy );
				rsofar += tocopy;

				if( MCF.BlockWrite64 ) 
				{
					int i;
					for( i = 0; i < sectorsize/64; i++ )
					{
						int r = MCF.BlockWrite64( dev, base+i*64, tempblock+i*64 );
						if( r ) return r;
					}
				}
				else
				{
					if( !InternalIsMemoryErased( iss, base ) )
						MCF.Erase( dev, base, sectorsize, 0 );
					MCF.WriteWord( dev, 0x40022010, CR_PAGE_PG ); // THIS IS REQUIRED, (intptr_t)&FLASH->CTLR = 0x40022010
					MCF.WriteWord( dev, 0x40022010, CR_BUF_RST | CR_PAGE_PG );  // (intptr_t)&FLASH->CTLR = 0x40022010

					int j;
					for( j = 0; j < sectorsize/4; j++ )
					{
						MCF.WriteWord( dev, j*4+base, *(uint32_t*)(tempblock + j * 4) );
						rsofar += 4;
					}
					MCF.WriteWord( dev, 0x40022014, base );  //0x40022014 -> FLASH->ADDR
					MCF.WriteWord( dev, 0x40022010, CR_PAGE_PG|CR_STRT_Set ); // 0x40022010 -> FLASH->CTLR
					InternalMarkMemoryNotErased( iss, base );
				}
				if( MCF.WaitForFlash && MCF.WaitForFlash( dev ) ) goto timedout;
			}
			else
			{
				// Accessing RAM.  Be careful to only do the needed operations.
				int j;
				for( j = 0; j < sectorsize; j++ )
				{
					uint32_t taddy = j*4;
					if( offset_in_block <= taddy && end_o_plus_one_in_block >= taddy + 4 )
					{
						MCF.WriteWord( dev, taddy + base, *(uint32_t*)(blob + rsofar) );
						rsofar += 4;
					}
					else if( ( offset_in_block & 1 ) || ( end_o_plus_one_in_block & 1 ) )
					{
						// Bytes only.
						int j;
						for( j = 0; j < 4; j++ )
						{
							if( taddy >= offset_in_block && taddy < end_o_plus_one_in_block )
							{
								MCF.WriteByte( dev, taddy + base, *(uint32_t*)(blob + rsofar) );
								rsofar ++;
							}
							taddy++;
						}
					}
					else
					{
						// Half-words
						int j;
						for( j = 0; j < 4; j+=2 )
						{
							if( taddy >= offset_in_block && taddy < end_o_plus_one_in_block )
							{
								MCF.WriteHalfWord( dev, taddy + base, *(uint32_t*)(blob + rsofar) );
								rsofar +=2;
							}
							taddy+=2;
						}
					}
				}
			}
		}
	}

	MCF.FlushLLCommands( dev );

#if 0
	{
		uint8_t scratch[blob_size];
		int rrr = DefaultReadBinaryBlob( dev, address_to_write, blob_size, scratch );
		int i;
		printf( "Read op: %d\n", rrr );
		for( i = 0; i < blob_size; i++ )
		{
			if( scratch[i] != blob[i] )
			{
				printf( "DISAGREE: %04x\n", i );
				i = (i & ~0x3f) + 0x40-1;
			}
		}
	}
#endif

	MCF.DelayUS( dev, 100 ); // Why do we need this? (We seem to need this on the WCH programmers?)
	return 0;
timedout:
	fprintf( stderr, "Timed out\n" );
	return -5;
}

static int DefaultReadWord( void * dev, uint32_t address_to_read, uint32_t * data )
{
	int r = 0;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);

	int autoincrement = 1;
	if( address_to_read == 0x40022010 || address_to_read == 0x4002200C )  // Don't autoincrement when checking flash flag. 
		autoincrement = 0;

	if( iss->statetag != STTAG( "RDSQ" ) || address_to_read != iss->currentstateval || autoincrement != iss->autoincrement)
	{
		if( iss->statetag != STTAG( "RDSQ" ) )
		{
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 0 ); // Disable Autoexec.

			// c.lw x8,0(x11) // Pull the address from DATA1
			// c.lw x9,0(x8)  // Read the data at that location.
			MCF.WriteReg32( dev, DMPROGBUF0, 0x40044180 );
			if( autoincrement )
			{
				// c.addi x8, 4
				// c.sw x9, 0(x10) // Write back to DATA0

				MCF.WriteReg32( dev, DMPROGBUF1, 0xc1040411 );
			}
			else
			{
				// c.nop
				// c.sw x9, 0(x10) // Write back to DATA0

				MCF.WriteReg32( dev, DMPROGBUF1, 0xc1040001 );
			}
			// c.sw x8, 0(x11) // Write addy to DATA1
			// c.ebreak
			MCF.WriteReg32( dev, DMPROGBUF2, 0x9002c180 );

			if( iss->statetag != STTAG( "WRSQ" ) )
			{
				StaticUpdatePROGBUFRegs( dev );
			}
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 1 ); // Enable Autoexec (different kind of autoinc than outer autoinc)
			iss->autoincrement = autoincrement;
		}

		MCF.WriteReg32( dev, DMDATA1, address_to_read );
		MCF.WriteReg32( dev, DMCOMMAND, 0x00241000 ); 

		iss->statetag = STTAG( "RDSQ" );
		iss->currentstateval = address_to_read;

		r |= MCF.WaitForDoneOp( dev, 0 );
		if( r ) fprintf( stderr, "Fault on DefaultReadWord Part 1\n" );
	}

	if( iss->autoincrement )
		iss->currentstateval += 4;

	r |= MCF.ReadReg32( dev, DMDATA0, data );

	if( iss->currentstateval == iss->ram_base + iss->ram_size )
		MCF.WaitForDoneOp( dev, 1 ); // Ignore any post-errors. 
	return r;
}

int InternalUnlockFlash( void * dev, struct InternalState * iss )
{
	int ret = 0;
	uint32_t rw;
	ret = MCF.ReadWord( dev, 0x40022010, &rw );  // FLASH->CTLR = 0x40022010
	if( rw & 0x8080 ) 
	{
		ret = MCF.WriteWord( dev, 0x40022004, 0x45670123 ); // FLASH->KEYR = 0x40022004
		if( ret ) goto reterr;
		ret = MCF.WriteWord( dev, 0x40022004, 0xCDEF89AB );
		if( ret ) goto reterr;
		ret = MCF.WriteWord( dev, 0x40022008, 0x45670123 ); // OBKEYR = 0x40022008  // For user word unlocking
		if( ret ) goto reterr;
		ret = MCF.WriteWord( dev, 0x40022008, 0xCDEF89AB );
		if( ret ) goto reterr;
		ret = MCF.WriteWord( dev, 0x40022024, 0x45670123 ); // MODEKEYR = 0x40022024
		if( ret ) goto reterr;
		ret = MCF.WriteWord( dev, 0x40022024, 0xCDEF89AB );
		if( ret ) goto reterr;

		ret = MCF.ReadWord( dev, 0x40022010, &rw ); // FLASH->CTLR = 0x40022010
		if( ret ) goto reterr;

		if( rw & 0x8080 ) 
		{
			fprintf( stderr, "Error: Flash is not unlocked (CTLR = %08x)\n", rw );
			return -9;
		}
	}
	iss->flash_unlocked = 1;
	return 0;
reterr:
	fprintf( stderr, "Error unlocking flash, got code %d from underlying system\n", ret );
	return ret;
}

int DefaultErase( void * dev, uint32_t address, uint32_t length, int type )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	uint32_t rw;

	if( !iss->flash_unlocked )
	{
		if( ( rw = InternalUnlockFlash( dev, iss ) ) )
			return rw;
	}

	if( type == 1 )
	{
		// Whole-chip flash
		iss->statetag = STTAG( "XXXX" );
		printf( "Whole-chip erase\n" );
		if( MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, 0 ) ) goto flashoperr;
		if( MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, FLASH_CTLR_MER  ) ) goto flashoperr;
		if( MCF.PrepForLongOp ) MCF.PrepForLongOp( dev );  // Give the programmer a headsup this next operation could take a while.
		if( MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_STRT_Set|FLASH_CTLR_MER ) ) goto flashoperr;
		rw = MCF.WaitForDoneOp( dev, 0 );
		if( MCF.WaitForFlash && MCF.WaitForFlash( dev ) ) { fprintf( stderr, "Error: Wait for flash error.\n" ); return -11; }
		MCF.VoidHighLevelState( dev );
		memset( iss->flash_sector_status, 1, sizeof( iss->flash_sector_status ) );
	}
	else
	{
		// 16.4.7, Step 3: Check the BSY bit of the FLASH_STATR register to confirm that there are no other programming operations in progress.
		// skip (we make sure at the end)
		int chunk_to_erase = address;
		while( chunk_to_erase < address + length )
		{
			if( ( chunk_to_erase & 0xff000000 ) == 0x08000000 )
			{
				int sector = ( chunk_to_erase & 0x00ffffff ) / iss->sector_size;
				if( sector < MAX_FLASH_SECTORS )
				{
					iss->flash_sector_status[sector] = 1;
				}
			}

			// Step 4:  set PAGE_ER of FLASH_CTLR(0x40022010)
			if( MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_PAGE_ER ) ) goto flashoperr; // Actually FTER
			// Step 5: Write the first address of the fast erase page to the FLASH_ADDR register.
			if( MCF.WriteWord( dev, (intptr_t)&FLASH->ADDR, chunk_to_erase ) ) goto flashoperr;
			// Step 6: Set the STAT bit of FLASH_CTLR register to '1' to initiate a fast page erase (64 bytes) action.
			if( MCF.PrepForLongOp ) MCF.PrepForLongOp( dev );  // Give the programmer a headsup this next operation could take a while.
			if( MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_STRT_Set | CR_PAGE_ER ) ) goto flashoperr;
			if( MCF.WaitForFlash && MCF.WaitForFlash( dev ) ) return -99;
			chunk_to_erase+=iss->sector_size;
		}
	}

	return 0;
flashoperr:
	fprintf( stderr, "Error: Flash operation error\n" );
	return -93;
}

int DefaultReadBinaryBlob( void * dev, uint32_t address_to_read_from, uint32_t read_size, uint8_t * blob )
{
	uint32_t rpos = address_to_read_from;
	uint32_t rend = address_to_read_from + read_size;

	while( rpos < rend )
	{
		int r;
		int remain = rend - rpos;

		if( ( rpos & 3 ) == 0 && remain >= 4 )
		{
			uint32_t rw;
			r = MCF.ReadWord( dev, rpos, &rw );
			//printf( "RW: %d %08x %08x\n", r, rpos, rw );
			if( r ) return r;
			int rem = remain;
			if( rem > 4 ) rem = 4;
			memcpy( blob, &rw, rem);
			blob += 4;
			rpos += 4;
		}
		else
		{
			if( ( rpos & 1 ) )
			{
				uint8_t rw;
				r = MCF.ReadByte( dev, rpos, &rw );
				if( r ) return r;
				memcpy( blob, &rw, 1 );
				blob += 1;
				rpos += 1;
				remain -= 1;
			}
			if( ( rpos & 2 ) && remain >= 2 )
			{
				uint16_t rw;
				r = MCF.ReadHalfWord( dev, rpos, &rw );
				if( r ) return r;
				memcpy( blob, &rw, 2 );
				blob += 2;
				rpos += 2;
				remain -= 2;
			}
			if( remain >= 1 )
			{
				uint8_t rw;
				r = MCF.ReadByte( dev, rpos, &rw );
				if( r ) return r;
				memcpy( blob, &rw, 1 );
				blob += 1;
				rpos += 1;
				remain -= 1;
			}
		}
	}
	int r = MCF.WaitForDoneOp( dev, 0 );
	if( r ) fprintf( stderr, "Fault on DefaultReadBinaryBlob\n" );
	return r;
}

int DefaultReadCPURegister( void * dev, uint32_t regno, uint32_t * regret )
{
	if( !MCF.WriteReg32 || !MCF.ReadReg32 )
	{
		fprintf( stderr, "Error: Can't read CPU register on this programmer because it is missing read/writereg32\n" );
		return -5;
	}

	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
	iss->statetag = STTAG( "REGR" );

	MCF.WriteReg32( dev, DMCOMMAND, 0x00220000 | regno ); // Read xN into DATA0.
	int r = MCF.ReadReg32( dev, DMDATA0, regret );

	return r;
}

int DefaultReadAllCPURegisters( void * dev, uint32_t * regret )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000001 ); // Disable Autoexec.
	iss->statetag = STTAG( "RER2" );
	int i;
	for( i = 0; i < 16; i++ )
	{
		MCF.WriteReg32( dev, DMCOMMAND, 0x00220000 | 0x1000 | i ); // Read xN into DATA0.
		if( MCF.ReadReg32( dev, DMDATA0, regret + i ) )
		{
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
			return -5;
		}
	}
	MCF.WriteReg32( dev, DMCOMMAND, 0x00220000 | 0x7b1 ); // Read xN into DATA0.
	int r = MCF.ReadReg32( dev, DMDATA0, regret + i );
	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
	return r;
}

int DefaultWriteAllCPURegisters( void * dev, uint32_t * regret )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000001 ); // Disable Autoexec.
	iss->statetag = STTAG( "WER2" );
	int i;
	for( i = 0; i < 16; i++ )
	{
		MCF.WriteReg32( dev, DMCOMMAND, 0x00230000 | 0x1000 | i ); // Read xN into DATA0.
		if( MCF.WriteReg32( dev, DMDATA0, regret[i] ) )
		{
			MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
			return -5;
		}
	}
	MCF.WriteReg32( dev, DMCOMMAND, 0x00230000 | 0x7b1 ); // Read xN into DATA0.
	int r = MCF.WriteReg32( dev, DMDATA0, regret[i] );
	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
	return r;
}


int DefaultWriteCPURegister( void * dev, uint32_t regno, uint32_t value )
{
	if( !MCF.WriteReg32 || !MCF.ReadReg32 )
	{
		fprintf( stderr, "Error: Can't read CPU register on this programmer because it is missing read/writereg32\n" );
		return -5;
	}

	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
	iss->statetag = STTAG( "REGW" );
	MCF.WriteReg32( dev, DMDATA0, value );
	return MCF.WriteReg32( dev, DMCOMMAND, 0x00230000 | regno ); // Write xN from DATA0.
}

int DefaultSetEnableBreakpoints( void * dev, int is_enabled, int single_step )
{
	if( !MCF.ReadCPURegister || !MCF.WriteCPURegister )
	{
		fprintf( stderr, "Error: Can't set breakpoints on this programmer because it is missing read/writereg32\n" );
		return -5;
	}
	uint32_t DCSR;
	if( MCF.ReadCPURegister( dev, 0x7b0, &DCSR ) )
		fprintf( stderr, "Error: DCSR could not be read\n" );
	DCSR |= 0xb600;
	if( single_step )
		DCSR |= 4;
	else
		DCSR &=~4;

	//printf( "Setting DCSR: %08x\n", DCSR );
	if( MCF.WriteCPURegister( dev, 0x7b0, DCSR ) )
		fprintf( stderr, "Error: DCSR could not be read\n" );

	return 0;
}


static int DefaultHaltMode( void * dev, int mode )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	switch ( mode )
	{
	case HALT_MODE_HALT_BUT_NO_RESET: // Don't reboot.
	case HALT_MODE_HALT_AND_RESET:
		MCF.WriteReg32( dev, DMSHDWCFGR, 0x5aa50000 | (1<<10) ); // Shadow Config Reg
		MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // CFGR (1<<10 == Allow output from slave)
		MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // Bug in silicon?  If coming out of cold boot, and we don't do our little "song and dance" this has to be called.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
		if( mode == 0 ) MCF.WriteReg32( dev, DMCONTROL, 0x80000003 ); // Reboot.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Re-initiate a halt request.
//		MCF.WriteReg32( dev, DMCONTROL, 0x00000001 ); // Clear Halt Request.  This is recommended, but not doing it seems more stable.
		// Sometimes, even if the processor is halted but the MSB is clear, it will spuriously start?
		MCF.FlushLLCommands( dev );
		break;
	case HALT_MODE_REBOOT:
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000003 ); // Reboot.
		MCF.WriteReg32( dev, DMCONTROL, 0x40000001 ); // resumereq
		MCF.FlushLLCommands( dev );
		break;
	case HALT_MODE_RESUME:
		MCF.WriteReg32( dev, DMSHDWCFGR, 0x5aa50000 | (1<<10) ); // Shadow Config Reg
		MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // CFGR (1<<10 == Allow output from slave)
		MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // Bug in silicon?  If coming out of cold boot, and we don't do our little "song and dance" this has to be called.

		MCF.WriteReg32( dev, DMCONTROL, 0x40000001 ); // resumereq
		MCF.FlushLLCommands( dev );
		break;
	case HALT_MODE_GO_TO_BOOTLOADER:
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.

		MCF.WriteWord( dev, (intptr_t)&FLASH->KEYR, FLASH_KEY1 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->KEYR, FLASH_KEY2 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->BOOT_MODEKEYR, FLASH_KEY1 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->BOOT_MODEKEYR, FLASH_KEY2 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->STATR, 1<<14 );
		MCF.WriteWord( dev, (intptr_t)&FLASH->CTLR, CR_LOCK_Set );

		MCF.WriteReg32( dev, DMCONTROL, 0x80000003 ); // Reboot.
		MCF.WriteReg32( dev, DMCONTROL, 0x40000001 ); // resumereq
		MCF.FlushLLCommands( dev );
		break;
	default:
		fprintf( stderr, "Error: Unknown halt mode %d\n", mode );
	}

	iss->flash_unlocked = 0;
	iss->processor_in_mode = mode;

	// In case processor halt process needs to complete, i.e. if it was in the middle of a flash op.
	MCF.DelayUS( dev, 3000 );

	return 0;
}

// Returns positive if received text, or request for input.
// Returns -1 if nothing was printed but received data.
// Returns negative if error.
// Returns 0 if no text waiting.
// maxlen MUST be at least 8 characters.  We null terminate.
int DefaultPollTerminal( void * dev, uint8_t * buffer, int maxlen, uint32_t leaveflagA, int leaveflagB )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	int r;
	uint32_t rr;
	if( iss->statetag != STTAG( "TERM" ) )
	{
		MCF.WriteReg32( dev, DMABSTRACTAUTO, 0x00000000 ); // Disable Autoexec.
		iss->statetag = STTAG( "TERM" );
	}
	r = MCF.ReadReg32( dev, DMDATA0, &rr );
	if( r < 0 ) return r;
	if( maxlen < 8 ) return -9;

	// DMDATA1:
	//  bit  7 = host-acknowledge.
	if( rr & 0x80 )
	{
		int num_printf_chars = (rr & 0xf)-4;
		if( num_printf_chars > 0 && num_printf_chars <= 7)
		{
			if( num_printf_chars > 3 )
			{
				uint32_t r2;
				r = MCF.ReadReg32( dev, DMDATA1, &r2 );
				memcpy( buffer+3, &r2, num_printf_chars - 3 );
			}
			int firstrem = num_printf_chars;
			if( firstrem > 3 ) firstrem = 3;
			memcpy( buffer, ((uint8_t*)&rr)+1, firstrem );
			buffer[num_printf_chars] = 0;
		}
		if( leaveflagA ) MCF.WriteReg32( dev, DMDATA1, leaveflagB );
		MCF.WriteReg32( dev, DMDATA0, leaveflagA ); // Write that we acknowledge the data.
		if( num_printf_chars == 0 ) return -1;      // was acked?
		if( num_printf_chars < 0 ) num_printf_chars = 0;
		return num_printf_chars;
	}
	else
	{
		return 0;
	}
}

int DefaultUnbrick( void * dev )
{
	printf( "Entering Unbrick Mode\n" );
	MCF.Control3v3( dev, 0 );

	MCF.DelayUS( dev, 60000 );
	MCF.DelayUS( dev, 60000 );
	MCF.DelayUS( dev, 60000 );
	MCF.DelayUS( dev, 60000 );
	MCF.Control3v3( dev, 1 );
	MCF.DelayUS( dev, 100 );
	MCF.FlushLLCommands( dev );
	printf( "Connection starting\n" );
	int timeout = 0;
	int max_timeout = 500;
	uint32_t ds = 0;
	for( timeout = 0; timeout < max_timeout; timeout++ )
	{
		MCF.DelayUS( dev, 10 );
		MCF.WriteReg32( dev, DMSHDWCFGR, 0x5aa50000 | (1<<10) ); // Shadow Config Reg
		MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // CFGR (1<<10 == Allow output from slave)
		MCF.WriteReg32( dev, DMCFGR, 0x5aa50000 | (1<<10) ); // Bug in silicon?  If coming out of cold boot, and we don't do our little "song and dance" this has to be called.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // No, really make sure.
		MCF.WriteReg32( dev, DMCONTROL, 0x80000001 );
		MCF.FlushLLCommands( dev );
		int r = MCF.ReadReg32( dev, DMSTATUS, &ds );
		if( r )
		{
			fprintf( stderr, "Error: Could not read DMSTATUS from programmers (%d)\n", r );
			return -99;
		}
		MCF.FlushLLCommands( dev );
		if( ds != 0xffffffff && ds != 0x00000000 ) break;
	}

	// Make sure we are in halt.
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // No, really make sure.
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 );

	int r = MCF.ReadReg32( dev, DMSTATUS, &ds );
	printf( "DMStatus After Halt: /%d/%08x\n", r, ds );

//  Many times we would clear the halt request, but in this case, we want to just leave it here, to prevent it from booting.
//  TODO: Experiment and see if this is needed/wanted in cases.  NOTE: If you don't clear halt request, progarmmers can get stuck.
//	MCF.WriteReg32( dev, DMCONTROL, 0x00000001 ); // Clear Halt Request.

	// After more experimentation, it appaers to work best by not clearing the halt request.
	MCF.FlushLLCommands( dev );

	// Override all option bytes and reset to factory settings, unlocking all flash sections.
	uint8_t option_data[] = { 0xa5, 0x5a, 0x97, 0x68, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00 };
	if( MCF.WriteBinaryBlob != DefaultWriteBinaryBlob )
	{
		fprintf( stderr, "Warning, using nonstandard WriteBinaryBlob.  Unbrick may not work.\n" );
	}
	MCF.WriteBinaryBlob(dev, 0x1ffff800, sizeof( option_data ), option_data );

	MCF.DelayUS( dev, 20000 );

	if( timeout == max_timeout ) 
	{
		fprintf( stderr, "Timed out trying to unbrick\n" );
		return -5;
	}
	MCF.Erase( dev, 0, 0, 1);
	MCF.FlushLLCommands( dev );
	return -5;
}

int DefaultConfigureNRSTAsGPIO( void * dev, int one_if_yes_gpio  )
{
	fprintf( stderr, "Error: DefaultConfigureNRSTAsGPIO does not work via the programmer here.  Please see the demo \"optionbytes\"\n" );
	return -5;
}

int DefaultConfigureReadProtection( void * dev, int one_if_yes_protect  )
{
	fprintf( stderr, "Error: DefaultConfigureReadProtection does not work via the programmer here.  Please see the demo \"optionbytes\"\n" );
	return -5;
}

int DefaultPrintChipInfo( void * dev )
{
	uint32_t reg;
	MCF.HaltMode( dev, HALT_MODE_HALT_BUT_NO_RESET );
	
	if( MCF.ReadWord( dev, 0x1FFFF800, &reg ) ) goto fail;	
	printf( "USER/RDPR  : %04x/%04x\n", reg>>16, reg&0xFFFF );
	if( MCF.ReadWord( dev, 0x1FFFF804, &reg ) ) goto fail;	
	printf( "DATA1/DATA0: %04x/%04x\n", reg>>16, reg&0xFFFF );
	if( MCF.ReadWord( dev, 0x1FFFF808, &reg ) ) goto fail;	
	printf( "WRPR1/WRPR0: %04x/%04x\n", reg>>16, reg&0xFFFF );
	if( MCF.ReadWord( dev, 0x1FFFF80c, &reg ) ) goto fail;	
	printf( "WRPR3/WRPR2: %04x/%04x\n", reg>>16, reg&0xFFFF );
	if( MCF.ReadWord( dev, 0x1FFFF7E0, &reg ) ) goto fail;
	printf( "Flash Size: %d kB\n", (reg&0xffff) );
	if( MCF.ReadWord( dev, 0x1FFFF7E8, &reg ) ) goto fail;	
	printf( "R32_ESIG_UNIID1: %08x\n", reg );
	if( MCF.ReadWord( dev, 0x1FFFF7EC, &reg ) ) goto fail;	
	printf( "R32_ESIG_UNIID2: %08x\n", reg );
	if( MCF.ReadWord( dev, 0x1FFFF7F0, &reg ) ) goto fail;	
	printf( "R32_ESIG_UNIID3: %08x\n", reg );
	return 0;
fail:
	fprintf( stderr, "Error: Failed to get chip details\n" );
	return -11;
}

int DefaultVoidHighLevelState( void * dev )
{
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)dev)->internal);
	iss->statetag = STTAG( "VOID" );
	return 0;
}

int DefaultDelayUS( void * dev, int us )
{
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
	Sleep( (us+9999) / 1000 );
#else
	usleep( us );
#endif
	return 0;
}

int SetupAutomaticHighLevelFunctions( void * dev )
{
	// Will populate high-level functions from low-level functions.
	if( MCF.WriteReg32 == 0 && MCF.ReadReg32 == 0 && MCF.WriteWord == 0 ) return -5;

	// Else, TODO: Build the high level functions from low level functions.
	// If a high-level function alrady exists, don't override.

	if( !MCF.SetupInterface )
		MCF.SetupInterface = DefaultSetupInterface;
	if( !MCF.WriteBinaryBlob )
		MCF.WriteBinaryBlob = DefaultWriteBinaryBlob;
	if( !MCF.ReadBinaryBlob )
		MCF.ReadBinaryBlob = DefaultReadBinaryBlob;
	if( !MCF.WriteWord )
		MCF.WriteWord = DefaultWriteWord;
	if( !MCF.WriteHalfWord )
		MCF.WriteHalfWord = DefaultWriteHalfWord;
	if( !MCF.WriteByte )
		MCF.WriteByte = DefaultWriteByte;
	if( !MCF.ReadCPURegister )
		MCF.ReadCPURegister = DefaultReadCPURegister;
	if( !MCF.WriteCPURegister )
		MCF.WriteCPURegister = DefaultWriteCPURegister;
	if( !MCF.WriteAllCPURegisters )
		MCF.WriteAllCPURegisters = DefaultWriteAllCPURegisters;
	if( !MCF.ReadAllCPURegisters )
		MCF.ReadAllCPURegisters = DefaultReadAllCPURegisters;
	if( !MCF.SetEnableBreakpoints )
		MCF.SetEnableBreakpoints = DefaultSetEnableBreakpoints;
	if( !MCF.ReadWord )
		MCF.ReadWord = DefaultReadWord;
	if( !MCF.ReadHalfWord )
		MCF.ReadHalfWord = DefaultReadHalfWord;
	if( !MCF.ReadByte )
		MCF.ReadByte = DefaultReadByte;
	if( !MCF.Erase )
		MCF.Erase = DefaultErase;
	if( !MCF.HaltMode )
		MCF.HaltMode = DefaultHaltMode;
	if( !MCF.PollTerminal )
		MCF.PollTerminal = DefaultPollTerminal;
	if( !MCF.WaitForFlash )
		MCF.WaitForFlash = DefaultWaitForFlash;
	if( !MCF.WaitForDoneOp )
		MCF.WaitForDoneOp = DefaultWaitForDoneOp;
	if( !MCF.PrintChipInfo )
		MCF.PrintChipInfo = DefaultPrintChipInfo;
	if( !MCF.Unbrick )
		MCF.Unbrick = DefaultUnbrick;
	if( !MCF.ConfigureNRSTAsGPIO )
		MCF.ConfigureNRSTAsGPIO = DefaultConfigureNRSTAsGPIO;
	if( !MCF.VoidHighLevelState )
		MCF.VoidHighLevelState = DefaultVoidHighLevelState;
	if( !MCF.DelayUS )
		MCF.DelayUS = DefaultDelayUS;

	return 0;
}




void TestFunction(void * dev )
{
	uint32_t rv;
	int r;
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
	MCF.WriteReg32( dev, DMCONTROL, 0x80000001 ); // Initiate a halt request.
	MCF.WriteReg32( dev, DMCONTROL, 0x00000001 ); // Clear Halt Request.

	r = MCF.WriteWord( dev, 0x20000100, 0xdeadbeef );
	r = MCF.WriteWord( dev, 0x20000104, 0xcafed0de );
	r = MCF.WriteWord( dev, 0x20000108, 0x12345678 );
	r = MCF.WriteWord( dev, 0x20000108, 0x00b00d00 );
	r = MCF.WriteWord( dev, 0x20000104, 0x33334444 );

	r = MCF.ReadWord( dev, 0x20000100, &rv );
	printf( "**>>> %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x20000104, &rv );
	printf( "**>>> %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x20000108, &rv );
	printf( "**>>> %d %08x\n", r, rv );


	r = MCF.ReadWord( dev, 0x00000300, &rv );
	printf( "F %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x00000304, &rv );
	printf( "F %d %08x\n", r, rv );
	r = MCF.ReadWord( dev, 0x00000308, &rv );
	printf( "F %d %08x\n", r, rv );

	uint8_t buffer[256];
	int i;
	for( i = 0; i < 256; i++ ) buffer[i] = 0;
	MCF.WriteBinaryBlob( dev, 0x08000300, 256, buffer );
	MCF.ReadBinaryBlob( dev, 0x08000300, 256, buffer );
	for( i = 0; i < 256; i++ )
	{
		printf( "%02x ", buffer[i] );
		if( (i & 0xf) == 0xf ) printf( "\n" );
	}

	for( i = 0; i < 256; i++ ) buffer[i] = i;
	MCF.WriteBinaryBlob( dev, 0x08000300, 256, buffer );
	MCF.ReadBinaryBlob( dev, 0x08000300, 256, buffer );
	for( i = 0; i < 256; i++ )
	{
		printf( "%02x ", buffer[i] );
		if( (i & 0xf) == 0xf ) printf( "\n" );
	}
}



