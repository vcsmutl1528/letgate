/*
   +----------------------------------------------------------------------+
   | ������� ��� ��������� ������ �����                                   |
   +----------------------------------------------------------------------+
   | 2009, https://github.com/vcsmutl1528/letgate                         |
   +----------------------------------------------------------------------+

  ������� ������������� ��� ��������� ������ ����� � ���������� ��� ��������
���������� ��������� ��� ������������� � ��������� ������.
  ������������� �������� ������������ �� ����� ���������:
letgate addroutes.bat ,
  ��� addroutes.bat - ��������� ���� ��������� �������������.

  ������:
letgate [/v<���_����������>] [/e] [/a] [--] ��������� [���������]
  <���_����������> - ���������� �������� ������ ���������� ��������� ��� �������� ����������
		(��� �� ��������� - GATEWAY)
  /e - ��������� ���������� ��������� (Ethernet/FDDI) ��� ������������ (PPP/SLIP)
  /a - �� ��������� ��������� ���� ������ ��������� (��� ���������������)

  ��������� ����� ���� ������ ������������ ������ ��� �������� ������, �� �� ����� ����
�������� ��������������.
  � ����� � ���, ��� �� ������� ������� �������� ����������� ���������� �����������,
�� ������ �������� ����������� ����������� �������� ������.

  ��� ����������:
  0 - �������� ����������
  1 - ������� ���������� ���������� ��������
  2 - ������� ���������� ���������� �������� (��������� �� ��������� - ���� /a)
  9 - ������ ��� ���������� �������
 10 - ��������� ����������
256 - ������������ ��������� ������

*/

#include <stdio.h>
#include <process.h>
#include <windows.h>
#include <iphlpapi.h>

#define MAX_ADAPTERS 100

IP_ADAPTER_INFO pAdapters [MAX_ADAPTERS];

int f_e = 0;
int f_a = 0;
char defenvar [] = "GATEWAY";
char *envar = defenvar;

char **cmd = NULL;

char aDesc [MAX_ADAPTER_DESCRIPTION_LENGTH + 4];

LPSTR GetSysErrorString (DWORD dwVal);

