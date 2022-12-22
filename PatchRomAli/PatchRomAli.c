#include    <stdlib.h>
#include    <stdio.h>
#include    <io.h>
#include    <SYS\Stat.h>
#include    <share.h>
#include    <conio.h>
#include    <fcntl.h>
//
//  This utility is designed to display checksum in byte
//
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
#define PUBLIC_KEY_OFFSET		0x7f8020L
#define PUBLIC_KEY_SIZE			404
#define HEADER_BINARY_SIZE		0x10000

void ShowUsage(void)
{
	printf("Tool to insert public key to a UBIOS.\n\n"
		"Usage:  InsertPK  UBIOS public_key [herder_bin]\n"
		"\n");
}

main(int argc, char* argv[])
{
	int fhUBIOS, fhPublicKey, fhHeaderBinary;
	int nDoubleWordChecksum = 0;
	int i;
	BYTE bufferPK[PUBLIC_KEY_SIZE];
	BYTE bufferHB[HEADER_BINARY_SIZE];
	DWORD Checksum;
	DWORD lROMSize = 0;
	char option_char;

	if (argc < 3) {
		ShowUsage();
		exit(-1);
	}
	//
	// processing parameters
	//
	if (argc > 1)
	{
		for (i = 1; i < argc; i++)
		{
			if ((*argv[i] == '/') || (*argv[i] == '-'))
			{
				option_char = (char)(*(argv[i] + 1));
				switch (option_char)
				{
				case 'H':
				case 'h':
				case '?':
					ShowUsage();
					exit(0);
					break;
				default:
					break;
				}
			}
		}
	}
	//
	// get public key and checksum to a word
	//
	if (_sopen_s(&fhPublicKey, argv[2], _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD) != 0)
	{
		printf("%s open error!\n", argv[2]);
		exit(-2);
	}
	//
	// Checksum by byte thru the file
	//
	_lseek(fhPublicKey, 0L, SEEK_SET);           /* Set to position 0 */
	Checksum = 0;
	if (_read(fhPublicKey, bufferPK, sizeof(bufferPK)))
	{
		//
		// Accumulate _read buffer
		//    
		for (i = 0; i < PUBLIC_KEY_SIZE; i += sizeof(BYTE))
		{
			//
			// Skip BIOSInfo block if requested & exist
			//
			Checksum += (BYTE)bufferPK[i];
		}
	}
	else
	{
		printf("%s read error!\n", argv[2]);
		exit(-2);
	}
	//
	// read header binary file
	//
	if (argc > 3) {
		if (_sopen_s(&fhHeaderBinary, argv[3], _O_BINARY | _O_RDONLY, _SH_DENYWR, _S_IREAD) != 0)
		{
			printf("%s open error!\n", argv[3]);
			exit(-3);
		}
		//
		// read header file
		//
		_lseek(fhHeaderBinary, 0L, SEEK_SET);           /* Set to position 0 */
		if (!_read(fhHeaderBinary, bufferHB, HEADER_BINARY_SIZE))
		{
			printf("%s read error!\n", argv[3]);
			exit(-3);
		}
	}
	//
	// open UBIOS
	//
	if (_sopen_s(&fhUBIOS, argv[1], _O_BINARY | _O_RDWR, _SH_DENYNO, _S_IREAD | _S_IWRITE) != 0)

	{
		printf("%s open error!\n", argv[1]);
		exit(-1);
	}
	//
	// Override header with Header File input
	//
	if (argc > 3) {
		_lseek(fhUBIOS, 0, SEEK_SET);
		if (_write(fhUBIOS, bufferHB, sizeof(bufferHB)) == -1)
		{
			printf("%s Header wrtie error!\n", argv[1]);
			_close(fhUBIOS);
			exit(-1);
		}
	}
	//
	// Checksum by byte thru the file
	//
	_lseek(fhUBIOS, PUBLIC_KEY_OFFSET, SEEK_SET);           /* Seek to PUBLIC_KEY_OFFSET */
	if (_write(fhUBIOS, bufferPK, sizeof(bufferPK)) == -1)
	{
		printf("%s Public Key wrtie error!\n", argv[1]);
		_close(fhUBIOS);
		exit(-1);
	}
	if (_write(fhUBIOS, &Checksum, sizeof(Checksum)) == -1)
	{
		printf("%s Checksum wrtie error!\n", argv[1]);
		_close(fhUBIOS);
		exit(-1);
	}

	_close(fhHeaderBinary);
	_close(fhPublicKey);
	_close(fhUBIOS);
	exit(0);
}
