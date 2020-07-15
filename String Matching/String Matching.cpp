#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include "mpi.h"


#pragma comment (lib, "msmpi.lib")

#define MASTER 0
#define PROCESS_SUBSTRING_PART 1
#define PROCESS_CONNECTION_PART 2
#define MILLION 1000000L

#define _CRT_SECURE_NO_WARNINGS
#if 0
void stringMatchingBruteForce(const char* text, const char* pattern) {
	std::cout << "stringMatchingBruteForce" << std::endl;
	std::cout << "Text: " << text << std::endl;
	std::cout << "Pattern: " << pattern << std::endl;

	int textLength = strlen(text);
	int patternLen = strlen(pattern);
	bool match = true;
	int j = 0;
	for (int i = 0; i < textLength - patternLen + 1; i++)
	{
		match = true;
		j = 0;
		while (match && j < patternLen) {
			if (text[i + j] != pattern[j]) {
				match = false;
			}
			else {
				j++;

			}

		}
		if (match)
		{
			std::cout << i << std::endl;
		}
	}
}
#endif

#define DefineZone "madafakas"
std::vector<int>* stringMatchingKMP(const char* string, const char* pattern);
void generateLps(const char* pattern, int* lps, int len);
void storeText(char* text, char* msg, int offset, int len);
int* getRealIndex(std::vector<int>* answer, int rank, int length, int patternLen, int* realLength);
void daklak(int* finalResult, int* result, int size);



