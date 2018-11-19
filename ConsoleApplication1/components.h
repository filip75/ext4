#pragma once
class Partition
{
private:
	int	sectorSize; // bytes
	int	blockSize; // sectors
	int	physicalDisc;
	unsigned long long bootOffset; // sector of boot sector
	unsigned long long mftOffset; // offset of mft from boot sector
	char letter;

public:
	Partition();
	~Partition();
};

class File {
private:
	unsigned long long mftSector;
public:
	File();
	~File();
};

