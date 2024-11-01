// The "bootloader" blob is (C) WCH.
// Tricky: You need to use wch link to use WCH-LinkRV.
//  you can always uninstall it in device manager.  It will be under USB devices or something like that at the bottom.

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "libusb.h"
#include "minichlink.h"

struct LinkEProgrammerStruct
{
	void * internal;
	libusb_device_handle * devh;
	int lasthaltmode; // For non-003 chips
};

// For non-ch32v003 chips.
//static int LEReadBinaryBlob( void * d, uint32_t offset, uint32_t amount, uint8_t * readbuff );
static int InternalLinkEHaltMode( void * d, int mode );
static int LEWriteBinaryBlob( void * d, uint32_t address_to_write, uint32_t len, uint8_t * blob );

#define WCHTIMEOUT 5000
#define WCHCHECK(x) if( (status = x) ) { fprintf( stderr, "Bad USB Operation on " __FILE__ ":%d (%d)\n", __LINE__, status ); exit( status ); }

void wch_link_command( libusb_device_handle * devh, const void * command_v, int commandlen, int * transferred, uint8_t * reply, int replymax )
{
	uint8_t * command = (uint8_t*)command_v;
	uint8_t buffer[1024];
	int got_to_recv = 0;
	int status;
	int transferred_local;
	if( !transferred ) transferred = &transferred_local;
	status = libusb_bulk_transfer( devh, 0x01, command, commandlen, transferred, WCHTIMEOUT );
	if( status ) goto sendfail;
	got_to_recv = 1;
	if( !reply )
	{
		reply = buffer; replymax = sizeof( buffer );
	}

//	printf("wch_link_command send (%d)", commandlen); for(int i = 0; i< commandlen; printf(" %02x",command[i++])); printf("\n");

	status = libusb_bulk_transfer( devh, 0x81, reply, replymax, transferred, WCHTIMEOUT );

//	printf("wch_link_command reply (%d)", *transferred); for(int i = 0; i< *transferred; printf(" %02x",reply[i++])); printf("\n"); 

	if( status ) goto sendfail;
	return;
sendfail:
	fprintf( stderr, "Error sending WCH command (%s): ", got_to_recv?"on recv":"on send" );
	int i;
	for( i = 0; i < commandlen; i++ )
	{
		printf( "%02x ", command[i] );
	}
	printf( "\n" );
	exit( status );
}

static void wch_link_multicommands( libusb_device_handle * devh, int nrcommands, ... )
{
	int i;
	va_list argp;
	va_start(argp, nrcommands);
	for( i = 0; i < nrcommands; i++ )
	{
		int clen = va_arg(argp, int);
		wch_link_command( devh, va_arg(argp, char *), clen, 0, 0, 0 );
	}
	va_end( argp );
}

static inline libusb_device_handle * wch_link_base_setup( int inhibit_startup )
{
	libusb_context * ctx = 0;
	int status;
	status = libusb_init(&ctx);
	if (status < 0) {
		fprintf( stderr, "Error: libusb_init_context() returned %d\n", status );
		exit( status );
	}
	
	libusb_device **list;
	libusb_device *found = NULL;
	ssize_t cnt = libusb_get_device_list(ctx, &list);
	ssize_t i = 0;
	for (i = 0; i < cnt; i++) {
		libusb_device *device = list[i];
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(device,&desc);
		if( r == 0 && desc.idVendor == 0x1a86 && desc.idProduct == 0x8010 ) { found = device; }
	}

	if( !found )
	{
		return 0;
	}

	libusb_device_handle * devh;
	status = libusb_open( found, &devh );
	if( status )
	{
		fprintf( stderr, "Error: couldn't open wch link device (libusb_open() = %d)\n", status );
		return 0;
	}
		
	WCHCHECK( libusb_claim_interface(devh, 0) );
	
	uint8_t rbuff[1024];
	int transferred;
	libusb_bulk_transfer( devh, 0x81, rbuff, 1024, &transferred, 1 ); // Clear out any pending transfers.  Don't wait though.

	return devh;
}

