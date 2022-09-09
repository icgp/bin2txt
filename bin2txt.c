// compile:
//        gcc -static -O3 -o bin2txt.exe bin2txt.c
// encode:
//        bin2txt.exe -i inputPath -o outputPath
// decode:
//        bin2txt.exe -x -i inputPath -outputPath

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char encoding_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								'w', 'x', 'y', 'z', '0', '1', '2', '3',
								'4', '5', '6', '7', '8', '9', '+', '/' };
static char decoding_table[256];
static int mod_table[] = { 0, 2, 1 };

void build_decoding_table() {
	for (int i = 0; i < 64; i++)
		decoding_table[(unsigned char)encoding_table[i]] = i;
}

char* base64_encode(const unsigned char* input,
	size_t input_length,
	char* output,
	size_t* output_length) {

	*output_length = 4 * ((input_length + 2) / 3);

	for (int i = 0, j = 0; i < input_length;) {

		uint32_t octet_a = i < input_length ? (unsigned char)input[i++] : 0;
		uint32_t octet_b = i < input_length ? (unsigned char)input[i++] : 0;
		uint32_t octet_c = i < input_length ? (unsigned char)input[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		output[j++] = encoding_table[(triple >> 18) & 0x3F];
		output[j++] = encoding_table[(triple >> 12) & 0x3F];
		output[j++] = encoding_table[(triple >> 6) & 0x3F];
		output[j++] = encoding_table[(triple) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		output[*output_length - 1 - i] = '=';

	output[*output_length] = '\0';

	return output;
}

unsigned char* base64_decode(const char* intput,
	size_t input_length,
	unsigned char* output,
	size_t* output_length) {

	if (input_length % 4 != 0) return NULL;

	*output_length = input_length / 4 * 3;
	if (intput[input_length - 1] == '=') (*output_length)--;
	if (intput[input_length - 2] == '=') (*output_length)--;

	for (int i = 0, j = 0; i < input_length;) {

		uint32_t sextet_a = intput[i] == '=' ? 0 & i++ : decoding_table[intput[i++]];
		uint32_t sextet_b = intput[i] == '=' ? 0 & i++ : decoding_table[intput[i++]];
		uint32_t sextet_c = intput[i] == '=' ? 0 & i++ : decoding_table[intput[i++]];
		uint32_t sextet_d = intput[i] == '=' ? 0 & i++ : decoding_table[intput[i++]];

		uint32_t triple = (sextet_a << 3 * 6)
			+ (sextet_b << 2 * 6)
			+ (sextet_c << 1 * 6)
			+ (sextet_d << 0 * 6);

		if (j < *output_length) output[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *output_length) output[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *output_length) output[j++] = (triple >> 0 * 8) & 0xFF;
	}

	output[*output_length] = '\0';

	return output;
}

int isX = 0;
char* inputPath = NULL;
char* outputPath = NULL;
int encodeSize = 60;
int decodeSize = 80;

void printHelp() {
	printf(
		"encode usage:\n\
bin2txt -i inputPath -o outputPath\n\
decode usage:\n\
bin2txt -x -i inputPath -o outputPath\n");
}

char* extract_file_name(char* path)
{
	size_t len = strlen(path);
	for (size_t i = len - 1; i > 0; i--)
	{
		if (path[i] == '\\' || path[i] == '/')
		{
			path = path + i + 1;
			break;
		}
	}
	return path;
}

int main(int argc, char* argv[]) {
	char* inputBuffer;
	char* outputBuffer;

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-x") == 0) {
			isX = 1;
		}
		else if (strcmp(argv[i], "-i") == 0) {
			if (i + 1 == argc)
				break;
			inputPath = argv[i + 1];
		}
		else if (strcmp(argv[i], "-o") == 0) {
			if (i + 1 == argc)
				break;
			outputPath = argv[i + 1];
		}
		else if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 == argc)
				break;
			encodeSize = atoi(argv[i + 1]);
			decodeSize = 4 * ((encodeSize + 2) / 3);
		}
	}
	if (inputPath == NULL || outputPath == NULL) {
		printf("inputPath and outputPath must set\n");
		printHelp();
		exit(1);
	}
	if (isX == 0) {
		inputBuffer = malloc((size_t)encodeSize + 1);
		if (inputBuffer == 0)
			return 2;

		outputBuffer = malloc((size_t)decodeSize + 1);
		if (outputBuffer == 0)
			return 3;

		char* fn = extract_file_name(inputPath);

		FILE* fw = fopen(outputPath, "w");
		FILE* fr = fopen(inputPath, "rb");

		while (1) {
			size_t size = fread(inputBuffer, 1, encodeSize, fr);
			if (size == 0)
				break;
			size_t outsize;
			base64_encode(inputBuffer, size, outputBuffer, &outsize);
			fprintf(fw, "%s\n", outputBuffer);
		}

		fclose(fr);
		fclose(fw);
	}
	else {
		inputBuffer = malloc((size_t)decodeSize + 1);
		if (inputBuffer == 0)
			return 4;
		outputBuffer = malloc((size_t)encodeSize + 1);
		if (outputBuffer == 0)
			return 5;

		build_decoding_table();

		FILE* fr = fopen(inputPath, "r");
		FILE* fw = fopen(outputPath, "wb");

		while (fgets(inputBuffer, decodeSize+1, fr)) {
			size_t len = strlen(inputBuffer);
			if (inputBuffer[len - 1] == '\n') {
				inputBuffer[len - 1] = 0;
				len--;
			}
			size_t outsize;
			base64_decode(inputBuffer, len, outputBuffer, &outsize);
			fwrite(outputBuffer, 1, outsize, fw);
		}

		fclose(fw);
		fclose(fr);
	}
}
