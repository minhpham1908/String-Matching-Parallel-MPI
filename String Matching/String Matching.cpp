#include <iostream>
#include <vector>
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


void stringMatchingKMP(const char* string, const char* pattern);
void generateLps(const char* pattern, int* lps, int len);
int main()
{
	std::string text = "ababa";
	std::string pattern = "aba";
	stringMatchingKMP(text.c_str(), pattern.c_str());


}


void stringMatchingKMP(const char* string, const char* pattern) {
	int stringLen = strlen(string);
	int patternLen = strlen(pattern);

	int* lps = new int[patternLen]();

	generateLps(pattern, lps, patternLen);
	bool found = false;
	int i = 0, j = 0;
	while (i < stringLen) {
		if (string[i] == pattern[j]) {
			i++;
			j++;
		}
		if (j == patternLen) {
			std::cout << "Tim thay giong nhau tai vi tri " << i - j << std::endl;
			j = lps[j - 1];
			found = true;
		}
		else if (string[i] != pattern[j]) {
			if (j == 0) {
				i++;
			}
			else {
				j = lps[j - 1];
			}


		}

	}
	if (found != true) {
		std::cout << -1 << std::endl;
	}
}

void generateLps(const char* pattern, int* lps, int len) {
	int j = 0;
	lps[0] = 0;
	int i = 1;
	while (i < len) {
		if (pattern[i] == pattern[j]) {
			lps[i++] = ++j;
		}
		else {
			if (j == 0) {
				lps[i++] = 0;
			}
			else {
				j = lps[j - 1];
			}
		}
	}
}