// DMI_OP decyphered From https://github.com/karlp/openocd-hacks/blob/27af153d4a373f29ad93dab28a01baffb7894363/src/jtag/drivers/wlink.c
// Thanks, CW2 for pointing this out.  See DMI_OP for more info.
int LEWriteReg32( void * dev, uint8_t reg_7_bit, uint32_t command )
{
	libusb_device_handle * devh = ((struct LinkEProgrammerStruct*)dev)->devh;

	const uint8_t iOP = 2; // op 2 = write
	uint8_t req[] = {
		0x81, 0x08, 0x06, reg_7_bit,
			(command >> 24) & 0xff,
			(command >> 16) & 0xff,
			(command >> 8) & 0xff,
			(command >> 0) & 0xff,
			iOP };

	uint8_t resp[128];
	int resplen;
	wch_link_command( devh, req, sizeof(req), &resplen, resp, sizeof(resp) );
	if( resplen != 9 || resp[3] != reg_7_bit )
	{
		fprintf( stderr, "Error setting write reg. Tell cnlohr. Maybe we should allow retries here?\n" );
		fprintf( stderr, "RR: %d :", resplen );
		int i;
		for( i = 0; i < resplen; i++ )
		{
			fprintf( stderr, "%02x ", resp[i] );
		}
		fprintf( stderr, "\n" );
	}
	return 0;
}

int LEReadReg32( void * dev, uint8_t reg_7_bit, uint32_t * commandresp )
{
	libusb_device_handle * devh = ((struct LinkEProgrammerStruct*)dev)->devh;
	const uint8_t iOP = 1; // op 1 = read
	uint32_t transferred;
	uint8_t rbuff[128] = { 0 };
	uint8_t req[] = {
		0x81, 0x08, 0x06, reg_7_bit,
			0, 0, 0, 0,
			iOP };
	wch_link_command( devh, req, sizeof( req ), (int*)&transferred, rbuff, sizeof( rbuff ) );
	*commandresp = ( rbuff[4]<<24 ) | (rbuff[5]<<16) | (rbuff[6]<<8) | (rbuff[7]<<0);
	if( transferred != 9 || rbuff[3] != reg_7_bit )
	{
		fprintf( stderr, "Error setting write reg. Tell cnlohr. Maybe we should allow retries here?\n" );
		fprintf( stderr, "RR: %d :", transferred );
		int i;
		for( i = 0; i < transferred; i++ )
		{
			fprintf( stderr, "%02x ", rbuff[i] );
		}
		fprintf( stderr, "\n" );
	}
	/*
	printf( "RR: %d :", transferred );
	int i;
	for( i = 0; i < transferred; i++ )
	{
		printf( "%02x ", rbuff[i] );
	}
	printf( "\n" );
	*/

	return 0;
}

int LEFlushLLCommands( void * dev )
{
	return 0;
}

