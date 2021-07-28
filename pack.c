#include <stdio.h>

void pack(FILE* lpFileFrom, FILE* lpFileTo);
void unpack(FILE* lpFileFrom, FILE* lpFileTo);

int main(int const argc, char** const argv)
{
	if (argc < 4) {
		printf("\nUsage: pack [-p/-u] file1 file2\n\t-p\tPack file1 to file2\n\t-u\tUnpack file1 to file2\n\n");
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
	char unsigned cByteCurr;
	char unsigned cByteTemp;
	char unsigned cCountDup = 0x1;
	char unsigned cCountNotDup = 0x1;
	char unsigned cBufferBytes[0x7F];
	int iEof = fscanf(lpFileFrom, "%c", &cBytePrev);
	
	while (iEof != EOF) {
		
		iEof = fscanf(lpFileFrom, "%c", &cByteCurr);
		
		if (cByteCurr == cBytePrev && iEof != EOF) {
			if (cCountNotDup > 0x1) {
				fprintf(lpFileTo, "%c", cCountNotDup - 0x1);
				for (int i = 0x0; i < cCountNotDup - 0x1; i++)
					fprintf(lpFileTo, "%c", cBufferBytes[i]);
				cCountNotDup = 0x1;
			}
			cCountDup++;
		}
		
		else if (iEof == EOF && cCountNotDup > 0x1) {
			fprintf(lpFileTo, "%c", cCountNotDup);
			for (int i = 0x0; i < cCountNotDup; i++)
				fprintf(lpFileTo, "%c", cBufferBytes[i]);
			cCountNotDup = 0x1;
		}
		
		else if (cCountDup == 0x1)
			cCountNotDup++;
		
		if (cCountNotDup == 0x2) {
			cBufferBytes[0x0] = cBytePrev;
			cBufferBytes[0x1] = cByteCurr;
		}
		
		else if (cCountNotDup == 0x7F) {
			cBufferBytes[cCountNotDup - 0x1] = cByteCurr;
			fprintf(lpFileTo, "%c", cCountNotDup);
			for (int i = 0; i < cCountNotDup; i++)
				fprintf(lpFileTo, "%c", cBufferBytes[i]);
			cCountNotDup = 0x1;
			fscanf(lpFileFrom, "%c", &cBytePrev);
			continue;
		}
		
		else if (cCountNotDup > 0x2)
			cBufferBytes[cCountNotDup - 0x1] = cByteCurr;
		
		if (cCountDup > 0x1 && (cByteCurr != cBytePrev || cCountDup == 0x7F || iEof == EOF)) {
			fprintf(lpFileTo, "%c%c", cCountDup + 0x80, cBytePrev);
			if (cCountDup == 0x7F) {
				fscanf(lpFileFrom, "%c", &cBytePrev);
				cCountDup = 0x1;
				continue;
			}
			cCountDup = 0x1;
		}

		
		if (iEof == EOF)
			break;
		cBytePrev = cByteCurr;
		
	}
}

void unpack(FILE* lpFileFrom, FILE* lpFileTo)
{
	char unsigned cByte;
	char unsigned cCount;
	fscanf(lpFileFrom, "%c", &cCount);
	while(fscanf(lpFileFrom, "%c", &cByte) == 1) {
		if (cCount > 0x80)
			for (int i = 0; i < cCount - 0x80; i++)
				fprintf(lpFileTo, "%c", cByte);
		else {
			fprintf(lpFileTo, "%c", cByte);
			for (int i = 0x1; i < cCount; i++) {
				fscanf(lpFileFrom, "%c", &cByte);
				fprintf(lpFileTo, "%c", cByte);
			}
		}
		fscanf(lpFileFrom, "%c", &cCount);
	}
}
