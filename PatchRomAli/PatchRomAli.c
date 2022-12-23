#include    <stdlib.h>
#include    <stdio.h>
#include    <io.h>
//
//  This utility is designed to display checksum in byte
//
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
#define PUBLIC_KEY_OFFSET		0x7f8020L
#define PUBLIC_KEY_SIZE			404
#define HEADER_BINARY_SIZE		0x10000

BYTE bufferPK[PUBLIC_KEY_SIZE];
BYTE bufferHB[HEADER_BINARY_SIZE];

void ShowUsage(void)
{
	printf("Tool to insert public key to a UBIOS.\n\n"
		"Usage:  InsertPK  UBIOS public_key [herder_bin]\n"
		"\n");
}

main(int argc, char* argv[])
{
	FILE *fpUBIOS, *fpPublicKey, *fpHeaderBinary;
	int nDoubleWordChecksum = 0;
	int i;
	DWORD Checksum;
	DWORD lROMSize = 0;
	char option_char;

	if (argc < 4) {
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
	if ((fopen_s(&fpPublicKey, argv[2], "rb"))!=0)
	{
		printf("%s open error!\n", argv[2]);
		exit(-2);
	}
	//
	// Checksum by byte thru the file
	//
	fseek(fpPublicKey, 0L, SEEK_SET);           /* Set to position 0 */
	Checksum = 0;
	if (fread_s(bufferPK, sizeof(bufferPK), 1, sizeof(bufferPK), fpPublicKey))
	{
		//
		// byte checksum public key
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
	if (fopen_s(&fpHeaderBinary, argv[3], "rb") != 0)
	{
		printf("%s open error!\n", argv[3]);
		exit(-3);
	}
	//
	// read header file
	//
	fseek(fpHeaderBinary, 0L, SEEK_SET);           /* Set to position 0 */
	if (!fread_s(bufferHB, HEADER_BINARY_SIZE, 1, HEADER_BINARY_SIZE, fpHeaderBinary))
	{
		printf("%s read error!\n", argv[3]);
		exit(-3);
	}
	//
	// open UBIOS
	//
	if (fopen_s(&fpUBIOS, argv[1], "wb") != 0)

	{
		printf("%s open error!\n", argv[1]);
		exit(-1);
	}
	//
	// Override header with Header File input
	//
	fseek(fpUBIOS, 0, SEEK_SET);
	if (fwrite(bufferHB, 1, sizeof(bufferHB), fpUBIOS) == -1)
	{
		printf("%s Header wrtie error!\n", argv[1]);
		fclose(fpUBIOS);
		exit(-1);
	}
	//
	// Checksum by byte thru the file
	//
	fseek(fpUBIOS, PUBLIC_KEY_OFFSET, SEEK_SET);           /* Seek to PUBLIC_KEY_OFFSET */
	if (fwrite(bufferPK, 1, sizeof(bufferPK), fpUBIOS) == -1)
	{
		printf("%s Public Key wrtie error!\n", argv[1]);
		fclose(fpUBIOS);
		exit(-1);
	}
	if (fwrite(&Checksum, 1, sizeof(Checksum), fpUBIOS) == -1)
	{
		printf("%s Checksum wrtie error!\n", argv[1]);
		fclose(fpUBIOS);
		exit(-1);
	}

	fclose(fpHeaderBinary);
	fclose(fpPublicKey);
	fclose(fpUBIOS);
	exit(0);
}