static int LESetupInterface( void * d )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;
	struct InternalState * iss = (struct InternalState*)(((struct ProgrammerStructBase*)d)->internal);
	uint8_t rbuff[1024];
	uint32_t transferred = 0;

	// This puts the processor on hold to allow the debugger to run.
	wch_link_command( dev, "\x81\x0d\x01\x03", 4, (int*)&transferred, rbuff, 1024 ); // Reply: Ignored, 820d050900300500

	// Place part into reset.
	wch_link_command( dev, "\x81\x0d\x01\x01", 4, (int*)&transferred, rbuff, 1024 );	// Reply is: "\x82\x0d\x04\x02\x08\x02\x00"
	switch(rbuff[5]) {
		case 1:
			fprintf(stderr, "WCH Programmer is CH549 version %d.%d\n",rbuff[3], rbuff[4]);
			break;
		case 2:
			fprintf(stderr, "WCH Programmer is CH32V307 version %d.%d\n",rbuff[3], rbuff[4]);
			break;
		case 3:
			fprintf(stderr, "WCH Programmer is CH32V203 version %d.%d\n",rbuff[3], rbuff[4]);
			break;
		case 4:
			fprintf(stderr, "WCH Programmer is LinkB version %d.%d\n",rbuff[3], rbuff[4]);
			break;
		case 18:
			fprintf(stderr, "WCH Programmer is LinkE version %d.%d\n",rbuff[3], rbuff[4]);
			break;
		default:
			fprintf(stderr, "Unknown WCH Programmer %02x (Ver %d.%d)\n", rbuff[5], rbuff[3], rbuff[4]);
			break;
	}

	// TODO: What in the world is this?  It doesn't appear to be needed.
	wch_link_command( dev, "\x81\x0c\x02\x09\x01", 5, 0, 0, 0 ); //Reply is: 820c0101

	// Note from further debugging:
	// My capture differs in this case: \x05 instead of \x09 -> But does not seem to be needed
	//wch_link_command( dev, "\x81\x0c\x02\x05\x01", 5, 0, 0, 0 ); //Reply is: 820c0101

	// This puts the processor on hold to allow the debugger to run.
	wch_link_command( dev, "\x81\x0d\x01\x02", 4, (int*)&transferred, rbuff, 1024 ); // Reply: Ignored, 820d050900300500
	if (rbuff[0] == 0x81 && rbuff[1] == 0x55 && rbuff[2] == 0x01 && rbuff[3] == 0x01)
	{
		fprintf(stderr, "link error, nothing connected to linker\n");
		return -1;
	}

	uint32_t mcu_series = rbuff[4] << 4;
    uint32_t target_chip_type = mcu_series + (rbuff[5] >> 4);
	fprintf(stderr, "Chip Type: %03x\n", target_chip_type);

	if( mcu_series == 0x300 || mcu_series == 0x200 )
	{
		fprintf( stderr, "CH32V30x or CH32V20x MCU detected. Using binary blob write for operation.\n" );
		MCF.WriteBinaryBlob = LEWriteBinaryBlob;

		iss->sector_size = 256;

		wch_link_command( dev, "\x81\x0d\x01\x03", 4, (int*)&transferred, rbuff, 1024 ); // Reply: Ignored, 820d050900300500
	}

	// For some reason, if we don't do this sometimes the programmer starts in a hosey mode.
	MCF.WriteReg32( d, DMCONTROL, 0x80000001 ); // Make the debug module work properly.
	MCF.WriteReg32( d, DMCONTROL, 0x80000001 ); // Initiate a halt request.
	MCF.WriteReg32( d, DMCONTROL, 0x80000001 ); // No, really make sure.
	MCF.WriteReg32( d, DMABSTRACTCS, 0x00000700 ); // Ignore any pending errors.
	MCF.WriteReg32( d, DMABSTRACTAUTO, 0 );
	MCF.WriteReg32( d, DMCOMMAND, 0x00221000 ); // Read x0 (Null command) with nopostexec (to fix v307 read issues)

	int r = 0;

	r |= MCF.WaitForDoneOp( d, 0 );
	if( r )
	{
		fprintf( stderr, "Fault on setup\n" );
	}
	else
	{
		fprintf( stderr, "Setup success\n" );
	}

	// This puts the processor on hold to allow the debugger to run.
	// Recommended to switch to 05 from 09 by Alexander M
	//	wch_link_command( dev, "\x81\x11\x01\x09", 4, (int*)&transferred, rbuff, 1024 ); // Reply: Chip ID + Other data (see below)
	wch_link_command( dev, "\x81\x11\x01\x05", 4, (int*)&transferred, rbuff, 1024 ); // Reply: Chip ID + Other data (see below)

	if( transferred != 20 )
	{
		fprintf( stderr, "Error: could not get part status\n" );
		return -1;
	}
	fprintf( stderr, "Flash Storage: %d kB\n", (rbuff[2]<<8) | rbuff[3] );  // Is this Flash size?
	fprintf( stderr, "Part UUID    : %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n", rbuff[4], rbuff[5], rbuff[6], rbuff[7], rbuff[8], rbuff[9], rbuff[10], rbuff[11] );
	fprintf( stderr, "PFlags       : %02x-%02x-%02x-%02x\n", rbuff[12], rbuff[13], rbuff[14], rbuff[15] );
	fprintf( stderr, "Part Type (B): %02x-%02x-%02x-%02x\n", rbuff[16], rbuff[17], rbuff[18], rbuff[19] );

	// Check for read protection
	wch_link_command( dev, "\x81\x06\x01\x01", 4, (int*)&transferred, rbuff, 1024 );
	if(transferred != 4) {
		fprintf(stderr, "Error: could not get read protection status\n");
		return -1;
	}

	if(rbuff[3] == 0x01) {
		fprintf(stderr, "Read protection: enabled\n");
	} else {
		fprintf(stderr, "Read protection: disabled\n");
	}

	iss->flash_size = ((rbuff[2]<<8) | rbuff[3])*1024;
	iss->target_chip_type = target_chip_type;

	return 0;
}

