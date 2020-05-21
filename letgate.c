/*
   +----------------------------------------------------------------------+
   | Утилита для получения адреса шлюза                                   |
   +----------------------------------------------------------------------+
   | 2009, https://github.com/vcsmutl1528/letgate                         |
   +----------------------------------------------------------------------+

  Утилита предназначена для получения адреса шлюза и присвоения его значения
переменной окружения для использования в командных файлах.
  Использование значения производится по схеме замыкания:
letgate addroutes.bat ,
  где addroutes.bat - командный файл настройки маршрутизации.

  Формат:
letgate [/v<имя_переменной>] [/e] [/a] [--] программа [параметры]
  <имя_переменной> - присвоение значения данной переменной окружения при успешном выполнении
		(имя по умолчанию - GATEWAY)
  /e - приоритет аппаратных адаптеров (Ethernet/FDDI) над виртуальными (PPP/SLIP)
  /a - не выполнять программу если шлюзов несколько (при неоднозначности)

  Программа может быть именем исполняемого модуля или пакетным файлом, но не может быть
командой интерпретатора.
  В связи с тем, что на рабочей станции возможно присутствие нескольких интерфейсов,
не всегда возможно однозначное определение значения адреса.

  Код завершения:
  0 - успешное завершение
  1 - наличие нескольких допустимых значений
  2 - наличие нескольких допустимых значений (программа не выполнена - ключ /a)
  9 - ошибка при выполнении команды
 10 - аварийное завершение
256 - недопустимая командная строка

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
							printf ("ЋиЁЎЄ : €¬п ЇҐаҐ¬Ґ­­®© ­Ґ § ¤ ­®.\n");
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
					printf ("ЋиЁЎЄ : €¬п ЇҐаҐ¬Ґ­­®© г¦Ґ § ¤ ­®.\n");
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
			printf ("ЋиЁЎЄ : ЌҐЁ§ўҐбв­л© Ї а ¬Ґва: '%s'\n", argv[i]);
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
		printf ("ЋиЁЎЄ : €¬п Їа®Ја ¬¬л ®вбгвбвўгҐв.\n");
		arg_err = 1;
	}
	if (arg_err)
	{
		printf ("ЋиЁЎЄ  Є®¬ ­¤­®© бва®ЄЁ.\n");
		help_req = 1;
	}
	if (help_req)
	{
		sc = strrchr (argv [0], '\\');
		if (sc == NULL)
			sc = argv [0];
		else	sc++;
		printf ("Џ®«гзҐ­ЁҐ  ¤аҐб  и«о§  Ё ЇаЁбў®Ґ­ЁҐ §­ зҐ­Ёп ЇҐаҐ¬Ґ­­®© ®Єаг¦Ґ­Ёп\n"
			"б Ї®б«Ґ¤гойЁ¬ § ЇгбЄ®¬ Їа®Ја ¬¬л Ї® беҐ¬Ґ § ¬лЄ ­Ёп.\n"
			"”®а¬ в: %s [/v<Ё¬п_ЇҐаҐ¬Ґ­­®©>] [/e] [/a] [--] Є®¬ ­¤  [Ї а ¬Ґвал]\n"
			"  /e\t- ЇаЁ®аЁвҐв  ЇЇ а в­ле  ¤ ЇвҐа®ў (Ethernet/FDDI) ­ ¤\n"
			"\t  ўЁавг «м­л¬Ё (PPP/SLIP)\n"
			"  /a\t- ­Ґ ўлЇ®«­пвм Їа®Ја ¬¬г Ґб«Ё и«о§®ў ­ҐбЄ®«мЄ® (­Ґ®¤­®§­ з­®бвм)\n"
			"€¬п ЇҐаҐ¬Ґ­­®© Ї® г¬®«з ­Ёо - %s\n"
			"€¬п Їа®Ја ¬¬л ¬®¦Ґв Ўлвм ЁбЇ®«­Ё¬л¬ ¬®¤г«Ґ¬ Ё«Ё Ї ЄҐв­л¬ д ©«®¬, ­® ­Ґ\n"
			"Є®¬ ­¤®© Ё­вҐаЇаҐв в®а .\n"
			"Љ®¤ § ўҐаиҐ­Ёп:\n"
			"   0 - гбЇҐи­®Ґ § ўҐаиҐ­ЁҐ\n"
			"   1 - ­ «ЁзЁҐ ­ҐбЄ®«мЄЁе ¤®ЇгбвЁ¬ле §­ зҐ­Ё©\n"
			"   2 - ­ «ЁзЁҐ ­ҐбЄ®«мЄЁе ¤®ЇгбвЁ¬ле §­ зҐ­Ё© (Їа®Ја ¬¬  ­Ґ ўлЇ®«­Ґ­ , /a)\n"
			"   9 - ®иЁЎЄ  ЇаЁ § ЇгбЄҐ Їа®Ја ¬¬л\n"
			"  10 -  ў аЁ©­®Ґ § ўҐаиҐ­ЁҐ\n"
			" 256 - ­Ґ¤®ЇгбвЁ¬ п Є®¬ ­¤­ п бва®Є \n", sc, defenvar);
		return 256;
	}

	ul = sizeof (pAdapters);
	r = GetAdaptersInfo (pAdapters, &ul);
	if (r == ERROR_NO_DATA)
	{
		printf ("ЂЄвЁў­ле бҐвҐўле  ¤ ЇвҐа®ў ­Ґ ®Ў­ аг¦Ґ­®.\n");
		return 10;
	}
	if (r != ERROR_SUCCESS)
	{
		printf ("ЋиЁЎЄ  GetAdaptersInfo (): %s.\n", GetSysErrorString (r));
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
		printf ("ЂЄвЁў­ле и«о§®ў ­Ґ ®Ў­ аг¦Ґ­®.\n");
		return 10;
	}
	if (f_a && gnum > 1)
	{
		printf ("ЋиЁЎЄ : Ќ «ЁзЁҐ ­ҐбЄ®«мЄЁе и«о§®ў (­Ґ®¤­®§­ з­®бвм).\n");
		return 2;
	}
	CharToOem (adesc, aDesc);
	printf ("Ђ¤ ЇвҐа: %s\nЋб­®ў­®© и«о§: %s\n", aDesc, curg);
	SetEnvironmentVariable (envar, curg);
	printf ("%s=%s\n", envar, curg);

	i = _spawnvp (_P_WAIT, cmd[0], &cmd[0]);
	if (i == -1)
	{
		printf ("ЋиЁЎЄ  ЇаЁ ўлЇ®«­Ґ­ЁЁ Їа®Ја ¬¬л '%s' (%d): %s",
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
