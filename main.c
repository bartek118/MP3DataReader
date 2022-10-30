#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dirent.h>

struct songProperties {
	char* title;
};

void initializeSongProperties(struct songProperties* sp) {
	sp->title = NULL;
}

void printSongData(const struct songProperties* sp) {
	if(sp->title !=NULL) {
		printf("Title: \t\t\t%s\n", sp->title);
	}
}

// returns 1 if filename ends with .mp3
int ifMP3Extension(const char filename[]) {
	char* findDot = strrchr(filename, '.');
	
	if(findDot && !strcmp(findDot, ".mp3")) {
		return 1;
	}

	return 0;
}

// we assume that the frame array has at least 10 bytes (the header); there is no checking
void handleID3frame(const char* frame, struct songProperties* sp) {
	unsigned int frameSize = frame[7] + 128*frame[6] + 128*128*frame[5] + 128*128*128*frame[4];
	char frameID[5];

	strncpy(frameID, frame, 4);

	if(!strcmp(frameID, "TIT2")) {
		sp->title = (char*) calloc(frameSize, sizeof(char));
		memset(sp->title, 0, frameSize*sizeof(char));	

		strncpy(sp->title, frame+11, frameSize-1);

		/*printf("Frame ID: %c%c%c%c\n", frame[0], frame[1], frame[2], frame[3]);
		printf("ID: %s\n", frameID);
        	printf("Frame size: %u\n", frameSize);
	        printf("Frame flags: %hhu %hhu\n", frame[8], frame[9]);*/
	}
}

void readMP3Data(const char directory[], const char filename[]) {
	char* fullPath;					// full path to the file
	FILE* MP3file;					// file

	unsigned char ID3header[10];			// 10-bytes ID3 header
	unsigned int size = 0, i = 0, frameSize;	// size = size of the ID3 data

	unsigned char* ID3frames;			// array of all ID3frames

	struct songProperties* sp = NULL;

	fullPath = (char*) calloc(strlen(directory ) + strlen(filename) + 2, sizeof(char));
	fullPath[0] = 0;

	// create the full path
	strcat(fullPath, directory);
	strcat(fullPath, "/");
	strcat(fullPath, filename);

	// open the file
	MP3file = fopen(fullPath, "rb");

	// free the fullPath variable - no longer needed
	free(fullPath);
	fullPath = NULL;

	// if we cannot read the whole ID3 header, the file is corrupted
	if(fread(ID3header, 1, 10, MP3file) != 10) {
		printf("File corrupted.\n\n\n");
		fclose(MP3file);
		return;
	}

	// Works only for v2.3.0 for now
	printf("ID3 version: \t\t%c%c%cv2.%hhu.%hhu\n", ID3header[0], ID3header[1], ID3header[2], ID3header[3], ID3header[4]);

	if(ID3header[3] != 0x3 || ID3header[4] != 0) {
		printf("Only ID3v2.3.0 supported by now\n\n\n");
		fclose(MP3file);
		return;
	}

	// For a moment we don't support any flags
	if(ID3header[5]) {
		printf("We don't support extended headers, etc.\n\n\n");
		fclose(MP3file);
		return;
	}

	// Compute the size of all ID3 frames (in bytes)
	size = ID3header[9] + 128*ID3header[8] + 128*128*ID3header[7] + 128*128*128*ID3header[6];

	// Prepare memory for frames
	ID3frames = (unsigned char*) calloc(size, sizeof(unsigned char));

	// and read them
	if(fread(ID3frames, 1, size, MP3file) != size) {
		printf("File corrupted.\n\n\n");
		fclose(MP3file);
		return;
	}

	// close the file
	fclose(MP3file);
	MP3file = NULL;

	sp = (struct songProperties*) malloc(sizeof(struct songProperties));
	initializeSongProperties(sp);

	// we iterate through all the frame data
	while(i < size) {
		// compute the size of the frame
		frameSize = ID3frames[i+7] + 128*ID3frames[i+6] + 128*128*ID3frames[i+5] + 128*128*128*ID3frames[i+4];

		// handle the frame if it is not empty
		if(frameSize != 0) {
			handleID3frame((char*)ID3frames+i, sp);
		}
	
		// add the frame size and the header size (10)
		i += (10+frameSize);
	}

	printSongData(sp);

	printf("\n\n");

	// free the memory
	free(ID3frames);
	free(sp);
}

int main(int argc, char* argv[]) {
	DIR* dir;
	struct dirent* dirEnt;

	// current directory if not set in argv[]
	if(argc < 2) {
		dir = opendir(".");
	}
	else {
		dir = opendir(argv[1]);
	}

	// if directory successfully opened
	if(dir) {
		// read its content
		while( (dirEnt = readdir(dir)) ) {
			// if this is MP3 file
			if(ifMP3Extension(dirEnt->d_name) == 1) {
				printf("Filename: \t\t%s\n", dirEnt->d_name);
				
				if(argc < 2) {
					readMP3Data(".", dirEnt->d_name);
				}
				else {
					readMP3Data(argv[1], dirEnt->d_name);
				}

			}
		}
		closedir(dir);
	}

	return 0;
}