static int LEControl3v3( void * d, int bOn )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;

	if( bOn )
		wch_link_command( (libusb_device_handle *)dev, "\x81\x0d\x01\x09", 4, 0, 0, 0 );
	else
		wch_link_command( (libusb_device_handle *)dev, "\x81\x0d\x01\x0a", 4, 0, 0, 0 );
	return 0;
}

static int LEControl5v( void * d, int bOn )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;

	if( bOn )
		wch_link_command( (libusb_device_handle *)dev, "\x81\x0d\x01\x0b", 4, 0, 0, 0 );
	else
		wch_link_command( (libusb_device_handle *)dev, "\x81\x0d\x01\x0c", 4, 0, 0, 0 );
	return 0;
}

static int LEUnbrick( void * d )
{
	printf( "Sending unbrick\n" );
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;
	wch_link_command( (libusb_device_handle *)dev, "\x81\x0d\x01\x0f\x09", 5, 0, 0, 0 );
	printf( "Done unbrick\n" );
	return 0;
}


static int LEConfigureNRSTAsGPIO( void * d, int one_if_yes_gpio )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;

	if( one_if_yes_gpio )
	{
		wch_link_multicommands( (libusb_device_handle *)dev, 2, 11, "\x81\x06\x08\x02\xff\xff\xff\xff\xff\xff\xff", 4, "\x81\x0b\x01\x01" );
	}
	else
	{
		wch_link_multicommands( (libusb_device_handle *)dev, 2, 11, "\x81\x06\x08\x02\xf7\xff\xff\xff\xff\xff\xff", 4, "\x81\x0b\x01\x01" );
	}
	return 0;
}

static int LEConfigureReadProtection( void * d, int one_if_yes_protect )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;

	if( one_if_yes_protect )
	{
		wch_link_multicommands( (libusb_device_handle *)dev, 2, 11, "\x81\x06\x08\x03\xf7\xff\xff\xff\xff\xff\xff", 4, "\x81\x0b\x01\x01" );
	}
	else
	{
		wch_link_multicommands( (libusb_device_handle *)dev, 2, 11, "\x81\x06\x08\x02\xf7\xff\xff\xff\xff\xff\xff", 4, "\x81\x0b\x01\x01" );
	}
	return 0;
}

int LEExit( void * d )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;

	wch_link_command( (libusb_device_handle *)dev, "\x81\x0d\x01\xff", 4, 0, 0, 0);
	return 0;
}

