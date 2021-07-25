#include <stdio.h>

void pack(FILE* lpFileFrom, FILE* lpFileTo);
void unpack(FILE* lpFileFrom, FILE* lpFileTo);

int main(int const argc, char** const argv)
{
	if (argc < 4) {
		printf("\nUsage: pack [-p/-u] file1 file2\n\t-p\tPack file1 to file2\n\t-u\tUnpack file1 to file2\n");
		return 0;
	}
	FILE* lpFileFrom = fopen(argv[2], "rb");
	FILE* lpFileTo   = fopen(argv[3], "w+b");
	
	if (argv[1][1] == 'p')
		pack(lpFileFrom, lpFileTo);
	else if (argv[1][1] == 'u')
		unpack(lpFileFrom, lpFileTo);
	else
		printf("Unknown key: \"%s\"", argv[1]);
	
	fclose(lpFileTo);
	fclose(lpFileFrom);
	return 0;
}

void pack(FILE* lpFileFrom, FILE* lpFileTo)
{
	char unsigned cBytePrev;
	int iEof = fscanf(lpFileFrom, "%c", &cBytePrev);
	char unsigned cByteCurr;
	char unsigned cCounterDup = (char)129; // 1000.0001 - highest bit, mean that the remaning 7 bit indicate count duplicates next byte
	char unsigned cCounterNotDup = 0;
	char unsigned cBufferBytes[127];
	while(iEof == 1) {
		iEof = fscanf(lpFileFrom, "%c", &cByteCurr);
		
		if (cByteCurr != cBytePrev && cCounterDup == 129)
			cCounterNotDup++;
			
		else {
			if (cCounterNotDup > 0) {
				fprintf(lpFileTo, "%c", cCounterNotDup);
				for (int i = 0; i < (int)cCounterNotDup; i++)
					fprintf(lpFileTo, "%c", cBufferBytes[i]);
			}
			cCounterNotDup = 0;
			cCounterDup++;
		}
		if (cCounterNotDup == 1) {
			cBufferBytes[0] = cBytePrev;
			cBufferBytes[1] = cByteCurr;
		}
		else if (cCounterNotDup == 126) {
			cBufferBytes[cCounterNotDup] = cByteCurr;
			fprintf(lpFileTo, "%c", cCounterNotDup + 1);
			for (int i = 0; i < (int)cCounterNotDup + 1; i++)
				fprintf(lpFileTo, "%c", cBufferBytes[i]);
			cCounterNotDup = 0;
			fscanf(lpFileFrom, "%c", &cBytePrev);
			continue;
		}
		else if (cCounterNotDup > 1)
			cBufferBytes[cCounterNotDup] = cByteCurr;
		
		if ( (int)cCounterDup - 128 > 1 && (cByteCurr != cBytePrev || (int)cCounterDup == 255 || iEof != 1)) {
			fprintf(lpFileTo, "%c%c", cCounterDup - 1, cBytePrev);
			cCounterDup = 129; // restore the variable
		}
		
		cBytePrev = cByteCurr;
	}
}

void unpack(FILE* lpFileFrom, FILE* lpFileTo)
{
	char unsigned cByte;
	char unsigned cCount;
	int iEof = fscanf(lpFileFrom, "%c", &cCount);
	while(iEof == 1) {
		iEof = fscanf(lpFileFrom, "%c", &cByte);
		if (cCount > 128)
			for (int i = 0; i < cCount - 128; i++)
				fprintf(lpFileTo, "%c", cByte);
		else {
			fprintf(lpFileTo, "%c", cByte);
			for (int i = 1; i < cCount; i++) {
				iEof = fscanf(lpFileFrom, "%c", &cByte);
				fprintf(lpFileTo, "%c", cByte);
			}
		}
		fscanf(lpFileFrom, "%c", &cCount);
	}
}
