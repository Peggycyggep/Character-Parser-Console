// CharacterParserConsole.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"

typedef struct _CharacterList
{
	wchar_t wchar[10];
	struct _CharacterList *next;
} CHARLIST;

typedef struct CharactersMap
{
	CHARLIST *list;
	int count;
} CHARMAP;

CHARMAP *CHARMAP_Create(void)
{
	CHARMAP *map = new CHARMAP();
	map->list = 0;
	map->count = 0;
	return map;
}

void CHARMAP_Destroy(CHARMAP * map)
{
	CHARLIST * c = map->list;
	while (c)
	{
		map->list = map->list->next;
		delete c;
		c = map->list;
	}
	map->count = 0;
	delete map;
}

void CHARMAP_Add(CHARMAP *map, wchar_t *value)
{
	CHARLIST * NewChar = new CHARLIST();

	wcsncpy(NewChar->wchar, value, 10);
	NewChar->next = map->list;
	map->list = NewChar;
	map->count++;
}

bool CHARMAP_Check(CHARMAP *map, wchar_t *value)
{
	bool bFound = false;
	CHARLIST * c = map->list;
	while (c && (bFound==false))
	{
		if (wcsncmp(c->wchar, value, 1) == 0)
			bFound = true;
		c = c->next;
	}
	return bFound;
}

int toCodePoint(const char *u, int *out);

int main(int argn, char **args)
{
	char szFilename[1024];
	char szPage[1024];
	char szTag[100];
	int iTagCnt;
	FILE *pFile;
	bool bBody, bTitle;
	CHARMAP * pMap = CHARMAP_Create();
	FILE * pFileOut = 0;
	
	if (argn < 3)
	{
		printf("not enough parameter\n");
		return -1;
	}
	
	//pFileOut = fopen("C:\\Users\\Peggy\\Documents\\chinese\\Story\\Letters.txt", "wt, ccs=UTF-8");
	pFileOut = fopen(args[1], "wt, ccs=UTF-8");
	//sprintf(szFilename, "C:\\Users\\Peggy\\Documents\\chinese\\Story\\寶貝盒1-1.txt");
	sprintf(szFilename, args[2]);
	pFile = fopen(szFilename, "r");

	fread(szPage, sizeof(char), 1024, pFile);

	for (int i = 0; (i < 1024) && (szPage[i] != 0); )
	{
		if (szPage[i] == '[')
		{
			szTag[0] = 0;
			iTagCnt = 1;
			i++;
		}
		else if (szPage[i] == ']')
		{
			szTag[iTagCnt] = 0;
			i++;
			iTagCnt = 0;

			if (szTag[0] == 'S')
				bBody = 1;
			else if (szTag[0] == 'T')
				bTitle = 1;
			else if (szTag[0] == '/')
			{
				bBody = 0;
				bTitle = 0;
			}
		}
		else if (iTagCnt > 0)
		{
			szTag[iTagCnt-1] = szPage[i];
			i++;
			iTagCnt++;
		}
		else if (bBody == 1)
		{
			int code = 0;
			int len = 0;
			wchar_t wchar[10];

			len = toCodePoint(&szPage[i], (int *)&wchar[0]);
			if (len > 0)
			{
				i = i + len;
				if (CHARMAP_Check(pMap, wchar) == false)
				{
					CHARMAP_Add(pMap, wchar);
					fwrite(wchar, sizeof(wchar_t), len - 1, pFileOut);
				}
			}
			else
				i++;
		}
		else
			i++;
	}

	CHARMAP_Destroy(pMap);
	fclose(pFileOut);
	fclose(pFile);

    return 0;
}

/*
parameter:
	u	[in] input string to convert into code
	out	[out] output variable containing the code
return:
	int		number of character used when convert
*/
int toCodePoint(const char *u, int *out) {
	//int l = u.length();
	//if (l < 1) {
	//	return -1;
	//}
	int l = 0;

	unsigned char u0 = u[0];
	if (u0 >= 0 && u0 <= 127) {
		l = 1;
		*out = u0;
		return l;
	}

	unsigned char u1 = u[1];
	if (u0 >= 192 && u0 <= 223) {
		l = 2;
		*out = (u0 - 192) * 64 + (u1 - 128);
		return l;
	}

	if (u[0] == 0xed && (u[1] & 0xa0) == 0xa0) {
		*out = 0;
		return -1; //code points, 0xd800 to 0xdfff
	}

	unsigned char u2 = u[2];
	if (u0 >= 224 && u0 <= 239) {
		l = 3;
		*out = (u0 - 224) * 4096 + (u1 - 128) * 64 + (u2 - 128);
		return l;
	}

	unsigned char u3 = u[3];
	if (u0 >= 240 && u0 <= 247) {
		l = 4;
		*out = (u0 - 240) * 262144 + (u1 - 128) * 4096 + (u2 - 128) * 64 + (u3 - 128);
		return l;
	}

	return -1;
}

void toUTF8(int cp, char *ch) {
	//char ch[5] = { 0x00 };

	ch[0] = 0;
	if (cp <= 0x7F) {
		ch[0] = cp;
		ch[1] = 0;
	}
	else if (cp <= 0x7FF) {
		ch[0] = (cp >> 6) + 192;
		ch[1] = (cp & 63) + 128;
		ch[2] = 0;
	}
	else if (0xd800 <= cp && cp <= 0xdfff) {} // 無效區塊
	else if (cp <= 0xFFFF) {
		ch[0] = (cp >> 12) + 224;
		ch[1] = ((cp >> 6) & 63) + 128;
		ch[2] = (cp & 63) + 128;
		ch[3] = 0;
	}
	else if (cp <= 0x10FFFF) {
		ch[0] = (cp >> 18) + 240;
		ch[1] = ((cp >> 12) & 63) + 128;
		ch[2] = ((cp >> 6) & 63) + 128;
		ch[3] = (cp & 63) + 128;
		ch[4] = 0;
	}
}