void * TryInit_WCHLinkE()
{
	libusb_device_handle * wch_linke_devh;
	wch_linke_devh = wch_link_base_setup(0);
	if( !wch_linke_devh ) return 0;

	struct LinkEProgrammerStruct * ret = malloc( sizeof( struct LinkEProgrammerStruct ) );
	memset( ret, 0, sizeof( *ret ) );
	ret->devh = wch_linke_devh;
	ret->lasthaltmode = 0;

	MCF.ReadReg32 = LEReadReg32;
	MCF.WriteReg32 = LEWriteReg32;
	MCF.FlushLLCommands = LEFlushLLCommands;

	MCF.SetupInterface = LESetupInterface;
	MCF.Control3v3 = LEControl3v3;
	MCF.Control5v = LEControl5v;
	MCF.Unbrick = LEUnbrick;
	MCF.ConfigureNRSTAsGPIO = LEConfigureNRSTAsGPIO;
	MCF.ConfigureReadProtection = LEConfigureReadProtection;

	MCF.Exit = LEExit;
	return ret;
};


#if 1

// Flash Bootloader for V20x and V30x series MCUs

const uint8_t * bootloader = (const uint8_t*)
"\x93\x77\x15\x00\x41\x11\x99\xcf\xb7\x06\x67\x45\xb7\x27\x02\x40" \
"\x93\x86\x36\x12\x37\x97\xef\xcd\xd4\xc3\x13\x07\xb7\x9a\xd8\xc3" \
"\xd4\xd3\xd8\xd3\x93\x77\x25\x00\x95\xc7\xb7\x27\x02\x40\x98\x4b" \
"\xad\x66\x37\x38\x00\x40\x13\x67\x47\x00\x98\xcb\x98\x4b\x93\x86" \
"\xa6\xaa\x13\x67\x07\x04\x98\xcb\xd8\x47\x05\x8b\x61\xeb\x98\x4b" \
"\x6d\x9b\x98\xcb\x93\x77\x45\x00\xa9\xcb\x93\x07\xf6\x0f\xa1\x83" \
"\x2e\xc0\x2d\x68\x81\x76\x3e\xc4\xb7\x08\x02\x00\xb7\x27\x02\x40" \
"\x37\x33\x00\x40\x13\x08\xa8\xaa\xfd\x16\x98\x4b\x33\x67\x17\x01" \
"\x98\xcb\x02\x47\xd8\xcb\x98\x4b\x13\x67\x07\x04\x98\xcb\xd8\x47" \
"\x05\x8b\x41\xeb\x98\x4b\x75\x8f\x98\xcb\x02\x47\x13\x07\x07\x10" \
"\x3a\xc0\x22\x47\x7d\x17\x3a\xc4\x69\xfb\x93\x77\x85\x00\xd5\xcb" \
"\x93\x07\xf6\x0f\x2e\xc0\xa1\x83\x3e\xc4\x37\x27\x02\x40\x1c\x4b" \
"\xc1\x66\x41\x68\xd5\x8f\x1c\xcb\xb7\x16\x00\x20\xb7\x27\x02\x40" \
"\x93\x08\x00\x04\x37\x03\x20\x00\x98\x4b\x33\x67\x07\x01\x98\xcb" \
"\xd8\x47\x05\x8b\x75\xff\x02\x47\x3a\xc2\x46\xc6\x32\x47\x0d\xef" \
"\x98\x4b\x33\x67\x67\x00\x98\xcb\xd8\x47\x05\x8b\x75\xff\xd8\x47" \
"\x41\x8b\x39\xc3\xd8\x47\xc1\x76\xfd\x16\x13\x67\x07\x01\xd8\xc7" \
"\x98\x4b\x21\x45\x75\x8f\x98\xcb\x41\x01\x02\x90\x23\x20\xd8\x00" \
"\x25\xb7\x23\x20\x03\x01\xa5\xb7\x12\x47\x13\x8e\x46\x00\x94\x42" \
"\x14\xc3\x12\x47\x11\x07\x3a\xc2\x32\x47\x7d\x17\x3a\xc6\xd8\x47" \
"\x09\x8b\x75\xff\xf2\x86\x5d\xb7\x02\x47\x13\x07\x07\x10\x3a\xc0" \
"\x22\x47\x7d\x17\x3a\xc4\x49\xf3\x98\x4b\xc1\x76\xfd\x16\x75\x8f" \
"\x98\xcb\x41\x89\x15\xc9\x2e\xc0\x0d\x06\x02\xc4\x09\x82\x32\xc6" \
"\xb7\x17\x00\x20\x98\x43\x13\x86\x47\x00\xa2\x47\x82\x46\x8a\x07" \
"\xb6\x97\x9c\x43\x63\x1c\xf7\x00\xa2\x47\x85\x07\x3e\xc4\xa2\x46" \
"\x32\x47\xb2\x87\xe3\xe0\xe6\xfe\x01\x45\xbd\xbf\x41\x45\xad\xbf" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff" \
"\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff";

