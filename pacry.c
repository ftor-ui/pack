#include <stdio.h>

void pack(FILE* lpFileFrom, FILE* lpFileTo);
void unpack(FILE* lpFileFrom, FILE* lpFileTo);
void encrypt(FILE *lpFileFrom, FILE *lpFileTo, char const *lpKey, int iLength);
void decrypt(FILE *lpFileFrom, FILE *lpFileTo, char const *lpKey, int iLength);

int main(int const argc, char** const argv)
{
	if (argc < 4 || argc < 5 && (argv[1][1] == 'e' || argv[1][1] == 'd')) {
		printf("\nUsage: pacry [-p/-u/-e/-d] file1 file2 [-/key]\n\t");
		printf("-p\tPack file1 to file2\n\t");
		printf("-u\tUnpack file1 to file2\n\t");
		printf("-e\tEncrypt with key file1 to file2\n\t");
		printf("-d\tDecrypt with key file1 to file2\n\n");
		return 0;
	}
	
	FILE* lpFileFrom = fopen(argv[2], "rb");
	FILE* lpFileTo   = fopen(argv[3], "w+b");
	if (lpFileFrom == NULL) {
		printf("File doesn`t exist: \"%s\"", argv[2]);
		return 0;
	}
	
	if (argv[1][1] == 'e' || argv[1][1] == 'd') {
		int iLength = 0;
		char const *lpKey = argv[4];
		for (int i = 0; lpKey[i] != '\0'; i++)
			iLength++;
		if (argv[1][1] == 'e')
			encrypt(lpFileFrom, lpFileTo, lpKey, iLength);
		else if (argv[1][1] == 'd')
			decrypt(lpFileFrom, lpFileTo, lpKey, iLength);
	}
	
	else if (argv[1][1] == 'p')
		pack(lpFileFrom, lpFileTo);
	else if (argv[1][1] == 'u')
		unpack(lpFileFrom, lpFileTo);
	else
		printf("Unknown option: \"%s\"", argv[1]);
	
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
	
	do {
		
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
		
		else if (iEof == EOF && cCountDup == 0x1) {
			fprintf(lpFileTo, "%c", cCountNotDup);
			for (int i = 0x0; i < cCountNotDup; i++)
				fprintf(lpFileTo, "%c", cBufferBytes[i]);
			break;
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

		cBytePrev = cByteCurr;
		
	} while (iEof != EOF);
}

void unpack(FILE* lpFileFrom, FILE* lpFileTo)
{
	char unsigned cByte;
	char unsigned cCount;
	fscanf(lpFileFrom, "%c", &cCount);
	while(fscanf(lpFileFrom, "%c", &cByte) != EOF) {
		if (cCount > 0x80)
			for (int i = 0x0; i < cCount - 0x80; i++)
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

void encrypt(FILE *lpFileFrom, FILE *lpFileTo, char const *lpKey, int iLength)
{
	char unsigned cByte = 0x0;
	int iIndexChar = 0x0;
	while (fscanf(lpFileFrom, "%c", &cByte) != EOF) {
		
		fprintf(lpFileTo, "%c", cByte + lpKey[iIndexChar]);
		
		iIndexChar++;
		
		if (iIndexChar == iLength)
			iIndexChar = 0x0;
	}
}

void decrypt(FILE *lpFileFrom, FILE *lpFileTo, char const *lpKey, int iLength)
{
	char unsigned cByte = 0x0;
	int iIndexChar = 0x0;
	while (fscanf(lpFileFrom, "%c", &cByte) != EOF) {
		
		fprintf(lpFileTo, "%c", cByte - lpKey[iIndexChar]);
		
		iIndexChar++;
		
		if (iIndexChar == iLength)
			iIndexChar = 0x0;
	}
}
