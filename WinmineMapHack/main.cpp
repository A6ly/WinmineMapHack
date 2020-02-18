#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <locale.h>
#include <wchar.h>
#include <conio.h>

#define BUFFER_SIZE 512
#define MAP_SIZE 700

typedef struct {
	BYTE MaxWidthX;
	BYTE MaXHeightY;
	BYTE ALLMineCount;
}GameInfo;

const unsigned int ADR_MINE_MEMORY = 0x01005361;   // 지뢰찾기의 메모리 주소
const unsigned int ADR_MINE_ALL_COUNT = 0x010056A4;   // 전체 지뢰 갯수의 주소
const unsigned int ADR_WIDTH_X = 0x1005334;
const unsigned int ADR_HEIGHT_Y = 0x1005338;

const BYTE EMPTY_VALUE = 0x0F;   // 빈 박스의 메모리 값
const BYTE MINE_VALUE = 0x8F;   // 지뢰가 있는 박스의 메모리 값

const wchar_t * EMPTY_BOX = L"□";
const wchar_t * MINE_BOX = L"★";

const char *GameName = "winmine.exe";

DWORD GetPID(LPCSTR gamename) {   // 지뢰찾기 프로세스의 PID를 얻는 함수
	char Buffer[BUFFER_SIZE] = "";   

	HANDLE ProcessSnapshot = INVALID_HANDLE_VALUE;

	DWORD ppid = 0xFFFFFFFF;   // winmine.exe의 PID 값을 담는 변수
	BOOL ProcessInfo = FALSE;   // 프로세스 정보를 담는 변수

	PROCESSENTRY32 pe;   // 프로세스 엔트리
	pe.dwSize = sizeof(PROCESSENTRY32);

	ProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);   // 프로세스 목록을 얻어오는 HANDLE 정의

	ProcessInfo = Process32First(ProcessSnapshot, &pe);   // 프로세스 엔트리의 자료를 가져와 저장

	while (ProcessInfo) {   // 프로세스 엔트리에 자료가 없다면 FALSE 반환 -> Break;
		WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, 100, Buffer, 100, NULL, NULL);   // 현재 실행중인 .exe 파일의 이름을 char형으로 변환하고 Buffer에 저장
		if (!strcmp(gamename, Buffer)) {   // gamename과 Buffer가 같으면 실행
			ppid = pe.th32ProcessID;   // ppid에 winmine.exe의 PID 넣음
			break;
		}
		ProcessInfo = Process32Next(ProcessSnapshot, &pe);   // Process32Next()루틴을 이용하여 pe의 다음 프로세스를 스냅샷에서 받아옴
	}
	CloseHandle(ProcessSnapshot);
	return ppid;   // PID 반환
}

void ReadMapInfo(HANDLE handle, GameInfo *MapInfo) {   // MapInfo의 메모리를 읽어주는 함수
	ReadProcessMemory(handle, (LPCVOID)ADR_WIDTH_X, &MapInfo->MaxWidthX, 4, NULL);
	ReadProcessMemory(handle, (LPCVOID)ADR_HEIGHT_Y, &MapInfo->MaXHeightY, 4, NULL);
	ReadProcessMemory(handle, (LPCVOID)ADR_MINE_ALL_COUNT, &MapInfo->ALLMineCount, 4, NULL);
}

void Hack(const BYTE *Map, const GameInfo MapInfo) {   // 반복문을 통해 맵정보를 출력하는 함수
	int i, count = 0;

	for (i = 0; i < MAP_SIZE; i++) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		if (Map[i] == EMPTY_VALUE) {   // 0x0F라면 빈박스 출력
			wprintf(L"%ls", EMPTY_BOX);
			count++;
		}
		else if (Map[i] == MINE_VALUE) {   // 0x8F라면 별 출력
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
			wprintf(L"%ls", MINE_BOX);
			count++;
		}
		else if (0x40 <= Map[i] && Map[i] <= 0x43) {   // 지뢰가 아닌 곳을 눌렀을 때 나오는 자연수 출력
			wprintf(L"%2d", Map[i] - 0x40);
			count++;
		}
		else if (Map[i] == 0x10)   // 0x10이라면 개행
		{
			printf("\n");
			while (Map[++i] != 0x10) { 
				continue; 
			}
			
		}
		else { continue; }

		if (count == MapInfo.MaxWidthX * MapInfo.MaXHeightY) {   // count가 맵의 넓이와 같아진다면 break
			break;
		}
	}
}

int main() {
	system("title winmine.exe MapHack!!!");

	system("mode con cols=80 lines=25");

	setlocale(LC_ALL, "Korean");   // 경로에 한글이 있을때 파일 입출력이 작동하지 않는 문제를 해결

	HANDLE handle;
	DWORD PID;

	GameInfo MapInfo = { NULL, };
	BYTE Map[MAP_SIZE] = { NULL, };

	PID = GetPID(GameName);
	
	if (!(handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID)))   // handle에 지뢰찾기의 PID를 넣는다. 지뢰찾기의 PID의 값이 없다면 에러 메세지 출력
	{
		printf("※ Not open proecss! ※\n\n※ Not open proecss! ※\n\n※ Not open proecss! ※");
		getch();
	}

	while (1) {
		ReadProcessMemory(handle, (LPCVOID)ADR_MINE_MEMORY, Map, MAP_SIZE, NULL);   // Map의 메모리 읽기

		ReadMapInfo(handle, &MapInfo);

		printf("@@ Winmain.exe MapHack!!! @@\n\n");
		Hack(Map, MapInfo);
		printf("\n\n");
		printf("지뢰 : %d개", MapInfo.ALLMineCount);
		Sleep(1000);
		system("cls");
	}
}