int bootloader_len = 512;
#endif

static int InternalLinkEHaltMode( void * d, int mode )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;
	if( mode == ((struct LinkEProgrammerStruct*)d)->lasthaltmode )
		return 0;
	((struct LinkEProgrammerStruct*)d)->lasthaltmode = mode;
	
	if( mode == 0 )
	{
		printf( "Holding in reset\n" );
		// Part one "immediately" places the part into reset.  Part 2 says when we're done, leave part in reset.
		wch_link_multicommands( (libusb_device_handle *)dev, 2, 4, "\x81\x0d\x01\x02", 4, "\x81\x0d\x01\x01" );
	}
	else if( mode == 1 )
	{
		// This is clearly not the "best" method to exit reset.  I don't know why this combination works.
		wch_link_multicommands( (libusb_device_handle *)dev, 3, 4, "\x81\x0b\x01\x01", 4, "\x81\x0d\x01\x02", 4, "\x81\x0d\x01\xff" );
	}
	else
	{
		return -999;
	}
	return 0;
}

#if 0
static int LEReadBinaryBlob( void * d, uint32_t offset, uint32_t amount, uint8_t * readbuff )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;

	InternalLinkEHaltMode( d, 0 );

	int i;
	int status;
	uint8_t rbuff[1024];
	int transferred = 0;
	int readbuffplace = 0;

	wch_link_command( (libusb_device_handle *)dev, "\x81\x06\x01\x01", 4, 0, 0, 0 );

	// Flush out any pending data.
	libusb_bulk_transfer( (libusb_device_handle *)dev, 0x82, rbuff, 1024, &transferred, 1 );

	// 3/8 = Read Memory
	// First 4 bytes are big-endian location.
	// Next 4 bytes are big-endian amount.
	uint8_t readop[11] = { 0x81, 0x03, 0x08, };
	
	readop[3] = (offset>>24)&0xff;
	readop[4] = (offset>>16)&0xff;
	readop[5] = (offset>>8)&0xff;
	readop[6] = (offset>>0)&0xff;

	readop[7] = (amount>>24)&0xff;
	readop[8] = (amount>>16)&0xff;
	readop[9] = (amount>>8)&0xff;
	readop[10] = (amount>>0)&0xff;
	
	wch_link_command( (libusb_device_handle *)dev, readop, 11, 0, 0, 0 );

	// Perform operation
	wch_link_command( (libusb_device_handle *)dev, "\x81\x02\x01\x0c", 4, 0, 0, 0 );

	uint32_t remain = amount;
	while( remain )
	{
		transferred = 0;
		WCHCHECK( libusb_bulk_transfer( (libusb_device_handle *)dev, 0x82, rbuff, 1024, &transferred, WCHTIMEOUT ) );
		memcpy( ((uint8_t*)readbuff) + readbuffplace, rbuff, transferred );
		readbuffplace += transferred;
		remain -= transferred;
	}

	// Flip internal endian.  Must be done separately in case something was unaligned when
	// reading.
	for( i = 0; i < readbuffplace/4; i++ )
	{
		uint32_t r = ((uint32_t*)readbuff)[i];
		((uint32_t*)readbuff)[i] = (r>>24) | ((r & 0xff0000) >> 8) | ((r & 0xff00)<<8) | (( r & 0xff )<<24); 
	}

	return 0;
}
#endif

