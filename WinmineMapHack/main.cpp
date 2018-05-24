#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <locale.h>
#include <wchar.h>
#include <conio.h>

#define BUFFER_SIZE 1024
#define MAP_SIZE 0x360

typedef struct {
	BYTE MaxWidthX;
	BYTE MaXHeightY;
	BYTE MaxMineTheNumber;
}GameInfo;

const unsigned int ADR_MINE_MEMORY = 0x01005361;
const unsigned int ADR_MINE_MAX_COUNT = 0x010056A4;
const unsigned int ADR_WIDTH_X = 0x1005334;
const unsigned int ADR_HEIGHT_Y = 0x1005338;

const BYTE EMPTY_VALUE = 0x0F;
const BYTE MINE_VALUE = 0x8F;

const wchar_t * EMPTY_BOX = L"��";
const wchar_t * MINE_BOX = L"��";

const char *GameName = "winmine.exe";

DWORD GetPID(LPCSTR gamename) {
	char Buffer[BUFFER_SIZE] = "";   

	HANDLE ProcessSnapshot = INVALID_HANDLE_VALUE;

	DWORD ppid = 0xFFFFFFFF;   // winmine.exe�� PID ���� ��� ����
	BOOL ProcessInfo = FALSE;   // ���μ��� ������ ��� ����

	PROCESSENTRY32 pe;   // ���μ��� ��Ʈ��
	pe.dwSize = sizeof(PROCESSENTRY32);

	ProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);   // ���μ��� ����� ������ HANDLE ����

	ProcessInfo = Process32First(ProcessSnapshot, &pe);   // ���μ��� ��Ʈ���� �ڷḦ ������ ����

	while (ProcessInfo) {   // ���μ��� ��Ʈ���� �ڷᰡ ���ٸ� FALSE ��ȯ -> Break;
		WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, 100, Buffer, 100, NULL, NULL);   // ���� �������� .exe ������ �̸��� char������ ��ȯ�ϰ� Buffer�� ����
		if (!strcmp(gamename, Buffer)) {   // gamename�� Buffer�� ������ ����
			ppid = pe.th32ProcessID;   // ppid�� winmine.exe�� PID ����
			break;
		}
		ProcessInfo = Process32Next(ProcessSnapshot, &pe);   // Process32Next()��ƾ�� �̿��Ͽ� pe�� ���� ���μ����� ���������� �޾ƿ�
	}
	CloseHandle(ProcessSnapshot);
	return ppid;   // PID ��ȯ
}

void ReadMapInfo(HANDLE handle, GameInfo *MapInfo) {
	if (!ReadProcessMemory(handle, (LPCVOID)ADR_WIDTH_X, &MapInfo->MaxWidthX, 4, NULL))
	{
		printf("Error in ADR_WIDTH_X\n");
	}

	if (!ReadProcessMemory(handle, (LPCVOID)ADR_HEIGHT_Y, &MapInfo->MaXHeightY, 4, NULL))
	{
		printf("Error in ADR_WIDTH_Y\n");
	}
	if (!ReadProcessMemory(handle, (LPCVOID)ADR_MINE_MAX_COUNT, &MapInfo->MaxMineTheNumber, 4, NULL))
	{
		printf("Error in ADR_MINE_MAX_COUNT\n");
	}
}

void Hack(const BYTE *Map, const GameInfo MapInfo) {
	int i, count = 0;

	for (i = 0; i < MAP_SIZE; i++) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		if (Map[i] == EMPTY_VALUE) {   // 0x0F��� ��ڽ� ���
			wprintf(L"%ls", EMPTY_BOX);
			count++;
		}
		else if (Map[i] == MINE_VALUE) {   // 0x8F��� �� ���
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
			wprintf(L"%ls", MINE_BOX);
			count++;
		}
		else if (0x40 <= Map[i] && Map[i] <= 0x43) {   // ���ڰ� �ƴ� ���� ������ �� ������ �ڿ��� ���
			wprintf(L"%2d", Map[i] - 0x40);
			count++;
		}
		else if (Map[i] == 0x10)   // 0x10�̶�� ����
		{
			printf("\n");
			while (Map[++i] != 0x10) { continue; }
			
		}
		else { continue; }

		if (count == MapInfo.MaxWidthX * MapInfo.MaXHeightY) {   // count�� ���� ���̿� �������ٸ� break
			break;
		}
	}
}

int main() {
	system("title winmine.exe MapHack!!!");

	setlocale(LC_ALL, "Korean");   // ��ο� �ѱ��� ������ ���� ������� �۵����� �ʴ� ������ �ذ�

	HANDLE handle;
	DWORD PID;

	GameInfo MapInfo = { NULL, };
	BYTE Map[MAP_SIZE] = { NULL, };

	PID = GetPID(GameName);
	
	if (!(handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID)))
	{
		printf("�� Not open proecss! ��\n\n�� Not open proecss! ��\n\n�� Not open proecss! ��");
		getch();
	}

	while (1) {
		if (!ReadProcessMemory(handle, (LPCVOID)ADR_MINE_MEMORY, Map, MAP_SIZE, NULL))
		{
			printf("Error in ADR_MINE_MEMORY\n");
		}
		ReadMapInfo(handle, &MapInfo);

		printf("@@ Winmain.exe MapHack!!! @@\n\n");
		Hack(Map, MapInfo);
		printf("\n\n");
		printf("���� : %d��", MapInfo.MaxMineTheNumber);
		Sleep(500);
		system("cls");
	}
}