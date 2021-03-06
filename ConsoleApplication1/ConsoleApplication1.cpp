// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include <Windows.h>

#define SECTOR_SIZE	512

#define MBR_PART_LIST_OFFSET	446
#define MBR_PART_TYPE_OFFSET	4
#define MBR_PART_LOC_OFFSET		8

#define BOOT_PER_SEC_OFFSET		0xB
#define BOOT_PER_CLUST_OFFSET	0xD
#define BOOT_MFT_LCN_OFFSET		0x30

#define TYPE_NTFS	0x7
#define TYPE_GPT	0xEE
#define TYPE_EBR	0xF

typedef unsigned char BYTE;



UINT32 littleToInt(BYTE* little) {
	UINT32* result = reinterpret_cast<UINT32*>(little);
	return *result;
}

UINT32 littleToInt(BYTE* little, int length) {
	int result = 0;
	int multiplier = 1;
	for (int i = 0; i < length; i++) {
		result += little[i] * multiplier;
		multiplier *= 0x100;
	}
	return result;
}

void printHex(BYTE b) {
	BYTE h = b / 16;
	BYTE l = b - (h * 16);
	if (h < 10)
		printf("%d", int(h));
	else
		printf("%c", 'a' + h - 10);

	if (l < 10)
		printf("%d", int(l));
	else
		printf("%c", 'a' + l - 10);
	printf(" ");
}

void printHexNumber(BYTE* b, int length) {
	for (int i = 0; i < length; i++)
		printHex(b[i]);
	printf("\n");
}

void printSector(char* sector, int length) {
	int i = 0;
	while (i < length) {
		printHex(sector[i]);
		i++;
		if (i % 16 == 0 && i > 0)
			printf("\n");
	}
}

int readSector(HANDLE diskHandle, UINT32 sectorNumber, BYTE* buffer);
void readMBR(HANDLE diskHandle);
void readBootSector(UINT32 sector, HANDLE diskHandle);
void readEBR(UINT32 sector, HANDLE diskHandle);
void readGPT(UINT32 sector, HANDLE diskHandle);


int main()
{
	//char buffer[SECTOR_SIZE];
	HANDLE hdrive = CreateFile(L"\\\\.\\PhysicalDrive2", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	readMBR(hdrive);
	//readSector(hdrive, 1, buffer);
	//printSector(buffer, SECTOR_SIZE);
}

int readSector(HANDLE diskHandle, UINT32 sectorNumber, BYTE* buffer) {
	DWORD bytesRead;
	SetFilePointer(diskHandle, SECTOR_SIZE*sectorNumber, 0, FILE_BEGIN);
	ReadFile(diskHandle, buffer, SECTOR_SIZE, &bytesRead, 0);
	return bytesRead;
}

void readMBR(HANDLE diskHandle) {
	BYTE mbr[SECTOR_SIZE];
	readSector(diskHandle, 0, mbr);
	BYTE* partitions[4];
	partitions[0] = &mbr[MBR_PART_LIST_OFFSET];
	partitions[1] = partitions[0] + 16;
	partitions[2] = partitions[1] + 16;
	partitions[3] = partitions[2] + 16;

	for (int i = 0; i < 4; i++) {
		UINT32 address = littleToInt(&partitions[i][MBR_PART_LOC_OFFSET], 4);
		printf("part addrss: %u\n", address);
		if (partitions[i][MBR_PART_TYPE_OFFSET] == TYPE_EBR) {
			readEBR(address, diskHandle);
		}
		else if (partitions[i][MBR_PART_TYPE_OFFSET] == TYPE_GPT) {
			readGPT(address, diskHandle);
		}
		else if (partitions[i][MBR_PART_TYPE_OFFSET] == TYPE_NTFS) {
			readBootSector(address, diskHandle);
		}
	}
}

void readBootSector(UINT32 sector, HANDLE diskHandle) {
	BYTE bootSector[SECTOR_SIZE];
	readSector(diskHandle, sector, bootSector);
	int bytesPerSector = littleToInt(&bootSector[BOOT_PER_SEC_OFFSET], 2);
	int sectorsPerCluster = littleToInt(&bootSector[BOOT_PER_CLUST_OFFSET], 1);
	UINT64 mftLcn = littleToInt(&bootSector[BOOT_MFT_LCN_OFFSET], 8);
	UINT32 mftSector = sector + mftLcn * sectorsPerCluster;

	printHexNumber(&bootSector[BOOT_PER_SEC_OFFSET], 2);
	printHexNumber(&bootSector[BOOT_PER_CLUST_OFFSET], 1);
	printHexNumber(&bootSector[BOOT_MFT_LCN_OFFSET], 8);

	printf("bytes per sector: %d; sectors per cluster: %d, mft LCN: %I64d; mft sector: %u\n",
		bytesPerSector, sectorsPerCluster, mftLcn, mftSector);

	printf("NTFS\n");
}

void readGPT(UINT32 sector, HANDLE diskHandle) {
	printf("GPT\n");
}

void readEBR(UINT32 sector, HANDLE diskHandle) {
	printf("EBR\n");
}
