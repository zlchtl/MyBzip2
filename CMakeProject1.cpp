#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <csignal>
#include <bzlib.h>

using namespace std;
namespace fs = std::filesystem;

const int BLOCK_SIZE = 1024 * 1024; // 1 МБ

volatile sig_atomic_t mSIG_STOP = 0;

void signalHandler(int signum) {
	if (signum == SIGINT) {
		mSIG_STOP = 1;
	}
}

void printProgressBar(int percent, int width = 50) {
	int pos = percent * width / 100;
	cout << "\r[";
	for (int i = 0; i < width; ++i) {
		if (i <= pos) cout << "#";
		else cout << "-";
	}
	cout << "] " << percent << "% " << flush;
}

bool compress(string iFile, string oFile) {
	vector<char> buffer(BLOCK_SIZE);
	int error = BZ_OK;
	bool success = false;

	FILE* out = nullptr;
	BZFILE* bzFile = nullptr;

	ifstream in(iFile, ios::binary);
	if (!in) {
		cerr << "Error: Cannot open input file\n";
		return false;
	}

	uintmax_t tSize = fs::file_size(iFile);
	uintmax_t pSize = 0;
	int lastPercent = -1;

	out = fopen(oFile.c_str(), "wb");
	if (!out) {
		cerr << "Error: Cannot create output file\n";
		in.close();
		return false;
	}

	bzFile = BZ2_bzWriteOpen(&error, out, 9, 0, 30);
	if (error != BZ_OK) {
		cerr << "Error: BZ2_bzWriteOpen failed\n";
		fclose(out);
		in.close();
		return false;
	}

	success = true;
	while (success && in && !mSIG_STOP) {
		in.read(buffer.data(), BLOCK_SIZE);
		int bytesRead = in.gcount();
		pSize += bytesRead;

		if (bytesRead > 0) {
			BZ2_bzWrite(&error, bzFile, buffer.data(), bytesRead);
			if (error != BZ_OK) {
				cerr << "Error: Compression failed\n";
				success = false;
				break;
			}

			int percent = static_cast<int>((pSize * 100) / tSize);
			if (percent != lastPercent) {
				printProgressBar(percent);
				lastPercent = percent;
			}
		}
	}

	if (bzFile) {
		BZ2_bzWriteClose(&error, bzFile, (!success || mSIG_STOP) ? 0 : 1, NULL, NULL);
	}
	if (out) {
		fclose(out);
	}
	in.close();

	if (mSIG_STOP) {
		cerr << "\nInterrupted by user\n";
		fs::remove(oFile);
		return false;
	}

	printProgressBar(100);

	return success && (error == BZ_OK);
}

bool uncompress(string iFile, string oFile) {
	vector<char> buffer(BLOCK_SIZE);
	int error = BZ_OK;
	bool success = false;

	FILE* in = nullptr;
	BZFILE* bzFile = nullptr;

	uintmax_t tSize = fs::file_size(iFile);
	uintmax_t pSize = 0;
	int lastPercent = -1;

	in = fopen(iFile.c_str(), "rb");
	if (!in) {
		cerr << "Error: Cannot open input file\n";
		return false;
	}

	ofstream out(oFile, ios::binary);
	if (!out) {
		cerr << "Error: Cannot create output file\n";
		fclose(in);
		return false;
	}

	bzFile = BZ2_bzReadOpen(&error, in, 0, 0, NULL, 0);
	if (error != BZ_OK) {
		cerr << "Error: BZ2_bzReadOpen failed\n";
		fclose(in);
		return false;
	}

	success = true;
	while (success && error == BZ_OK && !mSIG_STOP) {
		int bytesRead = BZ2_bzRead(&error, bzFile, buffer.data(), BLOCK_SIZE);
		pSize += bytesRead;

		if (error == BZ_OK || error == BZ_STREAM_END) {
			out.write(buffer.data(), bytesRead);
			if (!out) {
				cerr << "Error writing to output file\n";
				success = false;
				break;
			}

			int percent = static_cast<int>((pSize * 100) / tSize);
			if (percent > 100) percent = 100;
			if (percent != lastPercent) {
				printProgressBar(percent);
				lastPercent = percent;
			}
		}
	}

	success = success && (error == BZ_STREAM_END);

	if (bzFile) {
		BZ2_bzReadClose(&error, bzFile);
	}
	fclose(in);
	out.close();

	if (mSIG_STOP) {
		cerr << "\nInterrupted by user\n";
		fs::remove(oFile);
		return false;
	}

	printProgressBar(100);

	return success;
}

int main(int argc, char* argv[]){
	std::signal(SIGINT, signalHandler);
	if (argc != 4){
		cout << "Usage:\n"
			<< "  " << argv[0] << " a <input_file> <output_file> (compress)\n"
			<< "  " << argv[0] << " e <input_file> <output_file> (extract)\n";
		return -1;
	}

	string action = argv[1];
	string inputFile = argv[2];
	string outputFile = argv[3];

	if (action == "a"){
		if (compress(inputFile, outputFile)){
			cout << "Done: compressed\n";
		} else {
			cerr << "Error: compression failed\n";
		}
	}
	else if (action == "e"){
		if (uncompress(inputFile, outputFile)){
			cout << "Done: uncompressed\n";
		} else {
			cerr << "Error: uncompression failed\n";
		}
	}
	else {
		cerr << "Error: unknown action: " << action << "('a' or 'e')\n";
		return -1;
	}
	return 0;
}
