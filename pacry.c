#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>


// Basic functions
void pack(FILE *lpFileFrom, FILE *lpFileTo);
void unpack(FILE* lpFileFrom, FILE *lpFileTo);
void packd(char const *lpPath, FILE *lpFileTo, int iMode, int iLevel);
void unpackd(FILE *lpFileFrom, char const *lpPath);
void encrypt(FILE *lpFileFrom, FILE *lpFileTo, char const *lpKey, int iLength);
void decrypt(FILE *lpFileFrom, FILE *lpFileTo, char const *lpKey, int iLength);

// Secondery functions
int type(char const *lpNameObj); // 0 - dir, 1 - file; need in functions packd
char *getnamedir(char const *lpCurrentPath, char *lpBuffer); // write to 'cBuffer' final directory of path 'lpCurrentPath'
char *gluingpath(char const *lpFirstPath, char *lpSecondPath, char *lpBuffer); // gluing two path in one and return him; full path need for open files
char *cdtop(char const *lpPath, char *lpBuffer); // changes the path one directory above
char *copypath(char const *lpPathFrom, char *lpPathTo); // copy 'lpPathFrom' to 'lpPathTo'

int main(int const argc, char** const argv)
{
	if (argc < 4 || argc < 5 && (argv[1][1] == 'e' || argv[1][1] == 'd')) {
		printf("\nUsage: pacry [-p/-u/-pd/-ud/-e/-d] [dir1/file1] [dir2/file2] [-/key]\n\t");
		printf("-p \tPack file1 to file2\n\t");
		printf("-u \tUnpack file1 to file2\n\t");
		printf("-pd\tPack directory1 to file2\n\t");
		printf("-ud\tUnpack packed directory1 to directory2\n\t");
		printf("-e \tEncrypt with key file1 to file2\n\t");
		printf("-d \tDecrypt with key file1 to file2\n\n");
		return 0;
	}

	FILE *lpFileTo = NULL;
	FILE *lpFileFrom = NULL;

	if (!(argv[1][1] == 'u' && argv[1][2] == 'd'))
		lpFileTo   = fopen(argv[3], "w+b");
	if (!(argv[1][1] == 'p' && argv[1][2] == 'd'))
		lpFileFrom = fopen(argv[2], "rb");
	if (lpFileFrom == NULL && argv[1][2] != 'd') {
		printf("File doesn`t exist: \"%s\"\n", argv[2]);
		return 0;
	}

	if (argv[1][1] == 'p' && argv[1][2] == 'd')
		packd(argv[2], lpFileTo, 0, 0);
	else if (argv[1][1] == 'u' && argv[1][2] == 'd')
		unpackd(lpFileFrom, argv[3]);

	else if (argv[1][1] == 'e' || argv[1][1] == 'd') {
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
		printf("Unknown option: \"%s\"\n", argv[1]);

	if (!(argv[1][1] == 'u' && argv[1][2] == 'd'))
		fclose(lpFileTo);
	if (!(argv[1][1] == 'p' && argv[1][2] == 'd'))
		fclose(lpFileFrom);

	return 0;
}

void pack(FILE *lpFileFrom, FILE *lpFileTo)
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

void unpack(FILE *lpFileFrom, FILE *lpFileTo)
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

void packd(char const *lpPath, FILE *lpFileTo, int iMode, int iLevel)
{
	int iCountFile = 0;
	int iCountObject = 0;
	int iCountDir = 0;
	char lpBuffer[260] = {0x0};
	getnamedir(lpPath, lpBuffer);

	fprintf(lpFileTo, "%c", (char unsigned)iMode); // 0000.0000 means start directory bytes

	// Filling the name directory
	for (int i = 0x0; lpBuffer[i] != '\0'; i++) {
		fprintf(lpFileTo, "%c", (char unsigned)lpBuffer[i]);
	}
	fprintf(lpFileTo, "%c", 0x0); // 0000.0000 means end name of directory

	char unsigned cByteLevel = 0x0;
	for (int i = 0x0; i < iLevel; i++) {
		cByteLevel++;
		if (cByteLevel == 0xFF) {
			fprintf(lpFileTo, "%c", cByteLevel);
			cByteLevel = 0x0;
		}
	}
	if (cByteLevel != 0x0)
		fprintf(lpFileTo, "%c", cByteLevel);
	if (iLevel == 0x0)
		fprintf(lpFileTo, "%c", 0x0);
	fprintf(lpFileTo, "%c", 0x0);

	DIR *lpDir = opendir(lpPath);
	struct dirent *lpObject;
	FILE *lpFileFrom = NULL;

	// File handler
	while ((lpObject = readdir(lpDir)) != NULL) {
		if (type(gluingpath(lpPath, lpObject->d_name, lpBuffer)) == 0 || (lpObject->d_name[0] == '.' &&\
		lpObject->d_name[1] == '.' && lpObject->d_name[2] == '\0') || \
		(lpObject->d_name[0] == '.' && lpObject->d_name[1] == '\0'))
			continue;

		lpFileFrom = fopen(gluingpath(lpPath, lpObject->d_name, lpBuffer), "rb");
		fprintf(lpFileTo, "%c", 0x80); // means start file bytes

		// Filling the name file
		for (int i = 0x0; lpObject->d_name[i] != '\0'; i++) {
			fprintf(lpFileTo, "%c", (char unsigned)lpObject->d_name[i]);
		}
		fprintf(lpFileTo, "%c", 0x0); // 0000.0000 means end name of directory

		// Pack the file
		fseek(lpFileFrom, 0, SEEK_SET);
		printf("%s\n", gluingpath(lpPath, lpObject->d_name, lpBuffer));
		pack(lpFileFrom, lpFileTo);
		fclose(lpFileFrom);
		fprintf(lpFileTo, "%c%c%c%c", 0x80, 0x2, 0x2, 0x2); // means end bytes of file
	}
	rewinddir(lpDir);

	// Directory handler
	while ((lpObject = readdir(lpDir)) != NULL) {

		if (type(gluingpath(lpPath, lpObject->d_name, lpBuffer)) == 1 || (lpObject->d_name[0] == '.' &&\
		lpObject->d_name[1] == '.' && lpObject->d_name[2] == '\0') || \
		(lpObject->d_name[0] == '.' && lpObject->d_name[1] == '\0'))
			continue;

		packd(gluingpath(lpPath, lpObject->d_name, lpBuffer), lpFileTo, 0, iLevel + 1);

	}
	closedir(lpDir);
}

void unpackd(FILE *lpFileFrom, char const *lpPath)
{
	char lpBuffer[260] = {0};
	char lpDirName[260] = {0};
	char lpFileName[260] = {0};
	char lpCurrentPath[260] = {0};
	char unsigned lpFourBytes[4] = {0};
	char unsigned cByte;
	char unsigned cByteDir;
	char unsigned cCount;
	int iLevel = 0;
	int iLevelPrev = 0;

	DIR *lpDir = NULL;
	if ((lpDir = opendir(lpPath)) == NULL)
	#ifdef __linux__
		mkdir(lpPath, 0700);
	#elif _WIN32
		mkdir(lpPath);
	#endif
	else
		closedir(lpDir);

	copypath(lpPath, lpCurrentPath);

	while(fscanf(lpFileFrom, "%c", &cByte) != EOF) {
		if (cByte == 0x0) {

			cByteDir = cByte;

			fscanf(lpFileFrom, "%c", &cByte);
			lpDirName[0] = cByte;
			for (int i = 1; cByte != 0x0; i++) {
				fscanf(lpFileFrom, "%c", &cByte);
				lpDirName[i] = cByte;
			}

			for (int i = 0; ; i++) {
				fscanf(lpFileFrom, "%c", &cByte);
				if (cByte == 0x0 && i != 0)
					break;
				iLevel += (int)cByte;
			}

			if (iLevel < iLevelPrev)
				for (int i = 0; i < iLevelPrev - iLevel + 1; i++) {
					cdtop(lpCurrentPath, lpBuffer);
					copypath(lpBuffer, lpCurrentPath);
				}
			else if (iLevel == iLevelPrev && iLevel != 0) {
				cdtop(lpCurrentPath, lpBuffer);
				copypath(lpBuffer, lpCurrentPath);
			}

			gluingpath(lpCurrentPath, lpDirName, lpBuffer);
			copypath(lpBuffer, lpCurrentPath);


			iLevelPrev = iLevel;
			iLevel = 0;

			#ifdef __linux__
				mkdir(lpCurrentPath, 0700);
			#elif _WIN32
				mkdir(lpCurrentPath);
			#endif
		}
		else if (cByte == 0x80) {

			fscanf(lpFileFrom, "%c", &cByte);
			lpFileName[0] = cByte;
			for (int i = 1; cByte != 0x0; i++) {
				fscanf(lpFileFrom, "%c", &cByte);
				lpFileName[i] = cByte;
			}

			printf("%s\n", gluingpath(lpCurrentPath, lpFileName, lpBuffer));

			FILE *lpFileTo = fopen(gluingpath(lpCurrentPath, lpFileName, lpBuffer), "w+b");
			fscanf(lpFileFrom, "%c", lpFourBytes + 0);
			fscanf(lpFileFrom, "%c", lpFourBytes + 1);
			fscanf(lpFileFrom, "%c", lpFourBytes + 2);
			fscanf(lpFileFrom, "%c", lpFourBytes + 3);
			if (lpFourBytes[0] == 0x1 && lpFourBytes[2] == 0x80 && lpFourBytes[3] == 0x2) {
				for (int i = 0; i < 2; i++)
					lpFourBytes[i] = lpFourBytes[i + 2];
				fscanf(lpFileFrom, "%c", lpFourBytes + 2);
				fscanf(lpFileFrom, "%c", lpFourBytes + 3);
				if (lpFourBytes[0] == 0x80 && lpFourBytes[1] == 0x2 && lpFourBytes[2] == 0x2 && lpFourBytes[3] == 0x2) {
					fclose(lpFileTo);
					continue;
				}
				fseek(lpFileFrom, -2, SEEK_CUR);
			}
			while (!(lpFourBytes[0] == 0x80 && lpFourBytes[1] == 0x2 && lpFourBytes[2] == 0x2 && lpFourBytes[3] == 0x2)) {

				fseek(lpFileFrom, -4, SEEK_CUR);
				fscanf(lpFileFrom, "%c", &cCount);
				fscanf(lpFileFrom, "%c", &cByte);

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

				fscanf(lpFileFrom, "%c", lpFourBytes + 0);
				fscanf(lpFileFrom, "%c", lpFourBytes + 1);
				fscanf(lpFileFrom, "%c", lpFourBytes + 2);
				fscanf(lpFileFrom, "%c", lpFourBytes + 3);

			}
			fclose(lpFileTo);
		}
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

int type(char const *lpNameObj)
{
	DIR *lpDirTemp = opendir(lpNameObj);
	if (lpDirTemp == NULL)
		return 1;

	closedir(lpDirTemp);
	return 0;
}

char *getnamedir(char const *lpCurrentPath, char *lpBuffer)
{
	int iCount = 0; // count chars in 'lpCurrentPath' without '\0'
	for (int i = 0; lpCurrentPath[i] != '\0'; i++)
		iCount++;

	int iCountDir = 0; // count chars in name of finaly directory of path 'lpCurrentPath'
	for (int i = iCount - 1; i >= 0; i--) {
		if ((lpCurrentPath[i] == '\\' || lpCurrentPath[i] == '/') && i != iCount - 1)
			break;
		iCountDir++;
	}

	// Filling 'lpBuffer'
	for (int i = 0; i < iCountDir && i < 260; i++) { // 260 - max chars in 'lpBuffer'
		if (lpCurrentPath[iCount - iCountDir + i] == '\\' || lpCurrentPath[iCount - iCountDir + i] == '/')
			continue;
		lpBuffer[i] = lpCurrentPath[iCount - iCountDir + i];
	}

	return lpBuffer;
}

char *gluingpath(char const *lpFirstPath, char *lpSecondPath, char *lpBuffer)
{

	int iCountFirstPath = 0; // count chars in 'lpFirstPath' without '\0'
	for (int i = 0; lpFirstPath[i] != '\0' && i < 260; i++) {
		lpBuffer[i] = lpFirstPath[i];
		iCountFirstPath++;
	}

	if (lpFirstPath[iCountFirstPath - 1] != '\\' && lpFirstPath[iCountFirstPath - 1] != '/')
		lpBuffer[iCountFirstPath] = '/';
	else
		iCountFirstPath--;

	int iCountSecondPath = 0; // // count chars in 'lpSecondPath' without '\0'
	for (int i = 0; lpSecondPath[i] != '\0'; i++) {
		if (lpBuffer[iCountFirstPath] == '/')
			lpBuffer[iCountFirstPath + i + 1] = lpSecondPath[i];
		else
			lpBuffer[iCountFirstPath + i] = lpSecondPath[i];
		iCountSecondPath++;
	}
	lpBuffer[iCountFirstPath + iCountSecondPath + 1] = 0;

	return lpBuffer;
}

char *cdtop(char const *lpPath, char *lpBuffer)
{
	int iCount = 0;
	for (int i = 0; lpPath[i] != '\0' && i < 260; i++) {
		lpBuffer[i] = lpPath[i];
		iCount++;
	}
	lpBuffer[iCount] = 0;

	for (int i = iCount - 1; i >= 0; i--) {
		if (i == 0)
			lpBuffer[i] = '/';
		if ((lpBuffer[i] == '\\' || lpBuffer[i] == '/') && i != iCount - 1)
			break;
		lpBuffer[i] = 0;
	}

	return lpBuffer;
}

char *copypath(char const *lpPathFrom, char *lpPathTo)
{
	int iCount = 0;
	for (int i = 0; lpPathFrom[i] != '\0' && i < 260; i++) {
		lpPathTo[i] = lpPathFrom[i];
		iCount++;
	}
	lpPathTo[iCount] = 0;

	return lpPathTo;
}