int main(int argc, char** argv)
{
	int textLen = 10;
	errno_t err;
	double start = 0, stop = 0;
	int rank, nproc;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	char* nameProcess = new char[MPI_MAX_PROCESSOR_NAME];
	int nameLen;
	MPI_Get_processor_name(nameProcess, &nameLen);
	std::cout << "Ten processer thu " << rank << ": " << nameProcess << std::endl;
	MPI_Barrier(MPI_COMM_WORLD);


	int patternlen;
	char* pattern = new char;

	FILE* patFile;

	err = fopen_s(&patFile, "pattern.txt", "r");
	if (err == 0) {
		std::cout << "pattern.txt was opened!" << std::endl;
		fseek(patFile, 0, SEEK_END);
		patternlen = (int)ftell(patFile);
		rewind(patFile);
		fgets(pattern, patternlen + 1, patFile);
		std::cout << patternlen << std::endl;
		std::cout << strlen(pattern) << std::endl;
		fclose(patFile);
		free(patFile);
	}
	else {
		std::cout << "pattern.txt was nnot opened!" << std::endl;
	}

	int subtextLenPerProc = textLen / nproc;
	int subtextStartPosition = textLen / nproc;
	int end = subtextLenPerProc + (textLen % nproc);

	char* sendMsg = new char[(long int)end + 1];
	char* recvMsg = new char[(long int)end + 1];

	char* text = new char[(long int)end + patternlen];
	text[0] = '\0';

	if (rank == MASTER)
	{
		start = MPI_Wtime();
		//khoi tao du lieu
		char* fullText = new char[10];
		FILE* textStream;

		err = fopen_s(&textStream, "text.txt", "r");
		fgets(fullText, textLen, textStream);
		
		delete(textStream);
		// strncpy(fullText, "ab1ab2ab3ab4ab5ab6", textLen);
		int sizes = strlen(fullText);
		std::cout << sizes << std::endl;
		std::cout << "SubText Length per proces:" << subtextLenPerProc << std::endl;
		std::cout << "Pattern Length per proces:" << patternlen << std::endl;

		for (int i = 1; i < nproc; i++)
		{
			if (i == nproc - 1)
			{
				strncpy_s(sendMsg, end, fullText + i * subtextStartPosition, end);
				//strncpy(sendMsg, fullText + i * subtextStartPosition, end);
				sendMsg[end] = '\0';
				MPI_Send(sendMsg, end, MPI_CHAR, i, PROCESS_SUBSTRING_PART, MPI_COMM_WORLD);
			}
			else
			{
				strncpy_s(sendMsg, subtextLenPerProc, fullText + i * subtextStartPosition, subtextLenPerProc);
				//strncpy(sendMsg, fullText + i * subtextStartPosition, subtextLenPerProc);
				sendMsg[subtextLenPerProc] = '\0';
				MPI_Send(sendMsg, subtextLenPerProc, MPI_CHAR, i, PROCESS_SUBSTRING_PART, MPI_COMM_WORLD);
			}
		}
		strncpy_s(recvMsg, subtextLenPerProc, fullText, subtextLenPerProc);
		//strncpy(recvMsg, fullText, subtextLenPerProc);
		recvMsg[subtextLenPerProc] = '\0';
		storeText(text, recvMsg, 0, subtextLenPerProc);
	}
	else if (rank == nproc - 1)
	{
		MPI_Recv(recvMsg, end, MPI_CHAR, MASTER, PROCESS_SUBSTRING_PART, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		recvMsg[end] = '\0';
		storeText(text, recvMsg, 0, end);
	}
	else
	{
		MPI_Recv(recvMsg, subtextLenPerProc, MPI_CHAR, MASTER, PROCESS_SUBSTRING_PART, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		recvMsg[subtextLenPerProc] = '\0';
		storeText(text, recvMsg, 0, subtextLenPerProc);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// for (int i = 0; i < nproc; i++)
	// {
	//     if (i == rank)
	//     {
	//         std::cout << "Rank: " << rank << " text: " << text << std::endl;
	//         std::cout << "Rank: " << rank << " recv: " << recvMsg << std::endl;
	//     }
	// }

	MPI_Barrier(MPI_COMM_WORLD);
	free(sendMsg);
	free(recvMsg);
	//send connection part
	char* tailText = new char[patternlen - 1 + 1];
	tailText[patternlen] = '\0';
	if (rank != MASTER)
	{
		MPI_Send(text, patternlen - 1, MPI_CHAR, rank - 1, PROCESS_CONNECTION_PART, MPI_COMM_WORLD);
	}
	if (rank != nproc - 1)
	{
		MPI_Recv(tailText, patternlen - 1, MPI_CHAR, rank + 1, PROCESS_CONNECTION_PART, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		storeText(text, tailText, subtextLenPerProc, patternlen - 1);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// for (int i = 0; i < nproc; i++)
	// {
	//     if (i == rank)
	//     {
	//         std::cout << "Rank: " << rank << " text: " << text << std::endl;
	//     }
	// }

	std::vector<int>* answer = stringMatchingKMP(text, pattern);
	//IN VI TRI
	// for (int i = 0; i < nproc; i++)
	// {
	//     if (i == rank)
	//     {
	//         std::cout << "rank: " << i << " text: " << text << std::endl;
	//         for (int vitri : *answer)
	//         {
	//             std::cout << vitri << " ";
	//         }
	//         std::cout << std::endl;
	//     }
	// }

	MPI_Barrier(MPI_COMM_WORLD);
	int realLength = 0;
	int* result = getRealIndex(answer, rank, subtextLenPerProc, patternlen, &realLength);

	//SEND BACK TO MASTER
	if (rank != MASTER)
	{
		MPI_Send(result, realLength, MPI_INT, MASTER, 3, MPI_COMM_WORLD);
		free(answer);
		free(result);
	}
	else
	{
		int* finalResult = new int[textLen];
		daklak(finalResult, result, realLength);

		free(answer);
		free(result);
		for (int i = 1; i < nproc; i++)
		{
			int* result = new int[end + patternlen - 1];
			for (int j = 0; j < end + patternlen - 1; j++)
			{
				result[j] = 0;
			}
			MPI_Recv(result, end + patternlen - 1, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			daklak(finalResult, result, end + patternlen - 1);
			free(result);
		}
		stop = MPI_Wtime();
		printf("\n***\nVới %d processors, thời gian chạy sẽ là %.3f milisecs: \n", nproc, (stop - start) * 1000);
		int found = 0;
		bool foundd = false;
		for (int j = 0; j < textLen; j++)
		{
			if (finalResult[j] == 1)
			{
				printf("Find a matching substring starting at: %d.\n", j);
				found++;
				foundd = true;
			}
		}
		if (!foundd)
		{
			std::cout << "404" << std::endl;
		}
		printf("Số lần phát hiện là %d\n", found);
	}


	MPI_Finalize();
}

std::vector<int>* stringMatchingKMP(const char* string, const char* pattern)
{
	int stringLen = strlen(string);
	int patternLen = strlen(pattern);
	int* lps = new int[patternLen];
	std::vector<int>* answer = new std::vector<int>();
	generateLps(pattern, lps, patternLen);
	bool found = false;
	int i = 0, j = 0;
	while (i < stringLen)
	{
		if (string[i] == pattern[j])
		{
			i++;
			j++;
		}
		if (j == patternLen)
		{
			// std::cout << "Tim thay giong nhau tai vi tri " << i - j << std::endl;
			answer->push_back(i - j);
			j = lps[j - 1];
			found = true;
		}
		else if (string[i] != pattern[j])
		{
			if (j == 0)
			{
				i++;
			}
			else
			{
				j = lps[j - 1];
			}
		}
	}
	if (found != true)
	{
		// std::cout << -1 << std::endl;
		answer->push_back(-1);
	}
	return answer;
}

void generateLps(const char* pattern, int* lps, int len)
{
	int j = 0;
	lps[0] = 0;
	int i = 1;
	while (i < len)
	{
		if (pattern[i] == pattern[j])
		{
			lps[i++] = ++j;
		}
		else
		{
			if (j == 0)
			{
				lps[i++] = 0;
			}
			else
			{
				j = lps[j - 1];
			}
		}
	}
}

void storeText(char* text, char* src, int offset, int len)
{
	errno_t err;
	err = strncpy_s(text + offset, len, src, (size_t)len);
	//strncpy(text + offset, msg, len);
	text[offset + len] = '\0';
}

int* getRealIndex(std::vector<int>* answer, int rank, int length, int patternLen, int* realLength)
{
	int* result = new int[length + patternLen - 1];
	int i = 0;
	for (int localIndex : *answer)
	{
		int realIndex = rank * length + localIndex;
		result[i] = realIndex;
		i++;
	}
	result[i] = -1;
	*realLength = i;
	return result;
}

void daklak(int* finalResult, int* result, int size)
{
	int x = 0;
	for (; x < size; x++)
	{
		if (x > 0 && result[x] == 0)
		{
			break;
		}
		else
		{
			finalResult[result[x]] = 1;
		}
	}
}