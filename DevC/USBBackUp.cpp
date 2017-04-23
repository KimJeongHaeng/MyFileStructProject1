#include <iostream>
#include <fstream>
//#include <cstdio>    	// fopen, fclose, fread, fwrite, BUFSIZ -> for copy alternative
#include <ctime>		
#include <io.h>			// _finddata_t
#include <cstring>
#include <windows.h>		//createDirectory
using namespace std;

#define PATH_SIZE 150 	//max length of file path.
//If use char*, sprintf makes error so I used char[]
//I know that sprintf is not good function and sprintf_s is better
//however it didn't work and I cou;dn't find that reason
typedef struct _finddata_t FileData;//It is provided by MS and it has 
									//unsigned attrib : what kind of file
									//time_t time_create : time it was created
									//time_t time_access : tim it was accessed recently
									//time_t time_write : time it was writed recently
									//_fsize_t size : byte
									//char name [ _MAX_PATH] : name

char thisTime[30];
	
char* timeShow() {
	time_t curr_time;
	struct tm *curr_tm;//struct for time

	curr_time = time(NULL);
	curr_tm = localtime(&curr_time);

	sprintf(thisTime, "%d.%d.%d %d:%d:%d", curr_tm->tm_year + 1900, curr_tm->tm_mon + 1,
		curr_tm->tm_mday, curr_tm->tm_hour, curr_tm->tm_min, curr_tm->tm_sec);

	return thisTime;
}

class BackUp {
public:
	BackUp(char* src, char* dest) {
		strcpy(backUpFrom, src);
		strcpy(backUpTo, dest);
	}

	void run() {
		makeFullDir(backUpTo);
		log.open(((string)backUpTo + "\\log.txt").c_str(), ios::app);
		log << "\n<Back Up from " << backUpFrom << " to " << backUpTo << ">\n";
		copyDir(backUpFrom, backUpTo);
	}

private:
	ofstream log;
	char backUpFrom[PATH_SIZE];
	char backUpTo[PATH_SIZE];
	
	void copyFile(char* srcPath, char* destPath) {
		FileData srcData, destData;//I want to check its last write-time
		_findfirst(srcPath, &srcData);
		_findfirst(destPath, &destData);

		if (srcData.attrib == _A_SUBDIR) {//_A_SUBDIR = srcData is Sub Directory
			if (destData.attrib == 0) {//Doesn't exist!
				CreateDirectory(destPath, NULL);//make directory			
			}
			copyDir(srcPath, destPath);//copy sub directory
		}
		else if (srcData.time_write > destData.time_write) {//srcData was written recently
			cout << "Start back-up " << srcPath << endl;
			if (CopyFile(srcPath, destPath, false)) {//CopyFile in windows.h return true if it successed
				cout << "Back-up " << destPath << " success!\n" << endl;
				log << timeShow() << "\t" << destPath << " back up successed!\n";
			}
			else {
				cout << "Back-up " << destPath << " failed...\n" << endl;
				log << timeShow() << "\t" << destPath << " back up failed..\n";
			}

			/*this is copy function without using windows.h
			BUFSIZ = 512 so I use 4kByte instead of BUFSIZ to copy big size files*/
			/*char buf[4096];
			size_t size;

			FILE* source = fopen(srcPath, "rb");
			FILE* dest = fopen(destPath, "wb");

			while (size = fread(buf, 1, 4096, source)) {
			fwrite(buf, 1, size, dest);
			} // copy

			fclose(source);
			fclose(dest);
			cout << "End Back-up to " << destPath << "\n" << endl;*/
		}
		else {
			cout << "Start back-up " << srcPath << endl;//just show that file is not changed
			cout << "It was not changed...\n" << endl;
			//log << timeShow() << "\t" << destPath << " back up ignored..\n";
		}
	}

	void copyDir(char* srcPath, char* destPath) {

		long h_file;//for getting list of files
		char search_Path[PATH_SIZE];
		FileData fileSrc;

		sprintf(search_Path, "%s/*.*", srcPath);//all file
		h_file = _findfirst(search_Path, &fileSrc); //fileSrc is .
		if (h_file == -1L) {
			cout << "Directory does not exist!!" << endl;

		}
		else {
			while (_findnext(h_file, &fileSrc) == 0) {//from this line, fileSrc is a file in directory
				char srcFilePath[PATH_SIZE];
				char destFilePath[PATH_SIZE];
				sprintf(srcFilePath, "%s\\%s", srcPath, fileSrc.name);//make path of file
				sprintf(destFilePath, "%s\\%s", destPath, fileSrc.name);
				if ((string)fileSrc.name != "..") {//if source directory is C:\, D:\, .... it doesn't have ..
												   //so i check the file is .. or not
					copyFile(srcFilePath, destFilePath);
				}
			}
			_findclose(h_file);
		}
	}

	void makeFullDir(char* path) {//make directory recursive for dir\new dir1\new dir2\new dir3\...
		FileData fullDir;
		_findfirst(path, &fullDir);
		if (fullDir.attrib != _A_SUBDIR) {
			char shortPath[PATH_SIZE];
			char *pp;
			pp = strrchr(path, '\\');
			if (pp != NULL) {
				FileData shortDir;
				strcpy(shortPath, path);
				shortPath[pp - path] = '\0';	//String function is better I think. 
												//But program has only one path
												//so, I think this is better than several .c_str
				_findfirst(shortPath, &shortDir);
				if (shortDir.attrib != _A_SUBDIR) {
					makeFullDir(shortPath);
				}
				CreateDirectory(path, NULL);
			}
		}
	}

};

int main(int argc, char** argv) {
	if (argc != 3) {//argument error
		cout << "Unvalid arguments!!" << endl;
		cout << "Please use this pattern" << endl;
		cout << "USBBackUp.exe [source directory path] [destination directory path]" << endl;
		return -1;
	}
	else {
		BackUp backUp(argv[1], argv[2]);
		backUp.run();
	}
	return 0;
}


