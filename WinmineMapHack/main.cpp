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

const unsigned int ADR_MINE_MEMORY = 0x01005361;   // ����ã���� �޸� �ּ�
const unsigned int ADR_MINE_ALL_COUNT = 0x010056A4;   // ��ü ���� ������ �ּ�
const unsigned int ADR_WIDTH_X = 0x1005334;
const unsigned int ADR_HEIGHT_Y = 0x1005338;

const BYTE EMPTY_VALUE = 0x0F;   // �� �ڽ��� �޸� ��
const BYTE MINE_VALUE = 0x8F;   // ���ڰ� �ִ� �ڽ��� �޸� ��

const wchar_t * EMPTY_BOX = L"��";
const wchar_t * MINE_BOX = L"��";

const char *GameName = "winmine.exe";

DWORD GetPID(LPCSTR gamename) {   // ����ã�� ���μ����� PID�� ��� �Լ�
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

void ReadMapInfo(HANDLE handle, GameInfo *MapInfo) {   // MapInfo�� �޸𸮸� �о��ִ� �Լ�
	ReadProcessMemory(handle, (LPCVOID)ADR_WIDTH_X, &MapInfo->MaxWidthX, 4, NULL);
	ReadProcessMemory(handle, (LPCVOID)ADR_HEIGHT_Y, &MapInfo->MaXHeightY, 4, NULL);
	ReadProcessMemory(handle, (LPCVOID)ADR_MINE_ALL_COUNT, &MapInfo->ALLMineCount, 4, NULL);
}

void Hack(const BYTE *Map, const GameInfo MapInfo) {   // �ݺ����� ���� �������� ����ϴ� �Լ�
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
			while (Map[++i] != 0x10) { 
				continue; 
			}
			
		}
		else { continue; }

		if (count == MapInfo.MaxWidthX * MapInfo.MaXHeightY) {   // count�� ���� ���̿� �������ٸ� break
			break;
		}
	}
}

int main() {
	system("title winmine.exe MapHack!!!");

	system("mode con cols=80 lines=25");

	setlocale(LC_ALL, "Korean");   // ��ο� �ѱ��� ������ ���� ������� �۵����� �ʴ� ������ �ذ�

	HANDLE handle;
	DWORD PID;

	GameInfo MapInfo = { NULL, };
	BYTE Map[MAP_SIZE] = { NULL, };

	PID = GetPID(GameName);
	
	if (!(handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID)))   // handle�� ����ã���� PID�� �ִ´�. ����ã���� PID�� ���� ���ٸ� ���� �޼��� ���
	{
		printf("�� Not open proecss! ��\n\n�� Not open proecss! ��\n\n�� Not open proecss! ��");
		getch();
	}

	while (1) {
		ReadProcessMemory(handle, (LPCVOID)ADR_MINE_MEMORY, Map, MAP_SIZE, NULL);   // Map�� �޸� �б�

		ReadMapInfo(handle, &MapInfo);

		printf("@@ Winmain.exe MapHack!!! @@\n\n");
		Hack(Map, MapInfo);
		printf("\n\n");
		printf("���� : %d��", MapInfo.ALLMineCount);
		Sleep(1000);
		system("cls");
	}
}