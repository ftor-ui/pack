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
	char unsigned iBytePrev;
	int iEof = fscanf(lpFileFrom, "%c", &iBytePrev);
	char unsigned iByteCurr;
	char unsigned iCounter = (char)1;
	while(iEof == 1) {
		iEof = fscanf(lpFileFrom, "%c", &iByteCurr);
		
		if (iByteCurr != iBytePrev || (int)iCounter == 255 || iEof != 1) {
			fprintf(lpFileTo, "%c%c", iCounter, iBytePrev);
			iBytePrev = iByteCurr;
			iCounter = 0;
		}
		
		iCounter++;
	}
}

void unpack(FILE* lpFileFrom, FILE* lpFileTo)
{
	char unsigned iByte;
	char unsigned iCount;
	fscanf(lpFileFrom, "%c", &iCount);
	while(fscanf(lpFileFrom, "%c", &iByte) == 1) {
		for (int i = 0; i < (int)iCount; i++)
			fprintf(lpFileTo, "%c", iByte);
		fscanf(lpFileFrom, "%c", &iCount);
	}
}