static int LEWriteBinaryBlob( void * d, uint32_t address_to_write, uint32_t len, uint8_t * blob )
{
	libusb_device_handle * dev = ((struct LinkEProgrammerStruct*)d)->devh;
	struct InternalState * iss = (struct InternalState*)(((struct LinkEProgrammerStruct*)d)->internal);

	InternalLinkEHaltMode( d, 0 );

	int i;
	int status;
	uint8_t rbuff[1024];
	int transferred;

	int padlen = ((len-1) & (~(iss->sector_size-1))) + iss->sector_size;

	wch_link_command( (libusb_device_handle *)dev, "\x81\x06\x01\x01", 4, 0, 0, 0 );
	wch_link_command( (libusb_device_handle *)dev, "\x81\x06\x01\x01", 4, 0, 0, 0 ); // Not sure why but it seems to work better when we request twice.

	// This contains the write data quantity, in bytes.  (The last 2 octets)
	// Then it just rollllls on in.
	char rksbuff[11] = { 0x81, 0x01, 0x08,
						 // Address to write
						 (uint8_t)(address_to_write >> 24), (uint8_t)(address_to_write >> 16),
						 (uint8_t)(address_to_write >> 8), (uint8_t)(address_to_write & 0xff),
						 // Length to write
						 (uint8_t)(len >> 24), (uint8_t)(len >> 16),
						 (uint8_t)(len >> 8), (uint8_t)(len & 0xff) };
	wch_link_command( (libusb_device_handle *)dev, rksbuff, 11, 0, 0, 0 );
	
	wch_link_command( (libusb_device_handle *)dev, "\x81\x02\x01\x05", 4, 0, 0, 0 );
	
	int pplace = 0;
	for( pplace = 0; pplace < bootloader_len; pplace += iss->sector_size )
	{
		WCHCHECK( libusb_bulk_transfer( (libusb_device_handle *)dev, 0x02, (uint8_t*)(bootloader+pplace), iss->sector_size, &transferred, WCHTIMEOUT ) );
	}
	
	for( i = 0; i < 10; i++ )
	{
		wch_link_command( (libusb_device_handle *)dev, "\x81\x02\x01\x07", 4, &transferred, rbuff, 1024 );
		if( transferred == 4 && rbuff[0] == 0x82 && rbuff[1] == 0x02 && rbuff[2] == 0x01 && rbuff[3] == 0x07 )
		{
			break;
		}
	} 
	if( i == 10 )
	{
		fprintf( stderr, "Error, confusing responses to 02/01/07\n" );
		exit( -109 );
	}
	
	wch_link_command( (libusb_device_handle *)dev, "\x81\x02\x01\x02", 4, 0, 0, 0 );

	for( pplace = 0; pplace < padlen; pplace += iss->sector_size )
	{
		if( pplace + iss->sector_size > len )
		{
			uint8_t paddeddata[iss->sector_size];
			int gap = pplace + iss->sector_size - len;
			int okcopy = len - pplace;
			memcpy( paddeddata, blob + pplace, okcopy );
			memset( paddeddata + okcopy, 0xff, gap );
			WCHCHECK( libusb_bulk_transfer( (libusb_device_handle *)dev, 0x02, paddeddata, iss->sector_size, &transferred, WCHTIMEOUT ) );
		}
		else
		{
			WCHCHECK( libusb_bulk_transfer( (libusb_device_handle *)dev, 0x02, blob+pplace, iss->sector_size, &transferred, WCHTIMEOUT ) );
		}
	}
	return 0;
}