int main (int argc, char *argv[])
{
	PIP_ADAPTER_INFO pAdapter;
	DWORD r;
	ULONG ul;
	int i, arg_err = 0, envset = 0, help_req = 0;
	int gnum = 0, ghw = 0;
	char *sc, *adesc, *curg;

	for (i=1; i<argc; i++)
		if (strcmp (argv [i], "--") == 0)
		{
			cmd = &argv[i+1];
			break;
		}

	if (cmd == NULL || cmd != NULL && cmd [0] != NULL)
	for (i=1; i<argc; i++)
	{
		if (cmd != NULL && &argv[i] >= &cmd[-1])
			break;
		if (argv[i][0] == '-' || argv[i][0] == '/')
		{
			sc = argv[i] + 1;
			if (strnicmp (sc, "v", 1) == 0)
			{
				if (envset == 0)
				{
					if (sc[1] == '\0')
					{
						i++;
						if (argv[i] == NULL || cmd != NULL && &argv[i] >= &cmd[-1])
						{
							printf ("�訡��: ��� ��६����� �� ������.\n");
							arg_err = 1;
							continue;
						}
						envar = argv[i];
						envset = 1;
					} else
					{
						envar = &sc[1];
						envset = 1;
					}
					continue;
				} else
				{
					printf ("�訡��: ��� ��६����� 㦥 ������.\n");
					arg_err = 1;
					continue;
				}
			}
			if (stricmp (sc, "e") == 0)
			{
				f_e = 1;
				continue;
			}
			if (stricmp (sc, "a") == 0)
			{
				f_a = 1;
				continue;
			}
			if (stricmp (sc, "h") == 0 || stricmp (sc, "help") == 0 ||
				strcmp (sc, "?") == 0)
			{
				help_req = 1;
				continue;
			}
		}
		if (cmd != NULL)
		{
			printf ("�訡��: ��������� ��ࠬ���: '%s'\n", argv[i]);
			arg_err = 1;
			continue;
		} else
		{
			cmd = &argv[i];
			break;
		}
	}
	if ((cmd == NULL || cmd != NULL && cmd[0] == NULL) && help_req == 0)
	{
		printf ("�訡��: ��� �ணࠬ�� ���������.\n");
		arg_err = 1;
	}
	if (arg_err)
	{
		printf ("�訡�� ��������� ��ப�.\n");
		help_req = 1;
	}
	if (help_req)
	{
		sc = strrchr (argv [0], '\\');
		if (sc == NULL)
			sc = argv [0];
		else	sc++;
		printf ("����祭�� ���� � � ��᢮���� ���祭�� ��६����� ���㦥���\n"
			"� ��᫥���騬 ����᪮� �ணࠬ�� �� �奬� ���몠���.\n"
			"��ଠ�: %s [/v<���_��६�����>] [/e] [/a] [--] ������� [��ࠬ����]\n"
			"  /e\t- �ਮ��� �������� �����஢ (Ethernet/FDDI) ���\n"
			"\t  ����㠫�묨 (PPP/SLIP)\n"
			"  /a\t- �� �믮����� �ணࠬ�� �᫨ �� ��᪮�쪮 (���������筮���)\n"
			"��� ��६����� �� 㬮�砭�� - %s\n"
			"��� �ணࠬ�� ����� ���� �ᯮ����� ���㫥� ��� ������ 䠩���, �� ��\n"
			"�������� ��������.\n"
			"��� �����襭��:\n"
			"   0 - �ᯥ譮� �����襭��\n"
			"   1 - ����稥 ��᪮�쪨� �����⨬�� ���祭��\n"
			"   2 - ����稥 ��᪮�쪨� �����⨬�� ���祭�� (�ணࠬ�� �� �믮�����, /a)\n"
			"   9 - �訡�� �� ����᪥ �ணࠬ��\n"
			"  10 - ���਩��� �����襭��\n"
			" 256 - �������⨬�� ��������� ��ப�\n", sc, defenvar);
		return 256;
	}

	ul = sizeof (pAdapters);
	r = GetAdaptersInfo (pAdapters, &ul);
	if (r == ERROR_NO_DATA)
	{
		printf ("��⨢��� �⥢�� �����஢ �� �����㦥��.\n");
		return 10;
	}
	if (r != ERROR_SUCCESS)
	{
		printf ("�訡�� GetAdaptersInfo (): %s.\n", GetSysErrorString (r));
		return 10;
	}

#define IS_HARDWARE_MIB_IF_TYPE(t) ((t==MIB_IF_TYPE_ETHERNET)||(t==MIB_IF_TYPE_FDDI)||(t==MIB_IF_TYPE_TOKENRING))

	for (pAdapter = pAdapters; pAdapter != NULL; pAdapter = pAdapter->Next)
	{
		if (strlen (pAdapter->GatewayList.IpAddress.String) != 0)
		{
			i = IS_HARDWARE_MIB_IF_TYPE(pAdapter->Type);
			if (ghw && !i)
				continue;
			if (i && !ghw)
			{
				gnum = 0;
				ghw = 1;
			}
			if (gnum == 0)
			{
				adesc = pAdapter->Description;
				curg = pAdapter->GatewayList.IpAddress.String;
			}
			gnum ++;
			if (pAdapter->GatewayList.Next != NULL)
				gnum ++;
		}
	}
	if (gnum == 0)
	{
		printf ("��⨢��� �� �� �����㦥��.\n");
		return 10;
	}
	if (f_a && gnum > 1)
	{
		printf ("�訡��: ����稥 ��᪮�쪨� �� (���������筮���).\n");
		return 2;
	}
	CharToOem (adesc, aDesc);
	printf ("������: %s\n�᭮���� ��: %s\n", aDesc, curg);
	SetEnvironmentVariable (envar, curg);
	printf ("%s=%s\n", envar, curg);

	i = _spawnvp (_P_WAIT, cmd[0], &cmd[0]);
	if (i == -1)
	{
		printf ("�訡�� �� �믮������ �ணࠬ�� '%s' (%d): %s",
			cmd[0], errno, strerror (errno));
		return 9;
	}

	return gnum > 1 ? 1 : 0;
}

#define SYSERRORSTRINGLEN 1024

DWORD adwSysErrorCodes [] =
{
	ERROR_SUCCESS,
	ERROR_BUFFER_OVERFLOW,
	ERROR_INVALID_DATA,
	ERROR_INVALID_PARAMETER,
	ERROR_NO_DATA,
	ERROR_NOT_SUPPORTED
};

LPSTR aszSysErrorStrings [] =
{
	"ERROR_SUCCESS",
	"ERROR_BUFFER_OVERFLOW",
	"ERROR_INVALID_DATA",
	"ERROR_INVALID_PARAMETER",
	"ERROR_NO_DATA",
	"ERROR_NOT_SUPPORTED"
};

char szSysError [SYSERRORSTRINGLEN];

LPSTR GetSysErrorString (DWORD dwVal)
{
	LPVOID lpMsgBuf;
	char szTemp [256], *p;
	int i, l;

	szSysError [SYSERRORSTRINGLEN-1] = '\0';
	if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL ))
	{
		strncpy (szSysError, lpMsgBuf, SYSERRORSTRINGLEN-1);
		LocalFree (lpMsgBuf);
		CharToOem (szSysError, szSysError);
	}

	l = strlen (szSysError);
	if (l > 0)
	{
		p = szSysError + l-1;
		while (*p == '.' || *p == '\r' || *p == '\n')
			p--;
		p[1] = '\0';
	}
	l = strlen (szSysError);
	if (l >= SYSERRORSTRINGLEN-1)
		return szSysError;
	for (i=0; i<sizeof (adwSysErrorCodes)/sizeof(adwSysErrorCodes[0]); i++)
		if (adwSysErrorCodes [i] == dwVal) break;
	if (i < sizeof (adwSysErrorCodes)/sizeof(adwSysErrorCodes[0]))
		sprintf (szTemp, " (%#010x - %s)", dwVal, aszSysErrorStrings [i]);
	else
		sprintf (szTemp, " (%#010x)", dwVal);
	strncpy (szSysError + l, szTemp, SYSERRORSTRINGLEN-l-1);
	return szSysError;
}
