#include "stm32f2xx_hal.h"
#include  "DispLayLcd.h"
#include	"main.h"
#include "Uart.h"
#include	"Key.h"
#include	"string.h"
#include	"WireLessCom.h"
#include	"Esp8266.h"
#include	"gsmsend.h"

#define APPLICATION_ADDRESS   (uint32_t)0x08000000 
#define USER_FLASH_END_ADDRESS        0x080FFFFF
/* Define the user application size */
#define USER_FLASH_SIZE   (USER_FLASH_END_ADDRESS - APPLICATION_ADDRESS + 1)
__IO	unsigned char	pinlcddata[4][41];
unsigned char	noinit cursoraddress;
unsigned char	noinit flashcmd;
	const unsigned char	zoneedit[maxLanguage][40]={{"Use ENT into text input"},{"Use ENT na entrada de texto"},{"Use ENT en la entrada de texto"}};
	const unsigned char	zoneedit1[maxLanguage][26]={{"Input zone text"},{"Texto zona de entrada"},{"Entrada de texto de zona"}};
	const unsigned char	deviceedit[maxLanguage][32]={{"Use ENT into text input"},{"Use entrar entrada de texto"},{"Use ENT en la entrada de texto"}};
	const unsigned char	deviceedit1[maxLanguage][29]={{"Input device text"},{"Texto dispositivo de entrada"},{"Texto del disp. de entrada"}};
	const unsigned char  csh[maxLanguage][29]={{"ADDRESSABLE PANEL V3.2.2"},{'P','A','I','N','E','L',' ','E','N','D','E','R','E',0xc4,'A','V','E','L',' ',' ','V','3','.','2','.','2'},{"PANEL DIRECCIONABLE V3.2.2"}};
		// 3.1.1->3.1.2  增加了链接云的功能，并且wifi设置的显示有所调整
	//   3.1.2->3.1.3  根据6001的最新程序进行了一次修正一些可能会有问题的地方
	// 增加了8-4 读取EMIE 号的内容，同时可以检测模块通信是否异常
	//uart eeprom displaylcd key 文件，修改
	//le TEST queve 可能显示不对的问题
		//TCP  连接的keepalive 值设置为0原先是5，否则经常会自动重连导致TCP 不停地上下线
		//log 最大显示到65535
		//2020 0505  修改了netid 寻址时候可能出现无法通讯问题，最低字节原先比较位置出错 版本仍旧为3.1.3因为之前没有出去过
		//2020 0509  修改了监控的报文，增加了loop device ,zone text的校验，变化时，服务器会及时读取，同时修改了读取device zone text的报文，
		//一次的读取量由服务器控制，但是主机控制最大量，以防数据一出
		// 2020 1020  修改了uart中ReadSounderData 函数里面，24Sounder 编号23 手误成13
//const unsigned char  csh[2][25]={{"6002 V1.0.0"},{"6002 V1.0.0"}};
//3.2.0 有新的无线功能
//3.2.2 5-1-2菜单中 使用loop加减时候读取的deviceno内容将出错，读取时候缺少+1
	 const unsigned char csh1[maxLanguage][41]={{"INITIALISING"},{"INICIALIZAR"},{"INICIALIZANDO"}};
	const  unsigned char qingshur[maxLanguage][41]={{"** Access Denied **"},{"** Accesso Negado **"},{"** Acc Denegado **"}};
	const  unsigned char qingshur1[maxLanguage][41]={{"Press ENTER to get the code entry prompt"},{'P','r','e','s','s','i','o','n','e',' ','E','N','T','E','R',' ','e','i','n','t','r','o','u','d','u','z','a',' ','o',' ','c',0x8d,'d','i','g','o'},
		{'P','u','l','s','e',' ','E','N','T','E','R',' ','p' ,'/',' ','o','b','t',' ','s','o','l','i','c','i','t','u','d',' ','i','n','g',' ','d','e',' ', 'c',0x8d,'d','i'}};//超出40个字符
	const  unsigned char qingshur2[maxLanguage][2][41]={{{"Enter access code : ?????"},{"Enter engineer access code : ??????"}},
		{{'I','n','t','r','o','d','u','z','a',' ','c',0x8d,'d','i','g','o',' ','d','e',' ','a','c','e','s','s','o',' ',':',' ','?','?','?','?','?'},{'I','n','t','r','o','d','u','z','a',' ','c',0x8d,'d','i','g','o',' ','a','c','e','s','s','o','D','o',' ','E','N','G','E','.',' ',':',' ','?','?','?','?','?','?'}},
		{{'I','n','t','r','o','d',' ','c',0x8d,'d',' ','d','e',' ','a','c','c','e','s','o',' ',':',' ','?','?','?','?','?'},{'I','n','t','r','o','d',' ','c',0x8d,'d',' ','a','c','c','e','s','o',' ','i','n','g','e','n','i','e','r','o',' ',':',' ','?','?','?','?','?','?'}}};
const  unsigned char yijicaidan1[maxLanguage][32]={{"1     Review historic Log"},{"1     Rever historico eventos"},{'1',' ',' ',' ',' ',' ','R','e','v','i','s','i',0x8d,'n',' ','d','e',' ','R','e','g',' ','h','i','s','t',0X8D,'r','i','c','o'}};
	const  unsigned char yijicaidan2[maxLanguage][37]={{"2     Zones - disable & assign"},{"2     Zonas - desabilitar & atribuir"},{"2     Zonas - desactivar y asignar"}};
	const  unsigned char yijicaidan3[maxLanguage][39]={{"3     Sounders - disable & assign"},{"3     Sirenes - desabilitar & atribuir"},{"3     Sirenas - desactivar y asignar"}};
	const  unsigned char yijicaidan4[maxLanguage][38]={{"4     Input/Output - disable & assign"},{"4     E/S - desabilitar & atribuir"},{"4     E/S - desactivar y asignar"}};
	const  unsigned char yijicaidan5[maxLanguage][25]={{"5     Device set -up"},{"5     Dispositivos"},{"5     Config de dispots"}};	
	const  unsigned char yijicaidan6[maxLanguage][41]={{"6     Monitor device counts & test"},{'6',' ',' ',' ',' ',' ','V','i','s','u','a','l','i','z','a',0xc4,'a','o',' ','d','e',' ','d','i','s','p','.',' ','&',' ','t','e','s','t','e'},{"6     Monitor recuentos dispts y prueba"}};
	const  unsigned char yijicaidan7[maxLanguage][14]={{"7     General"},{"7     Geral"},{"7     General"}};
	const unsigned char  yijicaidan8[maxLanguage][28]={{"8     Manufacture test"},{"8     Teste de fabrica"},{'8',' ',' ',' ',' ',' ','P','r','u','e','b','a',' ','d','e',' ','f','a','b','r','i','c','a','c','i',0X8d,'n'}};
		const  unsigned char RunUser0[maxLanguage][24]={{"ACCESS LEVEL 1"},{'N',0xA3,'V','E','L',' ','A','C','E','S','S','O',' ','1'},{"NIVEL DE ACC USUARIO 1"}};
	const  unsigned char RunUser1[maxLanguage][22]={{"USER ACCESS LEVEL"},{'N',0xA3,'V','E','L',' ','D','E',' ','A','C','E','S','S','O'},{"NIVEL DE ACC USUARIO"}};
	const  unsigned char help1[maxLanguage][36]={{"Select item using     and ENTER"},{"SELECIONE USANDO     E ENTER"},{"Selec elemento usando     y ENTER"}};
	const  unsigned char help2[maxLanguage][41]={{"Press ENTER to start test sounders"},{"Pressi. ENTER para iniciar sirenes teste"},{"Pulse ENTER p/ Inic La Prueba Sirenas"}};
	const  unsigned char help3[maxLanguage][31]={{"PRESS ENTER TO END TEST"},{"PRIMA ENTER PARA FIM DE TESTE"},{"Pulse Enter P/ Final La Prueba"}};
	const  unsigned char RunUser2[maxLanguage][27]={{"INSTALLER ACCESS LEVEL"},{"NIVEL ACESSO DO INSTALADOR"},{"NIVEL ACC INSTALADOR"}};
	const  unsigned char RunUser3[maxLanguage][27]={{"ENGINEER ACCESS LEVEL"},{"NIVEL ACESSO DO ENGENHEIRO"},{"NIVEL ACC INGENIERO"}};
	//  1-xx
		const  unsigned char erjicaidan1_1[maxLanguage][40]={{"1-1   Display historic log"},{"1-1   Historico de eventos no mostrador"},{'1','-','1',' ',' ',' ','M','o','s','t','r','a','r',' ','r','e','g',' ','h','i','s','t',0X8D,'r','i','c','o'}};
	const  unsigned char erjicaidan1_2[maxLanguage][40]={{"1-2   Read/Clear autostart count"},{"1-2   Ler/Apagar contagem decrescente"},{"1-2   Leer/Borr recuento inicio aut"}};
	const  unsigned char erjicaidan1_denied[maxLanguage][40]={{"     ACCESS DENIED     PRESS ESCAPE"},{"   ACESSO NEGADO     PRESSIONE ESCAPE"},{"       ACC DENEGADO     PULSE ESC"}};
	const  unsigned char erjicaidan1_1_1[maxLanguage][36]={{"HELP: Display historic log"},{"AJUDA: Historico eventos no display"},{'A','Y','U','D','A',':',' ','M','o','s','t','r','a','r',' ','r','e','g',' ','h','i','s','t',0x8d,'r','c','o'}};
	const  unsigned char erjicaidan1_1_2[maxLanguage][34]={{"Use     to select the entry"},{"Use     para selecionar a entrada"},{"Use     p/ selecc la entrada"}};
	const  unsigned char erjicaidan1_2_1[maxLanguage][39]={{"Press ENTER to clear the count"},{"Pressione ENTER para limpar a contagem"},{"Press ENTER p/ borrar recuento"}};
	const  unsigned char erjicaidan1_2_2[maxLanguage][24]={{"AUTO-STARTUP COUNT ="},{"CONTAGEM DECRESCENTE ="},{"RECUENTO AUTO-STARTUP ="}};
	const  unsigned char erjicaidan1_2_3[maxLanguage][23]={{"READ ONLY ACCESS"},{"ACESSO SOMENTE LEITURA"},{"ACCESO S/ LECTURA"}};
	const  unsigned char erjicaidan1_1_1_1[maxLanguage][4][40]={
			{{"LOG ENTRY       AT"},
			{"ENTER KEYPAD FUNCTIONS ACCESS LEVEL"},//
			{"EXIT KEYPAD FUNCTIONS,NO CHANGES"},
			{"EXIT KEYPAD FUNCTIONS,CHANGES MODE"}},
			{{"EVENTO NUMERO       EM"},
				{""},
				{""},
				{""}

				},
			{{"ENT DE REGIST       EN"},
			{"INTROD FUNCS D/TECLADO NIVEL DE ACC"},
			{"SALIR FUNCS D/ TECLADO, S/ CAMBIOS"},
			{"SALIR FUNCS D/ TECLADO, MODO CAMBIO"}

				}



			};
	// 2-xx
	const  unsigned char erjicaidan2_1[maxLanguage][24]={{"2-1   Disable zones"},{"2-1   Desabilitar zonas"},{"2-1   Desactiv zonas"}}; 
	const  unsigned char erjicaidan2_2[maxLanguage][41]={{"2-2   Assign sounder groups to zones"},{"2-2   Atribuir grupos sirenes para zonas"},{"2-2   Asignar grups de sirenas a zonas"}}; //查看gfe的进行对比
	const  unsigned char erjicaidan2_3[maxLanguage][37]={{"2-3   Assign I/O groups to zones"},{"2-3   Atribuir groups E/S para zonas"},{"2-3   Asignar grupos de E/S a zonas"}}; 
	const  unsigned char erjicaidan2_4[maxLanguage][37]={{"2-4   Assign zone to device"},{"2-4   Atribuir zonas ao dispositivo"},{"2-4   Asignar zona al disptvo"}}; 
	const  unsigned char erjicaidan2_5[maxLanguage][36]={{"2-5   Zone sounder delay set-up"},{'2','-','5',' ',' ',' ','T','e','m','p','o','r','i','z','a',0xc4,0x9c,'o',' ','z','o','n','a','s',' ','d','e',' ','s','i','r','e','n','e','s'},{"2-5   Config retardo de sirena zona"}}; 
	const  unsigned char erjicaidan2_1_1[maxLanguage][24]={{"Use     to select"},{"Use     Para Selecionar"},{"Use     p/ Selecc"}};
	const  unsigned char erjicaidan2_1_2[maxLanguage][31]={{"Zone :              ENABLED"},{"Zona :              HABILITADO"},{"Zona :              HABILIT"}};
	const  unsigned char erjicaidan2_1_3[maxLanguage][33]={{"Zone :              DISABLED"},{"Zone :              DESABILITADO"},{"Zona :              DESACTIV"}};
	const  unsigned char erjicaidan2_2_1[maxLanguage][22]={{"ENTER number using"},{"Usar     para numerar"},{'E','N','T','R','A','R',' ','n',0x86,'m','e','r','o',' ','u','s','a','n','d','o'}};
	const  unsigned char erjicaidan2_2_2[maxLanguage][7]={{"ZONE :"},{"ZONA :"},{"ZONA :"}};;
	const  unsigned char  erjicaidan2_2_3[maxLanguage][41]={{"FIRST FIRE GRP:     SECOND FIRE GRP:"},{"GRUPO FOGO 1  :     GRUPO FOGO 2   :"},{"1o GRP DE FUEGO:    2o GRP DE FUEGO:"}};
	const  unsigned char  erjicaidan2_2_4[maxLanguage][41]={{"  select field and     alter value"},{"  selecciona campo e     altera valor"},{"  selecc Campo y     alterar el valor"}};
	const  unsigned char  erjicaidan2_2_5[]={"     1:    2:    3:    4:   , 5:   "};
	const  unsigned char  erjicaidan2_4_1[maxLanguage][18]={{"    select device"},{"    selecc disp."},{"    selecc disp"}};
	const  unsigned char  erjicaidan2_4_2[maxLanguage][16]={{"Loop:   Dev:"},{'L','a',0xc4,'o',':',' ',' ',' ','D','s','p',':'},{"Lazo:   Dsp:"}};;
	const  unsigned char  erjicaidan2_4_3[maxLanguage][29]={{"    to enter zone"},{"    enterar na zona"},{"    p/ entrar zona"}};
	const  unsigned char  erjicaidan2_4_5[maxLanguage][2][12]={{{"DELAYED"},{"IMMEDIATE"}},{{"TEMPORIZADO"},{"IMEDIATO  "}},{{"RETRASADO"},{"INMEDIATA"}}};
	const  unsigned char  erjicaidan2_4_6[maxLanguage][41]={{"Use +/- to change loop,"},{'U','s','e',' ','+','/','-',' ','s','e','l','e','c','c',' ','l','a',0xc4,'o',','},{"Use +/- p/cambiar Lazo,"}};
	// 3-xx
const  unsigned char erjicaidan3_1[maxLanguage][30]={{"3-1   Sounder configuartion"},{"3-1   Configurar sirenes"},{'3','-','1',' ',' ',' ','C','o','n','f','i','g','u','r','a','c','i',0x8d,'n',' ','d','e',' ','s','i','r','e','n','a'}};
	const  unsigned char erjicaidan3_2[maxLanguage][36]={{"3-2   Configure sounder groups"},{"3-2   Configurar grupos de sirenes"},{"3-2   Configurar grp de sirena"}};
	const  unsigned char erjicaidan3_3[maxLanguage][27]={{"3-3   Disable sounders"},{"3-3   Disabilitar sirenes"},{"3-3   Desactivar sirenas"}};
	const  unsigned char erjicaidan3_4[maxLanguage][40]={{"3-4   Assign sounder group to device"},{"3-4   Atribuir grupo de sirenes a disp."},{"3-4   Asignar grp de sirenas al disptv"}};	
	const  unsigned char erjicaidan3_5[maxLanguage][38]={{"3-5   Inhibit sounders for device"},{"3-5   Inibir sirenes para dispositivo"},{"3-5   Inhibir sirenas p/ dispotvs"}};	
	const  unsigned char erjicaidan3_6[maxLanguage][38]={{"3-6   Sounder delay set-up"},{'3','-','6',' ',' ',' ','D','e','f','i','n','i','r',' ','t','e','m','p','o','r','i','z','a',0xc4,'a','o',' ','d','e',' ','s','i','r','e','n','e','s'},{"3-6   Config del retraso sirena"}};
	const  unsigned char erjicaidan3_7[maxLanguage][40]={{"3-7   Override sounder delays"},{'3','-','7',' ',' ',' ','A','n','u','l','a','r',' ','t','e','m','p','o','r','i','z','a',0xc4,'a','o',' ','d','a','s',' ','s','i','r','e','n','e','s'},{"3-7   Anular retrasos sirena"}};
	const  unsigned char erjicaidan3_1_1[maxLanguage][32]={{"Use   and   to change status"},{"Use   and   para alterar estado"},{"Use   y   p/ cambiar Status"}};
	const  unsigned char erjicaidan3_1_2[maxLanguage][18]={{"SET AS :"},{"CONFIGURAR COMO :"},{"ESTAB COMO :"}};
	const  unsigned char erjicaidan3_1_3[maxLanguage][2][12]={{{"PRESET"},{"PROGRAMMED"}},{{"PRESET"},{"PROGRAMADO"}},{{"PRESET"},{"PROGRAMDO"}}};
	const  unsigned char erjicaidan3_1_4[maxLanguage][40]={{"WARNING: SOUNDER GROUPS MUST BE DEFINED"},{'A','T','E','N',0XC4,'A','O',':',' ','D','E','F','I','N','I','R',' ','O','S',' ','G','R','U','P','O','S',' ','D','E',' ','S','I','R','E','N','E','S'},{"ADVRTNCIA: GRPS SIRENA DEBEN SER DEFNDOS"}};
	const  unsigned char erjicaidan3_2_1[maxLanguage][8][10]={{{"GROUP :"},{"PANEL :"},{"SOUNDER :"},{""},{"LOOP"},{"NON-ADDR"},{""},{"CIE NO.:"}},{{"GRUPO :"},{"PAINEL:"},{"SIRENE  :"},{""},{'L','A',0XC4,'O'},{"NON-ADDR"},{""},{"CIE NO.:"}},
		{{"GRUPO :"},{"PANEL :"},{"SIRENA  :"},{""},{"LAZO"},{"NON-ADDR"},{""},{"CIE NO.:"}}};
	const  unsigned char  erjicaidan3_2_2[3]={"SPC"};
	const  unsigned char  erjicaidan3_3_1[maxLanguage][3]={{"DE"},{"DH"},{"DH"}};
	const  unsigned char  erjicaidan3_4_1[maxLanguage][20]={{"select device"},{"selcc disptvs"},{"selcc disptvs"}};
	const  unsigned char  erjicaidan3_4_2[maxLanguage][15]={{"GROUP NUMBER :"},{"GRUPO NUMERO :"},{'N',0x82,'M','E','R','O',' ','D','E',' ','G','R','P',':'}};
	const unsigned char    erjicaidan3_4_3[maxLanguage][28]={{"    select group"},{"    seleci. o grupo"},{"    selecc GRP"}};	
	const  unsigned char  erjicaidan3_4_5[maxLanguage][12]={{"INHIBITED :"},{"  INIBIDA :"},{" INHIBIDO :"}};
	const  unsigned char  erjicaidan3_5_1[maxLanguage][9]={{"STATUS :"},{"ESTADO : "},{"STATUS :"}};
const  unsigned char  erjicaidan3_5_2[maxLanguage][2][12]={{{"UNAFFECTED"},{"SWITCHED ON"}},{{"SEM EFEITO"},{"LIGADO    "}},{{"INAFECTADO"},{"ENCENDIDO"}}};
	const  unsigned char  erjicaidan3_4_6[maxLanguage][4][10]={{{"NONE"},{"COMMON"},{"ZONAL"},{"ALL"}},{{"NENHUMA"},{"COMUM"},{"POR ZONAS"},{"TODOS"}},{{"NINGUNO"},{'C','O','M',0X82,'N'},{"ZONAL"},{"TODO"}}};
	const  unsigned char  erjicaidan3_6_1[maxLanguage][3][15]={{{"MINS :"},{"SECS :"},{"OPERATE FROM:"}},{{"MINS :"},{"SEGS :"},{"ACTIVEDO POR:"}},{{"MINS :"},{"SEGS :"},{"OPERAR DESDE:"}}};
	const  unsigned char  erjicaidan3_6_2[maxLanguage][3][13]={{{"DISABLED"},{"ZONAL MODE"},{"GLOBAL MODE"}},{{"DESABILITADO"},{"MODO ZONA"},{"MODO GLOBAL"}},{{"DESHABLTD"},{"MODO ZONEAL"},{"MODO GLOBAL"}}};
	const  unsigned char  erjicaidan3_6_3[maxLanguage][3][20]={{{"ANY DEVICE"},{"CALL POINTS ONLY"},{"DETECTORS ONLY"}},{{"QUALQUER DISPOSITIV"},{"SOMENTE BOTONEIRAS"},{"SOMENTE DETECTOR"}},{{"CUALQ DISPTV"},{"APENAS PULSADORES"},{"APENAS DETECTS"}}};
	const  unsigned char  erjicaidan3_7_1[]={"0X"};

	
	const unsigned char  erjicaidan4_1[maxLanguage][38]={{"4-1   Configure I/O groups"},{"4-1   Configuara grupos entrada/saida"},{"4-1   Config Grp de E/S"}};
	const unsigned char  erjicaidan4_2[maxLanguage][40]={{"4-2   Select fault I/O group"},{"4-2   Selecionar grupo E/S prar avaria"},{"4-2   Seleccione Grp de E/S de fallas"}};
	const unsigned char  erjicaidan4_3[maxLanguage][38]={{"4-3   Assign I/O group to device"},{"4-3   Atribuir grupo E/S para Disp."},{"4-3   Asignar grupo de E/S al Disp."}};
	const unsigned char  erjicaidan4_4[maxLanguage][35]={{"4-4   Inhibit I/O for device"},{"4-4   Desabilitar E/S para disp."},{"4-4   Inhibir E/S p/ el disp"}};;
	const unsigned char  erjicaidan4_5[maxLanguage][36]={{"4-5   I/O unit action upon evacuate"},{'4','-','5',' ',' ',' ','E','/','S',' ','a','c','t','i','v','a',' ','c','o','m',' ','a',' ','e','v','a','c','u','a',0xc4,'a','o'}, 
		{'4','-','5',' ',' ',' ','A','c','c','i',0x8d,'n',' ','u','n','i','d',' ','d','e',' ','E','/','S',' ','a','l',' ','e','v','a','c','u','a','r'}};
	const unsigned char  erjicaidan4_6[maxLanguage][40]={{"4-6   I/O unit delay or immediate"},{"4-6   Unidade E/S temp. ou imediato"},{"4-6   Retraso unid de E/S o inmediato"}};
	const unsigned char  erjicaidan4_7[maxLanguage][30]={{"4-7   I/O delay set-up"},{'4','-','7',' ',' ',' ','t','e','m','p','o','r','i','z','a',0xc4,0x8c,'e','s',' ','p','a','r','a',' ','E','/','S'},{"4-7   Config de retardo E/S"}};
	const unsigned char  erjicaidan4_1_1[maxLanguage][4][8]={{{"GROUP"},{"ENTRY"},{"LOOP"},{"DEVICE"}},{{"GRUPO"},{"E/S"},{'L','A',0XC4,'O'},{"DISP."}},{{"GRUPO"},{"ENTRADA"},{"LAZO"},{"DISP."}}};
	const unsigned char  erjicaidan4_2_1[maxLanguage][26]={{"Use     to change group"},{"Use     para trocar grupo"},{"USE     p/ cambiar grp"}};
	const unsigned char  erjicaidan4_2_2[maxLanguage][37]={{"FAULT CONDITION ACTIVATES I/O GROUP:"},{"AVARIA ACCIONA GRUPO ENTRADA/SAIDA :"},{'C','O','N','D','I','C','I',0X89,'N',' ','D','E',' ','F','A','L','L','A',' ','A','C','T',' ','E','L',' ','G','R','P',' ','D','E',' ','E','/','S',':'}};
	const unsigned char  erjicaidan4_7_1[maxLanguage][28]={{"Use     to enter time"},{"Use     para atribuir tempo"},{"Use     p/ ingr. tiempo "}};
		
	const unsigned char  erjicaidan5_1[maxLanguage][14]={{"5-1   General"},{"5-1   Geral"},{"5-1   General"}};
	const unsigned char  sanjicaidan5_1_1[maxLanguage][24]={{"5-1-1 Disable loops"},{'5','-','1','-','1',' ','D','e','s','a','b','i','l','i','t','a','r',' ','L','a',0xc4,'o','s'},{"5-1-1 Desact Lazos"}};
	const unsigned char  sanjicaidan5_1_2[maxLanguage][30]={{"5-1-2 Device disable"},{"5-1-2 Desabilitar dispositivo"},{"5-1-2 Deshablt Dispt"}};
	const unsigned char  sanjicaidan5_1_3[maxLanguage][32]={{"5-1-3 Set selective disablement"},{'5','-','1','-','3',' ','S','e','l','e','c',0xc4,'a','o',' ','d','e',' ','d','e','s','a','b','i','l','i','t','a','d','o','s'},{"5-1-3 Estab inhabltc selectiva"}};
	const unsigned char  sanjicaidan5_1_4[maxLanguage][41]={{"5-1-4 Set device reporting details"},{"5-1-4 Detalhes do dispositivo"},{"5-1-4 Estab detlls informes de Dispt"}};
	const unsigned char  sanjicaidan5_1_5[maxLanguage][41]={{"5-1-5 Set immediate evacuate for device"},{'5','-','1','-','5',' ','D','I','S','P','.',' ','p','a','r','a',' ','e','v','a','c','u','a',0xc4,'a','o',' ','i','m','e','d','i','a','t','a'},
		{"5-1-5 Estab evacuac inmediata p/ Dispt"}};
	const unsigned char  sanjicaidan5_1_6[maxLanguage][41]={{"5-1-6 Device activation overrides delays"},{'5','-','1','-','6',' ','D','i','s','p','o','s','i','t','i','v','o',' ','a','n','u','l','a',' ','a','s',' ','t','e','m','p','o','r','i','z','a',0xc4,0x8c,'e','s'},
		{"5-1-6 La activac d/ dispt anula retrasos"}};
	const unsigned char  sanjicaidan5_1_7[maxLanguage][35]={{"5-1-7 Inhibit auxiliary relays"},{"5-1-7 Desabilitar reles auxiliares"},{'5','-','1','-','7',' ','I','n','h','i','b','i','r',' ','r','e','l',0x96,'s',' ','a','u','x','i','l','i','a','r','e','s'}};
	const unsigned char  sanjicaidan5_1_8[maxLanguage][33]={{"5-1-8 Global sensitivity set-up"},{"5-1-8 Sensibilidade global"},{"5-1-8 Config de sensibldd global"}};
	const unsigned char  sanjicaidan5_1_9[maxLanguage][41]={{"5-1-9 Configure Timed Sensitivity"},{'5','-','1','-','9',' ','C','o','n','f','i','g','u','r','a',0xc4,0x9c,'o',' ','P','e','r','i','o','d','o',' ','S','e','n','s','i','b','i','l','i','d','a','d','e'},{"5-1-9 Config sensibldd temporzd"}};
	const unsigned char  erjicaidan5_2[maxLanguage][38]={{"5-2   Other device options"},{"5-2   Outros detalhes do dispositivo"},{"5-2   Otras opcions de dispt"}};
	const unsigned char  sanjicaidan5_2_1[maxLanguage][40]={{"5-2-1 Select device heat grade"},{"5-2-1 Selecc. grau temp. do dispositivo"},{"5-2-1 Selec Grado calor d/ dispt"}};
	const unsigned char  sanjicaidan5_2_2[maxLanguage][40]={{"5-2-2 Select device smoke sensitivity"},{"5-2-2 Seleccione sensibilidade do disp."},{"5-2-2 Selec sensibldd humo d/ dispt"}};
	const unsigned char  sanjicaidan5_1_1_1[maxLanguage][6]={{"LOOP:"},{'L','A',0XC4,'O',':'},{"LAZO:"}};
	const unsigned char sanjicaidan5_1_1_2[maxLanguage][2][13]={{{"DISABLED"},{"ENABLED"}},{{"DESABILITADO"},{"HABILITADO"}},{{"DESHBILT"},{"HABILIT"}}};
	const unsigned char sanjicaidan5_1_1_3[maxLanguage][2][13]={{{"UNAFFECTED"},{"DISABLED"}},{{"SEM EFEITO"},{"DESABILITADO"}},{{"INAFECTADO"},{"DESHBILT"}}};
	const unsigned char sanjicaidan5_1_1_5[maxLanguage][2][13]={{{"UNAFFECTED"},{"IMMEDIATE"}},{{"SEM EFEITO"},{"IMEDIATO  "}},{{"INAFECTADO"},{"INMEDIATA"}}};
	const unsigned char sanjicaidan5_1_1_4[maxLanguage][3][11]={{{"FIRE"},{"FAULT"},{"PRE-ALARM"}},{{"FOGO"},{"FALHA"},{"PRE-ALARM"}},{{"FUEGO"},{"FALLA"},{"PRE-ALARMA"}}};
	const  unsigned char sanjicaidan5_1_1_6[maxLanguage][19]={{"    change status"},{"    para selecc"},{"    cmbiar status"}};
	
	const unsigned char sanjicaidan5_1_6_1[maxLanguage][14]={{"TIMER :"},{"TEMPORZADOR :"},{"TIMER :"}};
	const unsigned char sanjicaidan5_1_6_2[maxLanguage][2][13]={{{"UNAFFECTED"},{"ENDED"}},{{"SEM EFEITO"},{"FIM DO TEMPO"}},{{"INAFECTADO"},{'T',0X92,'R','M','N','O'}}};
	const unsigned char sanjicaidan5_1_7_1[maxLanguage][2][8]={{{"NONE"},{"ALL"}},{{"NENHUM"},{"TODOS"}},{{"NINGUNO"},{"TODO"}}};
	const unsigned char sanjicaidan5_1_8_1[maxLanguage][3][21]={{{"MONDAY - FRIDAY   :"},{"SATURDAY          :"},{"SUNDAY            :"}},{{"DIAS UTEIS        :"},{"SABADO            :"},{"DOMINGO           :"}},
		{{"LUNES - VIERNES   :"},{"SABADO            :"},{"DOMINGO           :"}}};
	const unsigned char sanjicaidan5_1_8_2[maxLanguage][4][21]={{{"NORMAL/MODE3"},{"HIGH/MODE1"},{"TIMED"},{"LOW/MODE5 (NOT EN54)"}},{{"MODO3/NORMAL"},{"MODO1/ALTO"},{"TIMED"},{"MODO5/BAXIO (N/EN54)"}},
		{{"NORMAL/MODO3"},{"ALTO/MODO1"},{"TMPORZDO"},{"BAJO/MODO5 (NO EN54)"}}};

	const unsigned char sanjicaidan5_1_9_1[maxLanguage][2][21]={{{"DAY SENSITIVITY   :"},{"NIGHT SENSITIVITY :"}},{{"MODO DIA          :"},{"MODO NOTIE        :"}},{{"SENSBILDAD DIA    :"},{"SENSBLID NOCTURNA :"}}};
	const unsigned char sanjicaidan5_1_9_2[maxLanguage][3][21]={{{"NORMAL/MODE3"},{"HIGH/MODE1"},{"LOW/MODE5 (NOT EN54)"}},{{"MODO3/NORMAL"},{"MODO1/ALTO"},{"MODO5/BAXIO (N/EN54)"}},
		{{"MODO3/NORMAL"},{"ALTO/MODO1"},{"BAJO/MODO5 (NO EN54)"}}};
	const unsigned char sanjicaidan5_2_1_1[maxLanguage][38]={{"Use   and   to select heat grade"},{"Usar   e   para selecc. grau de temp."},{"Use   y   p/ selecc grado de calor"}};
	const unsigned char sanjicaidan5_2_1_2[4][4]={{"55C"},{"65C"},{"75C"},{"85C"}};
	const unsigned char sanjicaidan5_2_2_1[maxLanguage][38]={{"Use   and   to select sensitivity"},{"Usar   e   para selecc. sensibilidade"},{"Use   y   seleccionar la sensibilidad"}};
	const unsigned char sanjicaidan5_2_2_2[maxLanguage][4][15]={{{"NORMAL"},{"LOW (NOT EN54)"},{"HIGH"},{"GLOBAL"}},{{"NORMAL"},{"BAIXO (N/EN54)"},{"ALTO"},{"GLOBAL"}},{{"NORMAL"},{"BAJO (NO EN54)"},{"ALTO"},{"GLOBAL"}}};
	const unsigned char  erjicaidan6_1[maxLanguage][41]={{"6-1   Device count,type & value"},{"6-1   Evento do disp., tipo & valor"},{"6-1   Rcuento d/ dispts, tpo y Valor"}};
	const unsigned char  erjicaidan6_2[maxLanguage][23]={{"6-2   Test sounders"},{"6-2   Teste de sirenes"},{"6-2   Probar sirenas"}};
	const unsigned char  erjicaidan6_3[maxLanguage][37]={{"6-3   Sounders on test activation"},{'6','-','3',' ',' ',' ','S','i','r','e','n','e','s',' ','e','m',' ','a','c','t','i','v','a',0xc4,0x9c,'o',' ','d','e',' ','t','e','s','t','e'},{"6-3   Sirenas en activac de prueba"}};
	const unsigned char  erjicaidan6_4[maxLanguage][21]={{"6-4   Test zones"},{"6-4   Teste de zonas"},{"6-4   Probar zonas"}};
	const unsigned char  erjicaidan6_5[maxLanguage][31]={{"6-5   Light LED on device"},{"6-5   Ligar LED do dispositivo"},{"6-5   LED encendido en Dispt"}};
	const unsigned char  erjicaidan6_1_1[maxLanguage][38]={{"+/- select loop       select device"},{'+','/','-','s','e','l','e','c','c','.','l','a',0xc4,'o',' ',' ',' ',' ',' ',' ','s','e','l','e','c','c','.','d','i','s','p'},{"+/- selecc lazo       selecc Dispt"}};
	const unsigned char  erjicaidan6_1_2[maxLanguage][41]={{"LOOP :         NUMENS-ST     COUNT ="},{'L','A',0XC4,'O',' ',':',' ',' ',' ',' ',' ',' ',' ',' ',' ','N','U','M','E','N','S','-','X','P',' ',' ',' ',' ',' ','T','O','T','A','L',' ','='},{"LAZO :        NUMENS-ST     CONTAR ="}};
	const unsigned char  erjicaidan6_1_3[maxLanguage][38]={{"DEV  :                       VALUE ="},{"DISP.:                       VALOR ="},{"DISPT:                       VALOR ="}};
	const unsigned char  erjicaidan6_1_4[maxLanguage][38]={{"DEV  :                        RSSI ="},{"DISP.:                        RSSI ="},{"DISPT:                        RSSI ="}};
	const unsigned char  erjicaidan6_2_1[maxLanguage][36]={{"Use     to select the panel"},{"Use     para seleccionar o painel"},{"Use     p/ selecc el panel"}};
	//const unsigned char  erjicaidan6_2_2[2][3][17]={{{"ALL SOUNDERS"},{"PANEL 01"},{"MAIN PANEL"}},{{"TODAS AS SIRENS"},{"PAINEL 01"},{"PAINEL PRINCIPAL"}}};

	const unsigned char  erjicaidan6_2_2[maxLanguage][3][17]={{{"ALL SOUNDERS"},{"LOOP SOUNDERS"},{"PANEL SOUNDERS"}},{{"TODAS AS SIRENES"},{"SIRENES DE LOOP"},{"SIRENS DE PAINEL"}},{{"TODAS SIRENAS"},{"SIRENA DE LOZA"},{"SIRENAS DE PANEL"}}};

	
	const unsigned char  erjicaidan6_3_1[maxLanguage][32]={{"Use   and   to select mode"},{"Use   e   para seleccionar modo"},{"USE   y   p/ selecc modo"}};
	//const unsigned char  erjicaidan6_3_2[2][3][40]={{{"NO SOUNDERS ON DETECTOR TEST"},{"SUB-PANEL SOUNDERS ON DETECTOR TEST"},{"ALL SOUNDERS ON DETECTOR TEST"}},
	//											{{"DESLIGAR SIRENES EM TESTE DE DETECTOR"},{"SIRENES SUB-PAINEL EM TESTE DE DETECTOR"},{"TODAS AS SIRENES EM TESTE DE DETECTOR"}}};
	const unsigned char  erjicaidan6_3_2[maxLanguage][3][40]={{{"NO SOUNDERS ON DETECTOR TEST"},{"LOOP SOUNDERS ON DETECTOR TEST"},{"ALL SOUNDERS ON DETECTOR TEST"}},
												{{"DESLIGAR SIRENES EM TESTE DE DETECTOR"},{"SIRENES LOOP EM TESTE DE DETECTOR"},{"TODAS AS SIRENES EM TESTE DE DETECTOR"}},
												{{"SIN SIRENAS EN PRUEBA DE DETEC"},{"SIRENAS LOZA ON EN TESTE DE DETEC"},{"TODAS SIRENAS ON EN TESTE DE DETEC"}}};
	const unsigned char  erjicaidan6_4_1[maxLanguage][41]={{"Use     to select"},{"Usar     para selecc"},{"Use     p/ selecionar"}};
	const unsigned char  erjicaidan6_4_2[maxLanguage][7]={{"ZONE :"},{"ZONA :"},{"ZONA :"}};
	const unsigned char  erjicaidan6_4_3[maxLanguage][2][28]={{{"NORMAL FIRE OPERATING"},{"TEST MODE"}},{{'M','O','D','O',' ','D','E','T','E','C',0XC4,'A','O',' ','I','N','C','E','N','D','I','O'},{"MODO TESTE"}},{{"OPERAC NORMAL EN INCENCD"},{"MODO TESTE"}}};
	const unsigned char  erjicaidan6_5_1[maxLanguage][40]={{"select device"},{"select device"},{"SELECC DISPT"}};
	const unsigned char  erjicaidan6_5_2[maxLanguage][8]={{"DEV:"},{"DSP:"},{"DSP:"}};
	const unsigned char erjicaidan6_5_3[maxLanguage][2][13]={{{"LIGHT OFF"},{"LIGHT ON"}},{{"DESLIGAR LED"},{"LIGAR LED"}},{{"LUZ APAG"},{"LUZ ENCEND"}}};

	
	const unsigned char  erjicaidan7_1[maxLanguage][32]={{"7-1   Time/Date & timers"},{'7','-','1',' ',' ',' ','H','o','r','a','/','D','a','t','a',' ','&',' ','t','e','m','p','o','r','i','z','a',0xc4,0x8c,'e','s'},{"7-1   Hra/Fecha y temporzds"}};
	const unsigned char  sanjicaidan7_1_1[maxLanguage][26]={{"7-1-1 Set date & time"},{"7-1-1 Acertar data & hora"},{"7-1-1 Estab fecha y hora"}};
	const unsigned char  sanjicaidan7_1_2[maxLanguage][34]={{"7-1-2 Define day & night"},{"7-1-2 Definir dia & notie"},{'7','-','1','-','2',' ','D','e','f','i','n','i','r',' ','d',0xa3,'a',' ','y',' ','n','o','c','h','e'}};
	const unsigned char  sanjicaidan7_1_3[maxLanguage][36]={{"7-1-3 Delays off at night"},{"7-1-3 Temp. desligads no modo noite"},{"7-1-3 Retrasos apagd noche"}};
	const unsigned char  sanjicaidan7_1_4[maxLanguage][40]={{"7-1-4 Configure evacuate timer"},{'7','-','1','-','4',' ','C','o','n','f','i','g','u','r','a','r',' ','t','e','m','p','o',' ','d','e',' ','e','v','a','c','u','a',0xc4,0x9c,'o'},
		{'7','-','1','-','4',' ','C','o','n','f','i','g',' ','t','e','m','p','/',' ','e','v','a','c','u','a','c','i',0x89,'n'}};
	const unsigned char  sanjicaidan7_1_5[maxLanguage][40]={{"7-1-5 Device starts evacuate timer"},{'7','-','1','-','5',' ','D','i','s','p','.',' ','i','n','i','c','i','a',' ','t','e','m','p','o',' ','d','e',' ','e','v','a','c','u','a',0xc4,0x9c,'o'},
		{"7-1-5 Dispt inicia temp/ de evacuac"}};
	const unsigned char  sanjicaidan7_1_1_1[maxLanguage][36]={{"Set using            DD-MM-YY HH:MM"},{"Usar                 DD-MM-YY HH:MM"},{"Estab            con DD-MM-YY HH:MM"}};
	const unsigned char  sanjicaidan7_1_1_2[maxLanguage][7][10]={{{"SUNDAY"},{"MONDAY"},{"TUESDAY"},{"WEDNESDAY"},{"THURSDAY"},{"FRIDAY"},{"SATURDAY"}},
													{{"DOMINGO"},{"SEGUNDA"},{'T','E','R',0XC4,'A'},{"QUARTA"},{"QUINTA"},{"SEXTA"},{'S',0x9d,'B','A','D','O'}},
													{{"DOMINGO"},{"LUNES"},{"MARTES"},{'M','I',0x92,'R','C','O','L','E','S'},{"JUEVES"},{"VIERNES"},{'S',0x9d,'B','A','D','O'}}};
	const unsigned char  sanjicaidan7_1_2_1[maxLanguage][12]={{"DAY STARTS"},{"AMANHECER"},{"DIA EMPIEZA"}};
	const unsigned char  sanjicaidan7_1_2_2[maxLanguage][14]={{"NIGHT STARTS"},{"ANOITECER"},{"NOCHE EMPIEZA"}};
	const  unsigned char  sanjicaidan7_1_1_3[maxLanguage][7]={{6,6,7,9,8,6,8},
											        {7,7,5,6,6,5,6},{7,5,6,9,6,7,6}};

	
	const unsigned char  sanjicaidan7_1_3_1[maxLanguage][20]={{"DELAYS AT NIGHT :"},{"TEMP. MODO NOITE  :"},{"RETRASOS NOCHE:"}};
	const unsigned char sanjicaidan7_1_3_2[maxLanguage][2][11]={{{"UNAFFECTED"},{"OFF"}},{{"SEM EFEITO"},{"DESLIGADO"}},{{"INAFECTADO"},{"OFF"}}};
	const unsigned char sanjicaidan7_1_4_1[maxLanguage][3][13]={{{"DISABLED"},{"DEVICE MODE"},{"GLOBAL MODE"}},{{"DESABILITADO"},{"MODO DISP."},{"MODO GLOBAL"}},{{"DESHABTD"},{"MODO DISPST"},{"MODO GLOBAL"}}};
	const unsigned char sanjicaidan7_1_4_2[maxLanguage][2][11]={{{"UNAFFECTED"},{"STARTED"}},{{"SEM EFEITO"},{"INICIADO"}},{{"INAFECTADO"},{"EMPEZADA"}}};
	const unsigned char sanjicaidan7_1_5_1[maxLanguage][32]={{"Use   and   to change status"},{"Usar   e   para alterar estado"},{"Use   y   p/ cambiar Status"}};
	const unsigned char  erjicaidan7_2[maxLanguage][40]={{"7-2   Special features set-up"},{'7','-','2',' ',' ',' ','C','o','n','f','i','g','u','r','a',0xc4,0x8c,'e','s',' ','e','s','p','e','c','i','a','i','s'},{"7-2   Config de caractrs especiales"}};
	const unsigned char sanjicaidan7_2_1[maxLanguage][39]={{"7-2-1 Two devices to evacuate"},{'7','-','2','-','1',' ','D','o','i','s',' ','D','i','s','p','o','s','i','t','i','v','o','s',' ','p','a','r','a',' ','e','v','a','c','u','a',0xc4,0x9c,'o'},{"7-2-1 Dos dispts p/ evacuar"}};
	const unsigned char sanjicaidan7_2_2[maxLanguage][32]={{"7-2-2 Call points to evacuate"},{'7','-','2','-','2',' ','B','o','t','o','n','e','i','r','a','s',' ','p','a','r','a',' ','e','v','a','c','u','a',0xc4,0x9c,'o'},
		{'7','-','2','-','2',' ','P','u','l','s','a','d','o','r','e','s',' ','p','/',' ','e','v','a','c','u','a','c','i',0x89,'n'}};
	const unsigned char sanjicaidan7_2_3[maxLanguage][33]={{"Use     to select setting"},{"Usar   e   para seleccionar modo"},{"Use     p/ selecc la configur"}};
	const unsigned char  erjicaidan7_3[maxLanguage][26]={{"7-3   Other features"},{'7','-','3',' ',' ',' ','O','u','t','r','a','s',' ','c','o','n','f','i','g','u','r','a',0xc4,0x9c,'o'},{"7-3   Otras caractertics"}};
	//const unsigned char  erjicaidan7_4[]={"7-4   Restore Factory Settings"};
	const unsigned char sanjicaidan7_3_1[maxLanguage][31]={{"7-3-1 Active/Installation mode"},{'7','-','3','-','1',' ','M','o','d','o',' ','A','c','t','i','v','o','/','I','n','s','t','a','l','a',0xc4,0x9c,'o'},
	{'7','-','3','-','1',' ','M','o','d','o',' ','a','c','t','i','v','o','/','i','n','s','t','a','l','a','c','i',0x89,'n'}};
	const unsigned char sanjicaidan7_3_2[maxLanguage][39]={{"7-3-2 Set user access code"},{'7','-','3','-','2',' ','I','n','t','r','o','d','.',' ','c','o','d','i','g','o',' ','a','c','e','s','s','o',' ','u','t','i','l','i','z','a','d','o','r'},
		{'7','-','3','-','2',' ','E','s','t','a','b',' ','c',0x89,'d','g',' ','a','c','c',' ','u','s','u','a','r'}};
	const unsigned char sanjicaidan7_3_3[maxLanguage][40]={{"7-3-3 Set user functions"},{'7','-','3','-','3',' ','I','n','t','r','o','d','u','z','i','r',' ','f','u','n',0xc4,0x8c,'e','s',' ','d','o',' ','u','t','i','l','i','z','a','d','o','r'},{"7-3-3 Estab funcc Usuar"}};
	const unsigned char sanjicaidan7_3_4[maxLanguage][39]={{"7-3-4 Set installer access code"},{"7-3-4 Introduzir codigo acesso instal."},{'7','-','3','-','4',' ','E','s','t','a','b',' ','c',0x8d,'d','g',' ','a','c','c',' ','i','n','s','t','a','l','a','d'}};
	const unsigned char sanjicaidan7_3_5[maxLanguage][41]={{"7-3-5 Restore factory settings"},{'7','-','3','-','5',' ','R','e','s','t','a','u','r','a','r',' ','c','o','n','f','i','g','u','r','a',0xc4,0x8c,'e','s',' ','d','e',' ','f','a','b','r','i','c','a'},
		{'7','-','3','-','5',' ','R','s','t','a','u','r','a','r',' ','a','j','u','s','t',' ','d','e',' ','f',0x9d,'b','r','i','c','a'}};
	const unsigned char sanjicaidan7_3_6[maxLanguage][25]={{"7-3-6 Select language"},{"7-3-6 Seleccionar idioma"},{"7-3-6 Selecc idioma"}};
	//const unsigned char sanjicaidan7_3_7[2][41]={{"7-3-7 Upload/Download link to pc"},{"7-3-7 Carregar/Descarregar dados para pc"}};
	
	const unsigned char sanjicaidan7_3_7[maxLanguage][41]={{"7-3-7 Configuration data transfer"},{"7-3-7 Transmision de datos configuracion"},{"7-3-7 Transf de datos de config"}};
	const unsigned char sanjicaidan7_3_8[maxLanguage][41]={{"7-3-8 Calculate customer EEPROM checksum"},{"7-3-8 Avaliar a memoria do cliente"},{"7-3-8 Calcular suma comprob EEPR cliente"}};	
	const unsigned char sanjicaidan7_3_9[maxLanguage][39]={{"7-3-9 Calculate program FLASH checksum"},{"7-3-9 Avaliar a memoria de programa"},{"7-3-9 Calcular suma comprob Flash Prog"}};

	const unsigned char erjicaidan7_4[maxLanguage][41]={{"7-4   Wireless devices management"},{"7-4   Wireless devices management"},{"7-4   Wireless devices management"}};
	//const unsigned char erjicaidan7_4_1[maxLanguage][41]={{"+- change loop &    field,   alter value"},{"+- change loop &    field,   alter value"},{"+- change loop &    field,   alter value"}};
	
	//const unsigned char erjicaidan7_4_2[maxLanguage][41]={{"WLoop:   Count:  Node:   Count:  "},{"WLoop:   Count:  Node:   Count:  "},{"WLoop:   Count:  Node:   Count:  "}};
	//const unsigned char erjicaidan7_4_3[maxLanguage][41]={{"Dev:    Type:         Count:   Remove: "},{"Dev:    Type:         Count:   Remove: "},{"Dev:    Type:         Count:   Remove: "}};
	//const unsigned char erjicaidan7_4_2[maxLanguage][41]={{"Wireless loop count:  loop NO.:  "},{"Wireless loop count:   loop NO.:  "},{"Wireless loop count:   loop NO.:  "}};
	//const unsigned char erjicaidan7_4_3[maxLanguage][41]={{"Node count:   node NO.:   DEV. count:  "},{"Node count:   node NO.:   DEV. count:  "},{"Node count:   node NO.:   DEV. count:  "}};
	const unsigned char erjicaidan7_4_1[maxLanguage][41]={{"    select field,    alter value"},{"    select field,    alter value"},{"    select field,    alter value"}};
	const unsigned char erjicaidan7_4_1_1[maxLanguage][41]={{"    select field,               "},{"    select field,               "},{"    select field,               "}};
	const unsigned char erjicaidan7_4_2[maxLanguage][41]={{"Wireless loop NO.:   Loop count:  "},{"Wireless loop NO.:   Loop count:  "},{"Wireless loop NO.:   Loop count:  "}};
       const unsigned char erjicaidan7_4_3[maxLanguage][41]={{"Group NO.:   Node count:   DEV. count: "},{"Group NO.:   Node count:   DEV. count: "},{"Group NO.:   Node count:   DEV. count: "}};
	const unsigned char erjicaidan7_4_4[maxLanguage][41]={{"DEV. address:    Type:          Remove: "},{"DEV. address:    Type:          Remove: "},{"DEV. address:    Type:          Remove: "}};
	const unsigned char erjicaidan7_4_5[maxLanguage][41]={{"   select N/Y and ENT to confirm"},{"   select N/Y and ENT to confirm"},{"   select N/Y and ENT to confirm"}};
	const unsigned char erjicaidan7_4_6[maxLanguage][41]={{"Wireless channle :    To Change:"},{"Wireless channle :     To Change:"},{"Wireless channle :     To Change:"}};	
	const unsigned char erjicaidan7_4_6_1[maxLanguage][41]={{"Wireless channle :              "},{"Wireless channle :              "},{"Wireless channle :              "}};	
	const unsigned char sanjicaidan7_3_1_1[maxLanguage][7]={{"MODE :"},{"MODO :"},{"MODO :"}};
	const unsigned char sanjicaidan7_3_1_2[maxLanguage][2][13]={{{"INSTALLATION"},{"ACTIVE"}},{{'I','N','S','T','A','L','A',0XC4,0X9C,'O'},{"ACTIVO"}},{{"INSTALACION"},{"ACTIVO"}}};
	const unsigned char sanjicaidan7_3_1_3[maxLanguage][38]={{"Use  ,   and   to enter the code"},{"Usar  ,   e   para introduzir codigo"},{'U','s','e',' ',' ',',',' ',' ',' ','y',' ',' ',' ','p','/',' ','i','n','t','r','o','d',' ','e','l',' ','c',0x8d,'d','i','g','o'}};
	const unsigned char sanjicaidan7_3_1_4[maxLanguage][27]={{"USER ACCESS CODE :"},{"COD ACESSO UTILIZ.:"},{'C',0x89,'D','G',' ','A','C','C',' ','D','/',' ','U','S','U','A','R','I','O',' ',':'}};
	const unsigned char sanjicaidan7_3_3_1[maxLanguage][40]={{"Use     to select function and status"},{'U','s','a','r',' ',' ',' ',' ',' ','p','a','r','a',' ','s','e','l','e','c','c','.',' ','f','u','n',0xc4,'a','o',' ','e',' ','e','s','t','a','d','o'},
		{'U','s','e',' ',' ',' ',' ',' ','p','/',' ','s','e','l','e','c','c',' ','l','a',' ','f','u','n','c','i',0x8d,'n',' ','y',' ','s','t','a','t','u','s'}};
	const unsigned char sanjicaidan7_3_3_2[maxLanguage][15]={{"ACCESS LEVEL :"},{"NIVEL ACESSO :"},{"NIVEL DE ACC :"}};
	const unsigned char sanjicaidan7_3_3_3[maxLanguage][3][17]={{{"NONE"},{"READ ONLY"},{"FULL ACCESS"}},{{"NENHUM"},{"SO LEITURA"},{"ACESSO TOTAL"}},{{"NINGUNO"},{'S',0X89,'L','O',' ','L','E','C','T','U','R','A'},{"ACCESO COMPLETO"}}};

	
//	const unsigned char sanjicaidan7_3_4_1[maxLanguage][38]={{"Use  ,   and   to enter the code"},{"Usar   ,   e   para introduzir codigo"}};
	const unsigned char sanjicaidan7_3_4_1[maxLanguage][41]={{"Use alpha-numeric keypad to enter code"},{"Usar digitar para introduzir codigo"},{'U','s','e',' ','e','l',' ','t','e','c','l','a','d','o',' ','a','l','f','a','n','u','m',' ','p','/',' ','i','n','t','r','o','d',' ','e','l',' ','c',0x8d,'d','i','g'}};
	const unsigned char sanjicaidan7_3_4_2[maxLanguage][27]={{"INSTALLER ACCESS CODE :"},{"CODIGO ACESSO INSTALADOR :"},{'C',0x89,'D','G',' ','A','C','C',' ','D','/',' ','I','N','S','T','A','L','A','D','O','R',':'}};
	const unsigned char sanjicaidan7_3_4_3[maxLanguage][4][21]={{{""},{"Old code"},{"New code"},{"New code (confirm)"}},{{""},{"Codigo antigo"},{"Codigo novo"},{"Confirme codigo novo"}},
		{{""},{'C',0x8d,'d','g',' ','a','n','t','i','g','u','o'},{'C',0x8d,'d','g',' ','n','u','e','v','o'},{'C',0x8d,'d','g',' ','n','u','e','v','o',' ','c','o','n','f','i','r','m','a','r'}}};
	const unsigned char sanjicaidan7_3_5_1[maxLanguage][40]={{"Enter the code and press the ENTER"},{"Digite o codigo e pressione tecla ENTER"},{'I','n','t','r','o','d','u','z','c','a',' ','e','l',' ','c',0x8d,'d','i','g','o',' ','y',' ','p','u','l','s','e',' ','e','n','t','e','r'}};
	const unsigned char sanjicaidan7_3_5_2[maxLanguage][8]={{"CODE:"},{"CODIGO:"},{'C',0X8D,'D','I','G','O',':'}};
	const unsigned char sanjicaidan7_3_5_3[maxLanguage][41]={{"ONLY AVAILBLE IN ENGINEER ACCESS LEVEL"},{"AVALIAR SOMENTE EM NIVEL DE ACESSO ENGE."},
	{'S',0X89,'L','O',' ','D','I','S','P','.',' ','E','/',' ','N','I','V','E','L',' ','D','E',' ','A','C','C',' ','D','E',' ','I','N','G','E','N','I','E','R','O'}};
	const unsigned char sanjicaidan7_3_6_1[maxLanguage][39]={{"USE     to select required language"},{"Usar     para seleccionar idioma"},{"USE     p/ selecc el idioma requerido"}};
	const unsigned char sanjicaidan7_3_6_2[maxLanguage][11]={{"SELECTED :"},{"SELECC. :"},{"SELECC. :"}};
	const unsigned char sanjicaidan7_3_6_3[maxLanguage][maxLanguage][11]={{{"ENGLISH"},{"PORTUGUESE"},{"SPANISH"}},{{"INGLES"},{"PORTUGUES"},{"ESPANHOL"}},{{'I','N','G','L',0X92,'S'},{'P','O','R','T','U','G','U',0X92,'S'},{'E','S','P','A',0XA6,'O','L'}}};
	const unsigned char sanjicaidan7_3_7_1[maxLanguage][40]={{"PRESS ENTER to start, ESC to exit"},{"Prima ENTER para iniciar,ESC para sair"},{"PULSE ENTER p/ empezar, ESC p/ salir"}};
	const unsigned char sanjicaidan7_3_7_2[maxLanguage][40]={{"Waiting for pc"},{"Waiting for pc"},{"Esperando pc"}};
	const unsigned char sanjicaidan7_3_7_3[maxLanguage][40]={{"Communications ok"},{"Communications ok"},{"Comunicaciones OK"}};
	const unsigned char sanjicaidan7_3_7_4[maxLanguage][40]={{"No communication with pc"},{"No communication with pc"},{'S','i','n',' ','c','o','m','u','n','i','c','a','c','i',0x8d,'n',' ','c','o','m',' ','P','C'}};
	const unsigned char sanjicaidan7_3_7_5[maxLanguage][41]={{"    select mode,Enter to confirm"},{"    selecione modo,Enter para confirmar"},{"    modo de selecc, ENTER p/ confirmar"}};
	const unsigned char sanjicaidan7_3_7_6[maxLanguage][41]={{"1.Upload/Download link to pc"},{"1.Carregar/Descarregar dados para pc"},{"1.Subir/Descargar enlace a pc"}};
	const unsigned char sanjicaidan7_3_7_7[maxLanguage][24]={{"2.Link to repeater"},{"2.Link para repeater"},{"2.Enlace al repetid."}};
	//const unsigned char sanjicaidan7_3_7_8[maxLanguage][22]={{"3.Link to main"},{"3.Link para maior"}};
	//const unsigned char sanjicaidan7_3_7_8[maxLanguage][30]={{"3.Set expansion pan"},{"3.Definir expansao"},{'3','.','E','s','t','a','b',' ','p','a','n','e','l',' ','d','e',' ','e','x'}};
	const unsigned char sanjicaidan7_3_7_8[maxLanguage][30]={{"3.Set Extinguishant"},{"3.Definir extintor"},{"3.Conjunto extintor"}};
	const unsigned char sanjicaidan7_3_7_8_1[maxLanguage][22]={{"Not Vaild"},{"Not Vaild"},{'N','o',' ','e','s',' ','v',0X9D,'l','i','d','o'}};
	//const unsigned char sanjicaidan7_3_7_8_1[maxLanguage][]={{"3.Link to main"},{"3.Link para princi."}};
	const unsigned char sanjicaidan7_3_7_9[maxLanguage][41]={{"    select number,Enter to confirm"},{"    selecione numero,Enter para confirm."},{' ',' ',' ',' ','s','e','l','e','c','c',' ','n',0X86,'m','e','r','o',',','E','n','t','e','r',' ','p','/',' ','c','o','n','f','i','r','m','a','r'}};
	//const unsigned char sanjicaidan7_3_7_11[maxLanguage][16]={{"Valid soon"},{"Valido em breve"}};
	//const unsigned char sanjicaidan7_3_7_11[maxLanguage][26]={{"Max expansion number :"},{"Max numero expansao :"},{'N',0x86,'m','e','r','o',' ','m',0x9d,'x',' ','d','e',' ','e','x','p','a','n','s','i',0x8d,'n',':'}};
	const unsigned char sanjicaidan7_3_7_11[maxLanguage][30]={{"Max Extinguishant number :"},{"Max numero extintor :"},{'N',0x86,'m','e','r','o',' ','m',0x9d,'x',' ','d','e',' ','e','x','t','i','n','t','o','r',':'}};
	
	const unsigned char sanjicaidan7_3_7_10[maxLanguage][28]={{"Max repeater number :"},{"Max numero repeater :"},{'N',0x86,'m','e','r','o',' ','m',0x9d,'x',' ','d','e',' ','r','e','p','e','t','i','d','o','r',':'}};

	//const unsigned char sanjicaidan7_3_7_12[maxLanguage][23]={{"Max expansion number :"},{"Max numero expansao :"}};
	const unsigned char sanjicaidan7_3_7_12[maxLanguage][41]={{"4.Read/Write wireless parameters"},{"4.Read/Write wireless parameters"},{'4','.','L','e','e','r','/','E','s','c','r',' ','p','a','r',0x9d,'m','e','t','r','o','s',' ','w','i','r','e','l','e','s','s'}};//{"4.Leer/Escr paramet/ wireless"}
	const unsigned char sanjicaidan7_3_7_12_WF[maxLanguage][40]={{"5.Configure wifi parameters"},{"5.Configure wifi parameters"},{"5.Config paramet/ wifi"}};
	const unsigned char sanjicaidan7_3_7_12_GSM[maxLanguage][40]={{"6.Configure GSM parameters"},{"6.Configure GSM parameters"},{"6.Config paramet/ GSM"}};
	const unsigned char sanjicaidan7_3_7_12_network[maxLanguage][40]={{"7.Configure network"},{"7.Configure network"},{"7.Cnonfig Red"}};
	const unsigned char	sanjicaidan7_3_7_12_GSM_1[maxLanguage][34]={{"    Select item,enter to confirm"},{"    Select item,enter to confirm"},{' ',' ',' ',' ','S','e','l','e','c','c',' ',0XA3,'t','e','m',',',' ','E','n','t','e','r',' ','p','/',' ','c','o','n','f'}};
	const unsigned char sanjicaidan7_3_7_12_WF_51[maxLanguage][40]={{"5.1 Connect to the AP"},{"5.1 Connect to the AP"},{"5.1 Connect al AP"}};
	const unsigned char sanjicaidan7_3_7_12_WF_52[maxLanguage][40]={{"5.2 Connect to LAN server"},{"5.2 Connect to LAN server"},{"5.2 Connect servidor LAN"}};
	//const unsigned char sanjicaidan7_3_7_12_WF_53[maxLanguage][40]={{"5.3 Panel ID & Net ID"},{"5.3 Panel ID & Net ID"}};	
	const unsigned char sanjicaidan7_3_7_12_WF_53[maxLanguage][40]={{"5.3 Connect to internet server"},{"5.3 Connect to internet server"},{"5.3 Connect servidor internet"}};
	const unsigned char sanjicaidan7_3_7_12_WF_54[maxLanguage][40]={{"5.4 CIE number & Serial ID"},{"5.4 CIE number & Serial ID"},{'5','.','4',' ','N',0X86,'m','e','r','o',' ','C','I','E',' ','&',' ','S','e','r','i','a','l',' ','I','D'}};	
	const unsigned char sanjicaidan7_3_7_12_WF_55[maxLanguage][40]={{"5.5 Graphic software authorization code"},{"5.5 Graphic software authorization code"},{'5','.','5',' ','C',0x8d,'d','i','g','o',' ','d','e',' ','a','u','t','o','r','i','z','a','c','i',0x8d,'n',' ','d','e',' ','s','o','f','t','w','a','r','e'}};	
	const unsigned char sanjicaidan7_3_7_12_WF_551[maxLanguage][40]={{"Access Code:"},{"Access Code:"},{'C',0X8D,'d','i','g','o',' ','d','e',' ','a','c','c','e','s','o'}};
	const unsigned char sanjicaidan7_3_7_12_WF_511[maxLanguage][40]={{"5.1.1 Use app to auto connect"},{"5.1.1 Use app to auto connect"},{"5.1.1 Usar app p/ auto-connect"}};
	const unsigned char sanjicaidan7_3_7_12_WF_512[maxLanguage][40]={{"5.1.2 Manual connect to AP"},{"5.1.2 Manual connect to AP"},{"5.1.2 Connect Manualmt al AP"}};
	const unsigned char sanjicaidan7_3_7_12_WF_513[maxLanguage][2][40]={{{"Use app to connect? N"},{"Use app to connect? Y"}},{{"Use app to connect? N"},{"Use app to connect? Y"}},{{"Usar app p/ auto-connect? N"},{"Usar app p/ auto-connect? Y"}}};
	const unsigned char sanjicaidan7_3_7_12_WF_514[maxLanguage][40]={{"Status :"},{"Status :"},{"Status:"}};
	//const unsigned char sanjicaidan7_3_7_12_WF_515[maxLanguage][40]={{"Panel ip :"},{"Panel ip :"}};
	const unsigned char sanjicaidan7_3_7_12_WF_515[maxLanguage][40]={{"CIE IP :"},{"CIE IP :"},{"CIE IP:"}};
	const unsigned char sanjicaidan7_3_7_12_WF_516[maxLanguage][3][40]={{{"No AP connected"},{"Connected to AP"},{"Connecting to AP..."}},{{"No AP connected"},{"Connected to AP"},{"Connecting to AP..."}},{{"No AP connectado"},{"Conectado a AP"},{"Conectando a AP..."}}};
	const unsigned char sanjicaidan7_3_7_12_WF_517[maxLanguage][40]={{"No IP address"},{"No IP address"},{"SIN Direcc IP"}};
	const unsigned char sanjicaidan7_3_7_12_WF_518[maxLanguage][2][40]={{{"Reconnect to AP? N"},{"Reconnect to AP? Y"}},{{"Reconnect to AP? N"},{"Reconnect to AP? Y"}},{{"Reconnectar al AP? N"},{"Reconnectar al AP? Y"}}};
	const unsigned char sanjicaidan7_3_7_12_WF_519[maxLanguage][40]={{"SSID:"},{"SSID:"},{"SSID:"}};
	const unsigned char sanjicaidan7_3_7_12_WF_51A[maxLanguage][40]={{"Password:"},{"Password:"},{'C','o','n','t','r','a','s','e',0xa7,'a',':'}};
	const unsigned char sanjicaidan7_3_7_12_WF_51B[maxLanguage][3][40]={{{"[No AP connected]"},{"[Connected to AP]"},{"[Connecting to AP...]"}},{{"[No AP connected]"},{"[Connected to AP]"},{"[Connecting to AP...]"}},{{"No AP connectado"},{"Conectado a AP"},{"Conectando a AP..."}}};
	const unsigned char sanjicaidan7_3_7_12_WF_51C[maxLanguage][40]={{"        Select and Enter to confirm"},{"        Select and Enter to confirm"},{"        Selecc y Enter p/ confim"}};
	const unsigned char sanjicaidan7_3_7_12_WF_521[maxLanguage][2][40]={{{"To Reconnect? N"},{"To Reconnect? Y"}},{{"To Reconnect? N"},{"To Reconnect? Y"}},{{"P/ reconnect? N"},{"P/ reconnect? Y"}}};
	const unsigned char sanjicaidan7_3_7_12_WF_522[maxLanguage][40]={{"IP:    .   .   .      Port:      "},{"IP:    .   .   .      Port:      "},{"IP:    .   .   .    Puerto:"}};
	const unsigned char sanjicaidan7_3_7_12_WF_523[maxLanguage][3][40]={{{"Status: Disconnected"},{"Status: Connected"},{"Status: Connecting"}},{{"Status: Disconnected"},{"Status: Connected"},{"Status: Connecting"}},{{"Status: Desconectado"},{"Status: conectado"},{"Status: Conectando"}}};
	//const unsigned char sanjicaidan7_3_7_12_WF_531[maxLanguage][40]={{"Change the panel ID,Enter to Confirm"},{"Change the panel ID,Enter to Confirm"}};
	//const unsigned char sanjicaidan7_3_7_12_WF_532[maxLanguage][40]={{"Panel ID:"},{"Panel ID:"}};
	//const unsigned char sanjicaidan7_3_7_12_WF_533[maxLanguage][40]={{"Net ID:"},{"Net ID:"}};
	const unsigned char sanjicaidan7_3_7_12_WF_531[maxLanguage][40]={{"Change the CIE number,Enter to Confirm"},{"Change the CIE number,Enter to Confirm"},{'C','a','m','b','i','a','r',' ','n',0x86,'m','e','r','o',' ','C','I','E',',',' ','E','n','t','e','r',' ','p','/',' ','C','o','n','f','i','m','a','r'}};
	const unsigned char sanjicaidan7_3_7_12_WF_532[maxLanguage][40]={{"CIE number:"},{"CIE number:"},{'N',0x86,'m','e','r','o',' ','C','I','E',':'}};
	const unsigned char sanjicaidan7_3_7_12_WF_533[maxLanguage][40]={{"CIE serial ID:"},{"CIE serial ID:"},{"Serial ID CIE:"}};

	const unsigned char sanjicaidan7_3_7_13[maxLanguage][33]={{"Change the Value,Enter to Write"},{"Change the Value,Enter to Write"},{"Cambiar el Valor, Enter p/ Escrib"}};
	const unsigned char  sanjicaidan7_3_7_14[maxLanguage][41]={{"W:Add     Netid     R:Add     Netid     "},{"W:Add     Netid     R:Add     Netid     "},{"W:Add     Netid     R:Add     Netid     "}};
	
	const unsigned char  sanjicaidan7_3_7_14_DW[maxLanguage][41]={{"W:Add     NID     Ch     FT   BUZ   SD "},{"W:Add     NID     Ch     FT   BUZ   SD "},{"W:Add     NID     Ch     FT   BUZ   SD "}};
	const unsigned char  sanjicaidan7_3_7_14_DR[maxLanguage][41]={{"R:Add     NID     Ch     FT   BUZ   SD "},{"R:Add     NID     Ch     FT   BUZ   SD "},{"R:Add     NID     Ch     FT   BUZ   SD "}};
	
	const unsigned char  sanjicaidan7_3_7_14_MW[maxLanguage][41]={{"W:Add     NID     Ch     Flash   "},{"W:Add     NID     Ch     Flash   "},{"W:Add     NID     Ch     Flash   "}};
	const unsigned char  sanjicaidan7_3_7_14_MR[maxLanguage][41]={{"R:Add     NID     Ch     Flash   Mode  "},{"R:Add     NID     Ch     Flash   Mode  "},{"R:Add     NID     Ch     Flash   Mode  "}};
	
	const unsigned char sanjicaidan7_3_7_15[maxLanguage][40]={{"4.1 R/W wireless node parameters"},{"4.1 R/W wireless node parameters"},{"4.1 L/E paramt/ nodo wireless"}};
	const unsigned char sanjicaidan7_3_7_16[maxLanguage][40]={{"4.2 R/W wireless device parameters"},{"4.2 R/W wireless device parameters"},{"4.2 L/E paramt/ disp wireless"}};
	const unsigned char   sanjicaidan7_3_7_17[maxLanguage][41]={{"Channel:    LPower:    DSend:    Beep:   "},{"Channel:    LPower:    DSend:    Beep:   "}};
	const unsigned char   sanjicaidan7_3_7_18[maxLanguage][41]={{"4.1.1 Read node version and Status"},{"4.1.1 Read node version and Status"},{"4.1.1 Leer version y status nodo"}};
//	const unsigned char   sanjicaidan7_3_7_20[maxLanguage][41]={{"4.1.3 R/W module set parameters"},{"4.1.3 R/W module set parameters"}};
//	const unsigned char   sanjicaidan7_3_7_19[maxLanguage][41]={{"4.1.2 R/W module net address parameters"},{"4.1.2 R/W module net address parameters"}};
	const unsigned char   sanjicaidan7_3_7_19[maxLanguage][41]={{"4.1.2 R/W node  parameters"},{"4.1.2 R/W node parameters"},{"4.1.2 L/E paramt/ nodo wireless"}};

	const unsigned char   sanjicaidan7_3_7_21[maxLanguage][41]={{"4.2.1 Read device version and Status"},{"4.2.1 Read device version and Status"},{"4.2.1 Leer version y status disposit"}};
//	const unsigned char   sanjicaidan7_3_7_23[maxLanguage][41]={{"4.2.3 R/W device set parameters"},{"4.2.3 R/W device set parameters"}};
//	const unsigned char   sanjicaidan7_3_7_22[maxLanguage][41]={{"4.2.2 R/W device net address parameters"},{"4.2.2 R/W device net address parameters"}};
	const unsigned char   sanjicaidan7_3_7_22[maxLanguage][41]={{"4.2.2 R/W device  parameters"},{"4.2.2 R/W device parameters"},{"4.2.2 L/E paramt/ disposit"}};

	const unsigned char   sanjicaidan7_3_7_24[maxLanguage][41]={{"Press ESC to return"},{"Press ESC to return"},{"Pulse ESC p/ Retornar"}};
	const unsigned char   sanjicaidan7_3_7_25[maxLanguage][41]={{"Module version:    Type:"},{"Module version:    Type:"},{"Version Modulo:    Tipo:"}};

	const unsigned char   sanjicaidan7_3_7_26[maxLanguage][41]={{"Module Value:    Low Battery:     "},{"Module Value:    Low Battery:     "},{"Valor Modulo:    Baja Bateria:     "}};
	const unsigned char   sanjicaidan7_3_7_27[maxLanguage][2][8]={{{"Normal"},{"Fault"}},{{"Normal"},{"Fault"}},{{"Normal"},{"Falla"}}};
	const unsigned char   sanjicaidan7_3_7_30[maxLanguage][2][8]={{{"N"},{"Y"}},{{"N"},{"Y"}},{{"N"},{"Y"}}};

	const unsigned char   sanjicaidan7_3_7_30_M[maxLanguage][2][8]={{{"OFF"},{"FLASH"}},{{"OFF"},{"FLASH"}},{{"OFF"},{"FLASH"}}};
	const unsigned char   sanjicaidan7_3_7_28[maxLanguage][41]={{"W: ch    fault     beep    "},{"W: ch    fault     beep    "},{"E: ch    falla     beep    "}};
	const unsigned char   sanjicaidan7_3_7_29[maxLanguage][41]={{"R: ch    fault     beep    "},{"R: ch    fault     beep    "},{"L: ch    falla     beep    "}};

	const unsigned char   sanjicaidan7_3_7_28_M[maxLanguage][41]={{"W: ch    fault     beep    led       "},{"W: ch    fault     beep    led       "},{"E: ch    falla     beep    led"}};
	const unsigned char   sanjicaidan7_3_7_29_M[maxLanguage][41]={{"R: ch    fault     beep    led       "},{"R: ch    fault     beep    led       "},{"L: ch    falla     beep    led"}};


		const unsigned char   sanjicaidan7_3_7_31[maxLanguage][41]={{"Device version:    Type:"},{"Device version:    Type:"},{"Version Disp  :    Tipo:"}};

	const unsigned char   sanjicaidan7_3_7_32[maxLanguage][41]={{"Device Value:    Low Battery:     "},{"Device Value:    Low Battery:     "},{"Valor Disp :     Baja Bateria:    "}};
	const unsigned char    sanjicaidan7_3_7_33[maxLanguage][2][10]={{{"Module"},{"Device"}},{{"Module"},{"Device"}},{{"ModulO"},{"Disp"}}};	
	const	unsigned char	sanjicaidan7_3_7_34[maxLanguage][6]={{"RSSI:"},{"RSSI:"},{"RSSI:"}};
	
	const 	unsigned char	sanjicaidan7_3_7_35_GSM[maxLanguage][30]={{"6.1 En(Dis)abled GSM module"},{"6.1 En(Dis)abled GSM module"},{"6.1 (Dis)Habilitar modulo GSM"}};
	const 	unsigned char	sanjicaidan7_3_7_35_GSM_1[maxLanguage][41]={{"    Enabled GSM module,enter to confirm"},{"    Enabled GSM module,enter to confirm"},{"    Habilitado Modulo GSM,Enter p/confim"}};
	const 	unsigned char	sanjicaidan7_3_7_35_GSM_2[maxLanguage][12]={{"GSM module:"},{"GSM module:"},{"Modulo GSM:"}};
	const 	unsigned char	sanjicaidan7_3_7_35_GSM_3[maxLanguage][2][10]={{{"Disabled"},{"Enabled"}},{{"Disabled"},{"Enabled"}},{{"Dishab"},{"Habilit"}}};
	const 	unsigned char	sanjicaidan7_3_7_36_GSM[maxLanguage][29]={{"6.2 Select events"},{"6.2 Select events"},{"6.2 Selecc eventos"}};
	const 	unsigned char	sanjicaidan7_3_7_36_GSM_1[maxLanguage][39]={{"    Select event,    toggle status"},{"    Select event,    toggle status"},{"    Selecc evento,    alternar status"}};
	const 	unsigned char	sanjicaidan7_3_7_36_GSM_2[maxLanguage][32]={{"Fire:                 Fault: "},{"Fire:                 Fault: "},{"Incd:                 Falla: "}};
	const 	unsigned char	sanjicaidan7_3_7_36_GSM_3[maxLanguage][32]={{"Pre-alarm:            Test: "},{"Pre-alarm:            Test: "},{"Pre-alarma:          Prueba: "}};
	const 	unsigned char	sanjicaidan7_3_7_37_GSM[maxLanguage][40]={{"6.3 Set phone numbers"},{"6.3 Set phone numbers"},{'6','.','3',' ','E','s','t','.',' ','N',0x86,'m','e','r','o',' ','t','e','l',0x96,'f'}};
	const 	unsigned char	sanjicaidan7_3_7_38_network[maxLanguage][41]={{"7.1 Configure network & loop assign"},{"7.1 Configure network & loop assign"},{"7.1 Config red & Assig lazo"}};
	const 	unsigned char	sanjicaidan7_3_7_38_network_1[maxLanguage][41]={{"7.2 Configure CIE network ID"},{"7.2 Configure CIE network ID"},{"7.2 Config ID red CIE"}};
	const 	unsigned char	sanjicaidan7_3_7_38_network_2[2][41]={{"    select field and Enter to confirm"},{"    select field and Enter to confirm"}};

	const 	unsigned char	sanjicaidan7_3_7_38_network_3[maxLanguage][41]={{"Quantity of network CIE :"},{"Quantity of network CIE :"},{"Cantidad de red CIE :"}};
	const 	unsigned char	sanjicaidan7_3_7_38_network_4[maxLanguage][41]={{"CIE    contains loops :"},{"CIE    contains loops :"},{"CIE    contiene lazos:"}};
	const 	unsigned char	sanjicaidan7_3_7_38_network_5[maxLanguage][41]={{"Input number and Enter to confirm"},{"Input number and Enter to confirm"},{'I','n','g','r','e','s','s',' ','n',0x86,'m','e','r','o',' ','y',' ','E','n','t','e','r',' ','p','/',' ','c','o','n','f','i','m'}};
	const 	unsigned char	sanjicaidan7_3_7_38_network_6[maxLanguage][41]={{"CIE network ID : "},{"CIE network ID : "},{"ID de red CIE:"}};
	const unsigned char sanjicaidan7_3_9_1[]={"CHECKSUM = "};
	const unsigned char sanjicaidan7_3_8_1[maxLanguage][18]={{"Calculating......"},{"Calculando......"},{"Calculando..."}};
	const unsigned char sanjicaidan7_3_8_2[]={"                                                    "};
	const unsigned char  erjicaidan8_1[maxLanguage][25]={{"8-1   Button test"},{'8','-','1',' ',' ',' ','B','o','t',0x9c,'o',' ','p','a','r','a',' ','t','e','s','t','e'},{'8','-','1',' ',' ',' ','B','o','t',0x9c,'o',' ','p','a','r','a',' ','t','e','s','t','e'}};
	const unsigned char  erjicaidan8_2[maxLanguage][30]={{"8-2   Relay & sounder test"},{"8-2   Teste de rele & sirene"},{"8-2   Teste de rele & sirene"}};
	const unsigned char  erjicaidan8_3[maxLanguage][35]={{"8-3   Analog value dispLay"},{	'8','-','3',' ',' ',' ','E','x','i','b','i',0xc4,0x9c,'o',' ','d','o',' ','v','a','l','o','r',' ','a','n','a','o','g','i','c','o'},{	'8','-','3',' ',' ',' ','E','x','i','b','i',0xc4,0x9c,'o',' ','d','o',' ','v','a','l','o','r',' ','a','n','a','o','g','i','c','o'}};
	const unsigned char  erjicaidan8_4[maxLanguage][35]={{"8-4   Inquire IMEI"},{"8-4 Solicitar IMEI"},{"8-4 Solicitar IMEI"}};
	const unsigned char  erjicaidan8_1_1[]={"Please Enter the button and confirm"};
	const unsigned char  erjicaidan8_1_2[]={"Enter ESC exit button test"};
	const unsigned char  erjicaidan8_1_3[]={"Button : "};
	const unsigned char  erjicaidan8_1_4[32][30]={{""},{"Fire Relay"},{"Fault Relay"},{"SELECTED DETECTORS"},{"DELAYS ACTIVE"},{"DISABLE SOUNDERS"},{"BUZZER SILENCE"},{"SYSTEM RESET"},{"SOUNDERS ACTIVATE/SILENCE"},{"ENT"},{"Up"},{"Down"},{"Right"},{"Left"},{"ESC"},
										{"QUEUE REVIEW FIRE"},{"QUEUE REVIEW FAULT"},{"QUEUE REVIEW TEST"},{"QUEUE REVIEW DISABLED"},{"LAMP TEST"},{"1"},{"2"},{"3"},{"4"},{"5"},{"6"},{"7"},{"8"},{"9"},{"0"},{"-"},{"+"}};
	const unsigned char  erjicaidan8_2_1[maxLanguage][40]={{"Please Enter the Button Fault,Fire,TEST"},{"To test the Relay,SOUNDERS"}};
	const unsigned char  erjicaidan8_2_2[5][7]={{"Fault:"},{"Fire1:"},{"Fire2:"},{"S1:"},{"S2:"}};
	const unsigned char  erjicaidan8_2_3[2][4]={{"OFF"},{"ON"}};
	const unsigned char  erjicaidan8_3_1[8][20]={{"Loop1 ADP:     ADN:"},{"Loop2 ADP:     ADN:"},{"Main Power:"},{"Battery:"},{"Charger:"},{"Earth:"},{"S1:"},{"S2:"}};
	const unsigned char  erjicaidan8_3_2[20]={"Loop  ADP:     ADN:"};
	const unsigned char  erjicaidan8_3_3[30]={"20mA:    PL:    PH:   "};
	const unsigned char  erjicaidan8_4_1[maxLanguage][40]={{"Press the Enter button to get the IMEI"},{"Pressione Enter para obter o IMEI"},{"Pressione Enter para obter o IMEI"}};
	const unsigned char  erjicaidan8_4_2[maxLanguage][35]={{"IMEI:"},{"IMEI:"}};
	const unsigned char  erjicaidan8_4_3[maxLanguage][2][35]={{{"GSM module status:disconnected"},{"GSM module status:connected"}},{{"Status do modulo GSM:desconectado"},{"Status do modulo GSM:conectado"}},{{"Status do modulo GSM:desconectado"},{"Status do modulo GSM:conectado"}}};
	const unsigned char erjicaidan8_4_4[maxLanguage][35]={{"Getting..."},{"Obtendo..."},{"Obtendo..."}};
	const unsigned char erjicaidan8_4_5[maxLanguage][35]={{"Retry it later"},{"Tente novamente mais tarde"},{"Tente novamente mais tarde"}};
	const unsigned char mima[7][7]={{"?????"},{"*????"},{"**???"},{"***??"},{"****?"},{"*****"}};//,{"******"}};
	const unsigned char mima1[7][7]={{"??????"},{"*?????"},{"**????"},{"***???"},{"****??"},{"*****?"},{"******"}};
	const unsigned char typename[maxLanguage][32][19]={{{"NOT FITTED"},{"SHOP MONITOR"},{"SOUNDER"},{"INPUT/OUTPUT"},{"IONISATION DET"},{"ZONE MONITOR"},{"OPTICAL DET"},{"HEAT DET"},{"CALL POINT"},
		{"GASEOUS MONITOR"},{"PERS PROT MONITOR"},{"ENVIRON GAS MONI"},{"SWITCH MONITOR"},{"ASPIRATING DEVICE"},{"BEAM DET"},{"FLAME DET"},{"MULTI DET"},
		{"HIGH TEMP DET"},{"CO DET"},{"WIRELESS NODE"},{"GAS DET"},{"REMOTE CONTROL"},{"OPTICAL DET-W"},{"HEAT DET-W"},{"CALL POINT-W"},{"MULTI DET-W"},{"CO DET-W"},{"GAS DET-W"},{"UNKNOW"},{"INPUT/OUTPUT-W"},{"SOUNDER-W"},{"UNKNOW"}},
	//{"OTHER"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"}},
																						//22 开始

	
		{{"INEXISTENTE"},{"SHOP MONITOR"},{"SIRENE"},{"ENTRDA/SAIDA"},{"DET. IONICO"},{"MONITOR ZONA "},{"DETECTOR OPT"},{"DETECTOR TEMP."},{"BONTONEIRA"},
		{"MONITOR GAS"},{"MONITOR PERS PROT"},{"MONI EVIRON GAS"},{"SWITCH MONITOR"},{"ASPIRATING DISP."},{"DETECTOR LINEAR"},{"DETECTOR CHAMA"},{"MULTI DET"},
		{"ALTA TEMP DET"},{"CO DET"},{"MODULO SEM FIO"},{"DET GAS"},{"CONTROLE REMOTO"},{"DETECTOR OPT-W"},{"DETECTOR TEMP.-W"},{"BONTONEIRA-W"},{"MULTI DET-W"},{"CO DET-W"},{"DET GAS-W"},{"DESCONHECIDO"},{"ENTRDA/SAIDA-W"},{"SIRENE-W"},{"DESCONHECIDO"}},


	       {{"NO EQUIPADO"},{"Monitor Tienda"},{"SIRENA"},{"ENTRADA/SALIDA"},{"DET DE IONIZACION"},{"MONITOR DE ZONA"},{"DETECTOR OPT"},{"DETECTOR TEMP."},{"PULSADOR"},
		{"MONITOR GASEOSO"},{"MONITOR PERS PROT"},{"MONI GAS AMBTAL"},{"MONITOR INTERRT"},{'D','I','S','P','T',' ','A','S','P','I','R','A','C','I',0x89,'N'},{"DET LINEAR"},{"DET LLAMA"},{"DET MULTI"},
		{"DET ALTA TEMP"},{"DET CO"},{"MODULO WIRELESS"},{"DET GAS"},{"CONTROL REMOTO"},{"DETECTOR OPT-W"},{"DETECTOR TEMP.-W"},{"PULSADOR-W"},{"DET MULTI-W"},{"DET CO-W"},{"DET GAS-W"},{"DESCONOCIDO"},{"ENTRDA/SAIDA-W"},{"SIRENA-W"},{"DESCONOCIDO"}}};
		







	const unsigned char typename3[maxLanguage][31][19]={{{"NOT FITTED"},{"SHOP MONITOR"},{"SOUNDER"},{"INPUT/OUTPUT"},{"IONISATION DET"},{"ZONE MONITOR"},{"OPTICAL DET-W"},{"HEAT DET-W"},{"CALL POINT-W"},
		{"GASEOUS MONITOR"},{"PERS PROT MONITOR"},{"ENVIRON GAS MONI"},{"SWITCH MONITOR"},{"ASPIRATING DEVICE"},{"BEAM DET"},{"FLAME DET"},{"MULTI DET-W"},
		{"HIGH TEMP DET"},{"CO DET-W"},{"WIRELESS NODE"},{"GAS DET-W"},{"REMOTE CONTROL"},{"OPTICAL DET-W"},{"HEAT DET-W"},{"CALL POINT-W"},{"MULTI DET-W"},{"CO DET-W"},{"GAS DET-W"},{"UNKNOW"},{"INPUT/OUTPUT-W"},{"SOUNDER-W"}},

	
		{{"INEXISTENTE"},{"SHOP MONITOR"},{"SIRENE"},{"ENTRDA/SAIDA"},{"DET. IONICO"},{"MONITOR ZONA"},{"DETECTOR OPT-W"},{"DETECTOR TEMP.-W"},{"BONTONEIRA-W"},
		{"MONITOR GAS"},{"MONITOR PERS PROT"},{"MONI EVIRON GAS"},{"SWITCH MONITOR"},{"ASPIRATING DISP."},{"DETECTOR LINEAR"},{"DETECTOR CHAMA"},{"MULTI DET-W"},
		{"ALTA TEMP DET"},{"CO DET-W"},{"MODULO SEM FIO"},{"DET GAS-W"},{"CONTROLE REMOTO"},{"DETECTOR OPT-W"},{"DETECTOR TEMP.-W"},{"BONTONEIRA-W"},{"MULTI DET-W"},{"CO DET-W"},{"DET GAS-W"},{"DESCONHECIDO"},{"ENTRDA/SAIDA-W"},{"SIRENE-W"}},

	
		{{"NO EQUIPADO"},{"Monitor Tienda"},{"SIRENA"},{"ENTRADA/SALIDA"},{"DET DE IONIZACION"},{"MONITOR DE ZONA"},{"DETECTOR OPT-W"},{"DETECTOR TEMP.-W"},{"PULSADOR-W"},
		{"MONITOR GASEOSO"},{"MONITOR PERS PROT"},{"MONI GAS AMBTAL"},{"MONITOR INTERRT"},{'D','I','S','P','T',' ','A','S','P','I','R','A','C','I',0x89,'N'},{"DET LINEAR"},{"DET LLAMA"},{"DET MULTI-W"},
		{"DET ALTA TEMP"},{"DET CO"},{"MODULO WIRELESS"},{"DET GAS"},{"CONTROL REMOTO"},{"DETECTOR OPT-W"},{"DETECTOR TEMP.-W"},{"PULSADOR-W"},{"DET MULTI-W"},{"DET CO-W"},{"DET GAS-W"},{"DESCONOCIDO"},{"ENTRADA/SALIDA-W"},{"SIRENA-W"}}};
		
	
	const unsigned char typenameno[maxLanguage][32]={{10,12,7,12,14,12,11,8,10,15,17,16,14,17,8,9,9,13,6,15,7,14,13,10,12,11,8,9,6,14,9,6},
											 {11,12,6,12,11,12,12,14,10,11,17,15,14,16,15,14,9,13,6,14,7,15,14,16,12,11,8,9,12,14,8,12},
											 {11,14,6,14,17,15,12,14,8,15,17,15,15,16,10,9,9,13,6,15,7,14,14,16,10,11,8,9,11,16,8,11}};

	const unsigned char typenameno3[maxLanguage][31]={{10,12,7,12,14,12,13,10,12,15,17,16,14,17,8,9,11,13,8,15,9,14,13,10,12,11,8,9,6,14,9},
													 {11,12,6,12,11,12,14,16,12,11,17,15,14,16,15,14,11,13,8,14,14,16,10,11,8,9,12,13,8},
													 {11,14,6,14,17,15,14,16,10,15,17,15,15,16,10,9,11,13,6,15,7,14,14,16,10,11,8,9,11,16,8}};
	const	unsigned char	typenorma[maxLanguage][7]={{"NORMAL"},{"NORMAL"},{"NORMAL"}};
	const	unsigned char	typenodedevice[maxLanguage][7][9]={{{"No det"},{"Optical"},{"Heat"},{"Multi"},{"I/O"},{"Sounder"},{"MCP"}},{{"No det"},{"Optical"},{"Heat"},{"Multi"},{"I/O"},{"Sounder"},{"MCP"}},{{"No det"},{"Optical"},{"Heat"},{"Multi"},{"I/O"},{"Sounder"},{"MCP"}}};
	const unsigned char  allon[41]={4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};
	const unsigned char	ModeDisp[maxLanguage][2][20]={{{"[Installation Mode]"},{"[Active Mode]"}},{{'[','M','o','d','o',' ','I','n','s','t','a','l','a',0xc4,0x9c,'o',']'},{"[Modo Ativo]"}},
	{{'[','M','o','d','o',' ','I','n','s','t','a','l','a','c','i',0x8d,'n',']'},{"[Modo Ativo]"}}};
	
	const unsigned	char   pcode[8][8]={
			{0x00,0x00,0x00,0x10,0x18,0x1c,0x18,0x10},
			{0x00,0x04,0x04,0x0e,0x0e,0x1f,0x1f,0x00},//↑
			{0x00,0x1f,0x1f,0x0e,0x0e,0x04,0x04,0x00},//↓
			{0x10,0x1c,0x1f,0x1c,0x10,0x00,0x00,0x00},//→
			{0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f},
			{0x01,0x07,0x1f,0x07,0x01,0x00,0x00,0x00},//向←
			{0x00,0x00,0x00,0x10,0x18,0x1c,0x18,0x10},
			{0x00,0x00,0x00,0x10,0x18,0x1c,0x18,0x10}};

	const unsigned char  history[maxLanguage][47][40]={{{"SYSTEM RESET          AUTO RESETS"},{"SYSTEM RESET          AUTO RESETS"},{"ENTER KEYPAD FUNCTIONS ACCESS LEVEL 0"},{"SOUND ALARMS ON"},{"SOUND ALARMS OFF"},{"ALARM SILENCED"},
		{"INTERNAL BUZZER SILENCED"},{"CLEAR AUTOSTART COUNT FROM"},{"EXIT KEYPAD FUNCTION, NO CHANGES"},{"EXIT KEYPAD FUNCTION, CHANGE MADE"},{"DISABLED          ZONE"},{"ENABLED           ZONE"},{"DISABLED          SOUNDER     LOOP"},
		{"ENABLED           SOUNDER     LOOP"},{"DISABLED     CONV SOUNDER    PANEL 01"},{"ENABLED      CONV SOUNDER    PANEL 01"},{"DISABLED          LOOP"},{"ENABLED           LOOP"},{"DISABLED          DEV     L"},{"ENABLED           DEV     L"},
		{"FIRE RELAYS ISOLATED"},{"FIRE RELAYS DE-ISOLATED"},{"SOUNDERS DISABLED"},{"SOUNDERS ENABLED"},{"DELAYS ACTIVE ISOLATED"},{"DELAY ACTIVE DE-ISOLATED"},{"SELECTED DETECTORS ISOLATED"},{"SELECTED DETECTORS DE-ISOLATED"},
		{"ACTIVE MODE SELECTED"},{"INSTALATION MODE SELECTED"},{""},{""},{""},{""},{""},{""},{""},{""},{""},{"TWO DEVICES ON ONE ADDRESS"},{"DEVICE REMOVED"},{""},{""},{""},{"WRONG DEVICE FITTED"},{"FAULT RELAYS ISOLATED"},{"FAULT RELAYS DE-ISOLATED"}},


		{{"SISTEMA DE RESET      RESETS AUTO"},{"SISTEMA DE RESET      RESETS AUTO"},{'E','N','T','R','A','R',' ','C','O','M',' ','F','U','N',0XC4,0X8C,'E','S',' ','D','E',' ','N','I','V','E','L',' ','A','C','C','E','S','S','O',' ','0'},{"SOM DE ALARMES LIGADO"},{"SOM DE ALARMES DESLIGADOS"},{"ALARME SILENCIADO"},
		{"BUZZER INTERNO SILENCIADO"},{"APAGAR CONTAGEM DECRESCENTE"},{'F','U','N',0XC4,0X9C,'O',' ','S','A','I','D','A',' ','S','E','M',' ','A','L','T','E','R','A',0XC4,0X9C,'O'},{'F','U','N',0XC4,0X9C,'O',' ','S','A','I','D','A',' ','C','O','M',' ','A','L','T','E','R','A',0XC4,0X9C,'O'},{"ZONA      DESABILITADA"},{"ZONA         ABILITADA"},{'D','E','S','A','B','I','L','I','T','A','D','O',' ',' ',' ',' ',' ',' ',' ','S','I','R','E','N','E',' ',' ',' ',' ',' ',' ','L','A',0XC4,'O'},
		{'H','A','B','I','L','I','T','A','D','O',' ',' ',' ',' ',' ',' ',' ',' ',' ','S','I','R','E','N','E',' ',' ',' ',' ',' ',' ','L','A',0XC4,'O'},{"DESABILITADA  SIRENE CONV    PAINEL 01"},{"HABILITADA    SIRENE CONV    PAINEL 01"},{'D','E','S','A','B','I','L','I','T','A','D','O',' ',' ',' ',' ',' ',' ','L','A',0XC4,'O'},{'H','A','B','I','L','I','T','A','D','O',' ',' ',' ',' ',' ',' ',' ',' ','L','A',0XC4,'O'},{"DESABILITADA     DISP     L"},{"HABILITADA       DISP     L"},
		{"FOGO RELES ISOLADOS"},{"FOGO RELES HABILITADA"},{"SIRENES DESABILITADAS"},{"SIRENES HABILITADAS"},{'T','E','M','P','O','R','I','Z','A',0XC4,0X8C,'E','S',' ','A','T','I','V','A',' ','D','E','S','A','B','I','L','I','T','A','D','A','S'},{'T','E','M','P','O','R','I','Z','A',0XC4,0X8C,'E','S',' ','A','T','I','V','A',' ','H','A','B','I','L','I','T','A','D','A','S'},{"DETECTORES SELECCIONADOS ISOLADOS"},{"DETECTORES SELECCIONADOS HABILITADOS"},
		{"MODO ATIVO SELECIONADO"},{'M','O','D','O',' ','I','N','S','T','A','L','A',0XC4,0X9C,'O',' ','S','E','L','E','C','I','O','N','A','D','O'},{""},{""},{""},{""},{""},{""},{""},{""},{""},{'D','O','I','S',' ','D','I','S','P','O','S','I','T','I','V','O','S',' ','C','O','M',' ','M','E','S','M','O',' ','E','N','D','E','R','E',0xc4,'O'},{"DOSPOSITIVOS REMOVIDO"},{""},{""},{""},{"DOSPOSITIVOS INCORRETO"},{"AVARIA RELES ISOLADOS"},{"AVARIA RELES HABILITADA"}},

		{{"RESET SISTEMA         RESETS AUTO"},{"RESET SISTEMA          RESETS AUTO"},{"NGRESE FUNCIONES TECLADO NIVEL ACCESO 0"},{"ALARM SONORAS ON"},{"ALARM SONORAS OFF"},{"ALARMA SILENC"},
		{"BUZZER INTERNO SILENCIADO"},{"BORRAR RECNTO INICIO AUT DESDE"},{'S','A','L','I','R',' ','F','U','N','C','I',0x89,'N',' ','T','E','C','L',',',' ','S','I','N',' ','C','A','M','B','I','O','S'},{'S','A','L','I','R',' ','F','U','N','C','I',0x89,'N',' ','T','E','C','L',',','C','A','M','B','I','O','S',' ','R','E','A','L','Z','D'},{"DESACTIVADA       ZONA"},{"HABILITADA        ZONA"},{"DESACT             SIRENA     LAZO"},
		{"HABILIT            SIRENA     LAZO"},{"DESACT        CONV SIRENA    PANEL 01"},{"HABILIT       CONV SIRENA    PANEL 01 "},{"DESACT           LAZO"},{"HABILIT           LAZO"},{"DESACT           DISP     L"},{"HABILIT          DISP     L"},
		{'F','U','E','G','O',' ','R','E','L',0X92,'S',' ','A','I','S','L','A','D','O','S'},{'F','U','E','G','O',' ','R','E','L',0X92,'S',' ','D','E','S','A','I','S','L','A','D','O','S'},{"SIRENAS DISACTS"},{"SIRENAS HABILITS"},{"RETARDOS ACTIVOS AISLADOS"},{"RETADOS ACTIVOS DEAISLADOS"},{"DETECTORES SELECCS AISLADOS"},{"DETECTORES SELECCS DEAISLADOS"},
		{"MODO ACTIVO SELECC"},{'M','O','D','O',' ','I','N','S','T','A','L','A','C','I',0X89,'N',' ','S','E','L','E','C','C'},{""},{""},{""},{""},{""},{""},{""},{""},{""},{'D','O','S',' ','D','I','S','P','T','S',' ','E','N',' ','U','N','A',' ','D','I','R','E','C','C','I',0x89,'N'},{"DISPT RETIRADO"},{""},{""},{""},{"DISPT INCORRECTO INST"},{'F','A','L','L','A',' ','R','E','L',0X92,'S',' ','A','I','S','L','A','D','O','S'},{'F','A','L','L','A',' ','R','E','L',0X92,'S',' ','D','E','S','A','I','S','L','A','D','O','S'}}
								};
	const unsigned char	firedisp[maxLanguage][4][5]={{{"FIRE"},{"ZONE"},{"DEV"},{"L"}},{{"FOGO"},{"ZONA"},{"DSP"},{"L"}},{{"FUEG"},{"ZONA"},{"DSP"},{"L"}}};
	const unsigned char	faultloopdisp[maxLanguage][4][7]={{{"FAULT"},{"ZONE"},{"DEV"},{"L"}},{{"AVARIA"},{"ZONA"},{"DSP"},{"L"}},{{"FALLA"},{"ZONA"},{"DSP"},{"L"}}};
	const unsigned char	peralarmdisp[maxLanguage][4][9]={{{"PREALARM"},{"ZONE"},{"DEV"},{"L"}},{{"PREALARM"},{"ZONA"},{"DSP"},{"L"}},{{"PREALARM"},{"ZONA"},{"DSP"},{"L"}}};
	const unsigned char	testdisp[maxLanguage][2][6]={{{"TEST"},{""}},{{"TESTE"},{""}},{{"TESTE"},{""}}};
	const unsigned char typename2[maxLanguage][31][11]={{{"NOT FITTED"},{"SHOP MONIT"},{"SOUNDER"},{"IO MODULE"},{"IONISATION"},{"ZONE MONIT"},{"OPTICAL-W"},{"HEAT DET-W"},{"MCP-W"},
		{"CO SENSOR"},{"CO SENSOR"},{"CO SENSOR"},{"SWITCH MON"},{"ASPIRATING"},{"BEAM DET"},{"FLAME DET"},{"MULT DET-W"},
		{"HIGH TEMP"},{"CO DET"},{"NODE-W"},{"GAS-W"},{"RM CONTROL"},{"OTHER"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"},{"UNKNOW"}},

	
		{{"INEXISTENT"},{"SHOP MON."},{"SIRENE"},{"ENTR./SAI."},{"DET.IONICO"},{"MONI. ZONA "},{"OPTICO-W"},{"DET. TEM-W"},{"BONTONEI-W"},
		{"CO SENSOR"},{"CO SENSOR"},{"CO SENSOR"},{"SWITCH MON"},{"ASPIRATING"},{"DET. LINEAR"},{"DET. CHAMA"},{"MULTI DET"},
		{"ALTA TEMP."},{"CO DET"},{"SEM FIO"},{"GAS-W"},{"RM CONTROL"},{"DETE OPT-W"},{"DETE TEM-W"},{"BONTONEI-W"},{"MULT DET-W"},{"CO DET-W"},{"DET GAS-W"},{"DESCONHEC."},{"DESCONHEC."},{"DESCONHEC."}},


		{{"NO EQUIPA."},{"Mon. Tien."},{"SIRENA"},{"ENTR./SAL."},{"IONIZACION"},{"MONI. ZONA"},{"DETE OPT-W"},{"DETE TEM-W"},{"PULSADOR-W"},
		{"SENSOR CO"},{"SENSOR CO"},{"SENSOR CO"},{"MONI INTER"},{'A','S','P','I','R','A','C','I',0x89,'N'},{"DET LINEAR"},{"DET LLAMA"},{"DET MULT-W"},
		{"ALTA TEMP."},{"DET CO"},{"PUERTA-W"},{"DET GAS-W"},{"CONTROL RM"},{"DETE OPT-W"},{"DETE TEM-W"},{"PULSADOR-W"},{"DET MULT-W"},{"DET CO-W"},{"DET GAS-W"},{"DESCONOCI."},{"DESCONOCI."},{"DESCONOCI."}}};

		const unsigned char typename1[maxLanguage][31][11]={{{"NOT FITTED"},{"SHOP MONIT"},{"SOUNDER"},{"IO MODULE"},{"IONISATION"},{"ZONE MONIT"},{"OPTICAL"},{"HEAT DET"},{"CALL POINT"},//8
		{"CO SENSOR"},{"CO SENSOR"},{"CO SENSOR"},{"SWITCH MON"},{"ASPIRATING"},{"BEAM DET"},{"FLAME DET"},{"MULTI DET"},//16
		{"HIGH TEMP"},{"CO DET"},{"NODE-W"},{"GAS-W"},{"RM CONTROL"},{"OPTICAL-W"},{"HEAT DET-W"},{"MCP-W"},{"MULT DET-W"},{"CO DET-W"},{"GAS-W"},{"UNKNOW"},{"IO-W"},{"SOUNDER-W"}},//30

	
		{{"INEXISTENT"},{"SHOP MONI."},{"SIRENE"},{"ENTR./SAI."},{"DET.IONICO"},{"MONI. ZONA "},{"OPTICO"},{"DET. TEMP."},{"BONTONEIRA"},
		{"CO SENSOR"},{"CO SENSOR"},{"CO SENSOR"},{"SWITCH MON"},{"ASPIRATING"},{"DET LINEAR"},{"DET. CHAMA"},{"MULTI DET"},
		{"ALTA TEMP."},{"CO DET"},{"SEM FIO"},{"GAS_W"},{"RM CONTROL"},{"DETE OPT-W"},{"DETE TEM-W"},{"BONTONEI-W"},{"MULT DET-W"},{"CO DET-W"},{"DET GAS-W"},{"DESCONHEC."},{"ENTR/SAI-W"},{"SIRENE-W"}},

		{{"NO EQUIPA."},{"Mon. Tien."},{"SIRENA"},{"ENTR./SAL."},{"IONIZACION"},{"MONI. ZONA"},{"DETE. OPT"},{"DET. TEMP."},{"PULSADOR"},
		{"SENSOR CO"},{"SENSOR CO"},{"SENSOR CO"},{"MONI INTER"},{'A','S','P','I','R','A','C','I',0x89,'N'},{"DET LINEAR"},{"DET LLAMA"},{"DET MULT-W"},
		{"ALTA TEMP."},{"DET CO"},{"PUERTA-W"},{"GAS-W"},{"CONTROL RM"},{"DETE OPT-W"},{"DETE TEM-W"},{"PULSADOR-W"},{"DET MULT-W"},{"DET CO-W"},{"DET GAS-W"},{"DESCONOCI."},{"ENTR/SAL-W"},{"SIRENA-W"}}	
		};
				
      const	 unsigned char	mainpanelfault[maxLanguage][14][41]={{{"MAIN PANEL"},{"CONVENTIONAL SOUNDER CIRCUIT   FAULT"},{"SUB-PANEL NUMBER"},{"OPEN OR SHORT CIRCUIT LOOP "},{"EARTH MONITORING FAULT"},{"LOOP SUPPLY FAULT"},{"CONV. SOUNDER SUPPLY FAULT,CHECK FUSE"},{"AUXILIARY SUPPLY FAULT, CHECK FUSE"},{"PRIMARY POWER SUPPLY FAULT"},{"SECONDARY SUPPLY FAULT"},{"CHARGE CIRCUIT FAULT, CHECK FUSE"},{"PROGRAM FLASH CHECKSUM FAULT"},{"EEPROM DATA CHECKSUM FAULT"},{"MAIN POWER SUPPLY FAULT,CHECK FUSE"}},
	  {{"PAINEL PRINCIPAL"},{"AVARIA NO CIRCUITO CONV. DE SIRENES"},{"SUB-PAINEL NUMERO"},{'A','V','A','R','I','A',' ','N','O',' ','L','A',0XC4,'O'},{"FUGA A TERRA"},{'A','V','A','R','I','A',' ','D','E',' ','A','I','M','E','N','T','A',0XC4,0X9C,'O'},{"AVARIA SIRENE CONV., VERIFIQUE O FUSIVEL"},{"AVARIA NA FONTE AUX.,VERIFIQUE O FUSIVEL"},{'A','V','A','R','I','A',' ','N','A',' ','A','L','I','M','E','N','T','A',0XC4,0X9C,'O',' ','P','R','I','M','A','R','I','A'},
{'A','V','A','R','I','A',' ','N','A',' ','A','L','I','M','E','N','T','A',0XC4,0X9C,'O',' ','S','E','C','U','N','D','A','R','I','A'},{"AVARIA CARREGADOR, VERIFIQUE O FUSIVEL"},{"AVARIA NA MEMORIA DE PROGRAMA"},{"AVARIA NA EPROM"},{"AVARIA DE ALIMENTA., VERIFIQUE O FUSIVEL"}},

	  {{"PANEL PRINC"},{"FALLA CIRCUITO SIRENA CONVENC"},{'N',0x82,'M','E','R','O',' ','S','U','B','-','P','A','I','N','E','L'},{"LAZO CIRC EM CORTO O ABIERTO"},{"FALLA MONITOREO TIERRA"},{"FALLA SUMINISTRO LAZO"},{"FALLA SUMINISTRO SIRENA CONV,VERIF FUSIB"},{"FALLA SUMINISTRO AUX,VERIF FUSIBLE"},{"FALLA SUMINISTRO DE ALIMT PRIM"},{"FALLA SUMINISTRO SECUND"},{"FALLA CIRC DE CARGA,VERF FUSIBLE"},{"FALLA PROGRAM FLASH CHECKSUM"},{"FALLA VERIFIC DATOS DE EEPROM"},{"FALLA ALIMEN PRINCP, VERIF FUSIBLE"}}
	  												};
      const	 unsigned char	Disablment[maxLanguage][14][13]={{{"DISABLED"},{"MORE"},{"SOUNDERS"},{"DELAYS"},{"RELAYS1"},{"RELAYS2"},{"L  /D"},{"ZONE"},{"P00/CS1"},{"P00/CS2"},{"P  /LS"},{"L02/D"},{"L"},{"L02"}},
	  										  {{"DESABILITADO"},{"MAIS"},{"SIRENES"},{"DELAYS"},{"RELES1"},{"RELES2"},{"L  /D"},{"ZONA"},{"P00/CS1"},{"P00/CS2"},{"P  /LS"},{"L02/D"},{"L01"},{"L02"}},
	  										  {{"DESHABID"},{'M',0x9d,'S'},{"SIRENAS"},{"DELAYS"},{'R','E','L',0X92,'S','1'},{'R','E','L',0X92,'S','2'},{"L  /D"},{"ZONA"},{"P00/CS1"},{"P00/CS2"},{"P  /LS"},{"L02/D"},{"L01"},{"L02"}}};
      const	unsigned char		totalfault[maxLanguage][23]={{"TOTAL NUMBER OF FAULTS"},{"NUMERO TOTAL DE FALHAS"},{'N',0X82,'M','E','R','O',' ','T','O','T','A','L',' ','D','E',' ','F','A','L','L','A','S'}};
     const unsigned char		totalfire[maxLanguage][2][30]={{{"LAST FIRE ZONE"},{"TOTAL ZONES IN FIRE"}},{{"ULTIMO ZONA DE FOGO"},{"TOTAL ZONA DE FOGO"}},{{0x82,'L','T','I','M','A',' ','Z','O','N','A',' ','F','U','E','G','O'},{"ZONAS TOTALES EN FUEGO"}}};

	 const unsigned char   TestQUE[maxLanguage][6][36]={{{"TEST MODE"},{"TESTING ALL SOUNDERS"},{"TESTING SOUNDERS ON PANEL 01"},{"TESTING SOUNDERS ON MAIN PANEL"},{"DETECTOR TEST MODE"},{"ALL ZONES"}},
	 									 {{"MODO TESTE"},{"TESTE GERAL DE SIRENES"},{"TESTE DE SIRENES NO PAINEL 01"},{"TESTE DE SIRENES NO PAINEL PRINCIPAL"},{"DETECTOR EM MODO TESTE"},{"ALL ZONAS"}},
	 									 {{"MODO PRUEBA"},{"TEST. TODAS SIRENAS"},{"TEST. SIRENAS PANEL 01"},{"TEST. SIRENAS PANEL PRINCP"},{"MODO PRUEBA DET"},{"TODAS ZONAS"}}};
	 const unsigned char	TestQUEStart[maxLanguage][3]={{10,6,5},{9,5,2},{10,8,7}};
	 const unsigned char	blank[]={"                                        "};
	 const	unsigned char	Repeaterlab[maxLanguage][16]={{"REPEATER PANEL"},{"PAINEL REPEATER"},{"PANEL REPETIDOR"}};
	 const	unsigned char	RepComFault[maxLanguage][21]={{"COMMUNICATION FAULT"},{'F','A','L','H','A',' ','D','E',' ','C','O','M','U','N','I','C','A',0xC4,0X9C,'O'},{'F','A','L','L','A',' ','C','O','M','U','N','I','C','A','C','I',0x89,'N'}};
	  const	unsigned char	NetworkComFault[maxLanguage][28]={{"NETWORK COMMUNICATION FAULT"},{'R','E','D','E','F','A','L','H','A',' ','D','E',' ','C','O','M','U','N','I','C','A',0xC4,0X9C,'O'},{'F','A','L','L','A',' ','D','E',' ','C','O','M','U','N','I','C','A','C','I',0x89,'N',' ','D','E',' ','R','E','D'}};
	 const	unsigned char	SysTemError[maxLanguage][16]={{"SYSTEM FAULT"},{"ERRO DO SISTEMA"},{"FALLA SISTEMA"}};
	 const     unsigned char	checksumlable[maxLanguage][12]={{"Calculating"},{"Calculando"},{"Calculando"}};
	 const	unsigned char	accesslevel[5][5]={{"AL 1"},{"AL 1"},{"AL 2"},{"AL 3"},{"AL 4"}};
	 const	unsigned char	Instaloractive[2][2]={{'I'},{'A'}};
	 const	unsigned char	WirelessType[maxLanguage][9][20]={{{"Non"},{"Wireless optical"},{"Wireless heat"},{"Wireless mult"},{"Wireless co"},{"Wireless gas"},{"Wireless mcp"},{"Wireless node"},{"Wireless remote"}},{{"Non"},{"Wireless optical"},{"Wireless heat"},{"Wireless mult"},{"Wireless co"},{"Wireless gas"},{"Wireless mcp"},{"Wireless node"},{"Wireless remote"}},{{"Non"},{"Wireless optical"},{"Wireless heat"},{"Wireless mult"},{"Wireless co"},{"Wireless gas"},{"Wireless mcp"},{"Wireless node"},{"Wireless remote"}}};
	 extern	UART_HandleTypeDef huart2;
	 extern	esp8266struct		ESP8266Status;
unsigned int	LcdBusTIME;
void		delay1(unsigned long i);
unsigned char		Lcd_StatusRead(unsigned char	chipcs);
unsigned char Statustemp[2*maxNetWorkpanel+32*LoopNum];
//lcdflag		LcdRunFlag;
noinit lcdstruct		LcdDispLayData;
unsigned char	CompanyName[29];
extern	sysf	SysTemFlag;
logD		logDispdata;
logD		TestDispdata;
unsigned char	LastMin;//主面板显示，每分钟更新一次
unsigned long	LCDTIME;
extern	unsigned long	DispTestTime;
extern	noinit	uint16_t	PROCHECKSUM;
extern	noinit	uint16_t PROCHECKSUMLast; 
	 extern	noinit	uint16_t	Prochecksumall;
	 extern	noinit	unsigned char		LoopNumactual;
	 extern	noinit unsigned char networkpanelamount;
	 extern	noinit sysfault	SyssubpanelFault[maxNetWorkpanel];
void		Lcd_Init(void);
void		Lcd_Clear(void);
void		Lcd_ClearReg(void);
void		Lcd_pos(unsigned char	chipcs,unsigned address);
void		Lcd_Write_char(unsigned char chipcs, unsigned char address, unsigned char data);
void		Lcd_Write_charreg(unsigned char chipcs, unsigned char address, unsigned char data);
void		Lcd_Write_charconst(unsigned char chipcs, unsigned char address, const unsigned char data);
void		Lcd_Write_charconstreg(unsigned char chipcs, unsigned char address, const unsigned char data);
void		Lcd_Write_pic(unsigned char chipcs,unsigned char add, const	unsigned char * p);
unsigned char		Lcd_Write_str(unsigned char chipcs, unsigned char address, const	unsigned char * p);
unsigned char		Lcd_Write_strreg(unsigned char chipcs, unsigned char address, const	unsigned char * p);
void		Lcd_Write_strlabel(unsigned char chipcs, unsigned char address, const	unsigned char * p,unsigned char	length);
void		Lcd_Write_strlabelreg(unsigned char chipcs, unsigned char address, const	unsigned char * p,unsigned char	length);
void		Lcd_CmdWtite(unsigned char	chipcs,unsigned char cmd);
void		Lcd_CmdNobfWtite(unsigned char	chipcs,unsigned char cmd);
void		Lcd_DataWrite(unsigned char chipcs, unsigned char data);
void 	MPX_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, unsigned char	 type);
void		LcdBackLightTimeCacl(void);
void		LcdinputReturnTimeCacl(unsigned long	 *timereset, lcdstruct * lcdptr);
void		Display_lcd(lcdstruct * displayptr);
void		FillBlanks(unsigned char	endaddress,unsigned char	blanklength,unsigned char	* blankptr);
void		Lcd_Write1(const	unsigned char * DP,lcdstruct *dispptr);
void		Lcd_DispLayX_X_X(lcdstruct	*disptr);
extern	void	MX_IWDG_CLEAR(void);
extern	uint16_t	PowerUpNum;//上电次数
extern	perm	PERMIT[7];
//extern	unsigned char	Eeprom[256];
extern	uint16_t		totalzoneinfire;
extern	LOPStatus	LoopStatus[LoopNum];
extern	noinit	unsigned long	  CalcCustsettingcase;
extern	unsigned int	CustomerCheckSum;
extern	noinit	unsigned int	 CustomerCheckSumLast;
extern	chsum	checksumflag;
//extern	noinit unsigned int	CustomerCheckSumall;
extern	noinit	uint16_t	CustomerCheckSumallLast;
extern	noinit	uint16_t PROCHECKSUMallLast; 
extern	noinit	wirelessstruct	WireLessData;
extern void		Read_EE1024(unsigned char cs, unsigned char * rxbuff, unsigned long address, uint16_t length);
extern	void 	calculateSettingchecksum(unsigned long calcase);
void		Logparse(lcdstruct	* dispptr,logD	* logdptr);
void		DispLayLog(lcdstruct	* dispptr,	logD	 * logdptr);

unsigned char		Lcd_DispLay1_1_1tm(lcdstruct	*dispptr,logD *logdptr);
void		AllLCDOn(void);
void		GetSoundStatustoShuzu_1(loopsndgr * p, unsigned char * Statustemp);
void		GetSoundStatustoShuzu(soundgr *p,unsigned char	*Statustemp);
void		GetSoundStatustoShuzu1(sounden *p,unsigned char	*Statustemp);
void		GetSoundStatustoShuzu1_1(loopsnden* p, unsigned char * Statustemp);
void		GetSoundStatustoShuzu2(soundover *p,unsigned char	*Statustemp);
void		GetSoundStatustoShuzu2_1(loopsndover	*p,unsigned char	*Statustemp);
void		PutSoundStatustobitfeild2(soundover *p,unsigned char	*Statustemp);
void		PutSoundStatustobitfeild(soundgr *p,unsigned char	*Statustemp);
void		PutSoundStatustobitfeild1(sounden *p,unsigned char	*Statustemp);

void		PutSoundStatustobitfeild_1(loopsndgr * p, unsigned char * Statustemp);
void		PutSoundStatustobitfeild1_1(loopsnden * p, unsigned char * Statustemp);
void		PutSoundStatustobitfeild2_1(loopsndover * p, unsigned char * Statustemp);

void		GetSoundStatustoShuzu_Conv(consndgr	* p, unsigned char * Statustemp);
void		PutSoundStatustobitfeild_Conv(consndgr	* p, unsigned char * Statustemp);
void		GetSoundStatustoShuzu1_Conv(consnden	* p, unsigned char * Statustemp);
void		PutSoundStatustobitfeild1_Conv(consnden	* p, unsigned char * Statustemp);
void		GetSoundStatustoShuzu2_Conv(consndover * p, unsigned char * Statustemp);
void		PutSoundStatustobitfeild2_Conv(consndover	* p, unsigned char * Statustemp);
void		Lcd_DispLay5_1_X_X(lcdstruct * disptr);
void		Lcd_DispLay5_2_X_X(lcdstruct * disptr);
void		Lcd_Write5_1_x(lcdstruct * disptr);
void		Lcd_Write5_2_x(lcdstruct * disptr);
void		Lcd_Write7_1_x(lcdstruct * disptr);
void		Lcd_Write7_2_x(lcdstruct * disptr);
void		Lcd_Write7_3_x(lcdstruct * disptr);
void			Lcd_DispLaySetLevel(lcdstruct	*disptr);
void		Display11_1(lcdstruct * disptr);
void		DispTEST(lcdstruct * disptr);
void		Lcd_DispLay7_1_X_X(lcdstruct * disptr);
void		Lcd_DispLay7_2_X_X(lcdstruct * disptr);
void		Lcd_DispLay7_3_X_X(lcdstruct * disptr);
void		Display12_1(lcdstruct * disptr);
void		Display13_1(lcdstruct * disptr);
void		Display11_2(lcdstruct * disptr);
void		Display11_2_1(lcdstruct * disptr);
void		Display11_2_2(lcdstruct * disptr);
void		Display11_2_3(lcdstruct * disptr);
void		Display11_2_4(lcdstruct * disptr);
void		Display11_2_5(lcdstruct * disptr);
void		Display11_2_6(lcdstruct * disptr);
void		Display11_2_7(lcdstruct * disptr);
void		Display11_2_10(lcdstruct * disptr);
void		Display11_2_11(lcdstruct * disptr);
void		Display11_2_12(lcdstruct * disptr);
void		Display11_2_13(lcdstruct * disptr);
void		Display11_2_14(lcdstruct * disptr);
void		Display11_2_15(lcdstruct * disptr);
void		Display11_2_16(lcdstruct * disptr);
void		Display11_2_17(lcdstruct * disptr);
void		Display11_2_18(lcdstruct * disptr);
void		Display11_2_19(lcdstruct * disptr);
void		Display11_2_20(lcdstruct	*disptr);
void		Display11_3(lcdstruct * disptr);
void		Display11_3_1(lcdstruct * disptr);
void		Display11_4(lcdstruct * disptr);
void		DispDisableMore(lcdstruct * disptr);
void		lcddispcal(lcdstruct	*disptr);

void		updowncaltime(lcdstruct	*disptr);
void		ReadLcdStatus(void);
void		ReadRamDataFun(unsigned char row, unsigned char * datatemp);
unsigned char	Readaddressram(unsigned char chipcs);
void		LcdReWrite(void);
extern	void readdevicetext(unsigned char Loopno, unsigned char deviceno, unsigned char * returntext);
extern	void	readzonetext(unsigned int ZonenUM, unsigned char * returntext);

unsigned char	readdatatemp[4][40];
unsigned	char	Lcdramaddress[2];
unsigned char	LcdPosaddress[2];
extern	unsigned   char	monitorKey;
extern	noinit gsmstruct GsmDataPar;
extern	uint32_t		RandData;
extern	uint32_t		RandDatalast;
extern		 unsigned char	gatewayfitted[LoopNum];//loop安装的桥数量
extern	wirelessgateway	GateWayLoopData[LoopNum][32];//gateway上的device
#define	sendlcd		HAL_GPIO_WritePin(T4_GPIO_Port,T4_Pin,GPIO_PIN_RESET);
#define	Readlcd		HAL_GPIO_WritePin(T4_GPIO_Port,T4_Pin,GPIO_PIN_SET);
unsigned char		Lcd_StatusRead(unsigned char	chipcs)
{

		static  GPIO_InitTypeDef GPIO_InitStruct;
		static   unsigned char	LCDstatus;
		 static uint16_t	ReadbfTime[2];
		 static Datac	char8bit;
	//	delay1(10);
	
	HAL_GPIO_WritePin(L_RS_GPIO_Port,L_RS_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(L_RW_GPIO_Port,L_RW_Pin,GPIO_PIN_RESET);
	//L_RS=!0;
	//L_RW=!1;
	delay1(10);
	if(chipcs==0)
		{
		//	L_E1=!1;
			ReadbfTime[chipcs]++;
		if(ReadbfTime[chipcs]%2==1)
			{
				
		HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_RESET);
			}
		else {

		HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);

			}
		HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);

		}
	else{

	//L_E2=!1;
			ReadbfTime[chipcs]++;
	if(ReadbfTime[chipcs]%2==1)
			{
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_RESET);
		}
	else{
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);

		}
	HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);

		}
	Readlcd;
	delay1(10);
	  /*Configure GPIO pin : WDTIN_Pin */
 	  GPIO_InitStruct.Pin = LD2_Pin|LD3_Pin|LD4_Pin|LD5_Pin  |LD6_Pin
                          |LD0_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LD7_Pin;
                         
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  delay1(10);
	if(HAL_GPIO_ReadPin(LD0_GPIO_Port, LD0_Pin))
		{
		LCDstatus=1;
	

		}
	else {
		LCDstatus=0;


		}
	char8bit.Bit.data0=HAL_GPIO_ReadPin(LD7_GPIO_Port, LD7_Pin);
	char8bit.Bit.data1=HAL_GPIO_ReadPin(LD6_GPIO_Port, LD6_Pin);
	char8bit.Bit.data2=HAL_GPIO_ReadPin(LD5_GPIO_Port, LD5_Pin);
	char8bit.Bit.data3=HAL_GPIO_ReadPin(LD4_GPIO_Port, LD4_Pin);
	char8bit.Bit.data4=HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin);
	char8bit.Bit.data5=HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
	char8bit.Bit.data6=HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin);
	char8bit.Bit.data7=1;

	
		  GPIO_InitStruct.Pin = LD2_Pin|LD3_Pin|LD4_Pin|LD5_Pin  |LD6_Pin
                          |LD0_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LD7_Pin;
                         
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//  sendlcd;
delay1(10);
  	if(chipcs==0)
		{
		if(LcdDispLayData.LcdRunFlag.Bit.ToReadPos)
			{
		Lcdramaddress[0]=char8bit.Value;
			}
		HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);
	//L_E1=!0;

		}
	else{

	//L_E2=!0;
	if(LcdDispLayData.LcdRunFlag.Bit.ToReadPos)
			{
		Lcdramaddress[1]=char8bit.Value;
				}
	HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);

		}
	return LCDstatus;

}



void		Lcd_Init(void)
{		static uint16_t	i;
	

        
    	Lcd_CmdNobfWtite(0, 0x38);   //设置为4行显示，5*8dot,8bit databus
     	Lcd_CmdNobfWtite(1, 0x38);
	delay1(100);
	Lcd_CmdNobfWtite(0, 0x38);   //设置为4行显示，5*8dot,8bit databus
     	Lcd_CmdNobfWtite(1, 0x38);
	delay1(100);
			LcdBusTIME=HAL_GetTick();
	do{

			if((HAL_GetTick()-LcdBusTIME)>=500)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}


		}while(Lcd_StatusRead(0)||Lcd_StatusRead(1));
	    if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
            if(SysmainpanelFault.Bit.LcdError)
        {
            
               MX_IWDG_CLEAR();
			goto lcdout;
            
        }
	Lcd_CmdWtite(0, 0x08);   //display on off control
     	Lcd_CmdWtite(1, 0x08);
	//delay(5000);
			LcdBusTIME=HAL_GetTick();
	do{
			if((HAL_GetTick()-LcdBusTIME)>=100)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}



		}while(Lcd_StatusRead(0)||Lcd_StatusRead(1));
	    if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
            if(SysmainpanelFault.Bit.LcdError)
        {
            
                MX_IWDG_CLEAR();
			goto lcdout;
            
        }
	Lcd_CmdWtite(0, 0x01);   //disp clear
			
	Lcd_CmdWtite(1, 0x01);   //disp clear
	delay1(100);
	LcdBusTIME=HAL_GetTick();
	do{

			if((HAL_GetTick()-LcdBusTIME)>=100)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}


		}while(Lcd_StatusRead(0)||Lcd_StatusRead(1));
	    if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
            if(SysmainpanelFault.Bit.LcdError)
        {
            
                MX_IWDG_CLEAR();
			goto lcdout;
            
        }
	Lcd_CmdWtite(0, 06);
	Lcd_CmdWtite(1, 06);
		LcdBusTIME=HAL_GetTick();
	//delay(5000);
	do{

			if((HAL_GetTick()-LcdBusTIME)>=100)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}


		}while(Lcd_StatusRead(0)||Lcd_StatusRead(1));
	   if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
          if(SysmainpanelFault.Bit.LcdError)
        {
            
                MX_IWDG_CLEAR();
			goto lcdout;
            
        }

	Lcd_CmdWtite(0, 0x0c);
	Lcd_CmdWtite(1, 0x0c);
	
	for(i=0;i<6;i++)
		{
			LcdBusTIME=HAL_GetTick();
		do{
			if((HAL_GetTick()-LcdBusTIME)>=100)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}



		}while(Lcd_StatusRead(0)||Lcd_StatusRead(1));
		    if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
          if(SysmainpanelFault.Bit.LcdError)
        {
            
                MX_IWDG_CLEAR();
			goto lcdout;
            
        }
		Lcd_Write_pic(0, i, pcode[i]);
		Lcd_Write_pic(1, i, pcode[i]);
	
		
		}
	
        	lcdout:
				i=0;
	
	
}
// 初始化ram内容
void		Lcd_Write_pic(unsigned char chipcs,unsigned char add, const	unsigned char * p)
{
		static unsigned char	i;
		
			add=add<<3;
			for(i=0;i<8;i++)
				{

				Lcd_CmdWtite(chipcs, ((0x40|add)+i));
			
				//
				Lcd_DataWrite(chipcs, *p++);

				}





}
void		Lcd_Write_char(unsigned char chipcs, unsigned char address, unsigned char data)
{
		static unsigned char	internaladdress;
		if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}
		Lcd_CmdWtite(chipcs, internaladdress);

		//

		Lcd_DataWrite(chipcs, data);
		if(chipcs==0)
			{
				pinlcddata[address/40][address%40]=data;

			}
		else 
			{
				pinlcddata[2+address/40][address%40]=data;

			}

}
void		Lcd_Write_charreg(unsigned char chipcs, unsigned char address, unsigned char data)
{
	/*	static unsigned char	internaladdress;
		if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}*/
		
		if(chipcs==0)
			{
				pinlcddata[address/40][address%40]=data;

			}
		else 
			{
				pinlcddata[2+address/40][address%40]=data;

			}

}
void		Lcd_Write_charconst(unsigned char chipcs, unsigned char address, const		unsigned char data)
{
		static unsigned char	internaladdress;
		if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}
		Lcd_CmdWtite(chipcs, internaladdress);

		//

		Lcd_DataWrite(chipcs, data);
		
		if(chipcs==0)
			{
				pinlcddata[address/40][address%40]=data;

			}
		else 
			{
				pinlcddata[2+address/40][address%40]=data;

			}
}
void		Lcd_Write_charconstreg(unsigned char chipcs, unsigned char address, const		unsigned char data)
{
		/*static unsigned char	internaladdress;
		if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}*/
		//Lcd_CmdWtite(chipcs, internaladdress);

		//

		//Lcd_DataWrite(chipcs, data);
		
		if(chipcs==0)
			{
				pinlcddata[address/40][address%40]=data;

			}
		else 
			{
				pinlcddata[2+address/40][address%40]=data;

			}
}
void		Lcd_pos(unsigned char	chipcs,unsigned address)
{
		static unsigned char	internaladdress;
		if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}
		Lcd_CmdWtite(chipcs, internaladdress);

			if(chipcs==0)
			{
					cursoraddress=address;


			}
		else{

						cursoraddress=address+80;


			}





}
unsigned char		Lcd_Write_str(unsigned char chipcs, unsigned char address,const	 unsigned char * p)//返回写入的个数
{

static unsigned char	internaladdress,i,j;
static unsigned char	datatemp[40];
static unsigned char	chipcstemp1,chipcstemp2;

			i=0;
			if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}
		
		Lcd_CmdWtite(chipcs, internaladdress);
			//

		while(*p)
			{
			
			Lcd_DataWrite(chipcs, *p);
			datatemp[i]=*p;
			p++;
			i++;
		//	
			}
		if(chipcs==0)
			{
					chipcstemp1=address/40;
					chipcstemp2=address%40;
				for(j=0;j<i;j++)
					{
							if(chipcstemp2+j<40)
						{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
						}
					}

			}
		else 
			{
					chipcstemp1=2+address/40;
					chipcstemp2=address%40;
					for(j=0;j<i;j++)
				{
						if(chipcstemp2+j<40)
						{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
						}
				}

			}
		return	i;



}
unsigned char		Lcd_Write_strreg(unsigned char chipcs, unsigned char address,const	 unsigned char * p)//返回写入的个数
{

static unsigned char	i,j;
static unsigned char	datatemp[40];
static unsigned char	chipcstemp1,chipcstemp2;

			i=0;
		

		while(*p)
			{
			
			
			datatemp[i]=*p;
			p++;
			i++;
			
			}
		if(chipcs==0)
			{
					chipcstemp1=address/40;
					chipcstemp2=address%40;
				for(j=0;j<i;j++)
					{
						if(chipcstemp2+j<40)
						{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
						}
					}

			}
		else 
			{
					chipcstemp1=2+address/40;
					chipcstemp2=address%40;
					for(j=0;j<i;j++)
				{
						if(chipcstemp2+j<40)
						{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
						}
				}

			}
		return	i;



}
void		Lcd_Write1(const	unsigned char * DP,lcdstruct *dispptr)
{
					static const	unsigned char	*p;
					static unsigned char	Address,length;
							Lcd_ClearReg();

								 if(SysTemFlag.Bit.RunLevel==1)
									{
								p=(const unsigned char	*)RunUser0[dispptr->LanguageType%maxLanguage];
								length=strlen((const  char	*)RunUser0[dispptr->LanguageType%maxLanguage]);
							/*	if((dispptr->LanguageType%maxLanguage)==0)
									{
								Address=12;
									}
								else{

								Address=12;

									}*/
								Address=(40-length)/2;
								Lcd_Write_strreg(0, Address, p);
									}
								else if(SysTemFlag.Bit.RunLevel==2)
									{
								p=(const unsigned char	*)RunUser1[dispptr->LanguageType%maxLanguage];
								length=strlen((const  char	*)RunUser1[dispptr->LanguageType%maxLanguage]);
								/*
								if((dispptr->LanguageType%maxLanguage)==0)
									{
								Address=13;
									}
								else{

								Address=14;

									}*/
									Address=(40-length)/2;
								Lcd_Write_strreg(0, Address, p);
									}
								else if(SysTemFlag.Bit.RunLevel==3)
									{
									length=strlen((const  char	*)RunUser2[dispptr->LanguageType%maxLanguage]);
									/*
								if((dispptr->LanguageType%maxLanguage)==0)
									{
								Address=9;
									}
								else{

								Address=6;

									}*/
									Address=(40-length)/2;
									p=(const unsigned char	*)RunUser2[dispptr->LanguageType%maxLanguage];
									Lcd_Write_strreg(0, Address, p);

									}
								else if(SysTemFlag.Bit.RunLevel==4)
									{
									length=strlen((const  char	*)RunUser2[dispptr->LanguageType%maxLanguage]);
									/*
								if((dispptr->LanguageType%maxLanguage)==0)
									{
								Address=9;
									}
								else{

								Address=6;

									}*/
									Address=(40-length)/2;
									p=(const unsigned char	*)RunUser3[dispptr->LanguageType%maxLanguage];
									Lcd_Write_strreg(0, Address, p);


									}
							
							

									p=(const unsigned char	*)help1[dispptr->LanguageType%maxLanguage];
									Lcd_Write_strreg(0, 40, p);


								
								p=DP;
								Lcd_Write_strreg(1, 0, p);
							
									if((dispptr->LanguageType%maxLanguage)==0)
									{
								Address=58;
									}
								else if((dispptr->LanguageType%maxLanguage)==1){

								Address=57;

									}
									else if((dispptr->LanguageType%maxLanguage)==2){

								Address=62;

									}
								Lcd_Write_charreg(0,Address, 1);
						
								Lcd_Write_charreg(0, Address+2, 2);
							

								Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
								Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
								Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);	




}
void		Lcd_Write_strlabel(unsigned char chipcs, unsigned char address, const unsigned char * p,unsigned char	length)
{

static unsigned char	internaladdress,i,j;
static unsigned char	datatemp[40];
static unsigned char	chipcstemp1,chipcstemp2;
			i=0;
			if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}
		
		Lcd_CmdWtite(chipcs, internaladdress);
			//

		while(i<length)
			{
			
			Lcd_DataWrite(chipcs, *p);
			datatemp[i]=*p;
			i++;
			p++;
		//	
			}

		if(chipcs==0)
			{
					chipcstemp1=address/40;
					chipcstemp2=address%40;
				for(j=0;j<i;j++)
					{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
					}

			}
		else 
			{
					chipcstemp1=2+address/40;
					chipcstemp2=address%40;
					for(j=0;j<i;j++)
				{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
				}

			}

}
void		Lcd_Write_strlabelreg(unsigned char chipcs, unsigned char address, const unsigned char * p,unsigned char	length)
{

static unsigned char	i,j;
static unsigned char	datatemp[40];
static unsigned char	chipcstemp1,chipcstemp2;
			i=0;
		/*	if(address<40)
			{
		internaladdress=0x80+address;
			}
		else {
		internaladdress=0xc0+(address-40);



			}*/
		
		//Lcd_CmdWtite(chipcs, internaladdress);
			//

		while(i<length)
			{
			
			//Lcd_DataWrite(chipcs, *p);
			datatemp[i]=*p;
			i++;
			p++;
		//	
			}

		if(chipcs==0)
			{
					chipcstemp1=address/40;
					chipcstemp2=address%40;
				for(j=0;j<i;j++)
					{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
					}

			}
		else 
			{
					chipcstemp1=2+address/40;
					chipcstemp2=address%40;
					for(j=0;j<i;j++)
				{
				pinlcddata[chipcstemp1][chipcstemp2+j]=datatemp[j];
				}

			}

}
void		Lcd_Clear(void)
{	
//	static uint16_t	i;
static unsigned char	i;
	
	Lcd_CmdWtite(0, 0x01);
	Lcd_CmdWtite(1, 0x01);
	for(i=0;i<40;i++)
		{
		pinlcddata[0][i]=0x20;
		pinlcddata[1][i]=0x20;
			pinlcddata[2][i]=0x20;
		pinlcddata[3][i]=0x20;
		}
	
	//for(i=0;i<10000;i++);
	/*
	Lcd_Write_str(0, 0, blank);
	Lcd_Write_str(1, 0, blank);
	//
	Lcd_Write_str(0, 40, blank);
	Lcd_Write_str(1, 40, blank);
	*/
	//
}

void		Lcd_ClearReg(void)
{	
//	static uint16_t	i;
static unsigned char	i;
	
	//Lcd_CmdWtite(0x01);

	for(i=0;i<40;i++)
		{
		pinlcddata[0][i]=0x20;
		pinlcddata[1][i]=0x20;
			pinlcddata[2][i]=0x20;
		pinlcddata[3][i]=0x20;
		}
	
		pinlcddata[0][40]=0x0;
		pinlcddata[1][40]=0x0;
			pinlcddata[2][40]=0x0;
		pinlcddata[3][40]=0x0;



}
void		Lcd_CmdNobfWtite(unsigned char	chipcs,unsigned char cmd)
{
//	static uint16_t	i;
	static Datac		databit;
	
		databit.Value=cmd;

		sendlcd;
		delay1(10);

		HAL_GPIO_WritePin(L_RS_GPIO_Port, L_RS_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(L_RW_GPIO_Port, L_RW_Pin, GPIO_PIN_SET);
	//L_RS=!0;
	//L_RW=!0;
	delay1(10);
		if(chipcs==0)
		{
		//	L_E1=!1;
		HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);

		}
	else{

	//L_E2=!1;
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);

		}
	delay1(10);
	MPX_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, databit.Bit.data7);
   	MPX_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, databit.Bit.data6);
	MPX_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, databit.Bit.data5);
	MPX_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, databit.Bit.data4);
	MPX_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, databit.Bit.data3);
	MPX_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, databit.Bit.data2);
	MPX_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, databit.Bit.data1);
	MPX_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, databit.Bit.data0);
	

	delay1(50);

			if(chipcs==0)
		{
		HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);
	//L_E1=!0;

		}
	else{

	//L_E2=!0;
	HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);

		}
	
		
}

void		Lcd_CmdWtite(unsigned char	chipcs,unsigned char cmd)
{
//	static uint16_t	i;
	static Datac		databit;
	
		databit.Value=cmd;
		LcdBusTIME=HAL_GetTick();
		do{

			if((HAL_GetTick()-LcdBusTIME)>=100)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}

		}while(Lcd_StatusRead(chipcs));
		  if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
		     if(SysmainpanelFault.Bit.LcdError)
      	  {
            
             MX_IWDG_CLEAR();
			goto lcdout1;
            
       	 }
		sendlcd;
		delay1(10);

		HAL_GPIO_WritePin(L_RS_GPIO_Port, L_RS_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(L_RW_GPIO_Port, L_RW_Pin, GPIO_PIN_SET);
	//L_RS=!0;
	//L_RW=!0;
	delay1(10);
		if(chipcs==0)
		{
		//	L_E1=!1;
		HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);

		}
	else{

	//L_E2=!1;
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);

		}
	delay1(10);
	MPX_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, databit.Bit.data7);
   	MPX_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, databit.Bit.data6);
	MPX_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, databit.Bit.data5);
	MPX_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, databit.Bit.data4);
	MPX_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, databit.Bit.data3);
	MPX_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, databit.Bit.data2);
	MPX_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, databit.Bit.data1);
	MPX_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, databit.Bit.data0);
	

	//for(i=0;i<100;i++);
	delay1(50);

			if(chipcs==0)
		{
		HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);
	//L_E1=!0;

		}
	else{

	//L_E2=!0;
	HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);

		}
	lcdout1:
	//	i=0;
		__ASM("NOP");
	if(((cmd&0xf8)==0x08)&&(!LcdDispLayData.LcdRunFlag.Bit.NoreadCmd))
		{
	flashcmd=cmd;
		}
		
}

void		Lcd_DataWrite(unsigned char chipcs, unsigned char data)
{


//static uint16_t	i;
static Datac databit;
databit.Value=data;
LcdBusTIME=HAL_GetTick();
		do{

			if((HAL_GetTick()-LcdBusTIME)>=100)
			{
			SysmainpanelFault.Bit.LcdError=1;
			break;
			}

		}while(Lcd_StatusRead(chipcs));
		  if((LcdBusTIME+100)<HAL_GetTick())
        	{

		SysmainpanelFault.Bit.LcdError=0;
        	}
     if(SysmainpanelFault.Bit.LcdError)
        {
            
              MX_IWDG_CLEAR();
			goto lcdout2;
            
        }
	sendlcd;
	delay1(10);
	HAL_GPIO_WritePin(L_RS_GPIO_Port, L_RS_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(L_RW_GPIO_Port, L_RW_Pin, GPIO_PIN_SET);
		if(chipcs==0)
		{
		//	L_E1=!1;
		HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);

		}
	else{

	//L_E2=!1;
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);

		}
	delay1(10);
   	MPX_GPIO_WritePin(LD0_GPIO_Port, LD0_Pin, databit.Bit.data7);
   	MPX_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, databit.Bit.data6);
	MPX_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, databit.Bit.data5);
	MPX_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, databit.Bit.data4);
	MPX_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, databit.Bit.data3);
	MPX_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, databit.Bit.data2);
	MPX_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, databit.Bit.data1);
	MPX_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, databit.Bit.data0);

	delay1(50);

		
			if(chipcs==0)
		{
		HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);
	//L_E1=!0;

		}
	else{

	//L_E2=!0;
	HAL_GPIO_WritePin(L_E1_GPIO_Port, L_E1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LE_2_GPIO_Port, LE_2_Pin, GPIO_PIN_SET);

		}
	lcdout2:
//		Nop();
		
	//	i=0;
		__ASM("NOP");


}
void MPX_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, unsigned char	 type)
{

		if(type==1)
			{

		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);


			}
		else{

		HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);

			}









}
void		AllLCDOn(void)
{		
		static const unsigned char *p;
		static unsigned char	i,j;
						p=allon;
						
					Lcd_Write_str(0, 0, p);
					//
					p=allon;
					Lcd_Write_str(0, 40, p);
					//
							p=allon;
					Lcd_Write_str(1, 0, p);
				//	
					p=allon;
					Lcd_Write_str(1, 40, p);
				//	
				for(j=0;j<4;j++)
					{
				for(i=0;i<40;i++)
					{
					pinlcddata[j][i]=allon[i];


					}
					}

}


void		FillBlanks(unsigned char endaddress, unsigned char blanklength, unsigned char * blankptr)
{

		static unsigned char	i;
		for(i=0;i<endaddress;i++)
			{
				if(i<blanklength)
					{
							*blankptr=' ';


					}
				else{
					*blankptr=0;
						

					}



			}










}

void		Display_lcd(lcdstruct	*displayptr)
{
	const	unsigned char		*p;
	static  unsigned char	Address,Address1,add,i,charlength;//,length;
	static unsigned long	DisplaylastTime;
//	unsigned char	datatemp[41];
//	unsigned char	*p2;
	static unsigned long	InputRetruenTime;
LcdBackLightTimeCacl();
LcdinputReturnTimeCacl(&InputRetruenTime, displayptr);
lcddispcal(displayptr);
//updowncaltime(displayptr);
if((displayptr->FaultEventNum>0)||(displayptr->FireEventNum>0))
{

	displayptr->LcdRunFlag.Bit.LcdBackLight=1;

}
MPX_GPIO_WritePin(BL_LCD_GPIO_Port, BL_LCD_Pin, displayptr->LcdRunFlag.Bit.LcdBackLight);
if(DisplaylastTime>HAL_GetTick())
{

	DisplaylastTime=HAL_GetTick();

}
	if((displayptr->Setlevel1==7)&&(displayptr->Setlevel2==4)&&(displayptr->Setlevel3>0))
						{

							if((HAL_GetTick()>DisplaylastTime)&&((HAL_GetTick()-DisplaylastTime)>200))
								{

								displayptr->LcdRunFlag.Bit.DispLayLcd=1;
								displayptr->LcdRunFlag.Bit.DispLayLcdrx=1;
								}

						}

if((displayptr->LcdRunFlag.Bit.DispLayLcd)&&(displayptr->LcdRunFlag.Bit.DispLayLcdrx)&&((HAL_GetTick()>DisplaylastTime)&&((HAL_GetTick()-DisplaylastTime)>50)))
{
	displayptr->LcdRunFlag.Bit.DispLayLcd=0;
	displayptr->LcdRunFlag.Bit.DispLayLcdrx=0;
	DisplaylastTime=HAL_GetTick();
	if(SysTemFlag.Bit.PanelSelfTest)
		{
							AllLCDOn();
							displayptr->dispfaulteventnum=displayptr->dispfaulteventnumlast;
							displayptr->displayfault1=displayptr->displayfault1last;
							displayptr->dispTestnum=displayptr->dispTestnumlast;
						displayptr->testlast=displayptr->testlastlast;
						displayptr->DispablmentNum=displayptr->DispablmentNumlast;
						displayptr->disptype=displayptr->disptypelast;
						displayptr->DispBegNO=displayptr->DispBegNOlast;
							displayptr->dispfireeventnum=displayptr->dispfireeventnumlast;
							displayptr->displayfire1=displayptr->displayfire1last;
			


		}
	else{
			Lcd_CmdWtite(0, 0x0c);
			Lcd_CmdWtite(1, 0x0c);
			switch(displayptr->dispcase1)
				{
					case 0:
						//Lcd_Clear();	
						for(i=0;i<28;i++)
						{
						if(CompanyName[27-i]!=0x20)
							{
                            
							break;



							}



						}/*
					//------------------------------------
					p=(const unsigned  char	*)&CompanyName;
                  			  Address=(40-(28-i));
                 			   Address=Address/2;
						
					 FillBlanks(41, Address, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;
					length=Lcd_Write_str(0,0,p1);
					length=Lcd_Write_str(0, Address, p);
					Address=Address+length;
						
					FillBlanks(41, 40-Address, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;
					length=Lcd_Write_str(0,Address,p1);
					
					//---------------------------------
					
					 FillBlanks(41, 8, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;	
					Lcd_Write_str(0,40,p1);
					p=csh[displayptr->LanguageType%maxLanguage];
					Address=48;
					length=Lcd_Write_str(0, Address, p);
					Address=Address+length;
					
					FillBlanks(41, 80-Address, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;
					length=Lcd_Write_str(0,Address,p1);
					//-------------------------------------------
					
					  FillBlanks(41, 14, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;	
					Lcd_Write_str(1,0,p1);
					p=csh1[displayptr->LanguageType%maxLanguage];
					Address=14;
					length=Lcd_Write_str(1, Address, p);
					
					Address=Address+length;
					
					FillBlanks(41, 40-Address, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;
					length=Lcd_Write_str(0,Address,p1);
					//-------------------------------------
					
						  FillBlanks(41, 40, (unsigned char*)(&datatemp));
					p1=(const unsigned  char	*)&datatemp;	
					Lcd_Write_str(1,40,p1);
					*/
					Lcd_ClearReg();
					p=(const unsigned char *)&CompanyName;
                  			  Address=(40-(28-i));
                 			   Address=Address/2;
					Lcd_Write_strreg(0, Address, p);
                    
					p=csh[displayptr->LanguageType%maxLanguage];
					charlength=strlen((const char *)csh[displayptr->LanguageType%maxLanguage]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, p);
				//Lcd_Write_str(0, 54, p);

					
					
					p=csh1[displayptr->LanguageType%maxLanguage];
					charlength=strlen((const char *)csh1[displayptr->LanguageType%maxLanguage]);
					Address=(40-charlength)/2;
					Lcd_Write_strreg(1, Address, p);
					p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
					Lcd_Write_strreg(1, 75, p);
					Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
					Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
					Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
					Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
				break;
				case 1:
						/*
						  FillBlanks(41, 40, (unsigned char*)(&datatemp));
						p1=(const unsigned  char	*)&datatemp;	
						Lcd_Write_str(0,0,p1);	

						//-----------------------------------------
						
						  FillBlanks(41, 10, (unsigned char*)(&datatemp));
						p1=(const unsigned  char	*)&datatemp;	
						Lcd_Write_str(0,40,p1);
						p=qingshur[displayptr->LanguageType%maxLanguage];
						Address=50;
						length=Lcd_Write_str(0, Address, p);
						Address=Address+length;
				
							FillBlanks(41, 40-Address, (unsigned char*)(&datatemp));
							p1=(const unsigned  char	*)&datatemp;
							length=Lcd_Write_str(0,Address,p1);
						//-----------------------------------------------
						
						  FillBlanks(41, 14, (unsigned char*)(&datatemp));
						p1=(const unsigned  char	*)&datatemp;	
						Lcd_Write_str(1,0,p1);
						Address=0;
						p=qingshur1[displayptr->LanguageType%maxLanguage];//
						length=Lcd_Write_str(1, Address, p);
							Address=Address+length;
							
							FillBlanks(41, 40-Address, (unsigned char*)(&datatemp));
							p1=(const unsigned  char	*)&datatemp;
							length=Lcd_Write_str(1,Address,p1);
					//---------------------------------------------------
						
						  FillBlanks(41, 14, (unsigned char*)(&datatemp));
						p1=(const unsigned  char	*)&datatemp;	
						Lcd_Write_str(1,40,p1);*/
								Lcd_ClearReg();
					
						p=qingshur[displayptr->LanguageType%maxLanguage];
						charlength=strlen((const char *)qingshur[displayptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength)/2;
						Lcd_Write_strreg(0, Address, p);
						
						p=qingshur1[displayptr->LanguageType%maxLanguage];//
						Lcd_Write_strreg(1, 0, p);
						p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
						Lcd_Write_strreg(1, 75, p);
						Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
						Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
						Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
						Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
						InputRetruenTime=HAL_GetTick();
						break;
					case 2:

							switch(displayptr->dispcase2)//输入密码部分
						{
							case 0:
							Lcd_ClearReg();
			
							if(!SysTemFlag.Bit.ToEnterSuper)
							{
							/*
								if((displayptr->LanguageType%maxLanguage)==0)
								{
									Address=47;


									}
							else if((displayptr->LanguageType%maxLanguage)==1)
							{
							Address=43;


							}*/
							charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][0]);
							Address=40+(40-charlength)/2;
							p=(const unsigned char	*)&qingshur2[displayptr->LanguageType%maxLanguage][0];
							Lcd_Write_strreg(0, Address, p);
							}
						else{
							/*
							if((displayptr->LanguageType%maxLanguage)==0)
							{
								Address=43;


						}
						else if((displayptr->LanguageType%maxLanguage)==1)
						{
							Address=40;


						}*/
						charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][1]);
							Address=40+(40-charlength)/2;
						p=(const unsigned char	*)&qingshur2[displayptr->LanguageType%maxLanguage][1];
						Lcd_Write_strreg(0, Address, p);	


					}
						p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
					Lcd_Write_strreg(1, 75, p);
					Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
					Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
					Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
					Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
					
					break;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 9:
						Lcd_ClearReg();
					
					if(!SysTemFlag.Bit.ToEnterSuper)
						{
						p=(const unsigned char	*)&qingshur2[displayptr->LanguageType%maxLanguage][0];
						if((displayptr->LanguageType%maxLanguage)==0)
						{
						//	Address=47;
						//	Address1=67;
						charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength)/2;
						Address1=Address+20;


						}
					else if((displayptr->LanguageType%maxLanguage)==1)
						{
						//	Address=43;
						//	Address1=72;
						charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength)/2;
						Address1=Address+29;


						}
				else if((displayptr->LanguageType%maxLanguage)==2)
						{
						//	Address=46;
						//	Address1=69;
						charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength)/2;
						Address1=Address+23;


						}
						Lcd_Write_strreg(0, Address, p);
					
						if(displayptr->dispcase2>5)
							{
							
							add=5;
							}
						else{
							add=displayptr->dispcase2;

							}
						for(i=0;i<add;i++)
							{
						Lcd_Write_charreg(0,Address1+i, '*');
					
							}
						}
					else{
						p=(const unsigned char	*)&qingshur2[displayptr->LanguageType%maxLanguage][1];
						if((displayptr->LanguageType%maxLanguage)==0)
						{
						//	Address=43;
						//	Address1=72;
						charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][1]);
						Address=40+(40-charlength)/2;
						Address1=Address+29;

						}
						else if((displayptr->LanguageType%maxLanguage)==1)
						{
							//Address=40;
							//Address1=74;
							charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][1]);
						Address=40+(40-charlength)/2;
						Address1=Address+34;
						}
							else if((displayptr->LanguageType%maxLanguage)==2)
						{
							//Address=42;
							//Address1=72;
							charlength=strlen((const char *)qingshur2[displayptr->LanguageType%maxLanguage][1]);
						Address=40+(40-charlength)/2;
						Address1=Address+30;


						}
						Lcd_Write_strreg(0, Address, p);
					
						if(displayptr->dispcase2>6)
							{
							
							add=6;
							}
						else{
							add=displayptr->dispcase2;

							}
						for(i=0;i<add;i++)
							{
						Lcd_Write_charreg(0,Address1+i, '*');
					
							}






						}
						p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
					Lcd_Write_strreg(1, 75, p);	
					Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
					Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
					Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
					Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
						
					break;
					}
							p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
					Lcd_Write_strreg(1, 75, p);
							InputRetruenTime=HAL_GetTick();
				break;
			case 3://进入后显示的菜单
					switch(displayptr->dispcase2)
						{

							case 0x01:
								Lcd_Write1((const unsigned char	*)yijicaidan1[displayptr->LanguageType%maxLanguage],displayptr);
								break;
							case 2:
								Lcd_Write1((const unsigned char	*)yijicaidan2[displayptr->LanguageType%maxLanguage],displayptr);
								break;
							case 3:
								Lcd_Write1((const unsigned char	*)yijicaidan3[displayptr->LanguageType%maxLanguage],displayptr);
								break;	
							case 4:
								Lcd_Write1((const unsigned char	*)yijicaidan4[displayptr->LanguageType%maxLanguage],displayptr);
								break;	
							case 5:
								Lcd_Write1((const unsigned char	*)yijicaidan5[displayptr->LanguageType%maxLanguage],displayptr);
								break;
							case 6:
								Lcd_Write1((const unsigned char	*)yijicaidan6[displayptr->LanguageType%maxLanguage],displayptr);
								break;	

							case 7:
								Lcd_Write1((const unsigned char	*)yijicaidan7[displayptr->LanguageType%maxLanguage],displayptr);
								break;
							case 8:
								Lcd_Write1((const unsigned char	*)yijicaidan8[displayptr->LanguageType%maxLanguage],displayptr);
								break;








						}
				break;
				case 4://二级菜单
					if(displayptr->Setlevel3==0)//二级菜单
						{
					switch(displayptr->dispcase2)
						{
							case 0x01:
								Lcd_Write1((const unsigned char	*)erjicaidan1_1[displayptr->LanguageType%maxLanguage],displayptr);
								
									
								break;
							case 0x02:
								Lcd_Write1((const unsigned char	*)erjicaidan1_2[displayptr->LanguageType%maxLanguage],displayptr);	
							
									break;


						}
						}
					else{//二级菜单按确定后，进入实际选项1-1,1-2连个，
							switch(displayptr->Setlevel3)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
											Lcd_DispLayX_X_X(displayptr);
										
										break;
									
									case 0xff:
										
										Lcd_Write1((const unsigned char	*)erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
										
										break;
								}



						}
					break;
					case 5://  菜单2显示部分
					if(displayptr->Setlevel3==0)//第一级
						{

							switch(displayptr->dispcase2)
								{
							case 0x01:
								Lcd_Write1(erjicaidan2_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								Lcd_Write1(erjicaidan2_2[displayptr->LanguageType%maxLanguage],displayptr);		
									break;

							case 0x03:
								Lcd_Write1(erjicaidan2_3[displayptr->LanguageType%maxLanguage],displayptr);	
									break;

							case 0x04:
								Lcd_Write1(erjicaidan2_4[displayptr->LanguageType%maxLanguage],displayptr);		
									break;
							case 0x05:
								Lcd_Write1(erjicaidan2_5[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
						

								}



						}
					else{// 2-2 的后面级别

							switch(displayptr->Setlevel3)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
									case 4:
									case 5:
									case 6:
									case 254:
									
											Lcd_DispLayX_X_X(displayptr);
										break;
									
									case 0xff:
										Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
										break;
								}






						}
					break;
					case 6://菜单3部分条码
						if(displayptr->Setlevel3==0)
				  		{
					switch(displayptr->dispcase2)
						{
								case 0x01:
								Lcd_Write1(erjicaidan3_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								Lcd_Write1(erjicaidan3_2[displayptr->LanguageType%maxLanguage],displayptr);	
									break;

							case 0x03:
								Lcd_Write1(erjicaidan3_3[displayptr->LanguageType%maxLanguage],displayptr);	
									break;

							case 0x04:
								Lcd_Write1(erjicaidan3_4[displayptr->LanguageType%maxLanguage],displayptr);		
									break;
							case 0x05:
								Lcd_Write1(erjicaidan3_5[displayptr->LanguageType%maxLanguage],displayptr);	
								break;
							case 0x06:
								Lcd_Write1(erjicaidan3_6[displayptr->LanguageType%maxLanguage],displayptr);
									break;
							case 0x07:
							
								Lcd_Write1(erjicaidan3_7[displayptr->LanguageType%maxLanguage],displayptr);
									break;
						}

					}
				  else{

						switch(displayptr->Setlevel3)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
									case 4:
									case 5:
									case 6:
									case 254:
									
											Lcd_DispLayX_X_X(displayptr);
										break;
									
									case 0xff:
										Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
										break;
								}









				  	}
					  break;
				  case 7://菜单4的条码
				  	if(displayptr->Setlevel3==0)
					{
				switch(displayptr->dispcase2)
					{
								case 0x01:
								Lcd_Write1(erjicaidan4_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								Lcd_Write1(erjicaidan4_2[displayptr->LanguageType%maxLanguage],displayptr);	
									break;

							case 0x03:
								Lcd_Write1(erjicaidan4_3[displayptr->LanguageType%maxLanguage],displayptr);	
									break;

							case 0x04:
								Lcd_Write1(erjicaidan4_4[displayptr->LanguageType%maxLanguage],displayptr);		
									break;
							case 0x05:
								Lcd_Write1(erjicaidan4_5[displayptr->LanguageType%maxLanguage],displayptr);	
								break;
							case 0x06:
								Lcd_Write1(erjicaidan4_6[displayptr->LanguageType%maxLanguage],displayptr);
									break;
							case 0x07:
							
								Lcd_Write1(erjicaidan4_7[displayptr->LanguageType%maxLanguage],displayptr);
									break;
					}

					}
				else{
								switch(displayptr->Setlevel3)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
									case 4:
									case 5:
									case 6:
									case 254:
									
											Lcd_DispLayX_X_X(displayptr);
										break;
									
									case 0xff:
										Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
										break;
								}






					}
					break;
					case 8://5菜单
					if(displayptr->Setlevel3==0)
						{
						switch(displayptr->dispcase2)
							{
								case 0x01:
								Lcd_Write1(erjicaidan5_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								Lcd_Write1(erjicaidan5_2[displayptr->LanguageType%maxLanguage],displayptr);	
								break;


						}

					}
					else {	
							if(displayptr->Setlevel4==0)
								{
									if(displayptr->Setlevel2==1)
										{
										Lcd_Write5_1_x(displayptr);
										}
									else if(displayptr->Setlevel2==2)
										{
										Lcd_Write5_2_x(displayptr);


										}
									
								}
							else{

									switch(displayptr->Setlevel4)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
									case 4:
									case 5:
									case 6:
									case 254:
									
										Lcd_DispLayX_X_X(displayptr);
										break;
									
									case 0xff:
										Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
										break;
								}




								}



						}
					break;
					case 9://菜单6 
						
						if(displayptr->Setlevel3==0)
						{
						switch(displayptr->dispcase2)
						{
							case 0x01:
								Lcd_Write1(erjicaidan6_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								if(displayptr->DispDevMonitorSet.testsoundflag==1)
									{
								
								Lcd_ClearReg();
								 if(SysTemFlag.Bit.RunLevel==1)
									{
								p=(const unsigned char	*)RunUser0[displayptr->LanguageType%maxLanguage];

								charlength=strlen((const char	*)&RunUser0[displayptr->LanguageType%maxLanguage]);
								/*if((displayptr->LanguageType%maxLanguage)==0)
									{
								Address=12;
									}
								else{

								Address=12;

									}*/
								Address=(40-charlength)/2;
								Lcd_Write_strreg(0, Address, p);
									}
								
								else if(SysTemFlag.Bit.RunLevel==2)
									{
										
							/*	if((displayptr->LanguageType%maxLanguage)==0)
									{
								Address=13;
									}
								else{

								Address=14;

									}*/
								charlength=strlen((const char	*)&RunUser1[displayptr->LanguageType%maxLanguage]);	
								p=RunUser1[displayptr->LanguageType%maxLanguage];
								Address=(40-charlength)/2;
								Lcd_Write_strreg(0, Address, p);
									}
								else if(SysTemFlag.Bit.RunLevel==3)
									{

									p=RunUser2[displayptr->LanguageType%maxLanguage];
									charlength=strlen((const char	*)&RunUser2[displayptr->LanguageType%maxLanguage]);
									Address=(40-charlength)/2;
									Lcd_Write_strreg(0, Address, p);

									}
								else if(SysTemFlag.Bit.RunLevel==4)
									{

									p=RunUser3[displayptr->LanguageType%maxLanguage];
									charlength=strlen((const char	*)&RunUser3[displayptr->LanguageType%maxLanguage]);
									Address=(40-charlength)/2;
									Lcd_Write_strreg(0, Address, p);

									}
								
							

									p=help2[displayptr->LanguageType%maxLanguage];
									Lcd_Write_strreg(0, 40, p);


								
									p=erjicaidan6_2[displayptr->LanguageType%maxLanguage];
									Lcd_Write_strreg(1, 0, p);
								
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								

									}
								else{

								Lcd_Write1(erjicaidan6_2[displayptr->LanguageType%maxLanguage],displayptr);

									}
									break;

							case 0x03:
								Lcd_Write1(erjicaidan6_3[displayptr->LanguageType%maxLanguage],displayptr);	
									break;

							case 0x04:
								Lcd_Write1(erjicaidan6_4[displayptr->LanguageType%maxLanguage],displayptr);		
									break;
							case 0x05:
								Lcd_Write1(erjicaidan6_5[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
						

						}
						}
					else{

						
								switch(displayptr->Setlevel3)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
									case 4:
									case 5:
									case 6:
									case 254:
									
											//Lcd_DispLay6_X_X(Setlevel2, Setlevel3);
											Lcd_DispLayX_X_X(displayptr);
										break;
									
									case 0xff:
										Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
										break;
								}







						}
					break;
				case 10:
					if(displayptr->Setlevel3==0)
						{
					switch(displayptr->dispcase2)
						{
							case 0x01:
								Lcd_Write1(erjicaidan7_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								Lcd_Write1(erjicaidan7_2[displayptr->LanguageType%maxLanguage],displayptr);		
									break;

							case 0x03:
								Lcd_Write1(erjicaidan7_3[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
							case 0x04:
								Lcd_Write1(erjicaidan7_4[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
							
						

							}
						
						}
					else{
									if(displayptr->Setlevel4==0)
										{
										
										if(displayptr->Setlevel2==1)
										{
										Lcd_Write7_1_x(displayptr);
										}
									else if(displayptr->Setlevel2==2)
										{
										Lcd_Write7_2_x(displayptr);


										}	
									else if(displayptr->Setlevel2==3)
										{
										Lcd_Write7_3_x(displayptr);


										}
									else if(displayptr->Setlevel2==4)
									{
											if(displayptr->Setlevel3<0xff)
												{
											Lcd_DispLayX_X_X(displayptr);	
												}
											else{
											Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);

												}
										
									}
										
										
									
										}
									else{
									if(displayptr->Setlevel4<255)
									
										{
										//Lcd_DispLay7_X_X_X(Setlevel2, Setlevel3,Setlevel4);
										Lcd_DispLayX_X_X(displayptr);
										}
										
									
									else if(displayptr->Setlevel4==255)
										{
										Lcd_Write1(erjicaidan1_denied[displayptr->LanguageType%maxLanguage],displayptr);
									
										}
										}


										









						}
					break;
				case 11://不再条码下面，最前页的显示
					if((displayptr->FaultEventNum==0)&&(displayptr->FireEventNum==0)&&((HAL_GetTick()-DispTestTime)>15000)&&(!SysTemFlag.Bit.CaretakerTest))
							{
						Display11_1(displayptr);//无故障，我报警的显示页面
						p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
						Lcd_Write_strreg(1, 75, p);
						Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
							}
						else if((displayptr->FireEventNum==0)&&((HAL_GetTick()-DispTestTime)<15000)&&(TestNum>0)&&(SysTemFlag.Bit.SysTestStatus))
							{


									DispTEST(displayptr);
									p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
									Lcd_Write_strreg(1, 75, p);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);




							}
				
						else if(((displayptr->FireEventNum==0)&&(displayptr->FaultEventNum>0))||((displayptr->dispcase3==1)&&(displayptr->FaultEventNum>0)))
							{
							displayptr->dispfaulteventnumlast=displayptr->dispfaulteventnum;
							displayptr->displayfault1last=displayptr->displayfault1;
							displayptr->lastfaultDisp=0;
							displayptr->faultnodisp=displayptr->FaultEventNum;
							Display11_2(displayptr);//显示故障
							p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
							Lcd_Write_strreg(1, 75, p);
							Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
							



							}
				
						else if(displayptr->dispcase3==2)
							{
							displayptr->dispfireeventnumlast=displayptr->dispfireeventnum;
							displayptr->displayfire1last=displayptr->displayfire1;
							Display11_3(displayptr);//显示火灾
							p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
							Lcd_Write_strreg(1, 75, p);
							Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

							}
						else if(SysTemFlag.Bit.CaretakerTest)
							{

							Display11_4(displayptr);
							p=&accesslevel[(SysTemFlag.Bit.RunLevel)%5][0];
							Lcd_Write_strreg(1, 75, p);
							Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

							}
						
					break;
				case 12://disabled
						displayptr->DispablmentNumlast=displayptr->DispablmentNum;
						displayptr->disptypelast=displayptr->disptype;
						displayptr->DispBegNOlast=displayptr->DispBegNO;
						Display12_1(displayptr);
						Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
						
						break;
				case 13://testque
				displayptr->dispTestnumlast=displayptr->dispTestnum;
				displayptr->testlastlast=displayptr->testlast;
				
				Display13_1(displayptr);
				Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
					break;
				case 14://菜单8 厂家专用
					if(displayptr->Setlevel3==0)
						{
						switch(displayptr->dispcase2)
							{
								case 0x01:
								Lcd_Write1(erjicaidan8_1[displayptr->LanguageType%maxLanguage],displayptr);

								break;
							case 0x02:
								Lcd_Write1(erjicaidan8_2[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
							case 0x03:
							Lcd_Write1(erjicaidan8_3[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
							case 0x04:
								Lcd_Write1(erjicaidan8_4[displayptr->LanguageType%maxLanguage],displayptr);	
									break;
								
							}
						}
					else{


							
								switch(displayptr->Setlevel3)
								{
									case 0x01://进入显示
									case 0x02:
									case 0x03:
								
									
											//Lcd_DispLay8_X_X(Setlevel2, Setlevel3);
											Lcd_DispLayX_X_X(displayptr);
										break;
								}



						}
					break;
					
					
				}








		}
}





}

void		Display13_1(lcdstruct	*disptr)
{
	static uint16_t	i,lcdaddress,DISPNUM,DISPNUMINT;
			Lcd_ClearReg();
			

			Lcd_Write_strreg(0,15,(const unsigned char	*) &TestQUE[disptr->LanguageType%maxLanguage][0]);
			/*
			for(i=0;i<16;i++)
				{
				TestzoneSend[i]=0;

				}*/
			if(disptr->DispDevMonitorSet.testsoundflag==1)
				{
				if(disptr->dispTestnum<9)
					{
					disptr->testlastlast=0;
					Lcd_Write_strreg(1,TestQUEStart[disptr->LanguageType%maxLanguage][(disptr->DispDevMonitorSet.TestSoundmode%3)], (const unsigned char	*)&TestQUE[disptr->LanguageType%maxLanguage][(disptr->DispDevMonitorSet.TestSoundmode%3)+1]);
					
					disptr->dispTestnum=1;
					for(i=0;i<ZoneNum;i++)
						{

							if(zones[i].TESTorNormal)
								{
								lcdaddress=40+((disptr->dispTestnum-1)%8)*5;
								Lcd_Write_charreg(1, lcdaddress, 'Z');
								
								DISPNUMINT=i+1;
								DISPNUM=DISPNUMINT/100;
								DISPNUMINT=DISPNUMINT%100;
								Lcd_Write_charreg(1, lcdaddress+1, DISPNUM+0x30);
								
								DISPNUM=DISPNUMINT/10;
								DISPNUMINT=DISPNUMINT%10;
								Lcd_Write_charreg(1, lcdaddress+2, DISPNUM+0x30);
								
								DISPNUM=DISPNUMINT;
							
								Lcd_Write_charreg(1, lcdaddress+3, DISPNUM+0x30);
								
								//TestzoneSend[(dispTestnum-1)%8]=i+1;
								disptr->dispTestnum++;
								if(disptr->dispTestnum==9)
									{
										
								
									disptr->testlastlast=i+1;
									
									if(disptr->testlastlast>=ZoneNum)
										{
									disptr->testlastlast=0;

										}
									disptr->testlast=disptr->testlastlast;
									break;
									}


								}
							
							

						}
						

				
					}
				else{
								for(i=disptr->testlastlast;i<ZoneNum;i++)
								{

							if(zones[i].TESTorNormal)
								{
								//TestzoneSend[(dispTestnum-1)%16]=i+1;
								if(((disptr->dispTestnum-1)%16)<8)
									{
									lcdaddress=40+((disptr->dispTestnum-1)%8)*5;

									}
								else{

									lcdaddress=((disptr->dispTestnum-1)%8)*5;


									}
									
								Lcd_Write_charreg(1, lcdaddress, 'Z');
								
								DISPNUMINT=i+1;
								DISPNUM=DISPNUMINT/100;
								DISPNUMINT=DISPNUMINT%100;
								Lcd_Write_charreg(1, lcdaddress+1, DISPNUM+0x30);
								
								DISPNUM=DISPNUMINT/10;
								DISPNUMINT=DISPNUMINT%10;
								Lcd_Write_charreg(1, lcdaddress+2, DISPNUM+0x30);
								
								DISPNUM=DISPNUMINT;
							
								Lcd_Write_charreg(1, lcdaddress+3, DISPNUM+0x30);
								
								disptr->dispTestnum++;
								if((disptr->dispTestnum-9)%16==0)
									{
										
								
									disptr->testlastlast=i+1;
									if(disptr->testlastlast>=ZoneNum)
										{
									disptr->testlastlast=0;

										}
								//	testlast=lastno;
										break;
									}


								}
							
							

						}		
						


					}
				}
			else{
						if(disptr->dispTestnum<16)
							{

							disptr->testlastlast=0;


							}
						
							for(i=disptr->testlastlast;i<ZoneNum;i++)
								{

							if((zones[i].TESTorNormal)&&(SysTemFlag.Bit.SysTestStatus))
								{
									
								if(((disptr->dispTestnum)%16)<8)
									{
									lcdaddress=((disptr->dispTestnum)%8)*5;

									}
								else{

									lcdaddress=40+((disptr->dispTestnum)%8)*5;


									}
									
								//TestzoneSend[(dispTestnum)%16]=i+1;
								Lcd_Write_charreg(1, lcdaddress, 'Z');
								
								DISPNUMINT=i+1;
								DISPNUM=DISPNUMINT/100;
								DISPNUMINT=DISPNUMINT%100;
								Lcd_Write_charreg(1, lcdaddress+1, DISPNUM+0x30);
								
								DISPNUM=DISPNUMINT/10;
								DISPNUMINT=DISPNUMINT%10;
								Lcd_Write_charreg(1, lcdaddress+2, DISPNUM+0x30);
								
								DISPNUM=DISPNUMINT;
							
								Lcd_Write_charreg(1, lcdaddress+3, DISPNUM+0x30);
								
								disptr->dispTestnum++;
								if((disptr->dispTestnum)%16==0)
									{
										
									
									disptr->testlastlast=i+1;
									if(disptr->testlastlast>=ZoneNum)
										{
									disptr->testlastlast=0;

										}
									break;
									}


								}
							
							

						}



				}
				if(disptr->dispTestnum<TestNum)
						{
							Lcd_Write_strreg(0,73,(const unsigned char	*) &Disablment[disptr->LanguageType%maxLanguage][1]);
								
							Lcd_Write_charreg(0, 78, 0x2d);
							
							Lcd_Write_charreg(0, 79, 0x3e);
							
						//	disptr->DisablementMore=1;

						}
					else{
							//DisablementMore=0;
						disptr->maxdispTesttnum=TestNum;

						}

			//	testlast=lastno;

				disptr->testlast=disptr->testlastlast;






}
unsigned char	Disablementtype[8];
uint16_t	DisablementDev[8];

void		Display12_1(lcdstruct	*disptr)//quedisabled
{
		static uint16_t address,address1,address2,i;
		//static unsigned int	datatemp1,datatemp2;
		
		
			Lcd_ClearReg();
			

			Lcd_Write_strreg(0,0, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][0]);
			
			for(i=0;i<8;i++)
				{
					Disablementtype[i]=0;

				}
			if(disptr->DispablmentNum<8)
				{
				disptr->DispablmentNum=0;
				if(SysTemFlag.Bit.SounderDisabled)
					{
						address=disptr->DispablmentNum%8;
						address1=address/4;
						address2=(address%4)*10+address1*40;
						Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][2]);
						
						Disablementtype[disptr->DispablmentNum]=1;
						disptr->DispablmentNum++;
						


					}
				/*if(SysTemFlag.Bit.panelDelay)
					{
						address=disptr->DispablmentNum%8;
						address1=address/4;
							address2=(address%4)*10+address1*40;

						Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][3]);
						
						Disablementtype[disptr->DispablmentNum]=2;
						disptr->DispablmentNum++;
						

					}*////////////delay delete
					if(SysTemFlag.Bit.panelfireauxiliary)
					{
						address=disptr->DispablmentNum%8;
						address1=address/4;
							address2=(address%4)*10+address1*40;
						Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][4]);
						
						Disablementtype[disptr->DispablmentNum]=3;
						disptr->DispablmentNum++;
					

					}
						if(SysTemFlag.Bit.panelfaultauxiliary)
					{
						address=disptr->DispablmentNum%8;
						address1=address/4;
							address2=(address%4)*10+address1*40;
						Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][5]);
						
						Disablementtype[disptr->DispablmentNum]=4;
						disptr->DispablmentNum++;
					

					}
				
					
						
		
						disptr->disptype=1;
						disptr->DispBegNO=0;
						
				}
						
					DispDisableMore(disptr);
					if(disptr->DispablmentNum<DisablmentNum)
						{
							//DisablementMore=1;
							Lcd_Write_strreg(0,73, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][1]);
								
							Lcd_Write_charreg(0, 78, 0x2d);
							
							Lcd_Write_charreg(0, 79, 0x3e);
							

						}
					else{

						maxdisabledNum=DisablmentNum;
						//DisablementMore=0;

						}

				}

void			DispDisableMore(lcdstruct	*disptr)
{
					static uint16_t i,address,address1,address2,Datatempint,Datatemp,j,k,l;
					static uint16_t	datatemp1,datatemp2,ConSndNum;
					static unsigned char	devtemp;
				//	static unsigned char	Statustemp[2+32*LoopNum];

							j=0;
							if(disptr->disptype==1)
								{
								k=disptr->DispablmentNum%8;
							for(i=disptr->DispBegNO;i<LoopNumactual;i++)
							{
							
							//if(LoopStatus[i].Bit.loophardenable&&(!LoopStatus[i].Bit.LoopEnableDisable))
							if((!LoopStatus[i].Bit.LoopEnableDisable))
								{


						
							address=disptr->DispablmentNum%8;
							address1=address/4;
							address2=(address%4)*10+address1*40;
							Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][12]);
							datatemp1=i+1;
							
							datatemp2=datatemp1%10;
							datatemp1=datatemp1/10;
							Lcd_Write_charreg(1, (address%4)*10+address1*40+1,datatemp1+0x30);
							Lcd_Write_charreg(1, (address%4)*10+address1*40+2,datatemp2+0x30);
							Disablementtype[disptr->DispablmentNum]=5;
							disptr->DispablmentNum++;
							k++;
							if((k>=8)||(disptr->DispablmentNum>=DisablmentNum))
																{
																	j=1;
																		if(i<(LoopNumactual-1))
																			{
																			//Type=1;
																			disptr->disptype=1;
																			disptr->DispBegNO=i+1;
																			//lastno=DispBegNO;
																			

																			}
																		else{
																				l=0;
																				//Type=2;
																				disptr->disptype=2;
																				disptr->DispBegNO=0;
																				//lastno=DispBegNO;


																			}
																	break;


																}
								}
								}
										if(i==LoopNumactual)
										{
											l=0;

											//Type=2;
											disptr->disptype=2;
											disptr->DispBegNO=0;
											//lastno=DispBegNO;


										}

							}
							if(disptr->disptype==2)
								{
								
								k=disptr->DispablmentNum%8;
								for(i=disptr->DispBegNO;i<ZoneNum;i++)
									{	
									if(!zones[i].ZoneEnable)
												{
														address=disptr->DispablmentNum%8;
														address1=address/4;
															address2=(address%4)*10+address1*40;
															Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][7]);
															
															

															Datatempint=i+1;
															Datatemp=Datatempint/100;
															Lcd_Write_charreg(1, address2+5, Datatemp+0x30);
															
															
															Datatempint=Datatempint%100;
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+6, Datatemp+0x30);
															
															Datatemp=Datatempint%10;
															Lcd_Write_charreg(1, address2+7, Datatemp+0x30);
															
															Disablementtype[disptr->DispablmentNum%8]=6;
															DisablementDev[disptr->DispablmentNum%8]=i;
															disptr->DispablmentNum++;
															k++;
															if((k>=8)||(disptr->DispablmentNum>=DisablmentNum))
																{
																	j=1;
																		if(i<(ZoneNum-1))
																			{
																			//Type=1;
																			disptr->disptype=2;
																			disptr->DispBegNO=i+1;
																			//lastno=DispBegNO;
																			

																			}
																		else{
																				l=0;
																				//Type=2;
																				disptr->disptype=3;
																				disptr->DispBegNO=0;
																				//lastno=DispBegNO;


																			}
																	break;


																}
	

												}
									else{

												


										}





									}
									if(i==ZoneNum)
										{
											l=0;

											//Type=2;
											disptr->disptype=3;
											disptr->DispBegNO=0;
											//lastno=DispBegNO;


										}
									}
                            llll1:
							if(disptr->disptype==3)
								{
								if(j==0)
									{
										k=disptr->DispablmentNum%8;
									
										for(i=disptr->DispBegNO;i<MaxDevice;i++)
											{	
											
											if(!devicedata[l][i].DevEnableDisabled)
												{
														address=disptr->DispablmentNum%8;
														address1=address/4;
															address2=(address%4)*10+address1*40;
															Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][6]);
															
															Datatempint=l+1;
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+1, Datatemp+0x30);

															
															Datatemp=Datatempint%10;
															Lcd_Write_charreg(1, address2+2, Datatemp+0x30);
															

															Datatempint=i+1;
															Datatemp=Datatempint/100;
															Lcd_Write_charreg(1, address2+5, Datatemp+0x30);
															
															
															Datatempint=Datatempint%100;
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+6, Datatemp+0x30);
															
															Datatemp=Datatempint%10;
															Lcd_Write_charreg(1, address2+7, Datatemp+0x30);
															
															Disablementtype[disptr->DispablmentNum%8]=7;
															DisablementDev[disptr->DispablmentNum%8]=i+l*MaxDevice;
															disptr->DispablmentNum++;
															k++;
															if((k>=8)||(disptr->DispablmentNum>=DisablmentNum))
																{
																	j=1;
																	if(i<(MaxDevice-1))
																			{
																			//Type=2;
																			disptr->disptype=3;
																			disptr->DispBegNO=i+1;
																			//lastno=DispBegNO;
																			

																			}
																	else{
																		l++;
																		if(l<LoopNumactual)
																			{
																				disptr->disptype=3;
																				disptr->DispBegNO=0;		
																				goto llll1;

																			}
																		else{

																				disptr->disptype=4;
																				disptr->DispBegNO=0;	
																				l=0;

																			}




																		}
																	
																	break;


																}
	

												}
											//	}





									}
									if(i==MaxDevice)
										{
										
																		l++;
																		if(l<LoopNumactual)
																			{
																				disptr->disptype=3;
																				disptr->DispBegNO=0;		
																				goto llll1;

																			}
																		else{

																				disptr->disptype=4;
																				disptr->DispBegNO=0;	
																				l=0;

																			}




																		
							


										}




									}
								}
								if(disptr->disptype==4)
									{
								if(j==0)
									{
									k=disptr->DispablmentNum%8;
										GetSoundStatustoShuzu1(&SoundCONFIG,(unsigned char	*)&Statustemp);
										if(networkpanelamount==0)
											{
												ConSndNum=2;

											}
										else {
												ConSndNum=2*networkpanelamount;

											}
										if(disptr->DispBegNO<ConSndNum)
											{
										for(i=disptr->DispBegNO;i<ConSndNum;i++)
											{
											if(Statustemp[i]==0)
												{
													address=disptr->DispablmentNum%8;
														address1=address/4;
															address2=(address%4)*10+address1*40;
															Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][8]);
															devtemp=i+1;
															address2=address2+6;
															devtemp=devtemp%100;
															datatemp1=devtemp/10;
															Lcd_Write_charreg(1,address2, datatemp1+0x30);
															address2++;
															datatemp1=devtemp%10;
															Lcd_Write_charreg(1,address2, datatemp1+0x30);

															
															/*

															if(i==0)
																{

																Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][8]);
															
															

																}
															else{

																Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][9]);
																


																}
																*/
															
															Disablementtype[disptr->DispablmentNum%8]=8;
															DisablementDev[disptr->DispablmentNum%8]=i;
															disptr->DispablmentNum++;
															k++;
															if((k>=8)||(disptr->DispablmentNum>=DisablmentNum))
																{
																	j=1;
																	if(i<ConSndNum-1)
																			{
																			//Type=3;
																			disptr->disptype=4;
																			disptr->DispBegNO=i+1;
																			//lastno=DispBegNO;
																			

																			}
																		else{
																				//Type=3;
																				disptr->disptype=4;
																				disptr->DispBegNO=ConSndNum;
																				//lastno=DispBegNO;


																			}
																	break;


																}
	
															

												}
											


											}
									if(i==ConSndNum){

																				//Type=3;
																				disptr->disptype=4;
																				disptr->DispBegNO=ConSndNum;
																				//lastno=DispBegNO;


											}
											}
										if(disptr->DispBegNO>=ConSndNum){
											
										if(j==0)
											{
												k=disptr->DispablmentNum%8;
												//for(i=disptr->DispBegNO;i<2+LoopNumactual*32;i++)
												if(disptr->DispBegNO<24)
													{

													disptr->DispBegNO=24;
													}
												for(i=disptr->DispBegNO;i<24+LoopNumactual*32;i++)
													{

														if(Statustemp[i]==0)
														{
														address=disptr->DispablmentNum%8;
														address1=address/4;
															address2=(address%4)*10+address1*40;
															Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][10]);

															Datatempint=(i-24)/32+1;
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+1, Datatemp+0x30);

															Datatemp=Datatempint%10;
															Lcd_Write_charreg(1, address2+2, Datatemp+0x30);
															
															Datatempint=(i-24)%32+94;

															

															
															Datatemp=Datatempint/100;
															Lcd_Write_charreg(1, address2+6, Datatemp+0x30);
															
															Datatempint=Datatempint%100;
															Datatemp=Datatempint/100;									
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+7, Datatemp+0x30);
															
															Datatempint=Datatempint%10;
															Datatemp=Datatempint;
															Lcd_Write_charreg(1, address2+8, Datatemp+0x30);
															
															Disablementtype[disptr->DispablmentNum%8]=8;
															DisablementDev[disptr->DispablmentNum%8]=i;
															disptr->DispablmentNum++;k++;
															if((k>=8)||(disptr->DispablmentNum>=DisablmentNum))
																{
																	j=1;
																	if(i<(24+LoopNumactual*32-1))
																			{
																			//Type=3;
																			disptr->disptype=4;
																			disptr->DispBegNO=i+1;
																			//lastno=DispBegNO;
																			

																			}
																		else{
																				//Type=4;
																				disptr->disptype=5;
																				disptr->DispBegNO=0;
																			//	lastno=DispBegNO;
																				l=0;


																			}
																	break;


																}
	

														}

													}
												if(i==24+32*LoopNumactual)
												{

												//Type=4;
												disptr->disptype=5;
												disptr->DispBegNO=0;
												//lastno=DispBegNO;
												l=0;


												}




											}
											}

										



									}
									}
								lll2:
								if(disptr->disptype==5)
									{
								if(j==0)
									{
											k=disptr->DispablmentNum%8;
												
											for(i=disptr->DispBegNO;i<MaxDevice;i++)
											{	
										
											if((devicedata[l][i].DevSelectDisablement)&&(SysTemFlag.Bit.SelDetDisablement))
												{
														address=disptr->DispablmentNum%8;
														address1=address/4;
															address2=(address%4)*10+address1*40;
															Lcd_Write_strreg(1,address2, (const unsigned char	*)&Disablment[disptr->LanguageType%maxLanguage][6]);
															
															Datatempint=l+1;
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+1, Datatemp+0x30);
Datatempint=l+1;
															Datatemp=Datatempint%10;
															Lcd_Write_charreg(1, address2+2, Datatemp+0x30);
															Datatempint=i+1;
															Datatemp=Datatempint/100;
															Lcd_Write_charreg(1, address2+5, Datatemp+0x30);
															
															
															Datatempint=Datatempint%100;
															Datatemp=Datatempint/10;
															Lcd_Write_charreg(1, address2+6, Datatemp+0x30);
															
															Datatemp=Datatempint%10;
															Lcd_Write_charreg(1, address2+7, Datatemp+0x30);
															
															Disablementtype[disptr->DispablmentNum%8]=7;
															DisablementDev[disptr->DispablmentNum%8]=i+l*MaxDevice;
															disptr->DispablmentNum++;
															k++;
															if((k>=8)||(disptr->DispablmentNum>=DisablmentNum))
																{
																//	j=1;
																	if(i<(MaxDevice-1))
																			{
																			//Type=4;
																			disptr->disptype=5;
																			disptr->DispBegNO=i+1;
																			//lastno=DispBegNO;
																			

																			}
																	else{
																						l++;
																						if(l<LoopNumactual)
																							{

																							disptr->disptype=5;
																							disptr->DispBegNO=0;

																								goto lll2;	
																							}
																						else{

																							disptr->disptype=5;
																							disptr->DispBegNO=0;

																							}





																		}
																	
																
																	break;


																}
	

												}
												}
											if(i==MaxDevice)
												{
													l++;
																						if(l<LoopNumactual)
																							{

																							disptr->disptype=5;
																							disptr->DispBegNO=0;

																								goto lll2;	
																							}
																						else{

																							disptr->disptype=5;
																							disptr->DispBegNO=0;

																							}



												}





									

									



									}
									}




}
void		Display11_4(lcdstruct	*disptr)
{
			static unsigned char	Address;
						Lcd_ClearReg();
						
						if(disptr->LanguageType%maxLanguage==0)
							{
							Address=51;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
							Address=49;

							}
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&TestQUE[disptr->LanguageType%maxLanguage][4]);
						
						Lcd_Write_strreg(1, 15, (const unsigned char	*)&TestQUE[disptr->LanguageType%maxLanguage][5]);
						


}
void	Display11_3(lcdstruct	*disptr)//que fire
{


						static unsigned long	address;
				static uint16_t	i,j;
								
				for(i=(FireStartNo+disptr->displayfire1);i<=(LogEntryNo);i++)
						{
			
							address=(unsigned long)((i-1)%MaxLogNum)*OneLogLength;
					
						Read_EE1024(1, Eeprom,address, OneLogLength);

								for(j=0;j<OneLogLength;j++)
									{
									logDispdata.data[j]=Eeprom[j];


									}	
								if(logDispdata.chardata.RecordType==Fireevent)
									{

											Display11_3_1(disptr);
											disptr->displayfire1++;
												disptr->dispfireeventnum++;
												if((disptr->dispfireeventnum==disptr->FireEventNum)&&(disptr->FireEventNum>1))
												{
																disptr->displayfire1=0;
																disptr->dispfireeventnum=0;
													if(disptr->maxdispfireeventnum!=disptr->FireEventNum)
													{
															

																	disptr->maxdispfireeventnum=disptr->FireEventNum;
																	

													


													}
												}
											


														
											
											break;



									}
								disptr->displayfire1++;
						}



}
void		Display11_3_1(lcdstruct	*disptr)
{
			static	uint16_t	Datatempint,Datatemp,i;
			static unsigned char	Address;
		//	static unsigned long	eepAddress;
			//unsigned long	totalzoneinfire;
			
							Lcd_ClearReg();
						
						Lcd_Write_strreg(0, 0, (const unsigned char	*)&firedisp[disptr->LanguageType%maxLanguage][0]);
						
						
					Datatempint=logDispdata.chardata.RecordThisNum;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 5,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 6,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 7,Datatemp+0x30);
					

					if(logDispdata.chardata.RecordZone>0)
						{

						Lcd_Write_strreg(0, 9, (const unsigned char	*)&firedisp[disptr->LanguageType%maxLanguage][1]);
						
						
					Datatempint=logDispdata.chardata.RecordZone;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 14,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 15,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 16,Datatemp+0x30);
					
					
														//eepAddress=((logDispdata.chardata.RecordZone-1)%ZoneNum)*18;
													//	Read_EE1024(2, Eeprom,eepAddress, 18);
													readzonetext(((logDispdata.chardata.RecordZone-1)%ZoneNum),&Eeprom[0]);
														if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
															{

														Lcd_Write_strlabelreg(0, 40, (const unsigned char	*)&Eeprom[2], 16);
															}




						}
					
						Lcd_Write_strreg(0, 18, (const unsigned char	*)&firedisp[disptr->LanguageType%maxLanguage][2]);
						
						
					Datatempint=logDispdata.chardata.RecordDev;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 22,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 23,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 24,Datatemp+0x30);
						

					Lcd_Write_charreg(0, 26,'L');
						

					Datatempint=logDispdata.chardata.RecordLoop%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 27,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 28,Datatemp+0x30);
					if(logDispdata.chardata.RecordDev<126)
						{
					
					Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename1[disptr->LanguageType%maxLanguage][logDispdata.chardata.RecordDevType%31]);//有bug
						}
					else{
					Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename2[disptr->LanguageType%maxLanguage][logDispdata.chardata.RecordDevType%31]);//有bug	


						}
					

						/*eepAddress=MaxDevice*24*((logDispdata.chardata.RecordLoop-1)%LoopNum)+((logDispdata.chardata.RecordDev-1)%MaxDevice)*24;
						Read_EE1024(3, Eeprom,eepAddress, 24);*/
						readdevicetext(((logDispdata.chardata.RecordLoop-1)%LoopNum), ((logDispdata.chardata.RecordDev-1)%MaxDevice), &Eeprom[0]);
						if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
							{

							Lcd_Write_strlabelreg(0, 57, (const unsigned char	*)&Eeprom[2], 22);
							}
						Address=(40-strlen((const char *)totalfire[disptr->LanguageType%maxLanguage][0])-4)/2;
					/*if(disptr->LanguageType%maxLanguage==0)
						{

						Address=0;
						}
					else if(disptr->LanguageType%maxLanguage==1)
						{

						Address=4;
						}*/

					Lcd_Write_strreg(1, Address, (const unsigned char	*)&totalfire[disptr->LanguageType%maxLanguage][0]);
					
					Address=Address+strlen((const char *)totalfire[disptr->LanguageType%maxLanguage][0])+1;
					Datatempint=LastfireZone%1000;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Address++;
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Address++;
					
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
						

					Address=(40-strlen((const char *)totalfire[disptr->LanguageType%maxLanguage][1])-5)/2;
					Address=40+Address;
					Lcd_Write_strreg(1, Address, (const unsigned char	*)&totalfire[disptr->LanguageType%maxLanguage][1]);
					
					totalzoneinfire=0;
					for(i=0;i<ZoneNum;i++)
						{

							if(zones[i].infire==1)
								{
								
							totalzoneinfire++;

								}


						}
					Address=Address+strlen((const char *)totalfire[disptr->LanguageType%maxLanguage][1])+1;
					Datatempint=totalzoneinfire%10000;
					Datatemp=Datatempint/1000;
					Datatempint=Datatempint%1000;
				//	Address++;
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
					
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;	
					Address++;
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Address++;
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
					Datatemp=Datatempint;
					Address++;
					Lcd_Write_charreg(1, Address,Datatemp+0x30);
						


}
void		Display11_2(lcdstruct	*disptr)//quefault
{
				static unsigned long i,address;
				static uint16_t	j,Datatemp,Datatempint;
				static unsigned char	NetworkfaultFlag;
								
	for(i=(FaultStartNo+disptr->displayfault1);i<=LogEntryNo;i++)
		{
			
							address=(unsigned long)((i-1)%MaxLogNum)*OneLogLength;
							
							Read_EE1024(1,Eeprom,address, OneLogLength);
								for(j=0;j<OneLogLength;j++)
									{
									logDispdata.data[j]=Eeprom[j];


									}
								NetworkfaultFlag=0;
								if((logDispdata.chardata.RecordType==NetWorkComFault)&&(networkpanelamount>0))
									{
									if(SyssubpanelFault[logDispdata.chardata.RecordDev%maxNetWorkpanel].Bit.networkcomfault)
										{
												NetworkfaultFlag=1;

										}



									}
								if(((logDispdata.chardata.RecordType>=FaultLoopdev)&&(logDispdata.chardata.RecordType<=systemfaultclrwdt)&&(logDispdata.chardata.RecordType!=TESTevent)&&(logDispdata.chardata.RecordType!=ENABLEDFaultRelay)&&(logDispdata.chardata.RecordType!=DISABLEDFaultRelay))||(NetworkfaultFlag==1))
									{
											if((logDispdata.chardata.RecordType==FaultLoopdev)||(logDispdata.chardata.RecordType==PERALARM)||(logDispdata.chardata.RecordType==loopDuplicateAddress)||(logDispdata.chardata.RecordType==loopdevideremove)||(logDispdata.chardata.RecordType==LoopWrongDeviceFitted))
												{
													Display11_2_1(disptr);
												}
											else if((logDispdata.chardata.RecordType==ConvenSFault)||(logDispdata.chardata.RecordType==RepeaterConvenSFault))
												{

													Display11_2_2(disptr);
	

												}
											else if((logDispdata.chardata.RecordType==EARTHFAULT)||(logDispdata.chardata.RecordType==RepeaterEarthfault))
												{
														Display11_2_3(disptr);
													

												}
											else if(logDispdata.chardata.RecordType==LoopPowerFault)
												{

													Display11_2_4(disptr);

												}
											else if((logDispdata.chardata.RecordType==convpowerfault)||(logDispdata.chardata.RecordType==Repeaterconvpowerfault))
												{
													Display11_2_5(disptr);


												}
											else if((logDispdata.chardata.RecordType==aux1powerFault)||(logDispdata.chardata.RecordType==RepeaterAUX1powerFault))
												{
													Display11_2_6(disptr);


												}
										
											else if(logDispdata.chardata.RecordType==loopshortopen)
												{
													Display11_2_7(disptr);


												}
												/*
											else if(logDispdata.chardata.RecordType==loopDuplicateAddress)
												{
													Display11_2_8();


												}
											else if(logDispdata.chardata.RecordType==loopdevideremove)
												{
													Display11_2_9();


												}*/
											else if((logDispdata.chardata.RecordType==mainpowerfault)||(logDispdata.chardata.RecordType==Repeaterpowerfault))
												{
													Display11_2_10(disptr);


												}
											else if((logDispdata.chardata.RecordType==batterypowerfault)||(logDispdata.chardata.RecordType==RepeaterBatFaul))
												{
													Display11_2_11(disptr);


												}

											else if((logDispdata.chardata.RecordType==batterychargerfault)||(logDispdata.chardata.RecordType==Repeaterbatterychargerfault))
												{
													Display11_2_12(disptr);


												}
											else if((logDispdata.chardata.RecordType==CustomerCheckSumError)||(logDispdata.chardata.RecordType==RepeaterCustomerCheckSumError))
												{
													Display11_2_13(disptr);


												}
											else if((logDispdata.chardata.RecordType==ProCheckSumError)||(logDispdata.chardata.RecordType==RepeaterProCheckSumError))
												{
													Display11_2_14(disptr);


												}
											else if((logDispdata.chardata.RecordType==mainpowerremove)||(logDispdata.chardata.RecordType==Repeatermainpowerremove))
												{
													Display11_2_15(disptr);


												}
												
											/*
											else if(logDispdata.chardata.RecordType==batterychargerfault)
												{
													Display11_2_13();

												}
												*/
												else if(logDispdata.chardata.RecordType==RepeatercomFault)
													{
													Display11_2_16(disptr);


													}
												else if(logDispdata.chardata.RecordType==Maincomfault)
													{
													Display11_2_17(disptr);


													}
												else if(logDispdata.chardata.RecordType==systemfaultclrwdt)
													{
													Display11_2_18(disptr);


													}
												else if(logDispdata.chardata.RecordType==SUBPANELcomFault)
													{
													Display11_2_19(disptr);



													}
												else if(logDispdata.chardata.RecordType==NetWorkComFault)
													{
													Display11_2_20(disptr);



													}
	
												address=strlen((const char *)totalfault[disptr->LanguageType%maxLanguage]);
												address=40+(40-address-5)/2;
											Lcd_Write_strreg(1, address, &totalfault[disptr->LanguageType%maxLanguage][0]);
											address=address+strlen((const char *)totalfault[disptr->LanguageType%maxLanguage])+1;
											Datatempint=disptr->FaultEventNum;
											Datatemp=Datatempint/1000;
											Datatempint=Datatempint%1000;
											Lcd_Write_charreg(1, address,Datatemp+0x30);//69
											
											Datatemp=Datatempint/100;
											Datatempint=Datatempint%100;	
											address++;
											Lcd_Write_charreg(1, address,Datatemp+0x30);
											
											Datatemp=Datatempint/10;
											Datatempint=Datatempint%10;	
											address++;
											Lcd_Write_charreg(1, address,Datatemp+0x30);
											
											Datatemp=Datatempint;
											//Datatempint=Datatempint%10;	
											address++;
											Lcd_Write_charreg(1, address,Datatemp+0x30);
											
													
												disptr->displayfault1++;
												disptr->dispfaulteventnum++;
												if((disptr->dispfaulteventnum==disptr->FaultEventNum))//&&(FaultEventNum>1))
												{
																disptr->displayfault1=0;
																disptr->dispfaulteventnum=0;
																
													if(disptr->maxdispfaulteventnum!=disptr->FaultEventNum)
													{
															

																	disptr->maxdispfaulteventnum=disptr->FaultEventNum;
																	

													


													}
												}
											


														
											
											break;

									}
							disptr->displayfault1++;

		}





}
void		Display11_2_1(lcdstruct	*disptr)
{

				static uint16_t	Datatemp,Datatempint;
				static unsigned char	Address;
		//		static unsigned long	eepAddress;


													Lcd_ClearReg();
														
														if((logDispdata.chardata.RecordType==FaultLoopdev)||(logDispdata.chardata.RecordType==loopDuplicateAddress)||(logDispdata.chardata.RecordType==loopdevideremove)||(logDispdata.chardata.RecordType==LoopWrongDeviceFitted))
															{
														Lcd_Write_strreg(0, 0, (const unsigned char	*)&faultloopdisp[disptr->LanguageType%maxLanguage][0]);
																}
													else if(logDispdata.chardata.RecordType==PERALARM)
														{
													Lcd_Write_strreg(0, 0, (const unsigned char	*)&peralarmdisp[disptr->LanguageType%maxLanguage][0]);


														}
														
					
															if(logDispdata.chardata.RecordDevType!=Wireleesmodule)
																{
													if(logDispdata.chardata.RecordZone>0)
																	{

														Lcd_Write_strreg(0, 9, (const unsigned char	*)&faultloopdisp[disptr->LanguageType%maxLanguage][1]);
														
						
														Datatempint=logDispdata.chardata.RecordZone;
														Datatemp=Datatempint/100;
														Datatempint=Datatempint%100;
														Lcd_Write_charreg(0, 14,Datatemp+0x30);
														
														Datatemp=Datatempint/10;
														Datatempint=Datatempint%10;	
														Lcd_Write_charreg(0, 15,Datatemp+0x30);
														
														Datatemp=Datatempint;
														Lcd_Write_charreg(0, 16,Datatemp+0x30);
														

														//eepAddress=((logDispdata.chardata.RecordZone-1)%ZoneNum)*18;
														//Read_EE1024(2, Eeprom,eepAddress, 18);
														readzonetext(((logDispdata.chardata.RecordZone-1)%ZoneNum), &Eeprom[0]);
														if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
															{

														Lcd_Write_strlabelreg(0, 40, (const unsigned char	*)&Eeprom[2], 16);
															}
					
														
														}
																}
					
														Lcd_Write_strreg(0, 18, (const unsigned char	*)&faultloopdisp[disptr->LanguageType%maxLanguage][2]);
														
						
														Datatempint=logDispdata.chardata.RecordDev%1000;
														Datatemp=Datatempint/100;
														Datatempint=Datatempint%100;
														Lcd_Write_charreg(0, 22,Datatemp+0x30);
														
														Datatemp=Datatempint/10;
														Datatempint=Datatempint%10;	
														Lcd_Write_charreg(0, 23,Datatemp+0x30);
														
														Datatemp=Datatempint;
														Lcd_Write_charreg(0, 24,Datatemp+0x30);
															

														Lcd_Write_charreg(0, 26,'L');
															

														Datatempint=logDispdata.chardata.RecordLoop%100;
														Datatemp=Datatempint/10;
														Datatempint=Datatempint%10;
															Lcd_Write_charreg(0, 27,Datatemp+0x30);
														
														Datatemp=Datatempint;
														Datatempint=Datatempint%10;	
														Lcd_Write_charreg(0, 28,Datatemp+0x30);
														
					
															if(logDispdata.chardata.RecordDev<126)
																{
					
																Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename1[disptr->LanguageType%maxLanguage][logDispdata.chardata.RecordDevType%31]);//有bug
																	}
																else{
																Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename2[disptr->LanguageType%maxLanguage][logDispdata.chardata.RecordDevType%31]);//有bug	


																	}
															if(logDispdata.chardata.RecordDevType!=Wireleesmodule)
																{
														readdevicetext(((logDispdata.chardata.RecordLoop-1)%LoopNum), ((logDispdata.chardata.RecordDev-1)%MaxDevice), &Eeprom[0]);
														//eepAddress=MaxDevice*24*((logDispdata.chardata.RecordLoop-1)%LoopNum)+((logDispdata.chardata.RecordDev-1)%MaxDevice)*24;
														//Read_EE1024(1, Eeprom,eepAddress, 24);
														if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
															{

															Lcd_Write_strlabelreg(0, 57, (const unsigned char	*)&Eeprom[2], 22);
															}
																}
																
														
														
														if(logDispdata.chardata.RecordType==loopDuplicateAddress)
														{


														/*
															if(disptr->LanguageType%maxLanguage==0)
																{
																Address=7;

																}
															else if(disptr->LanguageType%maxLanguage==0)
																{
																Address=2;

																	}*/


																	Address=(40-strlen((const char	*)&history[disptr->LanguageType%maxLanguage][loopDuplicateAddress-1]))/2;

																Lcd_Write_strreg(1, Address, (const unsigned char	*)&history[disptr->LanguageType%maxLanguage][loopDuplicateAddress-1]);


															}
														else if(logDispdata.chardata.RecordType==loopdevideremove)
															{
																/*if(disptr->LanguageType%maxLanguage==0)
																	{
																	Address=13;

																	}
																else if(disptr->LanguageType%maxLanguage==0)
																	{
																	Address=9;

																		}*/

																Address=(40-strlen((const char	*)&history[disptr->LanguageType%maxLanguage][loopdevideremove-1]))/2;

															Lcd_Write_strreg(1, Address, (const unsigned char	*)&history[disptr->LanguageType%maxLanguage][loopdevideremove-1]);



																}
													else if(logDispdata.chardata.RecordType==LoopWrongDeviceFitted)
															{
															/*	if(disptr->LanguageType%maxLanguage==0)
																	{
																	Address=11;

																	}
																else if(disptr->LanguageType%maxLanguage==0)
																	{
																	Address=9;

																		}
																		*/

																Address=(40-strlen((const char	*)&history[disptr->LanguageType%maxLanguage][LoopWrongDeviceFitted-1]))/2;


															Lcd_Write_strreg(1, Address, (const unsigned char	*)&history[disptr->LanguageType%maxLanguage][LoopWrongDeviceFitted-1]);



															}
														








}

void		Display11_2_2(lcdstruct	*disptr)
{

						static unsigned char	Address;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
							if(logDispdata.chardata.RecordType==ConvenSFault)
							{
						/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
						Address=(40-strlen((const char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0])-3)/2;
						Address=Address+40;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);


							//Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=logDispdata.chardata.RecordDev+1;//主机编号
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						/*if(disptr->LanguageType%maxLanguage==0)
							{

								Address=66;
							}
						else if(disptr->LanguageType%maxLanguage==1)
							{

								Address=69;
							}*/

						Address=Address+strlen((const char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0])+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						/*if(disptr->LanguageType%maxLanguage==0)
							{

								Address=67;
							}
						else if(disptr->LanguageType%maxLanguage==1)
							{

								Address=70;
							}*/
							Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);


						


						

							}
						else if(logDispdata.chardata.RecordType==RepeaterConvenSFault)
							{
							/*
							if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
							Address=(40-strlen((const char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]))/2;
						Address=Address+40;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						//Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+strlen((const char	*)&Repeaterlab[disptr->LanguageType%maxLanguage])+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

							}
					

							if(disptr->LanguageType%maxLanguage==0)
							{
						Lcd_Write_strreg(1, 2, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][1]);
						

						Lcd_Write_charreg(1, 31,(logDispdata.chardata.RecordLoop%3)+0x30);
						
							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
						Lcd_Write_strreg(1, 1, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][1]);
						

						Lcd_Write_charreg(1, 38,(logDispdata.chardata.RecordLoop%3)+0x30);
						
							}

						else if(disptr->LanguageType%maxLanguage==2)
							{
						Lcd_Write_strreg(1, 4, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][1]);
						

						Lcd_Write_charreg(1, 34,(logDispdata.chardata.RecordLoop%3)+0x30);
						
							}

}
void		Display11_2_3(lcdstruct	*disptr)
{
					static unsigned char Address,charlength;
					static uint16_t	Datatempint;
					static uint16_t	Datatemp;
						Lcd_ClearReg();
						
					
							if(logDispdata.chardata.RecordType==EARTHFAULT)
							{
					
					/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
					

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logDispdata.chardata.RecordType==RepeaterEarthfault)
							{
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						/*
						if(disptr->LanguageType%maxLanguage==0)
							{
										Address=9;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=14;

							}*/
						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][4]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][4]);
						






}
void		Display11_2_4(lcdstruct	*disptr)
{
	static unsigned char	Address;
	static unsigned char	charlength;
	static uint16_t	Datatempint,Datatemp;

						Lcd_ClearReg();
						
					

						/*	if(logDispdata.chardata.RecordDev==0)
								{
								
						
								
								charlength=strlen((const char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
								Address=40+(40-charlength)/2;
								Lcd_Write_str(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
								}
							else{
								charlength=strlen((const char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
								Address=40+(40-charlength-3)/2;


								Lcd_Write_str(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

								Address=40+(40-charlength-3)/2+charlength+1;

								}*/

						/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
								charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);

								Address=40+(40-charlength-3)/2+charlength+1;

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);	
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][5]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][5]);





}
void		Display11_2_5(lcdstruct	*disptr)
{

						static unsigned char	Address,charlength;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
						/*
						Lcd_Write_str(0, 55, &mainpanelfault[0]);
						
						Lcd_Write_str(1, 1, &mainpanelfault[6]);
						
						*/
							if(logDispdata.chardata.RecordType==convpowerfault)
							{
						/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+((40-charlength)/2);
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=Address+charlength+1;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;

							}
						else if(logDispdata.chardata.RecordType==Repeaterconvpowerfault)
							{
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+((40-charlength)/2);
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, 68, Datatemp+0x30);

							}
						
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=1;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=0;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][6]);
						Address=((40-charlength)/2);
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][6]);





}
void		Display11_2_6(lcdstruct *disptr)
{
		static unsigned char	Address,charlength;
		static uint16_t	Datatempint;
		static uint16_t	Datatemp;
	Lcd_ClearReg();
						
						
						if(logDispdata.chardata.RecordType==aux1powerFault)
							{
							
					/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
							
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logDispdata.chardata.RecordType==RepeaterAUX1powerFault)
							{
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
								charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=40+(40-charlength-3)/2+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, 68, Datatemp+0x30);
							}
						
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=3;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=0;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][7]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][7]);






}
void		Display11_2_7(lcdstruct *disptr)
{
				static uint16_t	Datatempint,Datatemp;
				static unsigned char	Address;
				static unsigned char	charlength;
						Lcd_ClearReg();
						
				
					/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=50;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=49;

							}*/
							/*
						if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
							
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					Address=strlen((const char *)mainpanelfault[disptr->LanguageType%maxLanguage][3]);
					Address=(40-(Address+3))/2;
					/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=6;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=13;

							}*/
					Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][3]);
						Address=Address+strlen((const char *)mainpanelfault[disptr->LanguageType%maxLanguage][3])+1;
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=33;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=28;

							}*/
					Lcd_Write_charreg(1, Address,(logDispdata.chardata.RecordLoop/10)+0x30);	
					Address++;
					Lcd_Write_charreg(1, Address,(logDispdata.chardata.RecordLoop%10)+0x30);
						





}
void		Display11_2_10(lcdstruct	*disptr)
{
					static unsigned char	Address,charlength;
					static uint16_t	Datatempint;
					static uint16_t	Datatemp;

						Lcd_ClearReg();
						
							if(logDispdata.chardata.RecordType==mainpowerfault)
							{
							/*
						if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}
						Lcd_Write_str(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						*/
						

						

					/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);

							}
						else if(logDispdata.chardata.RecordType==Repeaterpowerfault)
							{
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
							charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=40+(40-charlength-3)/2+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

							}
						
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=7;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=5;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][8]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][8]);
					





}
void		Display11_2_11(lcdstruct	*disptr)
{
						static unsigned char	Address,charlength;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
						

						if(logDispdata.chardata.RecordType==batterypowerfault)
							{
					
					/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
								charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logDispdata.chardata.RecordType==RepeaterBatFaul)
							{
							/*
						if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
								charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
					/*		if(disptr->LanguageType%maxLanguage==0)
							{
										Address=9;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=4;

							}*/
							Address=(40-strlen((const char	*)mainpanelfault[disptr->LanguageType%maxLanguage][9]))/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][9]);
						






}
void		Display11_2_12(lcdstruct	* disptr)
{

						static unsigned char	Address,charlength;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
					
							if(logDispdata.chardata.RecordType==batterychargerfault)
							{
							/*
							if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}
						Lcd_Write_str(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						*/
						/*
						if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logDispdata.chardata.RecordType==Repeaterbatterychargerfault)
							{
							/*
							if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
							charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=4;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=1;

							}*/

							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][10]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][10]);
						





}
void		Display11_2_13(lcdstruct	*disptr)
{
						static unsigned char	Address,charlength;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
					
							if(logDispdata.chardata.RecordType==CustomerCheckSumError)
							{
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=Address+charlength+1;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						
							}
						else if(logDispdata.chardata.RecordType==RepeaterCustomerCheckSumError)
							{
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
								charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, 68, Datatemp+0x30);
							}
						
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=6;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=12;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][12]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][12]);
						






}
void		Display11_2_14(lcdstruct	*disptr)
{


						static unsigned char	Address,charlength;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
					
							if(logDispdata.chardata.RecordType==ProCheckSumError)
							{
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
							
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=Address+charlength+1;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						//Address++;
						
							}
						else if(logDispdata.chardata.RecordType==RepeaterProCheckSumError)
							{
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][11]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, 5, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][11]);
						




}
void		Display11_2_15(lcdstruct	*disptr)
{
						static unsigned char	Address,charlength;
						static uint16_t	Datatempint;
						static uint16_t	Datatemp;
						Lcd_ClearReg();
						
						
						if(logDispdata.chardata.RecordType==mainpowerremove)
							{
							/*
						if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logDispdata.chardata.RecordType==Repeatermainpowerremove)
							{
							/*
							if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
							
						/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=2;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=0;

							}*/

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][13]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][13]);
						






}
void		Display11_2_16(lcdstruct	*disptr)
{

					static uint16_t	Address,charlength;
					static uint16_t	Datatempint,Datatemp;
					
					Lcd_ClearReg();
					/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[disptr->LanguageType%maxLanguage]);
						Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						charlength=strlen((const char *)&RepComFault[disptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, 10, (const unsigned char	*)&RepComFault[disptr->LanguageType%maxLanguage]);












}
void		Display11_2_17(lcdstruct	*disptr)
{

					static uint16_t	Address,charlength;
					//static uint16_t	Datatempint,Datatemp;
					
					Lcd_ClearReg();
						/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
							
						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage]);
					
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=13;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=12;

							}*/
							charlength=strlen((const char *)&SysTemError[disptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&SysTemError[disptr->LanguageType%maxLanguage]);












}
void		Display11_2_18(lcdstruct	*disptr)
{

					static uint16_t	Address,charlength;
					static unsigned char	Datatempint,Datatemp;
					//static uint16_t	Datatempint,Datatemp;
					
					Lcd_ClearReg();
					/*
						if(disptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/

						charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage]);
						Address=Address+charlength+1;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						//Address++;
							
							/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=13;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=12;

							}*/

							charlength=strlen((const char *)&SysTemError[disptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&SysTemError[disptr->LanguageType%maxLanguage]);












}
void		Display11_2_19(lcdstruct	*disptr)
{

					static uint16_t	Address,charlength;
					static uint16_t	Datatempint,Datatemp;
					
					Lcd_ClearReg();
					/*
						if(disptr->LanguageType%maxLanguage==0)
							{
										Address=50;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=49;

							}*/
							charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][2]);
						Datatempint=logDispdata.chardata.RecordDev;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							charlength=strlen((const char *)&RepComFault[disptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&RepComFault[disptr->LanguageType%maxLanguage]);












}
void		Display11_2_20(lcdstruct	*disptr)
{

					static uint16_t	Address,charlength;
					static uint16_t	Datatempint,Datatemp;
					
					Lcd_ClearReg();
					/*
						if(disptr->LanguageType%maxLanguage==0)
							{
										Address=50;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=45;

							}*/
								charlength=strlen((const char *)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[disptr->LanguageType%maxLanguage][0]);
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						charlength=strlen((const char *)&NetworkComFault[disptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&NetworkComFault[disptr->LanguageType%maxLanguage]);












}
void		Display11_1(lcdstruct	*disptr)
{			const unsigned char	*p;
			static volatile		unsigned char	Address,Datatempint,Datatemp,i,charlength;;

		

			
			Lcd_ClearReg();
			if(SysTemFlag.Bit.RunLevel<2)
				{

					p=(const unsigned char	*)RunUser0[disptr->LanguageType%maxLanguage];
					charlength=strlen((const char *)&RunUser0[disptr->LanguageType%maxLanguage]);
					Address=40+((40-charlength)/2);
				//	Address=52;


				}
			else if(SysTemFlag.Bit.RunLevel==2)
				{
				p=(const unsigned char	*)RunUser1[disptr->LanguageType%maxLanguage];
				/*if((disptr->LanguageType%maxLanguage)==0)
									{
								Address=51;
									}
								else{

								Address=52;

									}*/
				charlength=strlen((const char *)&RunUser1[disptr->LanguageType%maxLanguage]);
					Address=40+((40-charlength)/2);


				}
			else if(SysTemFlag.Bit.RunLevel==3)
				{
				p=(const unsigned char	*)RunUser2[disptr->LanguageType%maxLanguage];
					/*if((disptr->LanguageType%maxLanguage)==0)
									{
								Address=49;
									}
								else{

								Address=46;

									}
									*/
									
				
				charlength=strlen((const char *)&RunUser2[disptr->LanguageType%maxLanguage]);
					Address=40+((40-charlength)/2);

				}
			else if(SysTemFlag.Bit.RunLevel==4)
				{
				p=(const unsigned char	*)RunUser3[disptr->LanguageType%maxLanguage];
				/*if((disptr->LanguageType%maxLanguage)==0)
									{
								Address=49;
									}
								else{

								Address=46;

									}
									*/
									
					charlength=strlen((const char *)&RunUser3[disptr->LanguageType%maxLanguage]);
					Address=40+((40-charlength)/2);

				}
		//	if(SysTemFlag.Bit.RunLevel>1)
			//	{
				Lcd_Write_strreg(0, Address, p);
				
			//	}

				for(i=0;i<28;i++)
						{
						if(CompanyName[27-i]!=0x20)
							{
                            
							break;



							}



						}
					CompanyName[28-i]=0;
					p=(const unsigned char	*)&CompanyName;
                  			  Address=(40-(28-i));
                 			   Address=Address/2;
							
							Lcd_Write_strreg(0, Address, p);
			

									Address=15-sanjicaidan7_1_1_3[disptr->LanguageType%maxLanguage][DS1302TIME.systemweekday%7];
									Lcd_Write_strreg(1, Address, (const unsigned char	*)sanjicaidan7_1_1_2[disptr->LanguageType%maxLanguage][DS1302TIME.systemweekday%7]);
									
								
									
									Datatempint=DS1302TIME.systemday;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 16, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 17, Datatemp+0x30);
									
									Lcd_Write_charreg(1, 18, '-');
									
									Datatempint=DS1302TIME.systemmon;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 19, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 20, Datatemp+0x30);
									
									Lcd_Write_charreg(1, 21, '-');
									

									Datatempint=DS1302TIME.systemyear;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 22, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 23, Datatemp+0x30);
									

									Datatempint=DS1302TIME.systemhour;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 25, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 26, Datatemp+0x30);
									
									
									Lcd_Write_charreg(1, 27, ':');
									
									LastMin=DS1302TIME.systemmin;
									Datatempint=DS1302TIME.systemmin;
									
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 28, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 29, Datatemp+0x30);
									

									if(SysTemFlag.Bit.RunMode==0)//安装模式
										{
										/*	if(disptr->LanguageType%maxLanguage==0)
												{
											Address=50;
												}
											else if(disptr->LanguageType%maxLanguage==1)
												{
											Address=51;
												}*/
												charlength=strlen((const char *)&ModeDisp[disptr->LanguageType%maxLanguage][0]);
												Address=40+((40-charlength)/2);
											p=(const unsigned char	*)&ModeDisp[disptr->LanguageType%maxLanguage][0];


										}
									else{
											/*if(disptr->LanguageType%maxLanguage==0)
												{
											Address=53;
												}
											else if(disptr->LanguageType%maxLanguage==1)
												{
											Address=54;
												}*/
												

											charlength=strlen((const char *)&ModeDisp[disptr->LanguageType%maxLanguage][1]);
												Address=40+((40-charlength)/2);
											p=(const unsigned char	*)&ModeDisp[disptr->LanguageType%maxLanguage][1];	


										}
								Lcd_Write_strreg(1, Address, p);
									

}
void		DispTEST(lcdstruct	*disptr)
{
						static	unsigned  int	Datatempint,Datatemp;
					//	static unsigned long	eepAddress;
						Lcd_ClearReg();
								
													
													Lcd_Write_strreg(0, 0, (const unsigned char	*)&testdisp[disptr->LanguageType%maxLanguage][0]);
													if(TestDispdata.chardata.RecordZone>0)
																	{

														Lcd_Write_strreg(0, 9, (const unsigned char	*)&faultloopdisp[disptr->LanguageType%maxLanguage][1]);
														
						
														Datatempint=TestDispdata.chardata.RecordZone;
														Datatemp=Datatempint/100;
														Datatempint=Datatempint%100;
														Lcd_Write_charreg(0, 14,Datatemp+0x30);
														
														Datatemp=Datatempint/10;
														Datatempint=Datatempint%10;	
														Lcd_Write_charreg(0, 15,Datatemp+0x30);
														
														Datatemp=Datatempint;
														Lcd_Write_charreg(0, 16,Datatemp+0x30);
														readzonetext(TestDispdata.chardata.RecordZone-1, &Eeprom[0]);
														//eepAddress=(TestDispdata.chardata.RecordZone-1)*18;
														//Read_EE1024(2, Eeprom,eepAddress, 18);
														if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
															{

														Lcd_Write_strlabelreg(0, 40, (const unsigned char	*)&Eeprom[2], 16);
															}
																}
					
														Lcd_Write_strreg(0, 18, (const unsigned char	*)&faultloopdisp[disptr->LanguageType%maxLanguage][2]);
														
						
														Datatempint=TestDispdata.chardata.RecordDev%1000;
														Datatemp=Datatempint/100;
														Datatempint=Datatempint%100;
														Lcd_Write_charreg(0, 22,Datatemp+0x30);
														
														Datatemp=Datatempint/10;
														Datatempint=Datatempint%10;	
														Lcd_Write_charreg(0, 23,Datatemp+0x30);
														
														Datatemp=Datatempint;
														Lcd_Write_charreg(0, 24,Datatemp+0x30);
															

														Lcd_Write_charreg(0, 26,'L');
															

														Datatempint=TestDispdata.chardata.RecordLoop%100;
														Datatemp=Datatempint/10;
														Datatempint=Datatempint%10;
															Lcd_Write_charreg(0, 27,Datatemp+0x30);
														
														Datatemp=Datatempint;
														Datatempint=Datatempint%10;	
														Lcd_Write_charreg(0, 28,Datatemp+0x30);
														if(TestDispdata.chardata.RecordDev<126)
															{
														
					
														Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename1[disptr->LanguageType%maxLanguage][TestDispdata.chardata.RecordDevType%23]);
															}
														else{

														Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename2[disptr->LanguageType%maxLanguage][TestDispdata.chardata.RecordDevType%31]);

															}
														
														/*	eepAddress=MaxDevice*24*((TestDispdata.chardata.RecordLoop-1)%LoopNum)+((TestDispdata.chardata.RecordDev-1)%MaxDevice)*24;
														Read_EE1024(3, Eeprom,eepAddress, 24);*/
														readdevicetext(((logDispdata.chardata.RecordLoop-1)%LoopNum), ((logDispdata.chardata.RecordDev-1)%MaxDevice), &Eeprom[0]);
														if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
															{

															Lcd_Write_strlabelreg(0, 57, (const	unsigned char	*)&Eeprom[2], 22);
															}





}
void		Lcd_Write5_1_x(lcdstruct	*disptr)
{

									switch(disptr->Setlevel3)
										{
										case 1:
											Lcd_Write1(sanjicaidan5_1_1[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 2:
										  	Lcd_Write1(sanjicaidan5_1_2[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 3:
											Lcd_Write1(sanjicaidan5_1_3[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 4:
										  	Lcd_Write1(sanjicaidan5_1_4[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 5:
											Lcd_Write1(sanjicaidan5_1_5[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 6:
											Lcd_Write1(sanjicaidan5_1_6[disptr->LanguageType%maxLanguage],disptr);	
											break;
										case 7:
											Lcd_Write1(sanjicaidan5_1_7[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 8:
										  	Lcd_Write1(sanjicaidan5_1_8[disptr->LanguageType%maxLanguage],disptr);
											break;
										
										case 9:
										  	Lcd_Write1(sanjicaidan5_1_9[disptr->LanguageType%maxLanguage],disptr);
											break;
										}
										



}

void			Lcd_Write5_2_x(lcdstruct	*disptr)
{

							switch(disptr->Setlevel3)
										{
										case 1:
											Lcd_Write1(sanjicaidan5_2_1[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 2:
										  	Lcd_Write1(sanjicaidan5_2_2[disptr->LanguageType%maxLanguage],disptr);
											break;
										
											}



}

void		Lcd_DispLay5_1_X_X(lcdstruct	*disptr)
{

		static uint16_t	Datatempint;
		static uint16_t	Datatemp;
		static unsigned char	Address,charlength;
//		static unsigned long	eepAddress;
		switch(disptr->Setlevel3)
			{
				case 1:
						switch(disptr->Setlevel3)
							{
								
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_1[disptr->LanguageType%maxLanguage]);
						
									
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,sanjicaidan5_1_1_6[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
									
								

									
										
									Lcd_Write_strreg(1, 0, sanjicaidan5_1_1_1[disptr->LanguageType%maxLanguage]);
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
								
									Lcd_Write_strreg(1, 20, sanjicaidan5_1_1_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLoopEnableDisable%2]);
									
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[4].A==1))
										{
									
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);		
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
										}
									
									else{
									
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

											Lcd_pos(1, 20);
											
											Lcd_CmdWtite(1, 0x0d);
											


										
								
										}
									

							}


					break;
								
					case 2:
						
						Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_2[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
									if(disptr->SetLevelDeviceChar==0)
										{
										
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);

									
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
										}
									else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
											
										Lcd_Write_charreg(0, 44, 1);//↑
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if(disptr->LanguageType%maxLanguage==2)
										
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}



										}
									

									
								
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									

									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
									/*
										eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
												if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
									if(disptr->Setlevel4==254)
										{
									
								/*
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=(40-charlength)/2;
									Address=Address+40;
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {

											Lcd_Write_strreg(1, 52, erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);
											
											
											Lcd_Write_strreg(1, 61, sanjicaidan5_1_1_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayDevEnableDisabled%2]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1){
									
									
											if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
												}


											else{

												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);

												}
											


										
								
											}
									else if(disptr->Setlevel4==2)
										{


											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
						case 3:
							Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_3[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
										if((disptr->SetLevelDeviceChar==0))
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;

									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}
										else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
											
										Lcd_Write_charreg(0, 44, 1);//↑
										
											if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if(disptr->LanguageType%maxLanguage==2)
										
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}
										




										}
									

									
								
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
									
										/*eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
											if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
									if(disptr->Setlevel4==254)
										{
									
								//	Lcd_Write_str(1,52,erjicaidan1_2_3);
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
													charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=(40-charlength)/2;
									Address=Address+40;
											
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {

											Lcd_Write_strreg(1, 52, erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);
											
											
											Lcd_Write_strreg(1, 61, sanjicaidan5_1_1_3[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayDevSelectDisablement%2]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1){
									
									
										if(disptr->SetLevelDeviceChar==0)
											{
												
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
											}
										else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




											}
											


										
								
											}
									else if(disptr->Setlevel4==2)
										{


											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
					case  4:
						Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_4[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
										if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
										}
											else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
									
										}
									else if(disptr->Setlevel4==2)
										{
										Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
											
										Lcd_Write_charreg(0, 44, 1);//↑
										
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if(disptr->LanguageType%maxLanguage==2)
										
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}
										




										}
									

									
								
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
									
										/*eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, &Eeprom[2], 22);
										}*/
									if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
									if(disptr->Setlevel4==254)
										{
									
									
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
													charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=(40-charlength)/2;
									Address=Address+40;
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {

										

												/*if(disptr->LanguageType%maxLanguage==0)
												{

											Lcd_Write_strreg(1,52,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												}
											else if(disptr->LanguageType%maxLanguage==1)
												{

											Lcd_Write_strreg(1,43,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												}
											*/

											
												charlength=strlen((const char *)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												Address=40+(20-charlength);
													Lcd_Write_strreg(1,Address,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 61, sanjicaidan5_1_1_4[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayReportType%3]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1){
									
									
												if(disptr->SetLevelDeviceChar==0)
													{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
													}
													else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




											}
											


										
								
											}
									else if(disptr->Setlevel4==2)
										{


											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;

							case 5:
							Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_5[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
										if(disptr->SetLevelDeviceChar==0)
											{
										Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
										charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}
									else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
											
										Lcd_Write_charreg(0, 44, 1);//↑
										
										//Lcd_Write_charreg(0, 50, 2);//↓
										
											if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if(disptr->LanguageType%maxLanguage==2)
										
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}



										}
									

									
								
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
									/*
										eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
										
									if(disptr->Setlevel4==254)
										{
									
									//Lcd_Write_str(1,52,erjicaidan1_2_3);
									/*
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {
											charlength=strlen((const char *)erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);
												Address=40+(20-charlength);
													Lcd_Write_strreg(1,Address,erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);

										//	Lcd_Write_strreg(1, 52, erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);
											
											
											Lcd_Write_strreg(1, 61, sanjicaidan5_1_1_5[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayIMMEvac%2]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1){
									
									

												if(disptr->SetLevelDeviceChar==0)
													{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
													}

												else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




												}
											


										
								
											}
									else if(disptr->Setlevel4==2)
										{


											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
							case 6:
								Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_6[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
										if(disptr->SetLevelDeviceChar==0)
											{
										Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
												charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}

										else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
											
										Lcd_Write_charreg(0, 44, 1);//↑
										
										//Lcd_Write_charreg(0, 50, 2);//↓
										

											if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if(disptr->LanguageType%maxLanguage==2)
										
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}


										}
									

									
								
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
									
									/*	eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
									if(disptr->Setlevel4==254)
										{
									
									
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {
											/*	if(disptr->LanguageType%maxLanguage==0)
													{
											Lcd_Write_strreg(1, 52, sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);
													}
												else if(disptr->LanguageType%maxLanguage==1)
													{
											Lcd_Write_strreg(1, 46, sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);

													}*/

											charlength=strlen((const char *)sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);
												Address=40+(20-charlength);
													Lcd_Write_strreg(1,Address,sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);
											
											
											Lcd_Write_strreg(1, 61, sanjicaidan5_1_6_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayDEVOverrideDelay%2]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1){
									
									

												if(disptr->SetLevelDeviceChar==0)
													{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
													}

												else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




												}
											
											


										
								
											}
									else if(disptr->Setlevel4==2)
										{


											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
							case 7:
								Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_7[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
										if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
													charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}

											else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
											
										Lcd_Write_charreg(0, 44, 1);//↑
										
									//	Lcd_Write_charreg(0, 50, 2);//↓
										

										
											if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if(disptr->LanguageType%maxLanguage==2)
										
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}


										}
									

									
								
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									/*
									
										eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17,(const unsigned char	*) &Eeprom[2], 22);
										}*/
											if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
									if(disptr->Setlevel4==254)
										{
									
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
		
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {
											charlength=strlen((const char *)erjicaidan3_4_5[disptr->LanguageType%maxLanguage]);
									Address=40+(20-charlength);
											Lcd_Write_strreg(1, Address, erjicaidan3_4_5[disptr->LanguageType%maxLanguage]);
											
											
											Lcd_Write_strreg(1, 61, sanjicaidan5_1_7_1[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayDEVInhibitAuxRelay%2]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1){
									
									

												if(disptr->SetLevelDeviceChar==0)
													{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
													}

												else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




												}
											


										
								
											}
									else if(disptr->Setlevel4==2)
										{


											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
					case 8:
								Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_8[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(0,40,sanjicaidan5_1_8_1[disptr->LanguageType%maxLanguage][0]);
										
									Lcd_Write_strreg(1,0,sanjicaidan5_1_8_1[disptr->LanguageType%maxLanguage][1]);
										
									if(disptr->Setlevel4==254)
										{
										/*
										if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										
										}
									else{
									Lcd_Write_strreg(1,40,sanjicaidan5_1_8_1[disptr->LanguageType%maxLanguage][2]);
										
										}
									
									Lcd_Write_strreg(0,60,sanjicaidan5_1_8_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispGolbalTimedSensitivity.Workday]);
										
									Lcd_Write_strreg(1,20,sanjicaidan5_1_8_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispGolbalTimedSensitivity.Satuday]);
										
									if(disptr->Setlevel4!=254)
										{
									Lcd_Write_strreg(1,60,sanjicaidan5_1_8_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispGolbalTimedSensitivity.Sunday]);
										
										}
								
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									 if(disptr->DispDeviceSet.Disp5_1_8row==0)
										{


											Lcd_pos(0, 60);
											
											Lcd_CmdWtite(0, 0x0d);
											

										}
									 	else  if(disptr->DispDeviceSet.Disp5_1_8row==1)
										{


											Lcd_pos(1, 20);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
										else  if((disptr->DispDeviceSet.Disp5_1_8row==2)&&(disptr->Setlevel4!=254))
										{


											Lcd_pos(1, 60);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
						case 9:
								Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_1_9[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(1,0,sanjicaidan5_1_9_1[disptr->LanguageType%maxLanguage][0]);
										
									
									if(disptr->Setlevel4==254)
										{
									
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
												charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										
										}
									else{
									Lcd_Write_strreg(1,40,sanjicaidan5_1_9_1[disptr->LanguageType%maxLanguage][1]);
										
										}
			
									Lcd_Write_strreg(0,40,erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 40, 3);//↓
									
									if(disptr->LanguageType%maxLanguage==0)
										{
											Lcd_Write_charreg(0, 59, 1);//↑
									
								
									Lcd_Write_charreg(0, 61, 2);//↓
									
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
								
										Lcd_Write_charreg(0, 61, 1);//↑
									
								
									Lcd_Write_charreg(0, 63, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
								
										Lcd_Write_charreg(0, 57, 1);//↑
									
								
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
									Lcd_Write_strreg(1,20,sanjicaidan5_1_9_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispGolbalTimedSensitivity.TimedDay%3]);
										
									if(disptr->Setlevel4!=254)
										{
									Lcd_Write_strreg(1,60,sanjicaidan5_1_9_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispGolbalTimedSensitivity.TimedNight%3]);
										
										}
								
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								
									 	  if(disptr->DispDeviceSet.Disp5_1_8row==0)
										{


											Lcd_pos(1, 20);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
										else  if((disptr->DispDeviceSet.Disp5_1_8row==1)&&(disptr->Setlevel4!=254))
										{


											Lcd_pos(1, 60);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									break;
			}


}


void	Lcd_DispLay5_2_X_X(lcdstruct	 *disptr)
{
		static uint16_t	Datatempint;
		static uint16_t	Datatemp;
		static unsigned char	Address,charlength;
//		static unsigned long	eepAddress;
		switch(disptr->Setlevel3)
			{
				case 1:
						switch(disptr->Setlevel4)
							{
								
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_2_1[disptr->LanguageType%maxLanguage]);
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
									if(disptr->SetLevelDeviceChar==0)
										{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);

									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
										}
									else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										
									Lcd_Write_strreg(0,40,sanjicaidan5_2_1_1[disptr->LanguageType%maxLanguage]);
									if(disptr->LanguageType%maxLanguage==0)
										{
									
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									
									Lcd_Write_charreg(0, 45, 1);//↑
									
									Lcd_Write_charreg(0, 49, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
									
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 48, 2);//↓
									
										}

										}
									
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
										/*eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
									if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}

									
										if(disptr->Setlevel4==254)
										{
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
												charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {

											
											/*if(disptr->LanguageType%maxLanguage==0)
												{

											Lcd_Write_strreg(1,52,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												}
											else if(disptr->LanguageType%maxLanguage==1)
												{

											Lcd_Write_strreg(1,43,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												}
												*/
												charlength=strlen((const char *)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												Address=40+(20-charlength);
												Lcd_Write_strreg(1,Address,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 61, sanjicaidan5_2_1_2[disptr->DispDeviceSet.DispLayDevHeatGrade%4]);
											

										}
									
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1)
										{
									
									

												if(disptr->SetLevelDeviceChar==0)
													{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
													}

												else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




													}
											

											


										
								
										}
									else if	(disptr->Setlevel4==2)
										{
									
									

											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											


										
								
										}
									

							}


					break;
				case 2:
						switch(disptr->Setlevel4)
							{
								
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan5_2_2[disptr->LanguageType%maxLanguage]);
									
									if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
										{
									if(disptr->SetLevelDeviceChar==0)
										{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
										}

									else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}
									else if(disptr->Setlevel4==2)
										{
										
									Lcd_Write_strreg(0,40,sanjicaidan5_2_2_1[disptr->LanguageType%maxLanguage]);
									
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);//↑
									
									Lcd_Write_charreg(0,49, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//↓

									
										}

										}
									
									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispDeviceSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									Datatempint=disptr->DispDeviceSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									

										/*eepAddress=MaxDevice*24*(disptr->DispDeviceSet.SetLoopNum%LoopNum)+disptr->DispDeviceSet.SetDeviceNum*24;
									Read_EE1024(1,Eeprom, eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strreglabel(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}*/
									if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
										if(disptr->Setlevel4==254)
										{
									
									/*
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
											
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else {

											
											/*	if(disptr->LanguageType%maxLanguage==0)
												{

											Lcd_Write_strreg(1,52,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												}
											else if(disptr->LanguageType%maxLanguage==1)
												{

											Lcd_Write_strreg(1,43,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												}
												*/
											charlength=strlen((const char *)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
												Address=40+(20-charlength);
												Lcd_Write_strreg(1,Address,erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 61, sanjicaidan5_2_2_2[disptr->LanguageType%maxLanguage][disptr->DispDeviceSet.DispLayDevSmokeSensitivity%4]);
											

										}
									
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel4==1)
										{
									
									

												if(disptr->SetLevelDeviceChar==0)
													{
											Lcd_pos(1, 15);
											
											Lcd_CmdWtite(1, 0x0d);
													}

												else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




												}
											

											


										
								
										}
									else if	(disptr->Setlevel4==2)
										{
									
									

											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											


										
								
										}
									

							}


					break;
					
			}


}
void		Lcd_Write7_1_x(lcdstruct	*disptr)
{

		
									switch(disptr->Setlevel3)
										{
										case 1:
											Lcd_Write1(sanjicaidan7_1_1[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 2:
										  	Lcd_Write1(sanjicaidan7_1_2[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 3:
											Lcd_Write1(sanjicaidan7_1_3[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 4:
										  	Lcd_Write1(sanjicaidan7_1_4[disptr->LanguageType%maxLanguage],disptr);
											break;
										case 5:
											Lcd_Write1(sanjicaidan7_1_5[disptr->LanguageType%maxLanguage],disptr);
											break;
										}




}


void		Lcd_Write7_2_x(lcdstruct *disptr)
{
			if(disptr->Setlevel3==1)
				{
				Lcd_Write1(sanjicaidan7_2_1[disptr->LanguageType%maxLanguage],disptr);	


				}
			else if(disptr->Setlevel3==2)
				{

				
				Lcd_Write1(sanjicaidan7_2_2[disptr->LanguageType%maxLanguage],disptr);	


				
				}

		


}

void	Lcd_Write7_3_x(lcdstruct *disptr)
{
			static unsigned char	Address;
			if(disptr->Setlevel3==1)
				{
				Lcd_Write1(sanjicaidan7_3_1[disptr->LanguageType%maxLanguage],disptr);	


				}
			else if(disptr->Setlevel3==2)
				{

				
				Lcd_Write1(sanjicaidan7_3_2[disptr->LanguageType%maxLanguage],disptr);	


				
				}
			else if(disptr->Setlevel3==3)
				{
				Lcd_Write1(sanjicaidan7_3_3[disptr->LanguageType%maxLanguage],disptr);	


				}
			else if(disptr->Setlevel3==4)
				{

				
				Lcd_Write1(sanjicaidan7_3_4[disptr->LanguageType%maxLanguage],disptr);	


				
				}
			else if(disptr->Setlevel3==5)
				{
				Lcd_Write1(sanjicaidan7_3_5[disptr->LanguageType%maxLanguage],disptr);
				
				if(disptr->LanguageType%maxLanguage==0)
					{
					Address=41;

					}
				else if((disptr->LanguageType%maxLanguage==1)||(disptr->LanguageType%maxLanguage==2))
					{
					Address=40;

					}
				Lcd_Write_strreg(1, Address, sanjicaidan7_3_5_3[disptr->LanguageType%maxLanguage]);
				Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
			
				}
			else if(disptr->Setlevel3==6)
				{

				
				Lcd_Write1(sanjicaidan7_3_6[disptr->LanguageType%maxLanguage],disptr);	


				
				}
				else if(disptr->Setlevel3==7)
				{

				
				Lcd_Write1(sanjicaidan7_3_7[disptr->LanguageType%maxLanguage],disptr);	


				
				}
					else if(disptr->Setlevel3==8)
				{

				
				Lcd_Write1(sanjicaidan7_3_8[disptr->LanguageType%maxLanguage],disptr);	


				
				}
				else if(disptr->Setlevel3==9)
				{

				
				Lcd_Write1(sanjicaidan7_3_9[disptr->LanguageType%maxLanguage],disptr);	


				
				}



}

void		Lcd_DispLay7_1_X_X(lcdstruct	*disptr)
{


			static uint16_t	Datatempint;
			static uint16_t	Datatemp;
			static unsigned char	Address,charlength;
		//	static unsigned long	eepAddress;

			switch(disptr->Setlevel3)
				{
					case 1:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_1_1[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_1_1_1[disptr->LanguageType%maxLanguage]);
											
									
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 50, 1);
									
									Lcd_Write_charreg(0, 52, 2);
										
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);
									
									Lcd_Write_charreg(0, 47, 2);
										
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 46, 1);
									
									Lcd_Write_charreg(0, 48, 2);
										
										}
									Address=20-sanjicaidan7_1_1_3[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.dispweekday%7];
									Lcd_Write_strreg(1, Address, sanjicaidan7_1_1_2[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.dispweekday%7]);
									
								
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].A==1))
										{
									
									
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);	
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									Datatempint=disptr->DispGeneralSet.dispday;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 21, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 22, Datatemp+0x30);
									
									Lcd_Write_charreg(1, 23, '-');
									
									Datatempint=disptr->DispGeneralSet.dispmonth;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 24, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 25, Datatemp+0x30);
									
									Lcd_Write_charreg(1, 26, '-');
									

									Datatempint=disptr->DispGeneralSet.dispyear;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 27, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 28, Datatemp+0x30);
									

									Datatempint=disptr->DispGeneralSet.disphour;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 30, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 31, Datatemp+0x30);
									

									Lcd_Write_charreg(1, 32, ':');
									
									
									Datatempint=disptr->DispGeneralSet.dispmin;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 33, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 34, Datatemp+0x30);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if(disptr->Setlevel4==1)
												{
											Lcd_pos(1, 19);
											
												}
											else if(disptr->Setlevel4==2)
												{

											Lcd_pos(1, 22);
												}
											else if(disptr->Setlevel4==3)
												{

											Lcd_pos(1, 25);
												}
											else if(disptr->Setlevel4==4)
												{

											Lcd_pos(1, 28);
												}
											else if(disptr->Setlevel4==5)
												{

											Lcd_pos(1, 31);
												}
											else if(disptr->Setlevel4==6)
												{

											Lcd_pos(1, 34);
												}
												
											Lcd_CmdWtite(1, 0x0d);
											


					break;	
					case 2:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_1_2[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
									
								
									Lcd_Write_charreg(0, 40, 3);//↓
									
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 59, 1);//↑
									
									Lcd_Write_charreg(0, 61, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 61, 1);//↑
									
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
										else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 57, 1);//↑
									
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
									
									if(disptr->Setlevel4==254)
										{
									
								
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,50,sanjicaidan7_1_2_2[disptr->LanguageType%maxLanguage]);
											

										}
									
									Lcd_Write_strreg(1,10,sanjicaidan7_1_2_1[disptr->LanguageType%maxLanguage]);
									
									
									Datatempint=disptr->DispGeneralSet.DispLayDaynightDefine.DayHour;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 24, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 25, Datatemp+0x30);
									

									Lcd_Write_charreg(1, 26, ':');
									
									
									Datatempint=disptr->DispGeneralSet.DispLayDaynightDefine.DayMin;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 27, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 28, Datatemp+0x30);
									
										if(disptr->Setlevel4!=254)
										{
									Datatempint=disptr->DispGeneralSet.DispLayDaynightDefine.NightHour;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 64, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 65, Datatemp+0x30);
									

									Lcd_Write_charreg(1, 66, ':');
									
									
									Datatempint=disptr->DispGeneralSet.DispLayDaynightDefine.NightMin;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 67, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 68, Datatemp+0x30);
									
											}
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if(disptr->Setlevel4==1)
												{
											Lcd_pos(1, 25);
											
												}
											else if(disptr->Setlevel4==2)
												{

											Lcd_pos(1, 28);
												}
											else if(disptr->Setlevel4==3)
												{

											Lcd_pos(1, 65);
												}
											else if(disptr->Setlevel4==4)
												{

											Lcd_pos(1, 68);
												}
											
												
											if(disptr->Setlevel4!=254)
												{
											Lcd_CmdWtite(1, 0x0d);
											
													}


					break;	
				case 3:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_1_3[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
										
									Lcd_Write_charreg(0, 44, 1);//↑
									
									//Lcd_Write_charreg(0, 50, 2);//↓

									if(((disptr->LanguageType%maxLanguage)==0)||((disptr->LanguageType%maxLanguage)==1))
										{

										Lcd_Write_charreg(0, 50, 2);//↓

										}
									else if((disptr->LanguageType%maxLanguage)==2)
										{
										Lcd_Write_charreg(0, 48, 2);//↓
											
										}
									
									
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].C==1))
										{
									
								
								/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									/*if(disptr->LanguageType%maxLanguage==0)
										{
										Address=2;

										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
										Address=0;

										}*/
									charlength=strlen((const char	*)sanjicaidan7_1_3_1[disptr->LanguageType%maxLanguage]);
									Address=19-charlength;
									Lcd_Write_strreg(1,Address,sanjicaidan7_1_3_1[disptr->LanguageType%maxLanguage]);
										

									Lcd_Write_strreg(1,20,sanjicaidan7_1_3_2[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DispLayDaynightDefine.NightDelay]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
											if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].C==1))
											{

												}
											else{
											Lcd_pos(1, 20);
											
											
											
											Lcd_CmdWtite(1, 0x0d);
											
												
												}


					break;	
				case 4:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_1_4[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 40, 3);//↓
									
									if(disptr->LanguageType%maxLanguage==0)
										{
											Lcd_Write_charreg(0, 59, 1);//↑
									
								
									Lcd_Write_charreg(0, 61, 2);//↓
									
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
								
										Lcd_Write_charreg(0, 61, 1);//↑
									
								
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
								
										Lcd_Write_charreg(0, 57, 1);//↑
									
								
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].D==1))
										{
									
								//	Lcd_Write_str(1,52,erjicaidan1_2_3);
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
											
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									
									Lcd_Write_strreg(1, 5, erjicaidan3_6_1[disptr->LanguageType%maxLanguage][0]);
									
									Lcd_Write_strreg(1, 14, erjicaidan3_6_1[disptr->LanguageType%maxLanguage][1]);
									

									Datatempint=disptr->DispGeneralSet.DispEVACTimerDefine.Mins;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 11, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									

									Datatempint=disptr->DispGeneralSet.DispEVACTimerDefine.seconds;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 19, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(1, 20, Datatemp+0x30);
									


									Lcd_Write_strreg(1, 24, sanjicaidan7_1_4_1[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DispEVACTimerDefine.statusmode%3]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].D==1))
											{

												}
											else{
												if(disptr->Setlevel4==1)
													{
											Lcd_pos(1, 12);
													}
												if(disptr->Setlevel4==2)
													{
											Lcd_pos(1, 20);
													}
												else if(disptr->Setlevel4==3)
													{
											Lcd_pos(1, 24);

													}
											
											
											
											Lcd_CmdWtite(1, 0x0d);
											
												
												}


					break;	
					case	5:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_1_5[disptr->LanguageType%maxLanguage]);
									
									if(disptr->Setlevel4==2)
										{
									Lcd_Write_strreg(0,40,sanjicaidan7_1_5_1[disptr->LanguageType%maxLanguage]);
									
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1){

									Lcd_Write_charreg(0, 45, 1);//↑
									
									Lcd_Write_charreg(0,49, 2);//↓
									

										}
									else if(disptr->LanguageType%maxLanguage==2){

									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//↓
									

										}
										}
									else{
										if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);

									charlength=strlen((const char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}
										else  if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
										}

									Lcd_Write_strreg(1, 0, erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispGeneralSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispGeneralSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									

										
										/*eepAddress=MaxDevice*24*(disptr->DispGeneralSet.SetLoopNum%LoopNum)+disptr->DispGeneralSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
										}
									if(disptr->Setlevel4==254)
										{

									
									
								//	Lcd_Write_str(1,52,erjicaidan1_2_3);
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);		
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										

										}
									else{

											/*if(disptr->LanguageType%maxLanguage==0)
													{
											Lcd_Write_strreg(1, 52, sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);
													}
												else if(disptr->LanguageType%maxLanguage==1)
													{
											Lcd_Write_strreg(1, 46, sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);

													}	*/

										charlength=strlen((const char *)sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);
											Address=40+(20-charlength);
										Lcd_Write_strreg(1, Address, sanjicaidan5_1_6_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(1, 61, sanjicaidan7_1_4_2[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DispLayDeviceStartEVAC%2]);

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								if((disptr->Setlevel4==1)||(disptr->Setlevel4==254))
									{
										if(disptr->SetLevelDeviceChar==0)
											{
										Lcd_pos(1, 15);
										
										Lcd_CmdWtite(1, 0x0d);
											}
										

											else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




												}

										
										


									}
								else if(disptr->Setlevel4==2)
									{
										Lcd_pos(1, 61);
										
										Lcd_CmdWtite(1, 0x0d);
										


									}
					
						
								
				}
						


}


void		Lcd_DispLay7_2_X_X(lcdstruct	*disptr)
{


			static unsigned char	Address,charlength;
			

			switch(disptr->Setlevel3)
				{
					case 1:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_2_1[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_2_3[disptr->LanguageType%maxLanguage]);
											
									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==2))
										{
									Lcd_Write_charreg(0, 44, 1);
									
									Lcd_Write_charreg(0, 46, 2);
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);
									
									Lcd_Write_charreg(0, 49, 2);
										}
									
								
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].F==1))
										{
								
								/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);	
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									Lcd_Write_strreg(1,0,sanjicaidan5_1_1_2[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DispDeviceStartEVAC.TwoDeviceStartEVAC]);
									
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);		
											Lcd_pos(1, 0);	
											
											Lcd_CmdWtite(1, 0x0d);
											


									break;	
					case 2:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_2_2[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_2_3[disptr->LanguageType%maxLanguage]);
											
									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==2))
										{
									Lcd_Write_charreg(0, 44, 1);
									
									Lcd_Write_charreg(0, 46, 2);
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);
									
									Lcd_Write_charreg(0, 49, 2);
										}
									
								
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].G==1))
										{
									
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);		
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									Lcd_Write_strreg(1,0,sanjicaidan5_1_1_2[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DispDeviceStartEVAC.callpointsStartEVAC]);
									
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);	
											Lcd_pos(1, 0);	
											
											Lcd_CmdWtite(1, 0x0d);
											


					break;	
					

				}

	












}
//unsigned char	DisplayPermit;
//unsigned char Displaysetleveln;
void		Lcd_DispLay7_3_X_X(lcdstruct	*disptr)
{
			static uint16_t	Datatempint;
			static uint32_t Datatemplong;
			static uint16_t	Datatemp;//,checksumt;
//			static uint16_t	i;
			static unsigned char	Address,charlength;
			static unsigned char	i;
//static unsigned long	i;
//	static uint16_t	prochecksumtemp;
//		uint8_t *data;
		switch(disptr->Setlevel3)
			{
				case 1:
					
						
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_1[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_2_3[disptr->LanguageType%maxLanguage]);
											
									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==2))
										{
									Lcd_Write_charreg(0, 44, 1);
									
									Lcd_Write_charreg(0, 46, 2);
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);
									
									Lcd_Write_charreg(0, 49, 2);
										}
									
								
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].H==1))
										{
									
								/*
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									Lcd_Write_strreg(1,13,sanjicaidan7_3_1_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(1,20,sanjicaidan7_3_1_2[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DisplayRunMode%2]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
											Lcd_pos(1, 20);	
											
											Lcd_CmdWtite(1, 0x0d);
											
								break;

				case 2:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_2[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_4_1[disptr->LanguageType%maxLanguage]);
									
								/*	if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 44, 3);//↑
									
									Lcd_Write_charreg(0, 47, 1);//↓
									
									
									Lcd_Write_charreg(0, 53, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 3);//↑
									
									Lcd_Write_charreg(0, 48, 1);//↓
									
									
									Lcd_Write_charreg(0, 52, 2);//↓
									
										}
								else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 45, 3);//↑
									
									Lcd_Write_charreg(0, 47, 1);//↓
									
									
									Lcd_Write_charreg(0, 51, 2);//↓
									
										}*/
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].I==1))
										{
									
									//Lcd_Write_str(1,52,erjicaidan1_2_3);
									/*
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									/*
									if(disptr->LanguageType%maxLanguage==0)
										{
										Address=10;

										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
										Address=2;

										}*/
									charlength=strlen((const char *)sanjicaidan7_3_1_4[disptr->LanguageType%maxLanguage]);
									Address=28-charlength;
									Lcd_Write_strreg(1,Address,sanjicaidan7_3_1_4[disptr->LanguageType%maxLanguage]);
									
									
								
									Lcd_Write_charreg(1, 29, Level1code[0]+0x30);
									
									Lcd_Write_charreg(1, 30, Level1code[1]+0x30);
									
									Lcd_Write_charreg(1, 31, Level1code[2]+0x30);
									
									Lcd_Write_charreg(1, 32, Level1code[3]+0x30);
									
									Lcd_Write_charreg(1, 33, Level1code[4]+0x30);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
											Lcd_pos(1, 33);	
											
											Lcd_CmdWtite(1, 0x0d);
											
					
					
				break;
				case 3:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_3[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_3_1[disptr->LanguageType%maxLanguage]);
									
									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==2))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 46, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);//↑
									
									Lcd_Write_charreg(0, 47, 2);//↓
									
										}
									
									Lcd_DispLaySetLevel(disptr);
									
									if(disptr->Setlevel4==254)
										{
										
									//	Lcd_Write_str(1,52,erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);		
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										

										}
									else{

										charlength=strlen((const char *)sanjicaidan7_3_1_4[disptr->LanguageType%maxLanguage]);
										Address=40+(20-charlength)/2;
										Lcd_Write_strreg(1,Address,sanjicaidan7_3_3_2[disptr->LanguageType%maxLanguage]);
										

										Lcd_Write_strreg(1,61,sanjicaidan7_3_3_3[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DisplayPermit%3]);
										


										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
									if(disptr->Setlevel4==200)
										{
											
											Lcd_pos(1, 61);	
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel4!=254){
											if(disptr->DispGeneralSet.Displaysetleveln==0)
												{
											Lcd_pos(1, 0);	
												}
											else{

											Lcd_pos(1, 61);

												}
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									break;
				case 4:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_4[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_4_1[disptr->LanguageType%maxLanguage]);
									/*
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 44, 3);//↑
									
									Lcd_Write_charreg(0, 47, 1);//↓
									
									Lcd_Write_charreg(0, 53, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 3);//↑
									
									Lcd_Write_charreg(0, 49, 1);//↓
									
									Lcd_Write_charreg(0, 53, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 44, 3);//↑
									
									Lcd_Write_charreg(0, 47, 1);//↓
									
									Lcd_Write_charreg(0, 51, 2);//↓
									
										}*/
									//if(SetL4==254)
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].K==1))
										{
										
									//	Lcd_Write_str(1,52,erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);	
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										

										}
									else{

										
										Lcd_Write_strreg(1,56,sanjicaidan7_3_4_3[disptr->LanguageType%maxLanguage][disptr->Setlevel4%4]);
										

										


										}
											/*if(disptr->LanguageType%maxLanguage==0)
												{
												Address=5;

												}
											else if(disptr->LanguageType%maxLanguage==1)
												{
												Address=1;

												}*/
										charlength=strlen((const char *)sanjicaidan7_3_4_2[disptr->LanguageType%maxLanguage]);

											Address=28-charlength;
										Lcd_Write_strreg(1,Address,sanjicaidan7_3_4_2[disptr->LanguageType%maxLanguage]);
										
											
										Lcd_Write_strreg(1,29,mima[SetKeyCodeNum%6]);
										
											
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										
											Lcd_pos(1, 29+SetKeyCodeNum);	
											
											Lcd_CmdWtite(1, 0x0d);
											


										
					
					break;
				case 5:
						Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_5[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_5_1[disptr->LanguageType%maxLanguage]);
									
									
									
									
									if(disptr->Setlevel4==254)
										{
										
										
										/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);			
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										

										}
									
										/*if(disptr->LanguageType%maxLanguage==0)
											{
											Address=16;

											}
										else if(disptr->LanguageType%maxLanguage==1)
											{

											Address=14;

											}*/
										charlength=strlen((const char *)sanjicaidan7_3_5_2[disptr->LanguageType%maxLanguage]);

											Address=21-charlength;	
										Lcd_Write_strreg(1,Address,sanjicaidan7_3_5_2[disptr->LanguageType%maxLanguage]);
										

										


									
									
										
										Lcd_Write_strreg(1,22,mima1[SetDefault%7]);
										
											
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										
											Lcd_pos(1, 22+SetDefault);	
											
											Lcd_CmdWtite(1, 0x0d);
											
											break;
											
				case 6:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_6[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_6_1[disptr->LanguageType%maxLanguage]);
									
									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==2))
										{
									Lcd_Write_charreg(0, 44, 1);// ↑
									
									Lcd_Write_charreg(0, 46, 2);// ↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);// ↑
									
									Lcd_Write_charreg(0, 47, 2);// ↓
									
										}
									/*if(disptr->LanguageType%maxLanguage==0)
										{
										Address=9;

										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
										Address=10;

										}*/
									charlength=strlen((const char *)sanjicaidan7_3_6_2[disptr->LanguageType%maxLanguage]);

											Address=19-charlength;
									Lcd_Write_strreg(1,Address,sanjicaidan7_3_6_2[disptr->LanguageType%maxLanguage]);
									

									Lcd_Write_strreg(1,20,(const unsigned char	*)&sanjicaidan7_3_6_3[disptr->LanguageType%maxLanguage][disptr->DispGeneralSet.DisplayLanguage%maxLanguage]);
										
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].M==1))
										{

									
										/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);		
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
											

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											Lcd_pos(1, 20);	
											
											Lcd_CmdWtite(1, 0x0d);
											
											break;
				case 7:
					
									Lcd_ClearReg();
									if(disptr->Setlevel4<30)
										{
									Lcd_Write_strreg(0,0,sanjicaidan7_3_7[disptr->LanguageType%maxLanguage]);
										}
									
									
										if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].M==1))
										{

										/*
										if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+((40-charlength)/2);	
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
											

										}
										else{

											if(disptr->Setlevel4==1)
												{
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_5[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 40, 1);// ↑
									
									Lcd_Write_charreg(0, 42, 2);// ↓
									
									if(disptr->DispGeneralSet.ComTypeDisp<3)
										{
										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_6[disptr->LanguageType%maxLanguage]);		
														
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_7[disptr->LanguageType%maxLanguage]);

										Lcd_Write_strreg(1, 61, sanjicaidan7_3_7_8[disptr->LanguageType%maxLanguage]);
										}
									else if(disptr->DispGeneralSet.ComTypeDisp==3)
										{
										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_7[disptr->LanguageType%maxLanguage]);

										Lcd_Write_strreg(1, 21, sanjicaidan7_3_7_8[disptr->LanguageType%maxLanguage]);

										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12[disptr->LanguageType%maxLanguage]);

										



										}
									else if(disptr->DispGeneralSet.ComTypeDisp==4)
										{
											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_WF[disptr->LanguageType%maxLanguage]);



										}
									else if(disptr->DispGeneralSet.ComTypeDisp==5)
										{

											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_GSM[disptr->LanguageType%maxLanguage]);


												



										}
									else if(disptr->DispGeneralSet.ComTypeDisp==6)
										{

											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_GSM[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_network[disptr->LanguageType%maxLanguage]);


												



										}
									
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
										Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
										Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
										Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if(disptr->DispGeneralSet.ComTypeDisp==0)
												{
											Lcd_pos(1, 0);	
											
											Lcd_CmdWtite(1, 0x0d);
											

												}
											else if(disptr->DispGeneralSet.ComTypeDisp==1)
												{
											Lcd_pos(1, 40);	
											
											Lcd_CmdWtite(1, 0x0d);
											
												}
											else if(disptr->DispGeneralSet.ComTypeDisp==2)
												{
											Lcd_pos(1, 61);	
											
											Lcd_CmdWtite(1, 0x0d);
											
												}
											else if((disptr->DispGeneralSet.ComTypeDisp==3)||(disptr->DispGeneralSet.ComTypeDisp==4)||(disptr->DispGeneralSet.ComTypeDisp==5)||(disptr->DispGeneralSet.ComTypeDisp==6))
												{
											Lcd_pos(1, 40);	
											
											Lcd_CmdWtite(1, 0x0d);
											
												}
												}


											
									if(disptr->Setlevel4==2)
										{
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_6[disptr->LanguageType%maxLanguage]);	
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								//	Lcd_Write_strreg(1,40,sanjicaidan7_3_7_2);
									
										}
									else if(disptr->Setlevel4==3)
										{
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_6[disptr->LanguageType%maxLanguage]);	
									Lcd_Write_strreg(1,40,sanjicaidan7_3_7_2[disptr->LanguageType%maxLanguage]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										


										}
									else if(disptr->Setlevel4==4)
										{
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_6[disptr->LanguageType%maxLanguage]);	
									Lcd_Write_strreg(1,40,sanjicaidan7_3_7_3[disptr->LanguageType%maxLanguage]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								
										


										}
									else if(disptr->Setlevel4==10)
										{
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 40, 1);// ↑
									
									Lcd_Write_charreg(0, 42, 2);//
									Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_7[disptr->LanguageType%maxLanguage]);	
									charlength=strlen((const char *)sanjicaidan7_3_7_10[disptr->LanguageType%maxLanguage]);
									Address=40+27-charlength;
									Lcd_Write_strreg(1,Address,sanjicaidan7_3_7_10[disptr->LanguageType%maxLanguage]);

									Lcd_Write_charreg(1,68,disptr->DispGeneralSet.DispRepeaterNo%100+0x30);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

											Lcd_pos(1, 68);	
											
											Lcd_CmdWtite(1, 0x0d);
											
										}
									else if(disptr->Setlevel4==20){
										Lcd_Write_strreg(0,40,sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);
									
										Lcd_Write_charreg(0, 40, 1);// ↑
									
										Lcd_Write_charreg(0, 42, 2);//
									
										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_8[disptr->LanguageType%maxLanguage]);	
										if(disptr->LanguageType%maxLanguage==0)
												{
										Lcd_Write_strreg(1,40,sanjicaidan7_3_7_11[disptr->LanguageType%maxLanguage]);
											}
										else if(disptr->LanguageType%maxLanguage==1)
												{
												Lcd_Write_strreg(1,45,sanjicaidan7_3_7_11[disptr->LanguageType%maxLanguage]);
												}
										else if(disptr->LanguageType%maxLanguage==2)
												{
												Lcd_Write_strreg(1,43,sanjicaidan7_3_7_11[disptr->LanguageType%maxLanguage]);
												}
										/*
											if(disptr->LanguageType%maxLanguage==0)
												{

												Lcd_Write_strreg(1, 13, sanjicaidan7_3_7_11[disptr->LanguageType%maxLanguage]);	



												}
											else if(disptr->LanguageType%maxLanguage==1)
												{

												Lcd_Write_str(1, 13, sanjicaidan7_3_7_11[disptr->LanguageType%maxLanguage]);	



												}*/
											Lcd_Write_charreg(1,68,disptr->DispGeneralSet.DispExpansionNo%100+0x30);
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

											Lcd_pos(1, 68);	
											
											Lcd_CmdWtite(1, 0x0d);

										}
									else if(disptr->Setlevel4==30)
										{

										

										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);

										Lcd_Write_charreg(0, 40, 1);
										Lcd_Write_charreg(0, 42, 2);

										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_15[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_16[disptr->LanguageType%maxLanguage]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

										if(WireLessData.SelectCase==0)
											{

											Lcd_pos(1, 0);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else{

											Lcd_pos(1,40);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}








										}
									else if(disptr->Setlevel4==31)
										{
											Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_15[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);

											Lcd_Write_charreg(0, 40, 1);
											Lcd_Write_charreg(0, 42, 2);

											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_18[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_19[disptr->LanguageType%maxLanguage]);
											//Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_20[disptr->LanguageType%maxLanguage]);
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if(WireLessData.SelectCase1==0)
											{

											Lcd_pos(1, 0);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(WireLessData.SelectCase1==1){

											Lcd_pos(1,40);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										/*
										else if(WireLessData.SelectCase1==2){

											Lcd_pos(1,40);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}*/

											
			





										}
									else if(disptr->Setlevel4==32)
										{
											Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_16[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);

											Lcd_Write_charreg(0, 40, 1);
											Lcd_Write_charreg(0, 42, 2);

											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_21[disptr->LanguageType%maxLanguage]);
											Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_22[disptr->LanguageType%maxLanguage]);
											//Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_23[disptr->LanguageType%maxLanguage]);
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if(WireLessData.SelectCase1==0)
											{

											Lcd_pos(1, 0);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(WireLessData.SelectCase1==1){

											Lcd_pos(1,40);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										/*
										else if(WireLessData.SelectCase1==2){

											Lcd_pos(1,40);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}*/

											
			





										}
									else if(disptr->Setlevel4==33)
										{


												Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_18[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_24[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_25[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_26[disptr->LanguageType%maxLanguage]);
												Datatemp=WireLessData.Version/100;
												Datatempint=WireLessData.Version%100;

												Lcd_Write_charreg(1, 15,Datatemp+0x30);

												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;

												Lcd_Write_charreg(1, 16,Datatemp+0x30);

												Datatemp=Datatempint;

												Lcd_Write_charreg(1, 17,Datatemp+0x30);


												Datatemp=WireLessData.ReadReturnType&0x0f;

											//	Lcd_Write_char(1, 24,Datatemp+0x30);
												if(Datatemp<7)
													{

													Lcd_Write_strreg(1, 24, WirelessType[disptr->LanguageType%maxLanguage][Datatemp]);



													}
												else if((Datatemp==0x0e)||(Datatemp==0x0f))
													{

														Lcd_Write_strreg(1, 24, WirelessType[disptr->LanguageType%maxLanguage][Datatemp-7]);

													}

												Datatemp=WireLessData.Analogvalue/100;
												Datatempint=WireLessData.Analogvalue%100;

												Lcd_Write_charreg(1, 53,Datatemp+0x30);

												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;

												Lcd_Write_charreg(1, 54,Datatemp+0x30);

												Datatemp=Datatempint;

												Lcd_Write_charreg(1, 55,Datatemp+0x30);


												if((WireLessData.Analogvalue&0x02)&&(WireLessData.Analogvalue<8))
													{


															Lcd_Write_strreg(1, 71,sanjicaidan7_3_7_27[disptr->LanguageType%maxLanguage][1]);




													}
												else{

															Lcd_Write_strreg(1, 71,sanjicaidan7_3_7_27[disptr->LanguageType%maxLanguage][0]);

													}
													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);



												
												
												






										}
										else if(disptr->Setlevel4==34)
										{


												Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_19[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_13[disptr->LanguageType%maxLanguage]);
											
											//	Lcd_Write_strreg(0, 34, sanjicaidan7_3_7_33[disptr->LanguageType%maxLanguage][0]);
												Lcd_Write_strreg(1, 00, sanjicaidan7_3_7_14_MW[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_14_MR[disptr->LanguageType%maxLanguage]);

											//	Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_28[disptr->LanguageType%maxLanguage]);
											//	Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_29[disptr->LanguageType%maxLanguage]);

												//Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_28_M[disptr->LanguageType%maxLanguage]);
												//Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_29_M[disptr->LanguageType%maxLanguage]);
												
												Lcd_Write_charreg(1, 6,WireLessData.Address[0]+0x30);
												Lcd_Write_charreg(1, 7,WireLessData.Address[1]+0x30);
												Lcd_Write_charreg(1, 8,WireLessData.Address[2]+0x30);

												Lcd_Write_charreg(1, 14,WireLessData.NetId[0]+0x30);
												Lcd_Write_charreg(1, 15,WireLessData.NetId[1]+0x30);
												Lcd_Write_charreg(1, 16,WireLessData.NetId[2]+0x30);

												Lcd_Write_charreg(1, 21,WireLessData.wirelessChannel[0]+0x30);
												Lcd_Write_charreg(1, 22,WireLessData.wirelessChannel[1]+0x30);
												Lcd_Write_charreg(1, 23,WireLessData.wirelessChannel[2]+0x30);
												Lcd_Write_strreg(1, 31, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][(WireLessData.Writeinfo.Bit.WDSendinfo+1)%2]);
												//Lcd_Write_charreg(1, 38, WireLessData.Writeinfo.Bit.BeepOnOoff+0x30);
												//Lcd_Write_strreg(1, 24, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][(WireLessData.Writeinfo.Bit.BeepOnOoff+1)%2]);
												//Lcd_Write_strreg(1, 31, sanjicaidan7_3_7_30_M[disptr->LanguageType%maxLanguage][(WireLessData.Writeinfo.Bit.WDSendinfo+1)%2]);
												if((WireLessData.Readinfo.Bit.channel>=80)&&(WireLessData.Readinfo.Bit.channel<=103))
													{
												Datatempint=WireLessData.Readinfo.Bit.channel-79;///4;
													}
												else{

												Datatempint=WireLessData.Readinfo.Bit.channel;
													}
											//	if(Datatempint>24)
												//	{
												//				Datatempint=24;

												//	}
												Datatemp=Datatempint/100;
												Datatempint=Datatempint%100;



												Lcd_Write_charreg(1, 61,Datatemp+0x30);
												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;



												Lcd_Write_charreg(1, 62,Datatemp+0x30);
												Datatemp=Datatempint;
												Lcd_Write_charreg(1, 63,Datatemp+0x30);



												Lcd_Write_strreg(1, 71, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][(WireLessData.Readinfo.Bit.WDSendinfo+1)%2]);
												Lcd_Write_charreg(1, 78, WireLessData.Readinfo.Bit.BeepOnOoff+0x30+1);
												//Lcd_Write_strreg(1, 64, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][(WireLessData.Readinfo.Bit.BeepOnOoff+1)%2]);
												//Lcd_Write_strreg(1, 71, sanjicaidan7_3_7_30_M[disptr->LanguageType%maxLanguage][(WireLessData.Readinfo.Bit.WDSendinfo+1)%2]);


												Datatempint=WireLessData.ReadAdd;
										Datatempint=Datatempint%1000;
										Datatemp=Datatempint/100;
										Datatempint=Datatempint%100;

										Lcd_Write_charreg(1, 46, Datatemp+0x30);

										Datatemp=Datatempint/10;
										Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 47, Datatemp+0x30);

										Datatemp=Datatempint;
									//	Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 48, Datatemp+0x30);

										Datatempint=WireLessData.ReadNetId;
										
										Datatemp=Datatempint/100;
										Datatempint=Datatempint%100;

										Lcd_Write_charreg(1, 54, Datatemp+0x30);

										Datatemp=Datatempint/10;
										Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 55, Datatemp+0x30);

										Datatemp=Datatempint;
										

										Lcd_Write_charreg(1, 56, Datatemp+0x30);

										
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

									

										if(WireLessData.keyCase<3)
											{

											Lcd_pos(1, 6+WireLessData.keyCase);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(WireLessData.keyCase<6){

											Lcd_pos(1, 14+WireLessData.keyCase-3);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										else if(WireLessData.keyCase<9)
											{
												Lcd_pos(1, 21+WireLessData.keyCase-6);	
											
												Lcd_CmdWtite(1, 0x0d);


											}

										else if(WireLessData.keyCase==10)
											{
												Lcd_pos(1, 31);	
											
												Lcd_CmdWtite(1, 0x0d);


											}
										/*
										else if(WireLessData.keyCase==11)
											{
												Lcd_pos(1, 38);	
											
												Lcd_CmdWtite(1, 0x0d);


											}*/
										/*

											else if(WireLessData.keyCase==9)
											{
												Lcd_pos(1, 24);	
											
												Lcd_CmdWtite(1, 0x0d);


											}

											else if(WireLessData.keyCase==10)
											{
												Lcd_pos(1, 31);	
											
												Lcd_CmdWtite(1, 0x0d);


											}*/
												
												

												
												
												






										}
										/*
										else if(disptr->Setlevel4==35)
										{
												Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_20[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_13[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_28[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_29[disptr->LanguageType%maxLanguage]);


												Lcd_Write_charreg(1, 7,WireLessData.wirelessChannel[0]+0x30);
												Lcd_Write_charreg(1, 8,WireLessData.wirelessChannel[1]+0x30);



												Lcd_Write_strreg(1, 17, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.LbSendinfo]);
												Lcd_Write_strreg(1, 27, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.WDSendinfo]);
												Lcd_Write_strreg(1, 36, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.BeepOnOoff]);
												Datatempint=WireLessData.Readinfo.Bit.channel;
												if(Datatempint>24)
													{
																Datatempint=24;

													}
												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;



												Lcd_Write_charreg(1, 47,Datatemp+0x30);
												Datatemp=Datatempint;
												Lcd_Write_charreg(1, 48,Datatemp+0x30);



												Lcd_Write_strreg(1, 57, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.LbSendinfo]);
												Lcd_Write_strreg(1, 67, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.WDSendinfo]);
												Lcd_Write_strreg(1, 76, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.BeepOnOoff]);
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);



															if(WireLessData.keyCase<2)
											{

											Lcd_pos(1, 7+WireLessData.keyCase);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(WireLessData.keyCase==2){

											Lcd_pos(1, 17);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										else if(WireLessData.keyCase==3){

											Lcd_pos(1, 27);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										else if(WireLessData.keyCase==4){

											Lcd_pos(1, 36);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
												



												
											
											


												


										}*/

										else if(disptr->Setlevel4==36)
										{


												Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_21[disptr->LanguageType%maxLanguage]);
												
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_24[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 63, sanjicaidan7_3_7_34[disptr->LanguageType%maxLanguage]);//23  原来61
												Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_31[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_32[disptr->LanguageType%maxLanguage]);
												Datatemp=WireLessData.RssiLevel/100;
												Datatempint=WireLessData.RssiLevel%100;

												Lcd_Write_charreg(0, 68,Datatemp+0x30);

												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;

												Lcd_Write_charreg(0, 69,Datatemp+0x30);

												Datatemp=Datatempint;

												Lcd_Write_charreg(0,70,Datatemp+0x30);




												
												Datatemp=WireLessData.Version/100;
												Datatempint=WireLessData.Version%100;

												Lcd_Write_charreg(1, 15,Datatemp+0x30);

												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;

												Lcd_Write_charreg(1, 16,Datatemp+0x30);

												Datatemp=Datatempint;

												Lcd_Write_charreg(1, 17,Datatemp+0x30);


												Datatemp=WireLessData.ReadReturnType&0x0f;
												if(Datatemp<7)
													{

													Lcd_Write_strreg(1, 24, WirelessType[disptr->LanguageType%maxLanguage][Datatemp]);



													}
												else if((Datatemp==0x0e)||(Datatemp==0x0f))
													{

														Lcd_Write_strreg(1, 24, WirelessType[disptr->LanguageType%maxLanguage][Datatemp-7]);

													}


												//Lcd_Write_char(1, 24,Datatemp+0x30);

												Datatemp=WireLessData.Analogvalue/100;
												Datatempint=WireLessData.Analogvalue%100;

												Lcd_Write_charreg(1, 53,Datatemp+0x30);

												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;

												Lcd_Write_charreg(1, 54,Datatemp+0x30);

												Datatemp=Datatempint;

												Lcd_Write_charreg(1, 55,Datatemp+0x30);
												


												if((WireLessData.Analogvalue&0x02)&&(WireLessData.Analogvalue<8))
													{


															Lcd_Write_strreg(1, 71,sanjicaidan7_3_7_27[disptr->LanguageType%maxLanguage][1]);




													}
													
												else{

															Lcd_Write_strreg(1, 71,sanjicaidan7_3_7_27[disptr->LanguageType%maxLanguage][0]);

													}



													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
												
												






										}
										else if(disptr->Setlevel4==37)
										{


												Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_22[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_13[disptr->LanguageType%maxLanguage]);
												//Lcd_Write_strreg(0, 34, sanjicaidan7_3_7_33[disptr->LanguageType%maxLanguage][1]);
												Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_14_DW[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_14_DR[disptr->LanguageType%maxLanguage]);

												//Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_28[disptr->LanguageType%maxLanguage]);
											//	Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_29[disptr->LanguageType%maxLanguage]);
												
												Lcd_Write_charreg(1, 6,WireLessData.Address[0]+0x30);
												Lcd_Write_charreg(1, 7,WireLessData.Address[1]+0x30);
												Lcd_Write_charreg(1, 8,WireLessData.Address[2]+0x30);

												Lcd_Write_charreg(1, 14,WireLessData.NetId[0]+0x30);
												Lcd_Write_charreg(1, 15,WireLessData.NetId[1]+0x30);
												Lcd_Write_charreg(1, 16,WireLessData.NetId[2]+0x30);

												Lcd_Write_charreg(1, 21,WireLessData.wirelessChannel[0]+0x30);
												Lcd_Write_charreg(1, 22,WireLessData.wirelessChannel[1]+0x30);
												Lcd_Write_charreg(1, 23,WireLessData.wirelessChannel[2]+0x30);

												Lcd_Write_strreg(1, 28, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.LbSendinfo]);
												Lcd_Write_strreg(1, 34, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][(WireLessData.Writeinfo.Bit.BeepOnOoff+1)%2]);
												Lcd_Write_strreg(1, 39, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.WDSendinfo]);
													if((WireLessData.Readinfo.Bit.channel>=80)&&(WireLessData.Readinfo.Bit.channel<=103))
													{
												Datatempint=WireLessData.Readinfo.Bit.channel-79;///4;
													}
												else{

												Datatempint=WireLessData.Readinfo.Bit.channel;
													}
												//if(Datatempint>24)
												//	{
												//				Datatempint=24;

												//	}
												Datatemp=Datatempint/100;
												Datatempint=Datatempint%100;



												Lcd_Write_charreg(1, 61,Datatemp+0x30);
												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;
												Lcd_Write_charreg(1, 62,Datatemp+0x30);
												Datatemp=Datatempint;
												Lcd_Write_charreg(1, 63,Datatemp+0x30);



												Lcd_Write_strreg(1, 68, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.LbSendinfo]);
												Lcd_Write_strreg(1, 74, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][(WireLessData.Readinfo.Bit.BeepOnOoff+1)%2]);
												Lcd_Write_strreg(1, 79, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.WDSendinfo]);


												Datatempint=WireLessData.ReadAdd;
										Datatempint=Datatempint%1000;
										Datatemp=Datatempint/100;
										Datatempint=Datatempint%100;

										Lcd_Write_charreg(1, 46, Datatemp+0x30);

										Datatemp=Datatempint/10;
										Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 47, Datatemp+0x30);

										Datatemp=Datatempint;
									//	Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 48, Datatemp+0x30);

										Datatempint=WireLessData.ReadNetId;
										
										Datatemp=Datatempint/100;
										Datatempint=Datatempint%100;

										Lcd_Write_charreg(1, 54, Datatemp+0x30);

										Datatemp=Datatempint/10;
										Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 55, Datatemp+0x30);

										Datatemp=Datatempint;
										

										Lcd_Write_charreg(1, 56, Datatemp+0x30);

										
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

									

										if(WireLessData.keyCase<3)
											{

											Lcd_pos(1, 6+WireLessData.keyCase);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(WireLessData.keyCase<6){

											Lcd_pos(1, 14+WireLessData.keyCase-3);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										else if(WireLessData.keyCase<9)
											{
												Lcd_pos(1, 21+WireLessData.keyCase-6);	
											
												Lcd_CmdWtite(1, 0x0d);


											}

										else if(WireLessData.keyCase==9)
											{
												Lcd_pos(1, 28);	
											
												Lcd_CmdWtite(1, 0x0d);


											}

											else if(WireLessData.keyCase==10)
											{
												Lcd_pos(1, 34);	
											
												Lcd_CmdWtite(1, 0x0d);


											}
										else if(WireLessData.keyCase==11)
											{
												Lcd_pos(1, 39);	
											
												Lcd_CmdWtite(1, 0x0d);


											}		
												
												
												






										}
										/*
										else if(disptr->Setlevel4==38)
										{
												Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_23[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_13[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_28[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_29[disptr->LanguageType%maxLanguage]);


												Lcd_Write_charreg(1, 7,WireLessData.wirelessChannel[0]+0x30);
												Lcd_Write_charreg(1, 8,WireLessData.wirelessChannel[1]+0x30);



												Lcd_Write_strreg(1, 17, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.LbSendinfo]);
												Lcd_Write_strreg(1, 27, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.WDSendinfo]);
												Lcd_Write_strreg(1, 36, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Writeinfo.Bit.BeepOnOoff]);
												Datatempint=WireLessData.Readinfo.Bit.channel;
												if(Datatempint>24)
													{
																Datatempint=24;

													}
												Datatemp=Datatempint/10;
												Datatempint=Datatempint%10;



												Lcd_Write_charreg(1, 47,Datatemp+0x30);
												Datatemp=Datatempint;
												Lcd_Write_charreg(1, 48,Datatemp+0x30);



												Lcd_Write_strreg(1, 57, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.LbSendinfo]);
												Lcd_Write_strreg(1, 67, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.WDSendinfo]);
												Lcd_Write_strreg(1, 76, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][WireLessData.Readinfo.Bit.BeepOnOoff]);
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);



															if(WireLessData.keyCase<2)
											{

											Lcd_pos(1, 7+WireLessData.keyCase);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(WireLessData.keyCase==2){

											Lcd_pos(1, 17);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										else if(WireLessData.keyCase==3){

											Lcd_pos(1, 27);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
										else if(WireLessData.keyCase==4){

											Lcd_pos(1, 36);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}
												



												
											
											


												


										}*/
								/*	else if(disptr->Setlevel4==31)
										{

										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_15[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_13[disptr->LanguageType%maxLanguage]);





										}*/
									
									/*else if(disptr->Setlevel4==30)
										{
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_12[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_13[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_14[disptr->LanguageType%maxLanguage]);

										Lcd_Write_charreg(1, 46,WireLessData.Address[0]+0x30);
										Lcd_Write_charreg(1, 47,WireLessData.Address[1]+0x30);
										Lcd_Write_charreg(1, 48,WireLessData.Address[2]+0x30);

										Lcd_Write_charreg(1, 55,WireLessData.NetId[0]+0x30);
										Lcd_Write_charreg(1, 56,WireLessData.NetId[1]+0x30);
										Lcd_Write_charreg(1, 57,WireLessData.NetId[2]+0x30);
										Lcd_Write_charreg(1, 58,WireLessData.NetId[3]+0x30);
										Lcd_Write_charreg(1, 59,WireLessData.NetId[4]+0x30);


										




										Datatempint=WireLessData.ReadAdd;
										Datatempint=Datatempint%1000;
										Datatemp=Datatempint/100;
										Datatempint=Datatempint%100;

										Lcd_Write_charreg(1, 66, Datatemp+0x30);

										Datatemp=Datatempint/10;
										Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 67, Datatemp+0x30);

										Datatemp=Datatempint;
									//	Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 68, Datatemp+0x30);

										Datatempint=WireLessData.ReadNetId;
										
										Datatemp=Datatempint/10000;
										Datatempint=Datatempint%10000;

										Lcd_Write_charreg(1, 75, Datatemp+0x30);

										Datatemp=Datatempint/1000;
										Datatempint=Datatempint%1000;

										Lcd_Write_charreg(1, 76, Datatemp+0x30);

										Datatemp=Datatempint/100;
										Datatempint=Datatempint%100;

										Lcd_Write_charreg(1, 77, Datatemp+0x30);

										Datatemp=Datatempint/10;
										Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 78, Datatemp+0x30);

										Datatemp=Datatempint;
										//Datatempint=Datatempint%10;

										Lcd_Write_charreg(1, 79, Datatemp+0x30);

										if(WireLessData.keyCase<3)
											{

											Lcd_pos(1, 46+WireLessData.keyCase);	
											
											Lcd_CmdWtite(1, 0x0d);

											}
										else{

											Lcd_pos(1, 52+WireLessData.keyCase);	
											
											Lcd_CmdWtite(1, 0x0d);			


											}



										}*/
									else if(disptr->Setlevel4==40)
										{
											Lcd_ClearReg();
											Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(0, 40,sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);
											Lcd_Write_charreg(0, 40, 1);// ↑
									
											Lcd_Write_charreg(0, 42, 2);//
											if(ESP8266Status.SetWifiInterface.SelectCase<2)
												{

											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_51[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(1, 40,sanjicaidan7_3_7_12_WF_52[disptr->LanguageType%maxLanguage]);
												}
											else if(ESP8266Status.SetWifiInterface.SelectCase==2){
											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_52[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(1, 40,sanjicaidan7_3_7_12_WF_53[disptr->LanguageType%maxLanguage]);


												}
												else if(ESP8266Status.SetWifiInterface.SelectCase==3){
											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_53[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(1, 40,sanjicaidan7_3_7_12_WF_54[disptr->LanguageType%maxLanguage]);
												}
												else if(ESP8266Status.SetWifiInterface.SelectCase==4){
											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_54[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(1, 40,sanjicaidan7_3_7_12_WF_55[disptr->LanguageType%maxLanguage]);


												}
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

											if(ESP8266Status.SetWifiInterface.SelectCase==0)
												{
												Lcd_pos(1, 0);	
											
												


												}
											else if((ESP8266Status.SetWifiInterface.SelectCase==1)||(ESP8266Status.SetWifiInterface.SelectCase==2)||(ESP8266Status.SetWifiInterface.SelectCase==3)||(ESP8266Status.SetWifiInterface.SelectCase==4))
												{
												Lcd_pos(1, 40);	
											
												


												}
											

											Lcd_CmdWtite(1, 0x0d);

										}
									else if(disptr->Setlevel4==41)
										{
											Lcd_ClearReg();
											Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_51[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(0, 40,sanjicaidan7_3_7_9[disptr->LanguageType%maxLanguage]);
											Lcd_Write_charreg(0, 40, 1);// ↑
									
											Lcd_Write_charreg(0, 42, 2);//

											Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_511[disptr->LanguageType%maxLanguage]);	

											Lcd_Write_strreg(1, 40,sanjicaidan7_3_7_12_WF_512[disptr->LanguageType%maxLanguage]);
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

											if(ESP8266Status.SetWifiInterface.SelectCase==0)
												{
												Lcd_pos(1, 0);	
											
												


												}
											else if(ESP8266Status.SetWifiInterface.SelectCase==1)
												{
												Lcd_pos(1, 40);	
											
												


												}

											Lcd_CmdWtite(1, 0x0d);



										}
									else if(disptr->Setlevel4==42)
										{
										Lcd_ClearReg();
										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_52[disptr->LanguageType%maxLanguage]);
									
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_12_WF_522[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_523[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.TcpConnectstatus[0]%3]);
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_WF_521[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.SelectCase%2]);


										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[0][0]+0X30;
										Lcd_Write_charreg(0, 4+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[0][1]+0X30;
										Lcd_Write_charreg(0, 5+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[0][2]+0X30;
										Lcd_Write_charreg(0, 6+40,Datatemp);

										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[1][0]+0X30;
										Lcd_Write_charreg(0, 8+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[1][1]+0X30;
										Lcd_Write_charreg(0, 9+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[1][2]+0X30;
										Lcd_Write_charreg(0, 10+40,Datatemp);

										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[2][0]+0X30;
										Lcd_Write_charreg(0, 12+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[2][1]+0X30;
										Lcd_Write_charreg(0, 13+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[2][2]+0X30;
										Lcd_Write_charreg(0, 14+40,Datatemp);

										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[3][0]+0X30;
										Lcd_Write_charreg(0, 16+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[3][1]+0X30;
										Lcd_Write_charreg(0, 17+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.ServerIPtemp[3][2]+0X30;
										Lcd_Write_charreg(0, 18+40,Datatemp);

										Datatemp=ESP8266Status.SetWifiInterface.Serverporttemp[0]+0X30;
										Lcd_Write_charreg(0, 28+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.Serverporttemp[1]+0X30;
										Lcd_Write_charreg(0, 29+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.Serverporttemp[2]+0X30;
										Lcd_Write_charreg(0, 30+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.Serverporttemp[3]+0X30;
										Lcd_Write_charreg(0, 31+40,Datatemp);
										Datatemp=ESP8266Status.SetWifiInterface.Serverporttemp[4]+0X30;
										Lcd_Write_charreg(0, 32+40,Datatemp);


										
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
											if(ESP8266Status.SetWifiInterface.TcpSetCase==0)
												{
												Lcd_pos(1, 52+2);	
												Lcd_CmdWtite(1, 0x0d);
												


												}
												else if(ESP8266Status.SetWifiInterface.TcpSetCase<4)
												{
												Lcd_pos(0, 40+4+ESP8266Status.SetWifiInterface.TcpSetCase-1);	
												Lcd_CmdWtite(0, 0x0d);
												


												}
												else if(ESP8266Status.SetWifiInterface.TcpSetCase<7)
												{
												Lcd_pos(0, 40+8+ESP8266Status.SetWifiInterface.TcpSetCase-4);	
												Lcd_CmdWtite(0, 0x0d);
												


												}
											else if(ESP8266Status.SetWifiInterface.TcpSetCase<10)
												{
												Lcd_pos(0, 40+12+ESP8266Status.SetWifiInterface.TcpSetCase-7);	
												Lcd_CmdWtite(0, 0x0d);
												


												}
											else if(ESP8266Status.SetWifiInterface.TcpSetCase<13)
												{
												Lcd_pos(0, 40+16+ESP8266Status.SetWifiInterface.TcpSetCase-10);	
												Lcd_CmdWtite(0, 0x0d);
												


												}
											else 
												{
												Lcd_pos(0, 40+28+ESP8266Status.SetWifiInterface.TcpSetCase-13);	
												Lcd_CmdWtite(0, 0x0d);
												


												}
											



										}
									else if(disptr->Setlevel4==43)
										{
										Lcd_ClearReg();

										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_511[disptr->LanguageType%maxLanguage]);
										charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_513[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.SelectCase%2]);

										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_12_WF_513[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.SelectCase%2]);
										Lcd_Write_strreg(1, 00, sanjicaidan7_3_7_12_WF_514[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_WF_515[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 9, sanjicaidan7_3_7_12_WF_516[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.connectStatus%3]);
										if(ESP8266Status.SetWifiInterface.connectStatus==1)
											{
													Lcd_Write_strreg(1, 48,&ESP8266Status.SetWifiInterface.ReadIpAdd[0]);

												

											}
										else{
												
												Lcd_Write_strreg(1, 48, sanjicaidan7_3_7_12_WF_517[disptr->LanguageType%maxLanguage]);




											}
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										Lcd_pos(0, 39+charlength);
										Lcd_CmdWtite(0, 0x0d);





										}
									else if(disptr->Setlevel4==44)
										{
										Lcd_ClearReg();

										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_512[disptr->LanguageType%maxLanguage]);
									
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_12_WF_519[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 00, sanjicaidan7_3_7_12_WF_51A[disptr->LanguageType%maxLanguage]);

										
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_WF_518[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.SelectCase%2]);
										Lcd_Write_strreg(1, 61, sanjicaidan7_3_7_12_WF_51B[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.connectStatus%3]);

										Lcd_Write_strreg(0,46,&ESP8266Status.SetWifiInterface.ssidnametemp[0]);
										charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_51A[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,charlength+1,&ESP8266Status.SetWifiInterface.passwordtemp[0]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
										Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
										Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
										Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

										if(ESP8266Status.SetWifiInterface.manualconnectCase==0)
											{

													if(ESP8266Status.SetWifiInterface.manualconnectinputcase==0)
														{
															charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_518[disptr->LanguageType%maxLanguage]);
															Lcd_pos(1, charlength+40-1);
															Lcd_CmdWtite(1, 0x0d);



														}
													else{

														charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_519[disptr->LanguageType%maxLanguage]);


															Lcd_pos(0, 40+charlength+ESP8266Status.SetWifiInterface.manualconnectinputcase);
															Lcd_CmdWtite(0, 0x0d);



														}


											}

										else{

															charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_51A[disptr->LanguageType%maxLanguage]);
															Lcd_pos(1, charlength+1+ESP8266Status.SetWifiInterface.manualconnectinputcase);
															Lcd_CmdWtite(1, 0x0d);




											}
										









										}
									 else if(disptr->Setlevel4==45)
										{
										Lcd_ClearReg();

										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_53[disptr->LanguageType%maxLanguage]);
										
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_12_WF_521[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.SelectCase%2]);

										Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_12_WF_523[disptr->LanguageType%maxLanguage][ESP8266Status.SetWifiInterface.TcpConnectstatus[1]%3]);

										charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_521[disptr->LanguageType%maxLanguage]);
										
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
										Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
										Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
										Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										Lcd_pos(0, 39+charlength);	
										Lcd_CmdWtite(0, 0x0d);


										}
									 else if(disptr->Setlevel4==47)
										{
										Lcd_ClearReg();

										Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_54[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_12_WF_531[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 00, sanjicaidan7_3_7_12_WF_532[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_12_WF_533[disptr->LanguageType%maxLanguage]);
										charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_532[disptr->LanguageType%maxLanguage]);


										Address=charlength+1;

										
										Lcd_Write_charreg(1, Address, ESP8266Status.SetPanelNo.boxnochar[0]+0x30);
										Address++;
										Lcd_Write_charreg(1, Address, ESP8266Status.SetPanelNo.boxnochar[1]+0x30);
										Address++;
										Lcd_Write_charreg(1, Address, ESP8266Status.SetPanelNo.boxnochar[2]+0x30);
										Address++;
										Lcd_Write_charreg(1, Address, ESP8266Status.SetPanelNo.boxnochar[3]+0x30);
										Address++;
										Lcd_Write_charreg(1, Address, ESP8266Status.SetPanelNo.boxnochar[4]+0x30);


										Datatemplong=ESP8266Status.PanelNetid;
										Datatemp=Datatemplong/1000000000;
										Datatemplong=Datatemplong%1000000000;
										Address=55;
										Lcd_Write_charreg(1, Address, Datatemp+0x30);
										
										Address++;
										Datatemp=Datatemplong/100000000;
										Datatemplong=Datatemplong%100000000;
										Lcd_Write_charreg(1, Address, Datatemp+0x30);
										
										Address++;
										Datatemp=Datatemplong/10000000;
										Datatemplong=Datatemplong%10000000;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);
										
										Address++;
										Datatemp=Datatemplong/1000000;
										Datatemplong=Datatemplong%1000000;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);

										Address++;
										Datatemp=Datatemplong/100000;
										Datatemplong=Datatemplong%100000;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);

										Address++;
										Datatemp=Datatemplong/10000;
										Datatemplong=Datatemplong%10000;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);

										Address++;
										Datatemp=Datatemplong/1000;
										Datatemplong=Datatemplong%1000;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);

										Address++;
										Datatemp=Datatemplong/100;
										Datatemplong=Datatemplong%100;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);

										Address++;
										Datatemp=Datatemplong/10;
										Datatemplong=Datatemplong%10;						
										Lcd_Write_charreg(1, Address, Datatemp+0x30);

										Address++;
										Datatemp=Datatemplong;						
										Lcd_Write_charreg(1, Address,Datatemp+0x30);




										
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
										Lcd_pos(1, charlength+1+ESP8266Status.SetWifiInterface.SelectCase);
										Lcd_CmdWtite(1, 0x0d);





										}
									 else if(disptr->Setlevel4==48)
									 	{

											Lcd_ClearReg();
											Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_12_WF_55[disptr->LanguageType%maxLanguage]);	
											Lcd_Write_strreg(0, 55, sanjicaidan7_3_7_12_WF_551[disptr->LanguageType%maxLanguage]);

											charlength=strlen((const char *) sanjicaidan7_3_7_12_WF_551[disptr->LanguageType%maxLanguage]);

											Datatempint=RandData&0xffff;
											Datatemp=Datatempint/10000;
											Datatempint=Datatempint%10000;
											Address=55+charlength+1;
											Lcd_Write_charreg(0, Address, Datatemp+0x30);//

											Datatemp=Datatempint/1000;
											Datatempint=Datatempint%1000;
											Address++;
											Lcd_Write_charreg(0, Address, Datatemp+0x30);//

											Datatemp=Datatempint/100;
											Datatempint=Datatempint%100;
											Address++;
											Lcd_Write_charreg(0, Address, Datatemp+0x30);//

											Datatemp=Datatempint/10;
											Datatempint=Datatempint%10;
											Address++;
											Lcd_Write_charreg(0, Address, Datatemp+0x30);//

											Datatemp=Datatempint;
											Address++;
											Lcd_Write_charreg(0, Address, Datatemp+0x30);//
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
											

									 	}
									 else if(disptr->Setlevel4==50)
									 	{
									 	Lcd_Write_strreg(0,0,sanjicaidan7_3_7_12_GSM[disptr->LanguageType%maxLanguage]);	
										Lcd_Write_strreg(0,40, sanjicaidan7_3_7_12_GSM_1[disptr->LanguageType%maxLanguage]);
										Lcd_Write_charreg(0, 40, 1);// ↑
									
										Lcd_Write_charreg(0, 42, 2);//
										if((GsmDataPar.SetGsmPar.setgsmcase==0)||(GsmDataPar.SetGsmPar.setgsmcase==1))
											{

										Lcd_Write_strreg(1,0,sanjicaidan7_3_7_35_GSM[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,40,sanjicaidan7_3_7_36_GSM[disptr->LanguageType%maxLanguage]);



											}
										else 	if(GsmDataPar.SetGsmPar.setgsmcase==2)
											{

										Lcd_Write_strreg(1,0,sanjicaidan7_3_7_36_GSM[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,40,sanjicaidan7_3_7_37_GSM[disptr->LanguageType%maxLanguage]);



											}
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
										if(GsmDataPar.SetGsmPar.setgsmcase==0)
											{


											Lcd_pos(1, 0);
											Lcd_CmdWtite(1, 0x0d);





											}
										else 
											{
												Lcd_pos(1, 40);
												Lcd_CmdWtite(1, 0x0d);



											}








									 	}
									 else if(disptr->Setlevel4==51)
									 	{
									 	Lcd_Write_strreg(0,0, sanjicaidan7_3_7_35_GSM[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(0,40, sanjicaidan7_3_7_35_GSM_1[disptr->LanguageType%maxLanguage]);
										Lcd_Write_charreg(0, 40, 1);// ↑
									
										Lcd_Write_charreg(0, 42, 2);//
										Lcd_Write_strreg(1,12, sanjicaidan7_3_7_35_GSM_2[disptr->LanguageType%maxLanguage]);
										charlength=strlen((const char *) sanjicaidan7_3_7_35_GSM_2[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,12+charlength+1, sanjicaidan7_3_7_35_GSM_3[disptr->LanguageType%maxLanguage][GsmDataPar.SetGsmPar.displayenable%2]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
										Lcd_pos(1, 12+charlength+1);
										Lcd_CmdWtite(1, 0x0d);
										




									 	}
										 else if(disptr->Setlevel4==52)
									 	{
									 	Lcd_Write_strreg(0,0, sanjicaidan7_3_7_36_GSM[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(0,40, sanjicaidan7_3_7_36_GSM_1[disptr->LanguageType%maxLanguage]);
										Lcd_Write_charreg(0, 42, 3);// ?
										Lcd_Write_charreg(0, 40, 5);// ?
										Lcd_Write_charreg(0, 57, 1);// ↑
									
										Lcd_Write_charreg(0, 59, 2);//
										Lcd_Write_strreg(1,0, sanjicaidan7_3_7_36_GSM_2[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,40, sanjicaidan7_3_7_36_GSM_3[disptr->LanguageType%maxLanguage]);

										Lcd_Write_strreg(1, 6, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][GsmDataPar.SetGsmPar.dispreporttype.Bit.fireenabled]);
										Lcd_Write_strreg(1, 29, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][GsmDataPar.SetGsmPar.dispreporttype.Bit.faultenabled]);
										Lcd_Write_strreg(1, 51, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][GsmDataPar.SetGsmPar.dispreporttype.Bit.prealalarmenabled]);
										Lcd_Write_strreg(1, 69, sanjicaidan7_3_7_30[disptr->LanguageType%maxLanguage][GsmDataPar.SetGsmPar.dispreporttype.Bit.testenabled]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
										if(GsmDataPar.SetGsmPar.setreportcase==0)
											{
											


											Lcd_pos(1, 6);
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(GsmDataPar.SetGsmPar.setreportcase==1)
											{
											


											Lcd_pos(1, 29);
											Lcd_CmdWtite(1, 0x0d);

											}
										else if(GsmDataPar.SetGsmPar.setreportcase==2)
											{
											


											Lcd_pos(1, 51);
											Lcd_CmdWtite(1, 0x0d);

											}
									else if(GsmDataPar.SetGsmPar.setreportcase==3)
											{
											


											Lcd_pos(1, 69);
											Lcd_CmdWtite(1, 0x0d);

											}
										




									 	}
										 	 else if(disptr->Setlevel4==53)
									 	{
									 	Lcd_Write_strreg(0,0, sanjicaidan7_3_7_37_GSM[disptr->LanguageType%maxLanguage]);
										if(GsmDataPar.SetGsmPar.SetPhoneNo<3)
											{
										Lcd_Write_charreg(0, 40, 0+0x30);	
										Lcd_Write_charreg(0, 41, 1+0x30);
										Lcd_Write_charreg(0, 42, '.');
										
										Lcd_Write_charreg(1, 0, 0+0x30);	
										Lcd_Write_charreg(1, 1, 2+0x30);
										Lcd_Write_charreg(1, 2, '.');
										
										Lcd_Write_charreg(1, 40, 0+0x30);
										Lcd_Write_charreg(1, 41, 3+0x30);
										Lcd_Write_charreg(1, 42, '.');
										GsmDataPar.SetGsmPar.DisplayStartPhone=0;
											}
										else{
											Datatempint=GsmDataPar.SetGsmPar.SetPhoneNo-1;
											Datatemp=Datatempint/10;
												
										Lcd_Write_charreg(0, 40, Datatemp+0x30);	
											Datatemp=Datatempint%10;
										Lcd_Write_charreg(0, 41, Datatemp+0x30);
										Lcd_Write_charreg(0, 42, '.');

										Datatempint=GsmDataPar.SetGsmPar.SetPhoneNo;
										Datatemp=Datatempint/10;

										
										Lcd_Write_charreg(1, 0, Datatemp+0x30);
										Datatemp=Datatempint%10;
										Lcd_Write_charreg(1, 1, Datatemp+0x30);
										Lcd_Write_charreg(1, 2, '.');

										Datatempint=GsmDataPar.SetGsmPar.SetPhoneNo+1;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 40, Datatemp+0x30);	
										Datatemp=Datatempint%10;
										Lcd_Write_charreg(1, 41, Datatemp+0x30);
										Lcd_Write_charreg(1, 42, '.');
										GsmDataPar.SetGsmPar.DisplayStartPhone=GsmDataPar.SetGsmPar.SetPhoneNo-2;



											}

										if((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][0]==0xaa)&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][1]==0x55))
											{

											for(i=0;i<28;i++)
												{
													if((i==0)&&((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]=='+')||
														((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]>='0')&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]<='9'))))
														{
														Lcd_Write_charreg(0, 43+i, GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]);
														


														}
													else if((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]>='0')&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]<='9'))
														{
														Lcd_Write_charreg(0, 43+i, GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone][2+i]);




														}
													else{
														break;


														}





												}





											}
										if((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][0]==0xaa)&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][1]==0x55))
											{

											for(i=0;i<28;i++)
												{
													if((i==0)&&((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]=='+')||
														((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]>='0')&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]<='9'))))
														{
														Lcd_Write_charreg(1, 3+i, GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]);
														


														}
													else if((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]>='0')&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]<='9'))
														{
														Lcd_Write_charreg(1, 3+i, GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+1][2+i]);



														}
													else{
														break;


														}





												}





											}
										

										
										
											if((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][0]==0xaa)&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][1]==0x55))
											{

											for(i=0;i<28;i++)
												{
													if((i==0)&&((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]=='+')||
														((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]>='0')&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]<='9'))))
														{
														Lcd_Write_charreg(1, 43+i, GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]);
														


														}
													else if((GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]>='0')&&(GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]<='9'))
														{
														Lcd_Write_charreg(1, 43+i, GsmDataPar.SetGsmPar.phoneNumdisplay[GsmDataPar.SetGsmPar.DisplayStartPhone+2][2+i]);



														}
													else{
														break;


														}





												}





											}
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
										if(GsmDataPar.SetGsmPar.SetPhoneNo==0)
											{



											Lcd_pos(0, 43+GsmDataPar.SetGsmPar.SetPhoneNoFiled);
											Lcd_CmdWtite(0, 0x0d);
											





											}
										else if(GsmDataPar.SetGsmPar.SetPhoneNo==1)
											{



											Lcd_pos(1, 3+GsmDataPar.SetGsmPar.SetPhoneNoFiled);
											Lcd_CmdWtite(1, 0x0d);
											





											}
									else if(GsmDataPar.SetGsmPar.SetPhoneNo>=2)
											{

											Lcd_pos(1, 43+GsmDataPar.SetGsmPar.SetPhoneNoFiled);
											Lcd_CmdWtite(1, 0x0d);
											





											}




									 	}
											 else if(disptr->Setlevel4==60)
											 	{
													Lcd_Write_strreg(0,0, sanjicaidan7_3_7_12_network[disptr->LanguageType%maxLanguage]);				
													Lcd_Write_strreg(0,40, sanjicaidan7_3_7_12_GSM_1[disptr->LanguageType%maxLanguage]);
													Lcd_Write_charreg(0, 40, 1);// ↑
									
													Lcd_Write_charreg(0, 42, 2);//
													Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_38_network[disptr->LanguageType%maxLanguage]);
													Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_38_network_1[disptr->LanguageType%maxLanguage]);

													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
													Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
													Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
													Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

													if(disptr->DispGeneralSet.Setnetworkcase==0)
														{

															Lcd_pos(1, 0);
															Lcd_CmdWtite(1, 0x0d);




														}
													else {
															Lcd_pos(1, 40);
															Lcd_CmdWtite(1, 0x0d);

														}




											 	}
											 else if(disptr->Setlevel4==61)
											 	{
											 	
													Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_38_network[disptr->LanguageType%maxLanguage]);
													Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_38_network_2[disptr->LanguageType%maxLanguage]);
													Lcd_Write_charreg(0, 40, 1);// ↑
									
													Lcd_Write_charreg(0, 42, 2);//

													Lcd_Write_charreg(0, 44, 3);//

													Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_38_network_3[disptr->LanguageType%maxLanguage]);
													charlength=strlen((const char *) sanjicaidan7_3_7_38_network_3[disptr->LanguageType%maxLanguage]);

													Datatempint=disptr->DispGeneralSet.Displaynetworkamout%(maxNetWorkpanel+1);

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Address=charlength+1;

													Lcd_Write_charreg(1, Address, Datatemp+0x30);
													Datatemp=Datatempint;
													Address++;
													Lcd_Write_charreg(1, Address, Datatemp+0x30);

													Lcd_Write_strreg(1, 40, sanjicaidan7_3_7_38_network_4[disptr->LanguageType%maxLanguage]);

													

													if(disptr->DispGeneralSet.cieloopassignedcase<=1)
														{
														Lcd_Write_charreg(1, 44, 0+0x30);
														Lcd_Write_charreg(1, 45, 1+0x30);
														charlength=strlen((const char *) sanjicaidan7_3_7_38_network_4[disptr->LanguageType%maxLanguage]);

														Lcd_Write_charreg(1, 40+charlength+1, disptr->DispGeneralSet.DisplayEverypanelLoop[0]+0x30);
														

														



														}
													else{
														Datatemp=disptr->DispGeneralSet.cieloopassignedcase+1;
														if(Datatemp<11)
															{
															Lcd_Write_charreg(1, 44, 0+0x30);

															Lcd_Write_charreg(1, 45, Datatemp-1+0x30);
															}
														else{
															Lcd_Write_charreg(1, 44, (Datatemp-1)/10+0x30);
															Lcd_Write_charreg(1, 45, (Datatemp-1)%10+0x30);



															}
														charlength=strlen((const char *) sanjicaidan7_3_7_38_network_4[disptr->LanguageType%maxLanguage]);
														Lcd_Write_charreg(1, 40+charlength+1, disptr->DispGeneralSet.DisplayEverypanelLoop[disptr->DispGeneralSet.cieloopassignedcase-1]+0x30);

														



														}
													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
													Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
													Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
													Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

													if(disptr->DispGeneralSet.cieloopassignedcase==0)
														{
														charlength=strlen((const char *) sanjicaidan7_3_7_38_network_3[disptr->LanguageType%maxLanguage]);
														Address=charlength+1;

															if(disptr->DispGeneralSet.networkAmoutCase==0)
																{

															Lcd_pos(1, Address);
																}
															else 
																{
															Address++;
															Lcd_pos(1, Address);
																}
															Lcd_CmdWtite(1, 0x0d);




														}
													else {
															charlength=strlen((const char *) sanjicaidan7_3_7_38_network_4[disptr->LanguageType%maxLanguage]);

															Lcd_pos(1, 40+charlength+1);
															Lcd_CmdWtite(1, 0x0d);

														}
													
															




														}
													
													

													





											 	
											 else if(disptr->Setlevel4==62)
											 	{
											 	Lcd_Write_strreg(0, 0, sanjicaidan7_3_7_38_network_1[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(0, 40, sanjicaidan7_3_7_38_network_5[disptr->LanguageType%maxLanguage]);
												Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_38_network_6[disptr->LanguageType%maxLanguage]);

												charlength=strlen((const char *) sanjicaidan7_3_7_38_network_6[disptr->LanguageType%maxLanguage]);
												Address=charlength+1;
												Lcd_Write_charreg(1, Address, (disptr->DispGeneralSet.DisplayThisPanleNo)/10+0X30);
												Address++;
												Lcd_Write_charreg(1, Address, (disptr->DispGeneralSet.DisplayThisPanleNo)%10+0X30);
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
													Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
													Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
													Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
													if(disptr->DispGeneralSet.panelNoCase==0)
														{
													Lcd_pos(1, charlength+1);
														}
													else
														{
														Lcd_pos(1, charlength+2);
														}
													Lcd_CmdWtite(1, 0x0d);




											 	}

									else if(disptr->Setlevel4==200){
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_1[disptr->LanguageType%maxLanguage]);	
									Lcd_Write_strreg(1, 0, sanjicaidan7_3_7_6[disptr->LanguageType%maxLanguage]);	
									Lcd_Write_strreg(1,40,sanjicaidan7_3_7_4[disptr->LanguageType%maxLanguage]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									


										}
											}

									break;
				case 8:
					
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_8[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_1[disptr->LanguageType%maxLanguage]);

									Lcd_Write_strreg(1,0,sanjicaidan7_3_8_2);
									charlength=strlen((const char *) erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].O==1))
										{
/*
									
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
											

										}
											else{
													if(disptr->Setlevel4<2)
														{
																	Lcd_Write_strreg(1,12,sanjicaidan7_3_9_1);
														
													Datatempint=CustomerCheckSumallLast;
													Datatemp=Datatempint/10000;
													Datatempint=Datatempint%10000;
													Lcd_Write_charreg(1, 23, Datatemp+0x30);
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 24, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 25, Datatemp+0x30);
													

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 26, Datatemp+0x30);
													

													Datatemp=Datatempint;
													//Datatempint=Datatempint%10000;
													Lcd_Write_charreg(1, 27, Datatemp+0x30);


														}

													else if(disptr->Setlevel4==2)
													{

														Lcd_Write_strreg(1,14,checksumlable[disptr->LanguageType%maxLanguage]);

													}

												else if(disptr->Setlevel4==3)
													{
														/*
													if(checksumflag.Bit.ToChecksumCustomer)
														{
													
														checksumflag.Bit.ToChecksumCustomer=0;
														checksumflag.Bit.ChecksumCustomer=1;
														Lcd_Write_strreg(1,0,sanjicaidan7_3_8_1[disptr->LanguageType%maxLanguage]);
														
														for(CalcCustsettingcase=0;CalcCustsettingcase<4096;CalcCustsettingcase++)
															{
															MX_IWDG_CLEAR();
															calculateSettingchecksum(CalcCustsettingcase);

															}

														checksumflag.Bit.ChecksumCustomer=0;
														CalcCustsettingcase=0;
													
														checksumt=CustomerCheckSum;
														Eeprom[0]=getd0(CustomerCheckSum);
														Eeprom[1]=getd1(CustomerCheckSum);
														Eeprom[2]=0xaa;
														Eeprom[3]=0x55;
														Eeprom[4]=0x55;
														Eeprom[5]=0xaa;
														Write_EE1024(1,Eeprom, 60490, 6);
														CustomerCheckSumLast=CustomerCheckSum;
														
														}*/
													Lcd_Write_strreg(1,0,sanjicaidan7_3_8_2);
													
													Lcd_Write_strreg(1,12,sanjicaidan7_3_9_1);
														
													Datatempint=CustomerCheckSumallLast;
													Datatemp=Datatempint/10000;
													Datatempint=Datatempint%10000;
													Lcd_Write_charreg(1, 23, Datatemp+0x30);
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 24, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 25, Datatemp+0x30);
													

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 26, Datatemp+0x30);
													

													Datatemp=Datatempint;
													//Datatempint=Datatempint%10000;
													Lcd_Write_charreg(1, 27, Datatemp+0x30);
													
													
													
													}



											}
								       Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
									break;
				case 9:
							
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,sanjicaidan7_3_9[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(0,40,sanjicaidan7_3_7_1[disptr->LanguageType%maxLanguage]);

								//	
														
												
									
										if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[6].O==1))
										{
										charlength=strlen((const char *) erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;

										Lcd_Write_strreg(1,Address,erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);//read only access
											

										}
										else{
											if(disptr->Setlevel4<2)
												{
													Lcd_Write_strreg(1,12,sanjicaidan7_3_9_1);
													Datatempint=PROCHECKSUMallLast;
													Datatemp=Datatempint/10000;
													Datatempint=Datatempint%10000;
													Lcd_Write_charreg(1, 23, Datatemp+0x30);
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 24, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 25, Datatemp+0x30);
													

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 26, Datatemp+0x30);
													

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1, 27, Datatemp+0x30);
													





												}

												else if(disptr->Setlevel4==2)
													{

														Lcd_Write_strreg(1,14,checksumlable[disptr->LanguageType%maxLanguage]);

													}
												else if(disptr->Setlevel4==3)
													{
												/*
													if(checksumflag.Bit.ToChecksum)
														{
														checksumflag.Bit.ToChecksum=0;
														
														data=(uint8_t*)(APPLICATION_ADDRESS);
														prochecksumtemp=0;
														for(i=0;i<USER_FLASH_SIZE;i++)
														{

																prochecksumtemp+=*(data+i);



														}	
														PROCHECKSUM=prochecksumtemp;
														checksumt=PROCHECKSUM;
														Eeprom[0]=getd0(PROCHECKSUM);
														Eeprom[1]=getd1(PROCHECKSUM);
														Eeprom[2]=0xaa;
														Eeprom[3]=0x55;
														Eeprom[4]=0x55;
														Eeprom[5]=0xaa;
														Write_EE1024(1,Eeprom,60496 , 6);
														PROCHECKSUMLast=PROCHECKSUM;
														
														}*/
													Lcd_Write_strreg(1,12,sanjicaidan7_3_9_1);
														
													Datatempint=PROCHECKSUMallLast;
													Datatemp=Datatempint/10000;
													Datatempint=Datatempint%10000;
													Lcd_Write_charreg(1, 23, Datatemp+0x30);
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 24, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 25, Datatemp+0x30);
													

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 26, Datatemp+0x30);
													

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1, 27, Datatemp+0x30);
													
													
													
													}



											}
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									break;
			}











}
void			Lcd_DispLaySetLevel(lcdstruct	*disptr)
{

	if(disptr->Setlevel4==1)
		{

		Lcd_Write_strreg(1,0,erjicaidan1_1[disptr->LanguageType%maxLanguage]);//  1

		}
	else if(disptr->Setlevel4==2)
		{

		Lcd_Write_strreg(1,0,erjicaidan1_2[disptr->LanguageType%maxLanguage]);//  2

		}
	else if(disptr->Setlevel4==3)
		{

		Lcd_Write_strreg(1,0,erjicaidan2_1[disptr->LanguageType%maxLanguage]);// 3

		}
	else if(disptr->Setlevel4==4)
		{

		Lcd_Write_strreg(1,0,erjicaidan2_2[disptr->LanguageType%maxLanguage]);// 4

		}
	else if(disptr->Setlevel4==5)
		{

		Lcd_Write_strreg(1,0,erjicaidan2_3[disptr->LanguageType%maxLanguage]);// 5

		}
	else if(disptr->Setlevel4==6)
		{

		Lcd_Write_strreg(1,0,erjicaidan2_4[disptr->LanguageType%maxLanguage]);// 6

		}
	else if(disptr->Setlevel4==7)
		{

		Lcd_Write_strreg(1,0,erjicaidan2_5[disptr->LanguageType%maxLanguage]); //7

		}
	else if(disptr->Setlevel4==8)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_1[disptr->LanguageType%maxLanguage]);// 8

		}
	else if(disptr->Setlevel4==9)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_2[disptr->LanguageType%maxLanguage]);// 9

		}
	else if(disptr->Setlevel4==10)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_3[disptr->LanguageType%maxLanguage]);// 10

		}
	else if(disptr->Setlevel4==11)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_4[disptr->LanguageType%maxLanguage]);// 11

		}
	else if(disptr->Setlevel4==12)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_5[disptr->LanguageType%maxLanguage]);// 12

		}
	else if(disptr->Setlevel4==13)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_6[disptr->LanguageType%maxLanguage]);// 13

		}
	else if(disptr->Setlevel4==14)
		{

		Lcd_Write_strreg(1,0,erjicaidan3_7[disptr->LanguageType%maxLanguage]);// 14

		}
	else if(disptr->Setlevel4==15)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_1[disptr->LanguageType%maxLanguage]);// 15

		}
	else if(disptr->Setlevel4==16)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_2[disptr->LanguageType%maxLanguage]);// 16

		}
	else if(disptr->Setlevel4==17)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_3[disptr->LanguageType%maxLanguage]);// 17

		}
	else if(disptr->Setlevel4==18)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_4[disptr->LanguageType%maxLanguage]);// 18

		}
	else if(disptr->Setlevel4==19)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_5[disptr->LanguageType%maxLanguage]);// 19

		}
	else if(disptr->Setlevel4==20)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_6[disptr->LanguageType%maxLanguage]);// 20

		}
	else if(disptr->Setlevel4==21)
		{

		Lcd_Write_strreg(1,0,erjicaidan4_7[disptr->LanguageType%maxLanguage]);// 21

		}
	else if(disptr->Setlevel4==22)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_1[disptr->LanguageType%maxLanguage]);// 22

		}
	else if(disptr->Setlevel4==23)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_2[disptr->LanguageType%maxLanguage]);// 23

		}
	else if(disptr->Setlevel4==24)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_3[disptr->LanguageType%maxLanguage]);// 24

		}
	else if(disptr->Setlevel4==25)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_4[disptr->LanguageType%maxLanguage]);// 25

		}
	else if(disptr->Setlevel4==26)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_5[disptr->LanguageType%maxLanguage]);// 26

		}
	else if(disptr->Setlevel4==27)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_6[disptr->LanguageType%maxLanguage]);// 27

		}
	else if(disptr->Setlevel4==28)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_7[disptr->LanguageType%maxLanguage]);// 28

		}
	else if(disptr->Setlevel4==29)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_8[disptr->LanguageType%maxLanguage]);// 29

		}
	else if(disptr->Setlevel4==30)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_1_9[disptr->LanguageType%maxLanguage]);//30

		}
	else if(disptr->Setlevel4==31)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_2_1[disptr->LanguageType%maxLanguage]); // 31

		}
	else if(disptr->Setlevel4==32)
		{

		Lcd_Write_strreg(1,0,sanjicaidan5_2_2[disptr->LanguageType%maxLanguage]); // 32

		}
	else if(disptr->Setlevel4==33)
		{

		Lcd_Write_strreg(1,0,erjicaidan6_1[disptr->LanguageType%maxLanguage]);// 33

		}
	else if(disptr->Setlevel4==34)
		{

		Lcd_Write_strreg(1,0,erjicaidan6_2[disptr->LanguageType%maxLanguage]);// 34

		}
	else if(disptr->Setlevel4==35)
		{

		Lcd_Write_strreg(1,0,erjicaidan6_3[disptr->LanguageType%maxLanguage]);// 35

		}
	else if(disptr->Setlevel4==36)
		{

		Lcd_Write_strreg(1,0,erjicaidan6_4[disptr->LanguageType%maxLanguage]);// 36

		}
	else if(disptr->Setlevel4==37)
		{

		Lcd_Write_strreg(1,0,erjicaidan6_5[disptr->LanguageType%maxLanguage]);// 37

		}
	else if(disptr->Setlevel4==38)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_1_1[disptr->LanguageType%maxLanguage]); //38

		}
	else if(disptr->Setlevel4==39)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_1_2[disptr->LanguageType%maxLanguage]);// 39

		}
	else if(disptr->Setlevel4==40)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_1_3[disptr->LanguageType%maxLanguage]);// 40

		}
	else if(disptr->Setlevel4==41)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_1_4[disptr->LanguageType%maxLanguage]);// 41

		}
	else if(disptr->Setlevel4==42)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_1_5[disptr->LanguageType%maxLanguage]);// 42

		}
	else if(disptr->Setlevel4==43)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_2_1[disptr->LanguageType%maxLanguage]);// 43

		}
	else if(disptr->Setlevel4==44)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_2_2[disptr->LanguageType%maxLanguage]);// 44

		}

	else if(disptr->Setlevel4==45)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_1[disptr->LanguageType%maxLanguage]); //46

		}
	else if(disptr->Setlevel4==46)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_2[disptr->LanguageType%maxLanguage]); //47

		}
	else if(disptr->Setlevel4==47)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_3[disptr->LanguageType%maxLanguage]);// 48

		}
	else if(disptr->Setlevel4==48)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_4[disptr->LanguageType%maxLanguage]);// 49

		}
	else if(disptr->Setlevel4==49)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_5[disptr->LanguageType%maxLanguage]);// 49

		}
	else if(disptr->Setlevel4==50)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_6[disptr->LanguageType%maxLanguage]);// 49

		}
	else if(disptr->Setlevel4==51)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_7[disptr->LanguageType%maxLanguage]);// 49

		}
	else if(disptr->Setlevel4==52)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_8[disptr->LanguageType%maxLanguage]);// 49

		}
	else if(disptr->Setlevel4==53)
		{

		Lcd_Write_strreg(1,0,sanjicaidan7_3_9[disptr->LanguageType%maxLanguage]);// 49

		}
	else if(disptr->Setlevel4==54)
		{

		Lcd_Write_strreg(1,0,erjicaidan7_4[disptr->LanguageType%maxLanguage]);// 49

		}





}
void		Lcd_DispLayX_X_X(lcdstruct	*disptr)
{
	{
	static unsigned char	Datatemp,Datatemp1,charlength,wirelessLoopNum,NodeNum;//,DeviceNum;
	static uint16_t	Datatempint;
	static unsigned char	i,Address;
	//static unsigned long	eepAddress;
//	unsigned char	Statustemp[770];
	
	switch(disptr->Setlevel1)
		{
			case 1:
				switch(disptr->Setlevel2)// 1-1-1
					{
					case 1:
							{
						switch(disptr->Setlevel3)
							{
							case 1:
								Lcd_ClearReg();
							
								Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan1_1_1[disptr->LanguageType%maxLanguage]);
							
								Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan1_1_2[disptr->LanguageType%maxLanguage]);
							
								Lcd_Write_charreg(0, 44, 1);
							
								Lcd_Write_charreg(0, 46, 2);
								Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
								Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
								Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								disptr->DispdelayChange.dispdelay1_1=1;
								disptr->DispdelayTime.DispdelayTime1_1=HAL_GetTick();
								
								break;
							case 2:
							
								DispLayLog(disptr,&logDispdata);
								
								Address=Lcd_DispLay1_1_1tm(disptr,&logDispdata);
								
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, Address);
								
									Lcd_CmdWtite(1, 0x0d);
								break;
							case 3:
							
								Address=Lcd_DispLay1_1_1tm(disptr,&logDispdata);
									
								
								Lcd_Write_strreg(0, 0, (const unsigned char	*)erjicaidan1_1[disptr->LanguageType%maxLanguage]);
								
								Lcd_Write_strreg(0, 40, (const unsigned char	*)erjicaidan1_1_2[disptr->LanguageType%maxLanguage]);
								
								Lcd_Write_charreg(0, 44, 1);
								
								Lcd_Write_charreg(0, 46, 2);
								
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								Lcd_pos(1, Address);
								
								Lcd_CmdWtite(1, 0x0d);
								
								
							break;
						
							}
						break;
						}
					case 2:
							{
								Lcd_ClearReg();
								
								Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan1_2[disptr->LanguageType%maxLanguage]);
								
								Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan1_2_1[disptr->LanguageType%maxLanguage]);
								/*
								if(disptr->LanguageType%maxLanguage==0)
									{
										Address=9;

									}
								else if(disptr->LanguageType%maxLanguage==1)
									{
										Address=7;	

									}*/

								charlength=strlen((const char *)&erjicaidan1_2_2[disptr->LanguageType%maxLanguage]);
								Address=(40-charlength-5)/2;
								Lcd_Write_strreg(1,Address,(const unsigned char	*)erjicaidan1_2_2[disptr->LanguageType%maxLanguage]);
								
								Datatempint=PowerUpNum;
								Datatempint=Datatempint%10000;
								Datatemp=Datatempint/1000;
								Datatempint=Datatempint%100;
								Address=Address+charlength+1;
									
								Lcd_Write_charreg(1, Address, 0x30+Datatemp);
							
								Datatemp=Datatempint/100;
								Datatempint=Datatempint%100;
								Address++;
								Lcd_Write_charreg(1, Address, 0x30+Datatemp);
								
								Datatemp=Datatempint/10;
								Datatempint=Datatempint%10;
								Address++;
								Lcd_Write_charreg(1, Address, 0x30+Datatemp);
							
								Datatemp=Datatempint;
								Address++;
								Lcd_Write_charreg(1, Address, 0x30+Datatemp);
								
								if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[0].B==1))
									{
										/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);

									}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);	
									
						break;
					}
					}
				break;
			case 2:
				switch(disptr->Setlevel2)
					{
					case 1:
							{
						switch(disptr->Setlevel3)
							{
								case 1:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_1[disptr->LanguageType%maxLanguage]);
									if(disptr->SetLevelZonechar==0)
										{
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_1_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 44, 1);
									
									Lcd_Write_charreg(0, 46, 2);
										}
									else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);	


										}
									
										if(disptr->DispZoneSet.DispZoneEnable==1)
											{
											
											
									Lcd_Write_strreg(1, 0, (const unsigned char	*)erjicaidan2_1_2[disptr->LanguageType%maxLanguage]);
									
											}
										else if(disptr->DispZoneSet.DispZoneEnable==0)
											{
									Lcd_Write_strreg(1, 0, (const unsigned char	*)erjicaidan2_1_3[disptr->LanguageType%maxLanguage]);
									


											}
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
								

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
									
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[1].A==1))
									{
								
									/*	if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address,(const unsigned char	*) erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);

									}
									else{
											if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 40, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);
												}


										}
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->SetLevelZonechar==0)
										{
									Lcd_pos(1, 10);
									Lcd_CmdWtite(1, 0x0d);
										}
									else{
									Lcd_pos(1, (40+disptr->SetZoneTxt.SetZonetxtno));
									Lcd_CmdWtite(1, 0x0d);

										}
								break;
								case 2:
									Lcd_ClearReg();
								
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_1[disptr->LanguageType%maxLanguage]);

										if(disptr->SetLevelZonechar==0)
										{
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_1_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 44, 1);
									
									Lcd_Write_charreg(0, 46, 2);
										}
											else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);	


										}
									
								
										if(disptr->DispZoneSet.DispZoneEnable==1)
											{
											
											
									Lcd_Write_strreg(1, 0, (const unsigned char	*)erjicaidan2_1_2[disptr->LanguageType%maxLanguage]);
									
											}
										else if(disptr->DispZoneSet.DispZoneEnable==0)
											{
									Lcd_Write_strreg(1, 0, (const unsigned char	*)erjicaidan2_1_3[disptr->LanguageType%maxLanguage]);
									


											}
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
								

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
									/*
									eepAddress=disptr->DispZoneSet.SetZoneNum*18;
											Read_EE1024(2, Eeprom,eepAddress, 18);
											*/
											if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 40, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);
												}
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											if(disptr->SetLevelZonechar==0)
										{
									Lcd_pos(1, 20);
									Lcd_CmdWtite(1, 0x0d);
										}
									else{
									Lcd_pos(1, (40+disptr->SetZoneTxt.SetZonetxtno));
									Lcd_CmdWtite(1, 0x0d);

										}
									
									break;



								}
										
								//	break;

							}
						break;
					case 2:
							{
							switch(disptr->Setlevel3)
								{
									case 1:
									case 2:
									case 3:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_2[disptr->LanguageType%maxLanguage]);
									if(disptr->SetLevelZonechar==0)
										{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_2_1[disptr->LanguageType%maxLanguage]);
									
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 59, 1);//↑
									
									Lcd_Write_charreg(0, 61, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);//↑
									
									Lcd_Write_charreg(0, 47, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 61, 1);//↑
									
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
										}
									else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);	


										}
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_2_2[disptr->LanguageType%maxLanguage]);
								
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
								

										/*eepAddress=disptr->DispZoneSet.SetZoneNum*18;//zone label
											Read_EE1024(2, Eeprom,eepAddress, 18);
											if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&Eeprom[2],16);
												}
												*/
									if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);
												}
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan2_2_3[disptr->LanguageType%maxLanguage]);
									
									Datatempint=disptr->DispZoneSet.Dispsoundgrp1;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 56, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 57, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 58, Datatemp+0x30);
									

									Datatempint=disptr->DispZoneSet.Dispsoundgrp2;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 77, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 78, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 79, Datatemp+0x30);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->SetLevelZonechar==0)
										{
									if(disptr->Setlevel3==1)
										{
										Lcd_pos(1, 10);
										
										Lcd_CmdWtite(1, 0x0d);
	
										
										}
									else if(disptr->Setlevel3==2)
										{

										Lcd_pos(1, 58);
										
										Lcd_CmdWtite(1, 0x0d);
	
										

										}
									else if(disptr->Setlevel3==3)
										{

										Lcd_pos(1, 79);
									
										Lcd_CmdWtite(1, 0x0d);
	
										

										}
										}
									else{

										Lcd_pos(1, (13+disptr->SetZoneTxt.SetZonetxtno));
										Lcd_CmdWtite(1, 0x0d);


										}
	
									break;
								case 254:
									Lcd_ClearReg();
								
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_2[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_2_1[disptr->LanguageType%maxLanguage]);
									
									if(disptr->LanguageType%maxLanguage==0)
										{
									Lcd_Write_charreg(0, 59, 1);//↑
								
									Lcd_Write_charreg(0, 61, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
									Lcd_Write_charreg(0, 45, 1);//↑
									
									Lcd_Write_charreg(0, 47, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 61, 1);//↑
									
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_2_2[disptr->LanguageType%maxLanguage]);
								
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
								
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
								
									
										//eepAddress=disptr->DispZoneSet.SetZoneNum*18;//zone label
										//	Read_EE1024(2, Eeprom,eepAddress, 18);
										readzonetext(((disptr->DispZoneSet.SetZoneNum)%ZoneNum), &Eeprom[0]);
											if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&Eeprom[2],16);
												}
												/*
												if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);
												}*/

								//	Lcd_Write_str(1, 52,erjicaidan1_2_3);
								/*
										if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
								
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, 10);
									
									Lcd_CmdWtite(1, 0x0d);
	
									
									break;
								
									
									
								}
						}
						break;
					case 3:
							{
							switch(disptr->Setlevel3)
								{
									case 1:
									case 2:
									case 3:
									case 4:
									case 5:
									case 6:
									
									Lcd_ClearReg();
								
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_3[disptr->LanguageType%maxLanguage]);
									if(disptr->SetLevelZonechar==0)
										{
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
								
									Lcd_Write_charreg(0, 40, 3);//↓
									
									if(disptr->LanguageType%maxLanguage==0)
										{
											Lcd_Write_charreg(0, 59, 1);//↑
									
								
									Lcd_Write_charreg(0, 61, 2);//↓
									
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
								
										Lcd_Write_charreg(0, 61, 1);//↑
									
								
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
								
										Lcd_Write_charreg(0, 57, 1);//↑
									
								
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
										}
										else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);	


										}
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_2_2[disptr->LanguageType%maxLanguage]);
								
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
								
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
								

									/*
										eepAddress=disptr->DispZoneSet.SetZoneNum*18;//zone label
											Read_EE1024(2,Eeprom,eepAddress, 18);
											if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&Eeprom[2],16);
												}*/
												if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);
												}
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan2_2_5);
									
									Datatempint=disptr->DispZoneSet.DispIOGroup1;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 47, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 48, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 49, Datatemp+0x30);
									

									Datatempint=disptr->DispZoneSet.DispIOGroup2;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 53, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 54, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 55, Datatemp+0x30);
									

									Datatempint=disptr->DispZoneSet.DispIOGroup3;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 59, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 60, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 61, Datatemp+0x30);
									

									Datatempint=disptr->DispZoneSet.DispIOGroup4;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 65, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 66, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 67, Datatemp+0x30);
									
									Datatempint=disptr->DispZoneSet.DispIOGroup5;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 72, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 73, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 74, Datatemp+0x30);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->SetLevelZonechar==0)
										{
									if(disptr->Setlevel3==1)
										{
										Lcd_pos(1, 10);
									
										Lcd_CmdWtite(1, 0x0d);
	
										
										}
									else if(disptr->Setlevel3==2)
										{

										Lcd_pos(1, 49);
										
										Lcd_CmdWtite(1, 0x0d);
	
										

										}
									else if(disptr->Setlevel3==3)
										{

										Lcd_pos(1, 55);
										
										Lcd_CmdWtite(1, 0x0d);
	
										

										}
									else 	if(disptr->Setlevel3==4)
										{
										Lcd_pos(1, 61);
									
										Lcd_CmdWtite(1, 0x0d);
	
										
										}
									else if(disptr->Setlevel3==5)
										{

										Lcd_pos(1, 67);
										
										Lcd_CmdWtite(1, 0x0d);
	
										

										}
									else if(disptr->Setlevel3==6)
										{

										Lcd_pos(1, 74);
										
										Lcd_CmdWtite(1, 0x0d);
	
										

										}
										}
									else{


											Lcd_pos(1, (13+disptr->SetZoneTxt.SetZonetxtno));
										
										Lcd_CmdWtite(1, 0x0d);






										}
	
									break;
								case 254:
									Lcd_ClearReg();
								
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_3[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
								
									Lcd_Write_charreg(0, 40, 3);//↓
								
									if(disptr->LanguageType%maxLanguage==0)
										{
											Lcd_Write_charreg(0, 59, 1);//↑
									
								
										Lcd_Write_charreg(0, 61, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
								
										Lcd_Write_charreg(0, 61, 1);//↑
								
								
										Lcd_Write_charreg(0, 63, 2);//↓
									
										}
										else if(disptr->LanguageType%maxLanguage==2)
										{
								
										Lcd_Write_charreg(0, 57, 1);//↑
									
								
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_2_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
								

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
								
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
									
									
									//	eepAddress=disptr->DispZoneSet.SetZoneNum*18;//zone label
										//	Read_EE1024(2, Eeprom,eepAddress, 18);
										readzonetext(((disptr->DispZoneSet.SetZoneNum)%ZoneNum), &Eeprom[0]);
											if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 13, (const unsigned char	*)&Eeprom[2],16);
												}

								//	Lcd_Write_strreg(1, 52,erjicaidan1_2_3);
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/

									charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

									Lcd_pos(1, 10);
									
									Lcd_CmdWtite(1, 0x0d);
	
									
									break;
								
									
									
								}
						}
						
						break;
					case 4://assign  zone to device
								{
								switch(disptr->Setlevel3)
								{
									case 1:
									case 2:
									
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_4[disptr->LanguageType%maxLanguage]);
									
								
									
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
								
									
									Datatempint=(disptr->DispZoneSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
							
									
									Datatempint=disptr->DispZoneSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
								

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									
									

									//-------------------- 显示device label
									/*
									eepAddress=MaxDevice*24*(disptr->DispZoneSet.SetLoopNum%LoopNum)+disptr->DispZoneSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{

											Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
									//-------------------------
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan2_2_2[disptr->LanguageType%maxLanguage]);
									
									
									Datatempint=disptr->DispZoneSet.SetZoneNum;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 47, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 48, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 49, Datatemp+0x30);


										if(disptr->DispZoneSet.SetZoneNum>0)
											{
											/*
											eepAddress=(disptr->DispZoneSet.SetZoneNum-1)*18;//zone label
											Read_EE1024(2, Eeprom,eepAddress, 18);
											if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 52, (const unsigned char	*)&Eeprom[2],16);
												}
												*/
												if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
													{


														Lcd_Write_strlabelreg(1, 52, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);

													}
												
											}
										
										if((disptr->SetLevelZonechar==0)&&(disptr->SetLevelDeviceChar==0))
											{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);	
									
									
										if(disptr->Setlevel3==1)
										{
											charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
										Address=40+charlength;//(40-charlength)/2;
									Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, Address, 1);//↑
								Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, 15);
									
									Lcd_CmdWtite(1, 0x0d);
								
										}
									else if(disptr->Setlevel3==2)
										{
									
									Lcd_Write_strreg(0,63,(const unsigned char	*)erjicaidan2_4_3[disptr->LanguageType%maxLanguage]);
									
									
									Lcd_Write_charreg(0, 63, 1);//↑
									
									Lcd_Write_charreg(0, 65, 2);//↓
								
									
									
								

										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, 49);
									
									Lcd_CmdWtite(1, 0x0d);
								

										}
											}
											else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										Lcd_pos(1, (52+disptr->SetZoneTxt.SetZonetxtno));
										
										Lcd_CmdWtite(1, 0x0d);


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, (52+disptr->SetZoneTxt.SetZonetxtno));
										
										Lcd_CmdWtite(1, 0x0d);


										}
										else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
										Lcd_CmdWtite(1, 0x0d);


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
										Lcd_CmdWtite(1, 0x0d);


										}
	
									break;
								case 254:
									Lcd_ClearReg();
								
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_4[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);

									Datatempint=(disptr->DispZoneSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispZoneSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
								

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
								
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 15, Datatemp+0x30);
									/*

										eepAddress=MaxDevice*24*(disptr->DispZoneSet.SetLoopNum%LoopNum)+disptr->DispZoneSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{

											Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
								
										Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);	
									

								/*	Lcd_Write_strreg(0,63,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 63, 1);//↑
								
									Lcd_Write_charreg(0, 65, 2);//↓*/

									charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
										Address=40+charlength;//(40-charlength)/2;
									Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, Address, 1);//↑
								Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
									

									//Lcd_Write_str(1, 52,erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, 15);
								
									Lcd_CmdWtite(1, 0x0d);
								
									
									break;
								
									
									
								}
									
									
								
								
									
									
								}
						
						break;
					case 5:
							{
								switch(disptr->Setlevel3)
								{
									case 1:
									case 2:
									
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan2_5[disptr->LanguageType%maxLanguage]);
									
								
									if((disptr->SetLevelZonechar==0))
										{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_1_1[disptr->LanguageType%maxLanguage]);
									

									
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 46, 2);//↓
										}

									else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);	


										}
								

									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_2_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=disptr->DispZoneSet.SetZoneNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
									

									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 9, Datatemp+0x30);
									
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 10, Datatemp+0x30);
									

									
									Lcd_Write_strreg(1, 25,(const unsigned char	*) erjicaidan2_4_5[disptr->LanguageType%maxLanguage][disptr->DispZoneSet.dispZonestatus]);
									
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[1].E==1))
										{
										//	Lcd_Write_strreg(1, 52, erjicaidan1_2_3);
										/*
										if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);

											

										}
									else{
												/*eepAddress=disptr->DispZoneSet.SetZoneNum*18;//zone label
												Read_EE1024(2, Eeprom,eepAddress, 18);
											if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
												{
													
												Lcd_Write_strlabelreg(1, 40, (const unsigned char	*)&Eeprom[2],16);
												}*/
												if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
													{
													Lcd_Write_strlabelreg(1, 40, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);


													}



										}
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
										Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
										Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
										Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										if(disptr->SetLevelZonechar>0)
											{
												Lcd_pos(1, (40+disptr->SetZoneTxt.SetZonetxtno));
										
												Lcd_CmdWtite(1, 0x0d);
											}

										else{
										if(disptr->Setlevel3==1)
										{
									
									
									Lcd_pos(1, 10);
									
									Lcd_CmdWtite(1, 0x0d);
									
										}
									else if(disptr->Setlevel3==2)
										{
								
									Lcd_pos(1, 25);
									
									Lcd_CmdWtite(1, 0x0d);
									

										}
											}
	
									break;
					
								
									
									
								}
									
									
								
								
									
									
								}						
						break;




					}
			   

				break;
			case 3:
				switch(disptr->Setlevel2)
					{
						case 1:
							switch(disptr->Setlevel3)
								{
								case 1:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_1[disptr->LanguageType%maxLanguage]);
									
								
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
									

									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2){
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//?
										

										}
										charlength=strlen((const char	*)&erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
										Address=20-charlength;

									Lcd_Write_strreg(1,Address,(const unsigned char	*)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
										
									
									Lcd_Write_strreg(1, 21, (const unsigned char	*)erjicaidan3_1_3[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.DispSoundrConfiguration%2]);
									
									if(disptr->DispSounderSet.DispSoundrConfiguration==1)
										{	
									Lcd_Write_strreg(1, 40, (const unsigned char	*)erjicaidan3_1_4[disptr->LanguageType%maxLanguage]);
										
										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, 21);
									
									Lcd_CmdWtite(1, 0x0d);
									
									break;
								case 254:
										Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_1[disptr->LanguageType%maxLanguage]);
									
								
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
									

									
									if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2){
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//?
										

										}
								

									/*if(disptr->LanguageType%maxLanguage==0)
										{

									Lcd_Write_strreg(1,12,(const unsigned char	*)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{

									Lcd_Write_strreg(1,3,(const unsigned char	*)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
										}
									*/
												charlength=strlen((const char	*)&erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
										Address=20-charlength;

									Lcd_Write_strreg(1,Address,(const unsigned char	*)erjicaidan3_1_2[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(1, 21,(const unsigned char	*) erjicaidan3_1_3[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.DispSoundrConfiguration]);
								
									
								//	Lcd_Write_strreg(1, 52, erjicaidan1_2_3);
								/*
									if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char	*)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(1, 21);
									
									Lcd_CmdWtite(1, 0x0d);
								
									break;




								}
						break;
						case 2:
								switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 3:
									
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_2[disptr->LanguageType%maxLanguage]);
									
								
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][0]);
									
									//Lcd_Write_strreg(0,52,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][1]);
									Lcd_Write_strreg(0,52,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
									Datatempint=disptr->DispSounderSet.DispLaySounderGroupNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(0, 48, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 49, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(0, 50, Datatemp+0x30);
									

									Datatempint=disptr->DispSounderSet.DispLayPanelNum;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 60, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(0, 61, Datatemp+0x30);
								
									if(disptr->DispSounderSet.DispLayPanelNum==0)
										{

											if(disptr->Setlevel3==2)
												{
													Lcd_Write_strreg(0,64,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][7]);

														Datatempint=disptr->DispSounderSet.SetSounderNo/2+1;

														Datatemp=Datatempint/10;
													Lcd_Write_charreg(0, 74, Datatemp+0x30);
									
													Datatemp=Datatempint%10;
													Lcd_Write_charreg(0, 75, Datatemp+0x30);

													Lcd_Write_charreg(0, 77, 'S');
													Lcd_Write_charreg(0, 78, Datatempint=disptr->DispSounderSet.SetSounderNo%2+0X31);
														

												}
										
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][5]);
									GetSoundStatustoShuzu_Conv(&disptr->DispSounderSet.DispSoundGroupConfig.ConSounder, (unsigned char	*)&Statustemp);
									if(networkpanelamount==0)
										{
										Lcd_Write_charreg(1, 49, erjicaidan3_2_2[Statustemp[0]%3]);
									
										Lcd_Write_charreg(1, 50, erjicaidan3_2_2[Statustemp[1]%3]);



										}
									else
										{



									for(i=0;i<networkpanelamount*2;i++)
										{
										Lcd_Write_charreg(1, 49+i, erjicaidan3_2_2[Statustemp[i]%3]);



										}


										}
								//	Lcd_Write_charreg(1, 49, erjicaidan3_2_2[disptr->DispSounderSet.DispSoundGroupConfig.ConSounder0]);
									
								//	Lcd_Write_charreg(1, 50, erjicaidan3_2_2[disptr->DispSounderSet.DispSoundGroupConfig.ConSounder1]);
									
										}
									else //if(disptr->DispSounderSet.DispLayPanelNum==1)
										{
									if(disptr->Setlevel3==3)
										{
									Lcd_Write_strreg(0,64,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][2]);
									
									Datatempint=disptr->DispSounderSet.SetSounderNo+1+93;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(0, 74, Datatemp+0x30);
									Datatempint=Datatempint%100;
									
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 75, Datatemp+0x30);
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(0, 76, Datatemp+0x30);


										}
											
									Lcd_Write_strreg(1,40,erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
									
									//GetSoundStatustoShuzu((soundgr *)&disptr->DispSounderSet.DispSoundGroupConfig,(unsigned char	*)&Statustemp);
									GetSoundStatustoShuzu_1(&disptr->DispSounderSet.DispSoundGroupConfig.LoopSounder[disptr->DispSounderSet.DispLayPanelNum-1], (unsigned char	*)&Statustemp);
									for(i=0;i<32;i++)
										{
										/*
										if(disptr->DispSounderSet.SetSounderNo<31)
											{
										Lcd_Write_charreg(1, 48+i, erjicaidan3_2_2[Statustemp[i+2]]);
											}
										else{

										Lcd_Write_charreg(1, 48+i, (erjicaidan3_2_2[Statustemp[i+2+disptr->DispSounderSet.SetSounderNo-31]]));


											}
											*/
										Lcd_Write_charreg(1, 48+i, erjicaidan3_2_2[Statustemp[i+24]]);
										}
								

										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if((disptr->Setlevel3==1)&&(disptr->DispSounderSet.SetGrpOrPanel==0)&&(disptr->DispSounderSet.SetConOrLoop==0))
										{
									Lcd_pos(0,50);
								
									Lcd_CmdWtite(0, 0x0d);
									
										}
									
									else if((disptr->Setlevel3==2)&&(disptr->DispSounderSet.SetGrpOrPanel==1)&&(disptr->DispSounderSet.SetConOrLoop==0))//设置loop
										{
									Lcd_pos(0,61);
									
									Lcd_CmdWtite(0, 0x0d);
								


										}
									else if((disptr->Setlevel3==2)&&((disptr->DispSounderSet.SetGrpOrPanel==0)||(disptr->DispSounderSet.SetConOrLoop==1)))
										{
										
										Lcd_pos(1,49+disptr->DispSounderSet.SetSounderNo);
									
										Lcd_CmdWtite(1, 0x0d);
								

										}
									
										else if(disptr->Setlevel3==3)
										{
										if(disptr->DispSounderSet.SetSounderNo>31)
											{
										
										Lcd_pos(1,79);
											}
										else{
										Lcd_pos(1,48+disptr->DispSounderSet.SetSounderNo);
											}
								
									Lcd_CmdWtite(1, 0x0d);
								

										}

										break;
									case 254:
										Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_2[disptr->LanguageType%maxLanguage]);
									
								
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][0]);
									
									//Lcd_Write_strreg(0,52,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][1]);
									Lcd_Write_strreg(0,52,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
									Datatempint=disptr->DispSounderSet.DispLaySounderGroupNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(0, 48, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 49, Datatemp+0x30);
								
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(0, 50, Datatemp+0x30);
								

									Datatempint=disptr->DispSounderSet.DispLayPanelNum;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 60, Datatemp+0x30);
								
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(0, 61, Datatemp+0x30);
									
									if(disptr->DispSounderSet.DispLayPanelNum==0)
										{
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][5]);
									
									
									//Lcd_Write_charreg(1, 9, erjicaidan3_2_2[disptr->DispSounderSet.DispSoundGroupConfig.ConSounder0]);
								
									//Lcd_Write_charreg(1, 10, erjicaidan3_2_2[disptr->DispSounderSet.DispSoundGroupConfig.ConSounder1]);
										GetSoundStatustoShuzu_Conv(&disptr->DispSounderSet.DispSoundGroupConfig.ConSounder, (unsigned char	*)&Statustemp);
									if(networkpanelamount==0)
										{
										Lcd_Write_charreg(1, 49, erjicaidan3_2_2[Statustemp[0]%3]);
									
										Lcd_Write_charreg(1, 50, erjicaidan3_2_2[Statustemp[1]%3]);



										}
									else
										{



									for(i=0;i<networkpanelamount*2;i++)
										{
										Lcd_Write_charreg(1, 49+i, erjicaidan3_2_2[Statustemp[i]%3]);



										}
										}

									/////////////////////
									
								
										}
									
										//Lcd_Write_strreg(1, 54, erjicaidan1_2_3);
									/*	if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											Lcd_Write_strreg(1, 40, (const unsigned char	*)&blank[0]);	
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if((disptr->DispSounderSet.SetGrpOrPanel==0))
										{
									Lcd_pos(0,50);
									
									Lcd_CmdWtite(0, 0x0d);
								
										}
									
									else if((disptr->DispSounderSet.SetGrpOrPanel==1))//设置loop
										{
									Lcd_pos(0,61);
								
									Lcd_CmdWtite(0, 0x0d);
								


										}
									



										break;
									
									



									}
							break;
						case 3:
						{	
							switch(disptr->Setlevel3)
								{
								case 1:
								case 2:
								case 3:
								case 254:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_3[disptr->LanguageType%maxLanguage]);
								
								
									
									//Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][1]);
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
									
									Datatempint=disptr->DispSounderSet.DispLayPanelNum;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 46, Datatemp+0x30);
									
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(0, 47, Datatemp+0x30);
									
									if(disptr->DispSounderSet.DispLayPanelNum==0)
										{
											if(disptr->Setlevel3==2)
												{
													Lcd_Write_strreg(0,64,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][7]);

														Datatempint=disptr->DispSounderSet.SetSounderNo/2+1;

														Datatemp=Datatempint/10;
													Lcd_Write_charreg(0, 74, Datatemp+0x30);
									
													Datatemp=Datatempint%10;
													Lcd_Write_charreg(0, 75, Datatemp+0x30);

													Lcd_Write_charreg(0, 77, 'S');
													Lcd_Write_charreg(0, 78, Datatempint=disptr->DispSounderSet.SetSounderNo%2+0X31);
														

												}
											
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][5]);
									
										GetSoundStatustoShuzu1_Conv(&disptr->DispSounderSet.DisplaySoundCONFIG.ConSounder, (unsigned char	*)&Statustemp);
									if(networkpanelamount==0)
										{
										Lcd_Write_charconstreg(1, 49, erjicaidan3_3_1[disptr->LanguageType%maxLanguage][Statustemp[0]%2]);
									
										Lcd_Write_charconstreg(1, 50, erjicaidan3_3_1[disptr->LanguageType%maxLanguage][Statustemp[1]%2]);



										}
									else
										{



									for(i=0;i<networkpanelamount*2;i++)
										{
										Lcd_Write_charconstreg(1, 49+i, erjicaidan3_3_1[disptr->LanguageType%maxLanguage][Statustemp[i]%2]);



										}
										}
									//Lcd_Write_charconstreg(1, 9, erjicaidan3_3_1[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.DisplaySoundCONFIG.ConSounder0]);
									
									//Lcd_Write_charconstreg(1, 10, erjicaidan3_3_1[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.DisplaySoundCONFIG.ConSounder1]);
									
										}
									else// if(disptr->DispSounderSet.DispLayPanelNum==1)
										{
									if(disptr->Setlevel3==3)
										{
									Lcd_Write_strreg(0,63,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][2]);
								
									Datatempint=disptr->DispSounderSet.SetSounderNo+1+93;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(0, 73, Datatemp+0x30);
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 74, Datatemp+0x30);
									Datatemp=Datatempint%10;
									Lcd_Write_charreg(0, 75, Datatemp+0x30);
								


										}
										if(disptr->Setlevel3!=254){
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
								
									GetSoundStatustoShuzu1_1((loopsnden *)&disptr->DispSounderSet.DisplaySoundCONFIG.LoopSounder[disptr->DispSounderSet.DispLayPanelNum-1],(unsigned char	*)Statustemp);
									for(i=0;i<32;i++)
										{
										
										
										Lcd_Write_charconstreg(1, 48+i, erjicaidan3_3_1[disptr->LanguageType%maxLanguage][Statustemp[i+24]]);
										
									
										}
										}

										}

											if(disptr->Setlevel3==254)
												{
										//	Lcd_Write_strreg(1, 52, erjicaidan1_2_3);
										/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											Lcd_Write_strreg(1, 40, (const unsigned char	*)&blank[0]);
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);

											
												}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											
									if((disptr->Setlevel3==1))
										{
									Lcd_pos(0,47);
									
									Lcd_CmdWtite(0, 0x0d);
										
										}
									
									else if((disptr->Setlevel3==2)&&(disptr->DispSounderSet.DispLayPanelNum==0)&&(disptr->Setlevel3!=254))//设置loop
										{
									Lcd_pos(1,49+disptr->DispSounderSet.SetSounderNo);
									
									Lcd_CmdWtite(1, 0x0d);
									


										}
									else if((disptr->Setlevel3==2)&&(disptr->DispSounderSet.DispLayPanelNum>0))
										{
										
									Lcd_pos(0,47);
									
									Lcd_CmdWtite(0, 0x0d);
										

										}
									
										else if((disptr->Setlevel3==3)&&(disptr->Setlevel3!=254))
										{
										
										Lcd_pos(1,48+disptr->DispSounderSet.SetSounderNo);
											
									Lcd_CmdWtite(1, 0x0d);
									

										}

										break;	
								

								}
							}
							break;
						case 4:
								switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_4[disptr->LanguageType%maxLanguage]);
									

									
								
									if(disptr->Setlevel3==1)
										{
										if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}
										else{
											 if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}



									}
									
								//	Lcd_Write_strreg(0,44,erjicaidan3_4_1);
								//	


										}
										else if(disptr->Setlevel3==2)
										{
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan3_4_3[disptr->LanguageType%maxLanguage]);
									

									
									Lcd_Write_charreg(0, Address, 1);//↑
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
									
										
								






										}

									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									

										Datatempint=(disptr->DispSounderSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispSounderSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									/*
									eepAddress=MaxDevice*24*(disptr->DispSounderSet.SetLoopNum%LoopNum)+disptr->DispSounderSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
											if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{

											Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
									

									if(disptr->Setlevel3==254)
										{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);	

									charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;

									
									Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);//↑
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
								//	Lcd_Write_strreg(1,52,erjicaidan1_2_3);
								/*
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan3_4_2[disptr->LanguageType%maxLanguage]);
									
										

									
									Datatempint=disptr->DispSounderSet.DispLaySounderGroupNum;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 55, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 56, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 57, Datatemp+0x30);
									
									}
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{
											if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
												}
											else{
				
										Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
										Lcd_CmdWtite(1, 0x0d);
												}


										
								


												
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 57);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==254)
										{
										Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									
									}
								

										break;	
							case 5:
									switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_5[disptr->LanguageType%maxLanguage]);
									

									
								
									if(disptr->Setlevel3==1)
										{
										if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									
									Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);//↑
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
											}
											else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
								


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
										


										}
									


										}
									else if(disptr->Setlevel3==2)
										{
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
									

									
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2){
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//?
										

										}
									



										}

									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispSounderSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispSounderSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{

											Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
									

								
									if(disptr->Setlevel3==254)
										{
										Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
										charlength=strlen((const char	*)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
											Address=40+charlength;
										Lcd_Write_strreg(0,Address,(const unsigned char	*)erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);//↑
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
								
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}
										charlength=strlen((const char	*)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
											Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,49,(const unsigned char	*)erjicaidan3_4_5[disptr->LanguageType%maxLanguage]);
									
										

									
									Lcd_Write_strreg(1,61,(const unsigned char	*)erjicaidan3_4_6[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.Dispdeviceinhibitsounder]);
									
									}
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{


											if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
												}
											else{
													Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
													Lcd_CmdWtite(1, 0x0d);

												}
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==254)
										{
										Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									
									}


								break;

						//	break;
						case 6:
								switch(disptr->Setlevel3)
									{
										case 1:
										case 2:
										case 3:
										case 4:
										case 254:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_6[disptr->LanguageType%maxLanguage]);
									

									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 40, 3);//↓
									
									if(disptr->LanguageType%maxLanguage==0)
										{
											Lcd_Write_charreg(0, 59, 1);//↑
									
								
									Lcd_Write_charreg(0, 61, 2);//↓
									
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
								
										Lcd_Write_charreg(0, 61, 1);//↑
									
								
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
									
									else if(disptr->LanguageType%maxLanguage==2)
										{
									Lcd_Write_charreg(0, 57, 1);//↑
									
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
									
									Lcd_Write_strreg(1, 0, (const unsigned char	*)erjicaidan3_6_1[disptr->LanguageType%maxLanguage][0]);
									

									Lcd_Write_strreg(1, 10, (const unsigned char	*)erjicaidan3_6_1[disptr->LanguageType%maxLanguage][1]);
									
									if(disptr->Setlevel3!=254)
										{
									Lcd_Write_strreg(1, 40, (const unsigned char	*)erjicaidan3_6_1[disptr->LanguageType%maxLanguage][2]);
									
										}
									
									Datatempint= disptr->DispSounderSet.DisplaySounderDelayStatus.delaymin;
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 7, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 8, Datatemp+0x30);
										

									Datatempint=disptr->DispSounderSet.DisplaySounderDelayStatus.delaysec;
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 17, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 18, Datatemp+0x30);
										

									Lcd_Write_strreg(1,22, (const unsigned char	*)erjicaidan3_6_2[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.DisplaySounderDelayStatus.delaymode]);
									
									if(disptr->Setlevel3!=254)
										{
									Lcd_Write_strreg(1,54, (const unsigned char	*)erjicaidan3_6_3[disptr->LanguageType%maxLanguage][disptr->DispSounderSet.DisplaySounderDelayStatus.activetype]);
									
									
										}
									if(disptr->Setlevel3==254)
										{

									//	Lcd_Write_strreg(1,52, erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										charlength=strlen((const char	*)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
											Address=40+(40-charlength)/2;	
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									

										}
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{

											Lcd_pos(1, 8);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 18);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==3)
										{
										Lcd_pos(1, 22);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									else if(disptr->Setlevel3==4)
										{
										Lcd_pos(1, 54);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
								






									}
							break;
					case 7:
						{
							switch(disptr->Setlevel3)
								case 1:
								case 2:
								case 3:
								case 254:
								Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan3_7[disptr->LanguageType%maxLanguage]);
									

									//Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][1]);
									
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
									
									if(disptr->DispSounderSet.DispLayPanelNum==0)
										{
										
									Lcd_Write_strreg(1, 40, (const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][5]);

									if(disptr->Setlevel3==2)
												{
													Lcd_Write_strreg(0,64,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][7]);

														Datatempint=disptr->DispSounderSet.SetSounderNo/2+1;

														Datatemp=Datatempint/10;
													Lcd_Write_charreg(0, 74, Datatemp+0x30);
									
													Datatemp=Datatempint%10;
													Lcd_Write_charreg(0, 75, Datatemp+0x30);

													Lcd_Write_charreg(0, 77, 'S');
													Lcd_Write_charreg(0, 78, Datatempint=disptr->DispSounderSet.SetSounderNo%2+0X31);
														

												}
											
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][5]);
									
										GetSoundStatustoShuzu2_Conv(&disptr->DispSounderSet.DispSounderOverridDelay.ConSounder, (unsigned char	*)&Statustemp);
									if(networkpanelamount==0)
										{
										Lcd_Write_charconstreg(1, 49, erjicaidan3_7_1[Statustemp[0]%2]);
									
										Lcd_Write_charconstreg(1, 50, erjicaidan3_7_1[Statustemp[1]%2]);



										}
									else
										{



									for(i=0;i<networkpanelamount*2;i++)
										{
										Lcd_Write_charconstreg(1, 49+i, erjicaidan3_7_1[Statustemp[i]%2]);



										}
										}
									


									
								//	Lcd_Write_charconstreg(1, 49,erjicaidan3_7_1[disptr->DispSounderSet.DispSounderOverridDelay.ConSounder0]);
									
								//	Lcd_Write_charconstreg(1, 50, erjicaidan3_7_1[disptr->DispSounderSet.DispSounderOverridDelay.ConSounder1]);

										}
									
									Datatempint=disptr->DispSounderSet.DispLayPanelNum;
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 48, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(0, 49, Datatemp+0x30);
										
										if(((disptr->Setlevel3==2)&&(disptr->DispSounderSet.DispLayPanelNum>=1))||(disptr->Setlevel3==3))
											{
	
												Lcd_Write_strreg(1, 40, (const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][4]);
												
												GetSoundStatustoShuzu2_1(&disptr->DispSounderSet.DispSounderOverridDelay.LoopSounder[disptr->DispSounderSet.DispLayPanelNum-1],(unsigned char	*)Statustemp);
											for(i=0;i<32;i++)
												{
											Lcd_Write_charreg(1, 48+i, erjicaidan3_7_1[Statustemp[i+24]]);
												}
										
										
										
										}

											
										if(disptr->Setlevel3==3)
											{
												Lcd_Write_strreg(0, 63, (const unsigned char	*)erjicaidan3_2_1[disptr->LanguageType%maxLanguage][2]);
												
												
												Datatempint=disptr->DispSounderSet.SetSounderNo+1+93;
												Datatemp=Datatempint/100;
												Lcd_Write_charreg(0, 73, Datatemp+0x30);
									Datatempint=Datatempint%100;
									
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(0, 74, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(0, 75, Datatemp+0x30);
										

											}
								
									if(disptr->Setlevel3==254)
										{

									//	Lcd_Write_str(1,52, erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
											
											Lcd_Write_strreg(1, 40, (const unsigned char	*)&blank[0]);
											charlength=strlen((const char	*)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
											Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, (const unsigned char	*)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									

										}
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if((disptr->Setlevel3==1)||((disptr->Setlevel3==2)&&(disptr->DispSounderSet.DispLayPanelNum>=1))||(disptr->Setlevel3==254))
										{

											Lcd_pos(0, 49);
											
											Lcd_CmdWtite(0, 0x0d);
											


										}
									else if((disptr->Setlevel3==2)&&(disptr->DispSounderSet.DispLayPanelNum==0))
										{
											Lcd_pos(1, 49+disptr->DispSounderSet.SetSounderNo);
											
											Lcd_CmdWtite(1, 0x0d);
										


										}
									else if(disptr->Setlevel3==3)
										{
											if(disptr->DispSounderSet.SetSounderNo<32)
												{
											Lcd_pos(1, 48+disptr->DispSounderSet.SetSounderNo);
												}
											else{

											Lcd_pos(1, 79);
												}
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									
							


						}
					break;
			
					}
			
				break;
				case 4://setlevel4  4-xxx
						switch(disptr->Setlevel2)
						{

						
						case 1://    4-1
							{
								switch(disptr->Setlevel3)
									
										case 1:
										case 2:
										case 3:
										case 254:

											
										Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_1[disptr->LanguageType%maxLanguage]);
									
									if((disptr->Setlevel3==1)||(disptr->Setlevel3==2)||(disptr->Setlevel3==254))
										{
										Lcd_Write_strreg(0,40,erjicaidan2_2_4[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 40, 3);//↓
									
									if(disptr->LanguageType%maxLanguage==0)
										{
											Lcd_Write_charreg(0, 59, 1);//↑
									
								
									Lcd_Write_charreg(0, 61, 2);//↓
									
									
										}
									else if(disptr->LanguageType%maxLanguage==1)
										{
								
										Lcd_Write_charreg(0, 61, 1);//↑
									
								
									Lcd_Write_charreg(0, 63, 2);//↓
									
										}
									else if(disptr->LanguageType%maxLanguage==2)
										{
								
										Lcd_Write_charreg(0, 57, 1);//↑
									
								
									Lcd_Write_charreg(0, 59, 2);//↓
									
										}
										}
									else if(disptr->Setlevel3==3)
										{



									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);	
									charlength=strlen((const	char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);//↑
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);//↓
									



										}
									

									Lcd_Write_strreg(1,10,erjicaidan4_1_1[disptr->LanguageType%maxLanguage][0]);
									
									Lcd_Write_strreg(1,21,erjicaidan4_1_1[disptr->LanguageType%maxLanguage][1]);
									
										if(disptr->Setlevel3!=254)
										{
									Lcd_Write_strreg(1,40,erjicaidan4_1_1[disptr->LanguageType%maxLanguage][2]);
									
									

									Lcd_Write_strreg(1,48,erjicaidan4_1_1[disptr->LanguageType%maxLanguage][3]);
									
										}
									
									Datatempint=disptr->DispIOgroupSet.DispLayIOGroup+1;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 16, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 17, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 18, Datatemp+0x30);
									
									Datatempint=disptr->DispIOgroupSet.DispLayEntry+1;
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 30, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 31, Datatemp+0x30);
									

							
						
									if(disptr->Setlevel3!=254)
										{
									Datatempint=(disptr->DispIOgroupSet.DispIOgroupConfig.entry[disptr->DispIOgroupSet.DispLayEntry%32].Bit.LoopNO)%LoopNum+1;// LOOP  01 固定
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 45, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 46, Datatemp+0x30);
									

									Datatempint=disptr->DispIOgroupSet.DispIOgroupConfig.entry[disptr->DispIOgroupSet.DispLayEntry%32].Bit.DeviceNo;
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 55, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 56, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 57, Datatemp+0x30);
									
										}
									
									
									if(disptr->Setlevel3==254)
										{

									
										if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									

										}
													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										if(disptr->Setlevel3==1)
										{

											Lcd_pos(1, 18);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 31);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==3)
										{
											

											Lcd_pos(1, 57);
												
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									
								}
							
						break;
						
				
			
					case 2://    5-2 select fault i/o Group
								{
							switch(disptr->Setlevel3)
								
									case 1:
									case 254:
										Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_2[disptr->LanguageType%maxLanguage]);
									

									Lcd_Write_strreg(0,40,erjicaidan4_2_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 46, 2);//↓
									
									Lcd_Write_strreg(1,0,erjicaidan4_2_2[disptr->LanguageType%maxLanguage]);
									

									
									
									Datatempint=disptr->DispIOgroupSet.DispfaultActiveIOGroup;//0 代表没有，
									Datatempint=Datatempint%1000;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 37, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 38, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 39, Datatemp+0x30);
									
									
									
										
									
									
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[3].B))
										{

									//	Lcd_Write_strreg(1,52, erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const	char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									

										}

													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
								Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
											Lcd_pos(1, 39);
											
											Lcd_CmdWtite(1, 0x0d);
											


										
									
										
									







								}
								

							
						break;
				case 3:
						switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_3[disptr->LanguageType%maxLanguage]);
									
									
									if((disptr->Setlevel3==1)||(disptr->Setlevel3==254))
										{
										if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									charlength=strlen((const	char *)erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Address=40+charlength;
									Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, Address, 1);
									Address=Address+2;
									Lcd_Write_charreg(0, Address, 2);
											}
												else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
									
								//	


										}
										else if(disptr->Setlevel3==2)
										{
									Lcd_Write_strreg(0,40,erjicaidan4_2_1[disptr->LanguageType%maxLanguage]);
									
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 46, 2);//↓
									
									



										}

									Lcd_Write_strreg(1,0,erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									

									Datatempint=(disptr->DispIOgroupSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispIOgroupSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									/*
									eepAddress=MaxDevice*24*(disptr->DispIOgroupSet.SetLoopNum%LoopNum)+disptr->DispIOgroupSet.SetDeviceNum*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{
												Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
									if(disptr->Setlevel3==254)
										{
									
									//Lcd_Write_strreg(1,52,erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
										
									charlength=strlen((const	char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,40,erjicaidan3_4_2[disptr->LanguageType%maxLanguage]);
									
										

									
									Datatempint=disptr->DispIOgroupSet.DispLayIOGroup;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 55, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 56, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 57, Datatemp+0x30);
									
									}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{
											if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
												}
											else{

											Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
											Lcd_CmdWtite(1, 0x0d);

												}
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 57);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==254)
										{
										Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									
									}

					break;
						
					case 4:
								switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_4[disptr->LanguageType%maxLanguage]);
									
								
									if((disptr->Setlevel3==1)||(disptr->Setlevel3==254))
										{
											if(disptr->SetLevelDeviceChar==0)
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(0,63,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, 63, 1);
									
									Lcd_Write_charreg(0, 65, 2);
												}
									else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
												
									
									
								


										}
										else if(disptr->Setlevel3==2)
										{
									Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
									

									
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2){
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//?
										

										}
									



										}

									Lcd_Write_strreg(1,0,erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									

									Datatempint=(disptr->DispIOgroupSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispIOgroupSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									
										/*eepAddress=MaxDevice*24*(disptr->DispIOgroupSet.SetLoopNum%LoopNum)+disptr->DispIOgroupSet.SetDeviceNum*24;
									Read_EE1024(3,Eeprom,eepAddress, 24);
									
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	 *)&Eeprom[2], 22);
										}
										*/
											if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{
												Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}

									if(disptr->Setlevel3==254)
										{
									
								//	Lcd_Write_strreg(1,52,erjicaidan1_2_3);
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,49,erjicaidan3_4_5[disptr->LanguageType%maxLanguage]);
									
										

									
									Lcd_Write_strreg(1,61,erjicaidan3_4_6[disptr->LanguageType%maxLanguage][disptr->DispIOgroupSet.DispdeviceinhibitIO]);
									
									}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{

												if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
												}
											else{

											Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
											Lcd_CmdWtite(1, 0x0d);

												}
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 61);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==254)
										{
										Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									//break;
									
									}

						break;
					case 5:
							switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_5[disptr->LanguageType%maxLanguage]);
									
								
									if((disptr->Setlevel3==1)||(disptr->Setlevel3==254))
										{
										if((disptr->SetLevelDeviceChar==0))
											{
									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(0,63,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, 63, 1);
									
									Lcd_Write_charreg(0, 65, 2);
											}
														else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
									
								


										}
									else if(disptr->Setlevel3==2)
										{
									Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
									

									
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2){
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//?
										

										}
									



										}

									Lcd_Write_strreg(1,0,erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									
									Datatempint=(disptr->DispIOgroupSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispIOgroupSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									/*
										eepAddress=MaxDevice*24*(disptr->DispIOgroupSet.SetLoopNum%LoopNum)+disptr->DispIOgroupSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17,(const unsigned char	*) &Eeprom[2], 22);
										}*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{
												Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
									if(disptr->Setlevel3==254)
										{
									
								//	Lcd_Write_strreg(1,52,erjicaidan1_2_3);
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,51,erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);
									
										

									
									Lcd_Write_strreg(1,60,erjicaidan3_5_2[disptr->LanguageType%maxLanguage][disptr->DispIOgroupSet.DispdeviceEVAC]);
									
									}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{

												if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
												}
											else{

											Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
											Lcd_CmdWtite(1, 0x0d);

												}
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 60);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==254)
										{
										Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									//break;
									
									}


						break;
					case 6:
								switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_6[disptr->LanguageType%maxLanguage]);
									
								
									if((disptr->Setlevel3==1)||(disptr->Setlevel3==254))
										{
										if((disptr->SetLevelDeviceChar==0))
											{

									Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(0,63,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_charreg(0, 63, 1);
									
									Lcd_Write_charreg(0, 65, 2);
											}
											else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
									


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
									
									
								


										}
									else if(disptr->Setlevel3==2)
										{
									Lcd_Write_strreg(0,40,erjicaidan3_1_1[disptr->LanguageType%maxLanguage]);
									

									
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==1))
										{
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0, 50, 2);//↓
										}
									else if(disptr->LanguageType%maxLanguage==2){
									Lcd_Write_charreg(0, 44, 1);//↑
									
									Lcd_Write_charreg(0,48, 2);//?
										

										}
									



										}

									Lcd_Write_strreg(1,0,erjicaidan2_4_2[disptr->LanguageType%maxLanguage]);
									

									Datatempint=(disptr->DispIOgroupSet.SetLoopNum+1)%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 5, Datatemp+0x30);
							

									Datatemp=Datatempint%10;
									
									Lcd_Write_charreg(1, 6, Datatemp+0x30);
									
									Datatempint=disptr->DispIOgroupSet.SetDeviceNum+1;
									Datatemp=Datatempint/100;
									Lcd_Write_charreg(1, 12, Datatemp+0x30);
									
									Datatempint=Datatempint%100;
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 13, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 14, Datatemp+0x30);
									/*
										eepAddress=MaxDevice*24*(disptr->DispIOgroupSet.SetLoopNum%LoopNum)+disptr->DispIOgroupSet.SetDeviceNum*24;
									Read_EE1024(1, Eeprom,eepAddress, 24);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&Eeprom[2], 22);
										}
										*/
										if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{
												Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);

											}
									if(disptr->Setlevel3==254)
										{
									
								//	Lcd_Write_strreg(1,52,erjicaidan1_2_3);
								if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									
										}
									else{
									Lcd_Write_strreg(1,51,erjicaidan3_5_1[disptr->LanguageType%maxLanguage]);
									
										

									
									Lcd_Write_strreg(1,60,erjicaidan2_4_5[disptr->LanguageType%maxLanguage][disptr->DispIOgroupSet.DispdeviceIoDelayOrIMM]);
									
										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{

												if(disptr->SetLevelDeviceChar==0)
												{
											Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
												}
											else{

											Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
											Lcd_CmdWtite(1, 0x0d);

												}
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 60);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==254)
										{
										Lcd_pos(1, 14);
											
											Lcd_CmdWtite(1, 0x0d);
											

										}
									//break;
									
									}


						break;
						
					case 7:
						switch(disptr->Setlevel3)
									{
									case 1:
									case 2:
									case 254:
			
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,erjicaidan4_7[disptr->LanguageType%maxLanguage]);
									
									
									
									Lcd_Write_strreg(0,40,erjicaidan4_7_1[disptr->LanguageType%maxLanguage]);
											

									Lcd_Write_charreg(0,44,1);
										

									Lcd_Write_charreg(0,46,2);
										

									Lcd_Write_strreg(1,12,erjicaidan3_6_1[disptr->LanguageType%maxLanguage][0]);
										

									Lcd_Write_strreg(1,23,erjicaidan3_6_1[disptr->LanguageType%maxLanguage][1]);
										
									Datatempint=disptr->DispIOgroupSet.DispIODelayTime.iodelaymin;
									Datatemp=Datatempint%100;
									
									
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 19, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 20, Datatemp+0x30);
									

									Datatempint=disptr->DispIOgroupSet.DispIODelayTime.iodelaysec;
									Datatemp=Datatempint%100;
									
									
									Datatemp=Datatempint/10;
									Lcd_Write_charreg(1, 30, Datatemp+0x30);
									
									Datatempint=Datatempint%10;
									Datatemp=Datatempint;
									Lcd_Write_charreg(1, 31, Datatemp+0x30);
									
									if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[3].G==1))
										{
									
									//Lcd_Write_strreg(1,52,erjicaidan1_2_3);
									/*if((disptr->LanguageType%maxLanguage)==0)
											{	
											Address=52;

											}
										else if((disptr->LanguageType%maxLanguage)==1)
											{	
											Address=49;

											}*/
									charlength=strlen((const char *)erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Address=40+(40-charlength)/2;
									Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										}
									else{
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									if(disptr->Setlevel3==1)
										{

											Lcd_pos(1, 20);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
									else if(disptr->Setlevel3==2)
										{
											Lcd_pos(1, 31);
											
											Lcd_CmdWtite(1, 0x0d);
											


										}
										}
								
									
									}
						break;

							}
						break;
					case 5://菜单5
					switch(disptr->Setlevel2)
						{

							case 1:
								
								Lcd_DispLay5_1_X_X(disptr);
							break;

							case 2:
								Lcd_DispLay5_2_X_X(disptr);
								break;

						}
					break;
				case 	6:
						switch(disptr->Setlevel2)
							{

							
							case 1://    6-1
								{
									
										
											

										if((disptr->Setlevel3==1)||(disptr->Setlevel3==2)||(disptr->Setlevel3==3)||(disptr->Setlevel3==254))
											{
											Lcd_ClearReg();
										
										Lcd_Write_strreg(0,0,erjicaidan6_1[disptr->LanguageType%maxLanguage]);
										

										Lcd_Write_strreg(0,40,erjicaidan6_1_1[disptr->LanguageType%maxLanguage]);
										
										Lcd_Write_charreg(0, 57, 1);//↑
										
										Lcd_Write_charreg(0, 59, 2);//↓
										//
										//Lcd_Write_charreg(0, 40, 2);//↓
										//

									
										
										Lcd_Write_strreg(1,0,erjicaidan6_1_2[disptr->LanguageType%maxLanguage]);
										
										
										Datatempint=(disptr->DispDevMonitorSet.SetLoopNum+1)%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 7, Datatemp+0x30);
								

										Datatemp=Datatempint%10;
										
										Lcd_Write_charreg(1, 8, Datatemp+0x30);
										if((devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno<RemoteControl)||(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno==other))
											{

											disptr->LcdRunFlag.Bit.ReadAnOrRiss=0;


											}

										if(!disptr->LcdRunFlag.Bit.ReadAnOrRiss)
											{
										
										Lcd_Write_strreg(1,40,erjicaidan6_1_3[disptr->LanguageType%maxLanguage]);
											}
										else{
										Lcd_Write_strreg(1,40,erjicaidan6_1_4[disptr->LanguageType%maxLanguage]);


											}
										
											
										
										Datatempint=disptr->DispDevMonitorSet.SetDeviceNum+1;
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 46, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 47, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 48, Datatemp+0x30);
										
										
										Datatempint=disptr->DispDevMonitorSet.OnlineDeviceCount[disptr->DispDevMonitorSet.SetLoopNum];
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 37, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 38, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 39, Datatemp+0x30);
										if(!disptr->LcdRunFlag.Bit.ReadAnOrRiss)
											{
										disptr->DispDevMonitorSet.DispLayanalogue=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].TRDATA.Bit.analogue;
										
										
								
										Datatempint=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].TRDATA.Bit.analogue;
											}
										else{

											disptr->DispDevMonitorSet.DispLayRiss=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].Rssi;
										
										
								
											Datatempint=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].Rssi;



											}
										
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 77, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 78, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 79, Datatemp+0x30);
										
										if(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].lineon==0)
											{
											if(disptr->DispDevMonitorSet.SetDeviceNum<125)
												{
										Address=50+(19-typenameno[disptr->LanguageType%maxLanguage][0])/2;
										
										Lcd_Write_strreg(1,Address,typename[disptr->LanguageType%maxLanguage][0]);
												}
											else{
													Address=50+(19-typenameno3[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)])/2;
													Lcd_Write_strreg(1,Address,typenorma[disptr->LanguageType%maxLanguage]);


												}
											}
										else{
										LcdDispLayData.DispDevMonitorSet.DispLayTypeno=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno;	
										if(disptr->DispDevMonitorSet.SetDeviceNum<125)
											{
										Address=50+(19-typenameno[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)])/2;
										Lcd_Write_strreg(1,Address,typename[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)]);
											}
										else{
										Address=50+(19-typenameno3[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)])/2;
										if(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno!=0)
											{
										Lcd_Write_strreg(1,Address,typename3[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)]);
											}
										else{
												Lcd_Write_strreg(1,Address,typenorma[disptr->LanguageType%maxLanguage]);


											}



											}
										//LcdDispLayData.DispDevMonitorSet.DispLayTypeno=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno;
											}
										
										
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
												Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
												Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
												Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

												Lcd_pos(1, 48);
												
												Lcd_CmdWtite(1, 0x0d);
												
											}
									else if((disptr->Setlevel3==4)||(disptr->Setlevel3==5)||(disptr->Setlevel3==6))
										{
										Datatempint=disptr->DispDevMonitorSet.OnlineDeviceCount[disptr->DispDevMonitorSet.SetLoopNum];
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 37, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 38, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 39, Datatemp+0x30);

											
										//disptr->DispDevMonitorSet.DispLayanalogue=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].TRDATA.Bit.analogue;
										
										//Datatempint=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].TRDATA.Bit.analogue;
											if(!disptr->LcdRunFlag.Bit.ReadAnOrRiss)
											{
										disptr->DispDevMonitorSet.DispLayanalogue=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].TRDATA.Bit.analogue;
										
										
								
										Datatempint=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].TRDATA.Bit.analogue;
											}
										else{

											disptr->DispDevMonitorSet.DispLayRiss=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].Rssi;
										
										
								
											Datatempint=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].Rssi;



											}
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 77, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 78, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 79, Datatemp+0x30);
										
									
											for(Datatemp=0;Datatemp<19;Datatemp++)
												{

												Lcd_Write_charreg(1, 49+Datatemp,' ');
													

												}
												if(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].lineon==0)
											{
										if(disptr->DispDevMonitorSet.SetDeviceNum<125)
												{
										Address=50+(19-typenameno[disptr->LanguageType%maxLanguage][0])/2;
										
										Lcd_Write_strreg(1,Address,typename[disptr->LanguageType%maxLanguage][0]);
												}
											else{
													Address=50+(19-typenameno3[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)])/2;
													Lcd_Write_strreg(1,Address,typenorma[disptr->LanguageType%maxLanguage]);


												}
											}
										else{
											LcdDispLayData.DispDevMonitorSet.DispLayTypeno=devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno;
												if(disptr->DispDevMonitorSet.SetDeviceNum<125)
											{
										Address=50+(19-typenameno[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)])/2;
										Lcd_Write_strreg(1,Address,typename[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)]);
											}
										else{
										Address=50+(19-typenameno3[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)])/2;
										if(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno!=0)
											{
										Lcd_Write_strreg(1,Address,typename3[disptr->LanguageType%maxLanguage][(devicedata[disptr->DispDevMonitorSet.SetLoopNum%LoopNum][disptr->DispDevMonitorSet.SetDeviceNum].typeno)]);
											}
										else{
												Lcd_Write_strreg(1,Address,typenorma[disptr->LanguageType%maxLanguage]);


											}



											}
										
											}
										
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
												Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
												Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
												Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
								

												Lcd_pos(1, 48);
												
												Lcd_CmdWtite(1, 0x0d);
												
												disptr->Setlevel3=1;
											}

										
										
									}
							break;
							case 2:
								{
									switch(disptr->Setlevel3)
										{
											case 1:
											case 254:

												
											Lcd_ClearReg();
										
										Lcd_Write_strreg(0,0,erjicaidan6_2[disptr->LanguageType%maxLanguage]);
										

										Lcd_Write_strreg(0,40,erjicaidan6_2_1[disptr->LanguageType%maxLanguage]);
										
										Lcd_Write_charreg(0, 44, 1);//↑
										
										Lcd_Write_charreg(0, 46, 2);//↓
										
										Lcd_Write_strreg(1,0,erjicaidan6_2_2[disptr->LanguageType%maxLanguage][disptr->DispDevMonitorSet.TestSoundmode%3]);
										
										if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[5].B==1))
											{

										//Lcd_Write_strreg(1,52,erjicaidan1_2_3);
										/*
										if((disptr->LanguageType%maxLanguage)==0)
												{	
												Address=52;

												}
											else if((disptr->LanguageType%maxLanguage)==1)
												{	
												Address=49;

												}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
										Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										


											}
										

										
										
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
												Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
												Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
												Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);		

												Lcd_pos(1, 0);
												
												Lcd_CmdWtite(1, 0x0d);
												

											break;
										case 2:
								
										
										Lcd_ClearReg();
										
										Lcd_Write_strreg(0,0,erjicaidan6_2[disptr->LanguageType%maxLanguage]);
										
									/*	if(disptr->LanguageType%maxLanguage==0)
											{
										Address=9;
											}
										else if(disptr->LanguageType%maxLanguage==1){

													Address=5;

											}*/
										charlength=strlen((const char *)&help3[disptr->LanguageType%maxLanguage]);
										Address=(40-charlength)/2;
										Lcd_Write_strreg(1,Address,help3[disptr->LanguageType%maxLanguage]);
										
										
										
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);	
									
											

											break;
										
										}
									}
								
								
							break;
						case 3:
							{
										switch(disptr->Setlevel3)
										{
										case 1:
										Lcd_ClearReg();
										
										Lcd_Write_strreg(0,0,erjicaidan6_3[disptr->LanguageType%maxLanguage]);
										

										Lcd_Write_strreg(0,40,erjicaidan6_3_1[disptr->LanguageType%maxLanguage]);
										
										Lcd_Write_charreg(0, 44, 1);//↑
										
										if(disptr->LanguageType%maxLanguage==0)
											{
										Lcd_Write_charreg(0, 50, 2);//↓
											}
										else if((disptr->LanguageType%maxLanguage==1)||(disptr->LanguageType%maxLanguage==2))
											{
										Lcd_Write_charreg(0, 48, 2);//↓
											}
										
										Lcd_Write_strreg(1,0,erjicaidan6_3_2[disptr->LanguageType%maxLanguage][disptr->DispDevMonitorSet.DispSounderTestAct%3]);
										
										if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[5].C==1))
											{

									
									/*if((disptr->LanguageType%maxLanguage)==0)
												{	
												Address=52;

												}
											else if((disptr->LanguageType%maxLanguage)==1)
												{	
												Address=49;

												}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
										Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										


											}
										

										
										
												Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);		

												Lcd_pos(1, 0);
												
												Lcd_CmdWtite(1, 0x0d);
												

											break;






										}
							}
						break;
						case 4:
								{
										switch(disptr->Setlevel3)
											case 1:
											case 2:
											case 254:
										Lcd_ClearReg();
										
										Lcd_Write_strreg(0,0,erjicaidan6_4[disptr->LanguageType%maxLanguage]);
										
										if(disptr->SetLevelZonechar==0)
											{
										Lcd_Write_strreg(0,40,erjicaidan6_4_1[disptr->LanguageType%maxLanguage]);
										
										if((disptr->LanguageType%maxLanguage==0)||(disptr->LanguageType%maxLanguage==2))
											{
										Lcd_Write_charreg(0, 44, 1);//↑
										
										Lcd_Write_charreg(0, 46, 2);//↓
										
											}
										else if(disptr->LanguageType%maxLanguage==1)
											{
										Lcd_Write_charreg(0, 45, 1);//↑
										
										Lcd_Write_charreg(0, 47, 2);//↓
										
											}
											}
											else if((disptr->SetLevelZonechar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit[disptr->LanguageType%maxLanguage]);	


										}
									else if((disptr->SetLevelZonechar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)zoneedit1[disptr->LanguageType%maxLanguage]);	


										}
									

									
										
										Lcd_Write_strreg(1,0,erjicaidan6_4_2[disptr->LanguageType%maxLanguage]);
										
										
										if(disptr->Setlevel3==254)
											{
										/*	if((disptr->LanguageType%maxLanguage)==0)
												{	
												Address=52;

												}
											else if((disptr->LanguageType%maxLanguage)==1)
												{	
												Address=49;

												}*/

										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
										Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										


											}
										else{
										Lcd_Write_strreg(1,40,erjicaidan6_4_3[disptr->LanguageType%maxLanguage][disptr->DispDevMonitorSet.SetZoneMode]);
										
											}
										
										Datatempint=disptr->DispDevMonitorSet.SetZoneNum+1;
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 7, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 8, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 9, Datatemp+0x30);
										
										
												/*eepAddress=disptr->DispDevMonitorSet.SetZoneNum*18;
												Read_EE1024(2, Eeprom,eepAddress, 18);
												if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
													{
														
													Lcd_Write_strlabelreg(1, 11, (const unsigned char	*)&Eeprom[2],16);
													




													}
													*/
													if((disptr->SetZoneTxt.ZoneTextDisp[0]==0xaa)&&(disptr->SetZoneTxt.ZoneTextDisp[1]==0x55))
													{
														
													Lcd_Write_strlabelreg(1, 11, (const unsigned char	*)&disptr->SetZoneTxt.ZoneTextDisp[2],16);
													




													}

										
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
											Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
											Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
											Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
												if((disptr->Setlevel3==1)||(disptr->Setlevel3==254))
													{
													if(disptr->SetLevelZonechar==0)
														{
												Lcd_pos(1, 9);
												
												Lcd_CmdWtite(1, 0x0d);
														}
													else{
													Lcd_pos(1, (11+disptr->SetZoneTxt.SetZonetxtno));
													Lcd_CmdWtite(1, 0x0d);

													}
												
													}
												else{

												Lcd_pos(1, 40);
												
												Lcd_CmdWtite(1, 0x0d);
												

													}
									


								}
							break;
								case 5:
								{
										switch(disptr->Setlevel3)
											case 1:
											case 2:
											case 254:
										Lcd_ClearReg();
										
										Lcd_Write_strreg(0,0,erjicaidan6_5[disptr->LanguageType%maxLanguage]);
										
										if(disptr->SetLevelDeviceChar==0)
											{
										Lcd_Write_strreg(0,40,erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
										charlength=strlen((const char *)&erjicaidan2_4_6[disptr->LanguageType%maxLanguage]);
										Address=40+charlength;
										Lcd_Write_strreg(0,Address,erjicaidan2_4_1[disptr->LanguageType%maxLanguage]);
										Lcd_Write_charreg(0, Address, 1);
											Address=Address+2;
										Lcd_Write_charreg(0, Address, 2);
											}
												else if((disptr->SetLevelDeviceChar==1)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit[disptr->LanguageType%maxLanguage]);	
						


										}
									else if((disptr->SetLevelDeviceChar==2)){

									Lcd_Write_strreg(0,40,(const unsigned char	*)deviceedit1[disptr->LanguageType%maxLanguage]);	
									


										}
										
										
									

									
										
										Lcd_Write_strreg(1,0,sanjicaidan5_1_1_1[disptr->LanguageType%maxLanguage]);
										Datatempint=(disptr->DispDevMonitorSet.SetLoopNum+1)%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 5, Datatemp+0x30);
								

										Datatemp=Datatempint%10;
										
										Lcd_Write_charreg(1, 6, Datatemp+0x30);
										if((SysTemFlag.Bit.RunLevel==2)&&(PERMIT[5].E==1))
										
											{
											//Lcd_Write_strreg(1,52, erjicaidan1_2_3);
											/*
											if((disptr->LanguageType%maxLanguage)==0)
												{	
												Address=52;

												}
											else if((disptr->LanguageType%maxLanguage)==1)
												{	
												Address=49;

												}*/
										charlength=strlen((const char *)&erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);
										Address=40+(40-charlength)/2;
										Lcd_Write_strreg(1, Address, erjicaidan1_2_3[disptr->LanguageType%maxLanguage]);

												
											}
										
											
												Lcd_Write_strreg(1,42, erjicaidan6_5_3[disptr->LanguageType%maxLanguage][LcdDispLayData.DispDevMonitorSet.LIGHTON%2]);	
												
												
										
									
										
										Lcd_Write_strreg(1,8,erjicaidan6_5_2[disptr->LanguageType%maxLanguage]);
										
										
										Datatempint=disptr->DispDevMonitorSet.SetDeviceNum+1;
										Datatemp=Datatempint/100;
										Lcd_Write_charreg(1, 12, Datatemp+0x30);
										

										Datatempint=Datatempint%100;
										Datatemp=Datatempint/10;
										Lcd_Write_charreg(1, 13, Datatemp+0x30);
										
										
										Datatempint=Datatempint%10;
										Datatemp=Datatempint;
										Lcd_Write_charreg(1, 14, Datatemp+0x30);
										
										
											/*eepAddress=MaxDevice*24*(disptr->DispDevMonitorSet.SetLoopNum%LoopNum)+disptr->DispDevMonitorSet.SetDeviceNum*24;
										Read_EE1024(3, Eeprom,eepAddress, 24);
										if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
											{

											Lcd_Write_strlabelreg(1, 20, (const unsigned char	*)&Eeprom[2], 22);
											}*/

											if((disptr->SetDevicetxt.DeviceTextDisp[0]==0xaa)&&(disptr->SetDevicetxt.DeviceTextDisp[1]==0x55))
											{

											Lcd_Write_strlabelreg(1, 17, (const unsigned char	*)&disptr->SetDevicetxt.DeviceTextDisp[2], 22);
											}
										
											Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
												if(disptr->SetLevelDeviceChar==0)
													{
												Lcd_pos(1, 14);
												
												Lcd_CmdWtite(1, 0x0d);
													}
														else{
												Lcd_pos(1, (17+disptr->SetDevicetxt.SetDevicetxtno));
										
												Lcd_CmdWtite(1, 0x0d);




												}
												
											
									


								}
									break;

								
							}


					break;
					case 7:
						switch(disptr->Setlevel2)
							{
							case 1:
								Lcd_DispLay7_1_X_X(disptr);
								break;
							case 2:
								Lcd_DispLay7_2_X_X(disptr);
								break;
							case 3:
								Lcd_DispLay7_3_X_X(disptr);
								break;
							case 4:
								{
										switch(disptr->Setlevel3)
											case 1:
											case 2:
											case 254:
										Lcd_ClearReg();
										
										
										if(disptr->DispGeneralSet.DisplayNodeCase<2)
											{
										Lcd_Write_strreg(0,0,erjicaidan7_4[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(0,40,erjicaidan7_4_1[disptr->LanguageType%maxLanguage]);
										//老菜单
										//Lcd_Write_charreg(0, 57, 5);
										//Lcd_Write_charreg(0, 58, 3);
										//Lcd_Write_charreg(0, 66, 1);
										//Lcd_Write_charreg(0, 67, 2);
										Lcd_Write_charreg(0, 40, 5);
										Lcd_Write_charreg(0, 42, 3);

										Lcd_Write_charreg(0, 57, 1);
										Lcd_Write_charreg(0, 59, 2);
										
										Lcd_Write_strreg(1,0,erjicaidan7_4_2[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,40,erjicaidan7_4_3[disptr->LanguageType%maxLanguage]);
											wirelessLoopNum=0;
											
										for(i=0;i<LoopNum;i++)
											{

											if(gatewayfitted[i]>0)
												{

													wirelessLoopNum++;		

												}
											if(wirelessLoopNum>=4)
												{
													break;

												}

											}
											Lcd_Write_charreg(1,32,wirelessLoopNum%10+0x30);

											for(i=disptr->DispGeneralSet.DisplayWireLessLoop;i<(LoopNum+disptr->DispGeneralSet.DisplayWireLessLoop+1);i++)
												{

													if(gatewayfitted[i%LoopNum]>0)
														{

														disptr->DispGeneralSet.DisplayWireLessLoop=i%LoopNum;
														break;
														}


												}
												Lcd_Write_charreg(1,18,(disptr->DispGeneralSet.DisplayWireLessLoop+1)/10+0x30);
												Lcd_Write_charreg(1,19,(disptr->DispGeneralSet.DisplayWireLessLoop+1)%10+0x30);

													NodeNum=0;
												for(i=0;i<32;i++)
													{

														if(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][i].GatewayFitted)
															{

																NodeNum++;

															}




													}
												Lcd_Write_charreg(1,64,(NodeNum)/10+0x30);
												Lcd_Write_charreg(1,65,(NodeNum)%10+0x30);

												for(i=disptr->DispGeneralSet.DisplayWireLessNode;i<32+disptr->DispGeneralSet.DisplayWireLessNode+1;i++)
													{

														if(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][i%32].GatewayFitted)
															{
																

																				disptr->DispGeneralSet.DisplayWireLessNode=i%32;
																				break;

															}

													}
												//Lcd_Write_charreg(1,50,(disptr->DispGeneralSet.DisplayWireLessNode+1)/10+0x30);
												//Lcd_Write_charreg(1,51,(disptr->DispGeneralSet.DisplayWireLessNode+1)%10+0x30);
												Lcd_Write_charreg(1,50,(disptr->DispGeneralSet.DisplayWireLessNode)/10+0x30);
												Lcd_Write_charreg(1,51,(disptr->DispGeneralSet.DisplayWireLessNode)%10+0x30);

												Lcd_Write_charreg(1,78,(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceCount)/10+0x30);
												Lcd_Write_charreg(1,79,(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceCount)%10+0x30);
												
										
											}
										else{

											
											if((disptr->DispGeneralSet.DisplayNodeCase==2)||(disptr->DispGeneralSet.DisplayNodeCase==4))
												{
										Lcd_Write_strreg(0,0,erjicaidan7_4_1[disptr->LanguageType%maxLanguage]);
										Lcd_Write_charreg(0, 0, 5);
										Lcd_Write_charreg(0, 2, 3);
										Lcd_Write_charreg(0, 17, 1);
										Lcd_Write_charreg(0, 19, 2);
												}
											else if((disptr->DispGeneralSet.DisplayNodeCase==3)||(disptr->DispGeneralSet.DisplayNodeCase==5))
												{

										Lcd_Write_strreg(0,0,erjicaidan7_4_5[disptr->LanguageType%maxLanguage]);
										Lcd_Write_charreg(0, 0, 1);
										Lcd_Write_charreg(0, 1, 2);

												}
										
										Lcd_Write_strreg(0,40,erjicaidan7_4_2[disptr->LanguageType%maxLanguage]);
										Lcd_Write_strreg(1,0,erjicaidan7_4_3[disptr->LanguageType%maxLanguage]);
										if(disptr->DispGeneralSet.DisplayNodeCase<4)
											{
										Lcd_Write_strreg(1,40,erjicaidan7_4_4[disptr->LanguageType%maxLanguage]);
											}
										else{

											

										Lcd_Write_strreg(1,40,erjicaidan7_4_6[disptr->LanguageType%maxLanguage]);
											}
										
											wirelessLoopNum=0;
											
										for(i=0;i<LoopNum;i++)
											{

											if(gatewayfitted[i]>0)
												{

													wirelessLoopNum++;		

												}
											if(wirelessLoopNum>=4)
												{
													break;

												}

											}
											Lcd_Write_charreg(0,72,wirelessLoopNum%10+0x30);

											for(i=disptr->DispGeneralSet.DisplayWireLessLoop;i<(LoopNum+disptr->DispGeneralSet.DisplayWireLessLoop+1);i++)
												{

													if(gatewayfitted[i%LoopNum]>0)
														{

														disptr->DispGeneralSet.DisplayWireLessLoop=i%LoopNum;
														break;
														}


												}
												Lcd_Write_charreg(0,58,(disptr->DispGeneralSet.DisplayWireLessLoop+1)/10+0x30);
												Lcd_Write_charreg(0,59,(disptr->DispGeneralSet.DisplayWireLessLoop+1)%10+0x30);

													NodeNum=0;
												for(i=0;i<32;i++)
													{

														if(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][i].GatewayFitted)
															{

																NodeNum++;

															}




													}
												Lcd_Write_charreg(1,24,(NodeNum)/10+0x30);
												Lcd_Write_charreg(1,25,(NodeNum)%10+0x30);

												for(i=disptr->DispGeneralSet.DisplayWireLessNode;i<32+disptr->DispGeneralSet.DisplayWireLessNode+1;i++)
													{

														if(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][i%32].GatewayFitted)
															{
																

																				disptr->DispGeneralSet.DisplayWireLessNode=i%32;
																				if(disptr->DispGeneralSet.DisplayNodeCase<4)
																					{
																				disptr->DispGeneralSet.DisplayWireLessCh=GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][i%32].BaseFreq;
																					}
																				break;

															}

													}
												Lcd_Write_charreg(1,10,(disptr->DispGeneralSet.DisplayWireLessNode)/10+0x30);
												Lcd_Write_charreg(1,11,(disptr->DispGeneralSet.DisplayWireLessNode)%10+0x30);

												Lcd_Write_charreg(1,38,(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceCount)/10+0x30);
												Lcd_Write_charreg(1,39,(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceCount)%10+0x30);

												
												if(disptr->DispGeneralSet.DisplayNodeCase<4)
											{
												for(i=disptr->DispGeneralSet.DisplayDeviceAdd;i<32+disptr->DispGeneralSet.DisplayDeviceAdd+1;i++)
													{

														if((GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceType[i%32]>0)&&(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceAddress[i%32]))
															{
																

																				disptr->DispGeneralSet.DisplayDeviceAdd=i%32;
																				break;

															}

													}
												Datatempint=GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceAddress[disptr->DispGeneralSet.DisplayDeviceAdd];
												Datatemp=Datatempint/100;
												
												Lcd_Write_charreg(1,53,Datatemp+0x30);
												Datatempint=Datatempint%100;
												Datatemp=Datatempint/10;
												Lcd_Write_charreg(1,54,Datatemp+0x30);
												Datatemp=Datatempint%10;
												Lcd_Write_charreg(1,55,Datatemp+0x30);


												Datatemp=GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].DeviceType[disptr->DispGeneralSet.DisplayDeviceAdd];

												
												if(Datatemp==1)
													{
														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][1]);

													}
												else if(Datatemp==2)
													{
														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][2]);

													}
												else if(Datatemp==3)
													{
														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][3]);

													}
													else if(Datatemp==12)
													{
														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][4]);

													}
													else if(Datatemp==15)
													{
														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][5]);

													}
													else if(Datatemp==16)
													{
														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][6]);

													}
													else{

														Lcd_Write_strreg(1,62, typenodedevice[disptr->LanguageType%maxLanguage][0]);

														}
													if(disptr->DispGeneralSet.ToRemoved==0)
														{
													Lcd_Write_charreg(1,79,'N');
														}
													else{

													Lcd_Write_charreg(1,79,'Y');	

														}
													}
												else{


																if(!GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].BridgeStatus)
																	{


																		Lcd_Write_strreg(0,0,erjicaidan7_4_1_1[disptr->LanguageType%maxLanguage]);
																		Lcd_Write_charreg(0, 0, 5);
																		Lcd_Write_charreg(0, 2, 3);
																		Lcd_Write_strreg(1,40,erjicaidan7_4_6_1[disptr->LanguageType%maxLanguage]);



																	}

														
															

																Datatemp1=disptr->DispGeneralSet.DisplayWireLessCh;
																Datatemp=Datatemp1/100;
																Lcd_Write_charreg(1,58,Datatemp+0x30);

																Datatemp1=Datatemp1%100;
																Datatemp=Datatemp1/10;
																Lcd_Write_charreg(1,59,Datatemp+0x30);
																

																Datatemp=Datatemp1%10;
																Lcd_Write_charreg(1,60,Datatemp+0x30);
													if(GateWayLoopData[disptr->DispGeneralSet.DisplayWireLessLoop][disptr->DispGeneralSet.DisplayWireLessNode].BridgeStatus)
														{

													if(disptr->DispGeneralSet.ToChangeWireLessCh==0)
														{
													Lcd_Write_charreg(1,72,'N');
														}
													else{

													Lcd_Write_charreg(1,72,'Y');	

														}
														}




													}
													
												
										
											}











											
										
										Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
										Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
										Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
										Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
										if(disptr->DispGeneralSet.DisplayNodeCase==0)
											{

											Lcd_pos(1, 19);
											
											Lcd_CmdWtite(1, 0x0d);


											}
											else if(disptr->DispGeneralSet.DisplayNodeCase==1)
											{

											Lcd_pos(1, 51);
											
											Lcd_CmdWtite(1, 0x0d);


											}
												else if(disptr->DispGeneralSet.DisplayNodeCase==2)
											{

											Lcd_pos(1, 55);
											
											Lcd_CmdWtite(1, 0x0d);


											}
											else if(disptr->DispGeneralSet.DisplayNodeCase==3)
											{

											Lcd_pos(1, 79);
											
											Lcd_CmdWtite(1, 0x0d);


											}
											else if(disptr->DispGeneralSet.DisplayNodeCase==4)
											{

											Lcd_pos(1, 60);
											
											Lcd_CmdWtite(1, 0x0d);


											}
											else if(disptr->DispGeneralSet.DisplayNodeCase==5)
											{

											Lcd_pos(1, 72);
											
											Lcd_CmdWtite(1, 0x0d);


											}
									//break;
												


											
											
									


								}
								
								






								break;

							}
							break;
						
					case 8:
							switch(disptr->Setlevel2)
						{
					case 1:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan8_1);
									

									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan8_1_1);
									
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan8_1_2);
									
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan8_1_3);
									
									if(TestKeyNum==9)
										{
									Lcd_Write_charreg(1,49,1);


										}
									else{
									Lcd_Write_strreg(1,49,erjicaidan8_1_4[TestKeyNum]);
										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
								
						break;
					case 2:
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan8_2);
									

									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan8_2_1[0]);
									
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan8_2_1[1]);
									
									Lcd_Write_strreg(1,30,(const unsigned char	*)erjicaidan8_2_2[0]);
									
									Lcd_Write_strreg(1,36,(const unsigned char	*)erjicaidan8_2_3[testRelaySounder.Fault]);
									
									Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan8_2_2[1]);
									
									Lcd_Write_strreg(1,46,(const unsigned char	*)erjicaidan8_2_3[testRelaySounder.Fire1]);
									
									Lcd_Write_strreg(1,50,(const unsigned char	*)erjicaidan8_2_2[2]);
									
									Lcd_Write_strreg(1,56,(const unsigned char	*)erjicaidan8_2_3[testRelaySounder.Fire2]);
									
									Lcd_Write_strreg(1,60,(const unsigned char	*)erjicaidan8_2_2[3]);
									
									Lcd_Write_strreg(1,63,(const unsigned char	*)erjicaidan8_2_3[testRelaySounder.S1]);
									
									Lcd_Write_strreg(1,67,(const unsigned char	*)erjicaidan8_2_2[4]);
									
									Lcd_Write_strreg(1,70,(const unsigned char	*)erjicaidan8_2_3[testRelaySounder.S2]);
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									
									
						break;
					case 3:
						
									Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan8_3);
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan8_3_2);
									Lcd_Write_charreg(0,44,disptr->DispManfacture.SetLoopNum%LoopNum+0x31);
													Datatempint=disptr->DispManfacture.ADaveragenoload[disptr->DispManfacture.SetLoopNum%LoopNum];
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(0, 50, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(0, 51, Datatemp+0x30);
											
													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(0, 52, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(0,53, Datatemp+0x30);	
													Datatempint=disptr->DispManfacture.ADaveragewithload[disptr->DispManfacture.SetLoopNum%LoopNum];
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(0, 59, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(0, 60, Datatemp+0x30);
											
													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(0, 61, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(0,62, Datatemp+0x30);			
													Lcd_Write_strreg(0,65,erjicaidan8_3_1[2]);
													Datatempint=disptr->DispManfacture.ADmainpower;
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(0, 76, Datatemp+0x30);
													

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(0, 77, Datatemp+0x30);
											
													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(0, 78, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(0,79, Datatemp+0x30);
													
									
													Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan8_3_3);
														Datatempint=disptr->DispManfacture.AD20mAValue[disptr->DispManfacture.SetLoopNum%LoopNum];
													
													//Datatempint=Datatempint%10000;
													
													//Datatemp=Datatempint/1000;
													//Datatempint=Datatempint%1000;
													//Lcd_Write_charreg(1, 10, Datatemp+0x30);
													Datatempint=Datatempint%1000;

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 5, Datatemp+0x30);
											
													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 6, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,7, Datatemp+0x30);	

													Datatempint=disptr->DispManfacture.ADLowValue[disptr->DispManfacture.SetLoopNum%LoopNum];
													
													//Datatempint=Datatempint%10000;
													
													//Datatemp=Datatempint/1000;
													//Datatempint=Datatempint%1000;
													//Lcd_Write_charreg(1, 19, Datatemp+0x30);
													Datatempint=Datatempint%1000;

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 12, Datatemp+0x30);
											
													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 13, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,14, Datatemp+0x30);
													
													Datatempint=disptr->DispManfacture.ADHighValue[disptr->DispManfacture.SetLoopNum%LoopNum];
													
												//	Datatempint=Datatempint%10000;
													
												//	Datatemp=Datatempint/1000;
												//	Datatempint=Datatempint%1000;
												//	Lcd_Write_charreg(1, 19, Datatemp+0x30);
													Datatempint=Datatempint%1000;

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 19, Datatemp+0x30);
											
													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1,20, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,21, Datatemp+0x30);
													
													Lcd_Write_strreg(1,25,erjicaidan8_3_1[3]);
													Datatempint=disptr->DispManfacture.ADbattery;
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 33, Datatemp+0x30);
												

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 34, Datatemp+0x30);
												

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 35, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,36, Datatemp+0x30);
													
													Lcd_Write_strreg(1,40,erjicaidan8_3_1[4]);
									
													Datatempint=disptr->DispManfacture.ADbatcarg;
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 48, Datatemp+0x30);
												

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 49, Datatemp+0x30);
												

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 50, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,51, Datatemp+0x30);
													Lcd_Write_strreg(1,54,erjicaidan8_3_1[5]);
													Datatempint=disptr->DispManfacture.ADearth;
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 60, Datatemp+0x30);
												

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 61, Datatemp+0x30);
												

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 62,Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,63, Datatemp+0x30);
													Lcd_Write_strreg(1,65,erjicaidan8_3_1[6]);
													Datatempint=disptr->DispManfacture.ADbell1;
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 68, Datatemp+0x30);
												

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1, 69, Datatemp+0x30);
												

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 70,Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,71, Datatemp+0x30);
													Lcd_Write_strreg(1,73,erjicaidan8_3_1[7]);
													Datatempint=disptr->DispManfacture.ADbell2;
													
													Datatempint=Datatempint%10000;
													
													Datatemp=Datatempint/1000;
													Datatempint=Datatempint%1000;
													Lcd_Write_charreg(1, 76, Datatemp+0x30);
												

													Datatemp=Datatempint/100;
													Datatempint=Datatempint%100;
													Lcd_Write_charreg(1,77, Datatemp+0x30);
												

													Datatemp=Datatempint/10;
													Datatempint=Datatempint%10;
													Lcd_Write_charreg(1, 78, Datatemp+0x30);
												

													Datatemp=Datatempint;
												
													Lcd_Write_charreg(1,79, Datatemp+0x30);
													Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);
									Lcd_pos(0, 44);
									Lcd_CmdWtite(0, 0x0d);
					break;
					case 4:
								Lcd_ClearReg();
									
									Lcd_Write_strreg(0,0,(const unsigned char	*)erjicaidan8_4[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(0,40,(const unsigned char	*)erjicaidan8_4_1[disptr->LanguageType%maxLanguage]);
									Lcd_Write_strreg(1,0,(const unsigned char	*)erjicaidan8_4_2[disptr->LanguageType%maxLanguage]);
									 Lcd_Write_strreg(1,40,(const unsigned char	*)erjicaidan8_4_3[disptr->LanguageType%maxLanguage][disptr->DispManfacture.gsmconnectstatus%2]);
									if(disptr->DispManfacture.imeidisplaycase==1)
										{
									Lcd_Write_strreg(1,6,(const unsigned char	*)erjicaidan8_4_4[disptr->LanguageType%maxLanguage]);

										}
									else if(disptr->DispManfacture.imeidisplaycase==2)
										{

									Lcd_Write_strreg(1,6,(const unsigned char	*)(&GsmDataPar.EMIE[0]));


										}
									else if(disptr->DispManfacture.imeidisplaycase==3)
										{

									Lcd_Write_strreg(1,6,(const unsigned char	*)erjicaidan8_4_5[disptr->LanguageType%maxLanguage]);


										}
									Lcd_Write_str(0,0, (const unsigned char *)&pinlcddata[0][0]);
									Lcd_Write_str(0,40, (const unsigned char *)&pinlcddata[1][0]);
									Lcd_Write_str(1,0, (const unsigned char *)&pinlcddata[2][0]);
									Lcd_Write_str(1,40, (const unsigned char *)&pinlcddata[3][0]);

					break;
									

















				}
						break;
					
			






			
						
		}




		







}








}

unsigned char			Lcd_DispLay1_1_1tm(lcdstruct	*dispptr,logD *logdptr)
{			
				static unsigned char	Datatemp;
				static uint16_t		Datatempint;
				static unsigned char	Address;
				static unsigned char	Endaddress;
				
				
								
							//	Lcd_ClearReg();
							/*if(dispptr->LanguageType%maxLanguage==0)
								{
								Address=44;

								}
							else if(dispptr->LanguageType%maxLanguage==1)
								{
								Address=40;

								}*/
							Address=40+((40-strlen((const char	*)erjicaidan1_1_1_1[dispptr->LanguageType%maxLanguage][0])-15)/2);
								Lcd_Write_strreg(1, Address, (const unsigned char	*)&erjicaidan1_1_1_1[dispptr->LanguageType%maxLanguage][0]);
									Endaddress=Address+strlen((const char	*)erjicaidan1_1_1_1[dispptr->LanguageType%maxLanguage][0]);
								Datatemp=logdptr->chardata.RecordNum/10000;
								Datatempint=logdptr->chardata.RecordNum%10000;
								Address=Endaddress-8;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);

								
								//Datatemp=logdptr->chardata.RecordNum/1000;
								//Datatempint=logdptr->chardata.RecordNum%1000;
								Datatemp=Datatempint/1000;
								Datatempint=Datatempint%1000;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=Datatempint/100;
								Datatempint=Datatempint%100;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=Datatempint/10;
								Datatempint=Datatempint%10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=Datatempint;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
							Endaddress=Address;
								Address=Address+6;
								Datatemp=logdptr->chardata.RecordHour/10;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=logdptr->chardata.RecordHour%10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								Address++;
								Lcd_Write_charreg(1, Address,':');
								Datatemp=logdptr->chardata.RecordMin/10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=logdptr->chardata.RecordMin%10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								

								Datatemp=logdptr->chardata.RecordDay/10;
								Address=Address+2;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=logdptr->chardata.RecordDay%10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								Address++;
								
								Lcd_Write_charreg(1, Address,'-');
								

								Datatemp=logdptr->chardata.RecordMonth/10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								Datatemp=logdptr->chardata.RecordMonth%10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
							Address++;
								Lcd_Write_charreg(1, Address,'-');
							

								
								Datatempint=logdptr->chardata.RecordYear;
								Datatemp=Datatempint/10;
								Datatempint=Datatempint%10;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
							
								Datatemp=Datatempint;
								Address++;
								Lcd_Write_charreg(1, Address,Datatemp+0x30);
								
								//Lcd_pos(1, 57);
								
								//Lcd_CmdWtite(1, 0x0d);
								
								
								
								return	Endaddress;




}

void		DispLayLog(lcdstruct	* dispptr,	logD	 * logdptr)
{
			static unsigned long	address;
			static uint16_t	i;
								address=(unsigned long)((dispptr->NowDispNum-1)%MaxLogNum)*OneLogLength;
								
							Read_EE1024(1,Eeprom, address, OneLogLength);

								for(i=0;i<OneLogLength;i++)
									{
									logdptr->data[i]=Eeprom[i];


									}
								Logparse(dispptr,logdptr);//日志解析
								//显示部分



}
void		Logparse(lcdstruct	* dispptr,logD	* logdptr)
{
				static uint16_t	Datatemp;
				static uint16_t		Datatempint;
				static unsigned char	Address,charlength;
			//	static unsigned long	eepAddress;
	switch(logdptr->chardata.RecordType)
		{
			case  PowerReset:
			case   ResetSYS:
					Lcd_ClearReg();
					
					Lcd_Write_strreg(0, 41, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
					Datatempint=logdptr->chardata.RecordThisNum;
					Datatemp=Datatempint/1000;
					Datatempint=Datatempint%1000;
					Lcd_Write_charreg(0, 75,Datatemp+0x30);
					
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 76,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 77,Datatemp+0x30);
					

					Datatemp=Datatempint;
					
					Lcd_Write_charreg(0, 78,Datatemp+0x30);
					

					
					
				break;
			case  InterKeypad:
					Lcd_ClearReg();
					
					Lcd_Write_strreg(0, 40, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
				
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+charlength;
					if(logdptr->chardata.RecordRunLevel==2)
						{
						Lcd_Write_charreg(0, Address, '2');


						}
					else if(logdptr->chardata.RecordRunLevel==3){

								Lcd_Write_charreg(0, Address, '3');


						}
						else if(logdptr->chardata.RecordRunLevel==4){

								Lcd_Write_charreg(0, Address, '4');


						}
				
					
				break;
			case AlarmOff:
					Lcd_ClearReg();

					
					/*if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=52;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=47;
						}*/
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
				
					break;
			case AlarmOn:
				Lcd_ClearReg();
				
					/*if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=53;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=49;
						}*/
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
					break;
			case AlarmSilence:
				Lcd_ClearReg();
					
				/*	if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=53;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=52;
						}*/
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
					break;
			case InterBuzSilence:
					Lcd_ClearReg();
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address,(const unsigned char	*) &history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
				
				break;
			case OutKeypad1:
				Lcd_ClearReg();
					
					/*if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=43;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=46;
						}*/
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
				break;
			case OutKeypad2:
				Lcd_ClearReg();
					
					/*if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=43;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=46;
						}*/
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
				break;
			case ClearAutoStartCount:
				Lcd_ClearReg();
					
				/*	if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=44;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=43;
						}*/
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength-5)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
					
					Datatempint=logdptr->chardata.RecordThisNum;
					Datatemp=Datatempint/1000;
					Datatempint=Datatempint%1000;
					Address=Address+charlength+2;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Address++;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Address++;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					
					Datatemp=Datatempint;
					Address++;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
				break;
			case  DisabledZone:
			case ENabledZone:
					Lcd_ClearReg();
				
					Lcd_Write_strreg(0, 0, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=charlength+2;
					Datatempint=logdptr->chardata.RecordZone;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Address++;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Address++;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					
					break;
			case  DISABLEDConvS:
			case  ENABLEDConvS:
					Lcd_ClearReg();
					
					Lcd_Write_strreg(0, 0, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						
						Datatempint=logdptr->chardata.RecordDev;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 26,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 27,Datatemp+0x30);

					Datatemp=logdptr->chardata.RecordDev;

					if(Datatemp>0)
						{

					Datatemp--;
						}
					Datatempint=(Datatemp/2)+1;

					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 35,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 36,Datatemp+0x30);

					
				
				
					break;
			case  DISABLEDLoopS:
			case ENABLEDLoopS:
						Lcd_ClearReg();
					
					Lcd_Write_strreg(0, 0, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
					Datatempint=logdptr->chardata.RecordDev;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 26,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 27,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 28,Datatemp+0x30);
					
				

					Datatempint=logdptr->chardata.RecordLoop;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 35,Datatemp+0x30);
					
					Datatemp=Datatempint;
					
					Lcd_Write_charreg(0, 36,Datatemp+0x30);
					
					break;
			case  DISABLEDLOOP:
			case ENABLEDLOOP:
						Lcd_ClearReg();
				
					Lcd_Write_strreg(0, 0, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Datatempint=logdptr->chardata.RecordLoop;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Address=charlength+1;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
				
					Datatemp=Datatempint;
					Address++;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					
					
					break;

			case  DISABLEDdev:
			case ENABLEDdev:
						Lcd_ClearReg();
					
					Lcd_Write_strreg(0, 0, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					
					Datatempint=logdptr->chardata.RecordDev;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 22,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 23,Datatemp+0x30);
				
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 24,Datatemp+0x30);
					

					Datatempint=logdptr->chardata.RecordLoop;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 27,Datatemp+0x30);
					
					Datatemp=Datatempint;
					
					Lcd_Write_charreg(0, 28,Datatemp+0x30);
					
					break;
			case DISABLEDFireRelay:
			case ENABLEDFireRelay:
						Lcd_ClearReg();
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					/*
					if(logdptr->chardata.RecordType==ENABLEDFireRelay)
						{
						
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						}
					else{

					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);

						}*/
					
					break;
			case DISABLEDFaultRelay:
			case ENABLEDFaultRelay:
						Lcd_ClearReg();
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-27]);
					Address=40+(40-charlength)/2;
					if(logdptr->chardata.RecordType==ENABLEDFaultRelay)
						{
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-27]);
						}
					else{

					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-27]);

						}
					
					break;
			case DISABLEDSounder:
			case ENABLEDSounder:
						Lcd_ClearReg();
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					Address=40+(40-charlength)/2;
					if(logDispdata.chardata.RecordType==ENABLEDSounder)
						{
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						}
					else{

					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);

						}
					
				
					break;
			case DISABLEDDelay:
			case ENABLEDDelay:
						Lcd_ClearReg();
						charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						Address=40+(40-charlength)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					/*
					if(logdptr->chardata.RecordType==ENABLEDDelay)
						{
						if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=47;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=44;
						}
						
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						}
					else{
						if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=49;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=43;
						}

					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);

						}
						*/
					
				
					break;
			case DISABLEDseldect:
			case ENABLEDseldect:
						Lcd_ClearReg();
					charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						/*
					if(logdptr->chardata.RecordType==ENABLEDseldect)
						{
							if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=45;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=43;
						}
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						}
					else{
						if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=45;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=42;
						}
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);

						}*/

						
		
					break;
			case ActiveModeSEL:
			case InstallModeSEL:
						Lcd_ClearReg();
							charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						Address=40+(40-charlength)/2;
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
					/*
					if(logdptr->chardata.RecordType==InstallModeSEL)
						{
						if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=47;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=46;
						}
					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						}
					else{ 
						if(dispptr->LanguageType%maxLanguage==0)
						{

						Address=50;
						}
					else if(dispptr->LanguageType%maxLanguage==1)
						{

						Address=49;
						}

					Lcd_Write_strreg(0, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);

						}*/

					
					
						
					break;
			case Fireevent:
						Lcd_ClearReg();
						
						Lcd_Write_strreg(0, 0, (const unsigned char	*)&firedisp[dispptr->LanguageType%maxLanguage][0]);
						
						
					Datatempint=logdptr->chardata.RecordThisNum;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 5,Datatemp+0x30);
				
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 6,Datatemp+0x30);
				
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 7,Datatemp+0x30);
				

					if(logdptr->chardata.RecordZone>0)
						{

						Lcd_Write_strreg(0, 9, (const unsigned char	*)&firedisp[dispptr->LanguageType%maxLanguage][1]);
						
						
					Datatempint=logdptr->chardata.RecordZone;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 14,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 15,Datatemp+0x30);
				
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 16,Datatemp+0x30);
				
							//eepAddress=((logdptr->chardata.RecordZone-1)%ZoneNum)*18;
							//		Read_EE1024(2, Eeprom,eepAddress, 18);//zone 第二片
							readzonetext(((logdptr->chardata.RecordZone-1)%ZoneNum), &Eeprom[0]);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(0, 40, (const unsigned char	*)&Eeprom[2], 16);
										}
					


						}
					
						Lcd_Write_strreg(0, 18, (const unsigned char	*)&firedisp[dispptr->LanguageType%maxLanguage][2]);
						
						
					Datatempint=logdptr->chardata.RecordDev;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 22,Datatemp+0x30);
					
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 23,Datatemp+0x30);
				
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 24,Datatemp+0x30);
					

					Lcd_Write_charreg(0, 26,'L');
					

					
					
					Datatempint=logdptr->chardata.RecordLoop%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 27,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 28,Datatemp+0x30);
					
					if(logdptr->chardata.RecordDev<126)
						{
					Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename1[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordDevType%31]);//有bug
						}
					else{

					Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename2[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordDevType%31]);//有bug

						}
				

							/*eepAddress=MaxDevice*24*((logdptr->chardata.RecordLoop-1)%LoopNum)+((logdptr->chardata.RecordDev-1)%MaxDevice)*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);*/
									readdevicetext(((logDispdata.chardata.RecordLoop-1)%LoopNum), ((logDispdata.chardata.RecordDev-1)%MaxDevice), &Eeprom[0]);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(0, 57, (const unsigned char	*)&Eeprom[2], 22);
										}
			
						break;
			case FaultLoopdev:
			case PERALARM:
			case loopDuplicateAddress:
			case loopdevideremove:
			case LoopWrongDeviceFitted:
			case TESTevent:
						Lcd_ClearReg();
						
						if((logdptr->chardata.RecordType==FaultLoopdev)||(logdptr->chardata.RecordType==loopDuplicateAddress)||(logdptr->chardata.RecordType==loopdevideremove)||(logdptr->chardata.RecordType==LoopWrongDeviceFitted))
							{
						Lcd_Write_strreg(0, 0, (const unsigned char	*)&faultloopdisp[dispptr->LanguageType%maxLanguage][0]);
							}
						else if(logdptr->chardata.RecordType==PERALARM)
							{
						Lcd_Write_strreg(0, 0, (const unsigned char	*)&peralarmdisp[dispptr->LanguageType%maxLanguage][0]);


							}
						else if(logdptr->chardata.RecordType==TESTevent)
							{

								Lcd_Write_strreg(0, 0, (const unsigned char	*)&testdisp[dispptr->LanguageType%maxLanguage][0]);

							}
					if(logdptr->chardata.RecordDevType!=Wireleesmodule)
							{
					if(logdptr->chardata.RecordZone>0)
						{

						Lcd_Write_strreg(0, 9, (const unsigned char	*)&faultloopdisp[dispptr->LanguageType%maxLanguage][1]);
					
					Datatempint=logdptr->chardata.RecordZone;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 14,Datatemp+0x30);
				
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 15,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 16,Datatemp+0x30);
					
						//eepAddress=((logdptr->chardata.RecordZone-1)%ZoneNum)*18;
						//			Read_EE1024(2, Eeprom,eepAddress, 18);
						
						readzonetext(((logdptr->chardata.RecordZone-1)%ZoneNum), &Eeprom[0]);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(0, 40, (const unsigned char	*)&Eeprom[2], 16);
										}



						}
						}
					
						Lcd_Write_strreg(0, 18, (const unsigned char	*)&faultloopdisp[dispptr->LanguageType%maxLanguage][2]);
					
						
					Datatempint=logdptr->chardata.RecordDev%1000;
					Datatemp=Datatempint/100;
					Datatempint=Datatempint%100;
					Lcd_Write_charreg(0, 22,Datatemp+0x30);
				
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 23,Datatemp+0x30);
				
					Datatemp=Datatempint;
					Lcd_Write_charreg(0, 24,Datatemp+0x30);
						

					Lcd_Write_charreg(0, 26,'L');
				

					Datatempint=logdptr->chardata.RecordLoop%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, 27,Datatemp+0x30);
					
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, 28,Datatemp+0x30);
				
					if(logdptr->chardata.RecordDev<126)
						{
					Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename1[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordDevType%31]);
						}
					else{
						Lcd_Write_strreg(0, 30, (const unsigned char	*)&typename2[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordDevType%31]);


						}
					
					if(logdptr->chardata.RecordType==loopDuplicateAddress)
						{
								/*if(dispptr->LanguageType%maxLanguage==0)
									{
										Address=7;

									}
								else if(dispptr->LanguageType%maxLanguage==1)
									{
										Address=2;

									}*/
										charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						Address=(40-charlength)/2;

							Lcd_Write_strreg(1, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);


						}
					//-----------------------------------
					else if(logdptr->chardata.RecordType==loopdevideremove)
						{
								/*if(dispptr->LanguageType%maxLanguage==0)
									{
										Address=13;

									}
								else if(dispptr->LanguageType%maxLanguage==1)
									{
										Address=9;

									}*/
								
							charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						Address=(40-charlength)/2;
							Lcd_Write_strreg(1, Address, (const unsigned char	*)&history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);



						}
					else if(logdptr->chardata.RecordType==LoopWrongDeviceFitted)
						{
															/*if(dispptr->LanguageType%maxLanguage==0)
																	{
																	Address=11;

																	}
															else if(dispptr->LanguageType%maxLanguage==1)
																	{
																	Address=9;

																		}*/
							charlength=strlen((const char	*)history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);
						Address=(40-charlength)/2;

							Lcd_Write_strreg(1, Address,(const unsigned char	*) &history[dispptr->LanguageType%maxLanguage][logdptr->chardata.RecordType-1]);



						}
					
				/*	eepAddress=MaxDevice*24*((logdptr->chardata.RecordLoop-1)%LoopNum)+((logdptr->chardata.RecordDev-1)%MaxDevice)*24;
									Read_EE1024(3, Eeprom,eepAddress, 24);*/
									if(logdptr->chardata.RecordType!=Wireleesmodule)
										{
						readdevicetext(((logDispdata.chardata.RecordLoop-1)%LoopNum), ((logDispdata.chardata.RecordDev-1)%MaxDevice), &Eeprom[0]);
									if((Eeprom[0]==0xaa)&&(Eeprom[1]==0x55))
										{

										Lcd_Write_strlabelreg(0, 57, (const unsigned char	*)&Eeprom[2], 22);
										}
										}
						break;
			case ConvenSFault:
			case RepeaterConvenSFault:
						Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==ConvenSFault)
							{
					/*	if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
						charlength=strlen((const char	*)mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);

							//Datatempint=logDispdata.chardata.RecordLoop;
						Datatempint=logDispdata.chardata.RecordDev+1;//主机编号
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						/*
						if(dispptr->LanguageType%maxLanguage==0)
							{

								Address=66;
							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{

								Address=69;
							}*/
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						/*
						if(dispptr->LanguageType%maxLanguage==0)
							{

								Address=67;
							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{

								Address=70;
							}*/
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

							}
						else if(logdptr->chardata.RecordType==RepeaterConvenSFault)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char	*)Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						//Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

							}
						
						if(dispptr->LanguageType%maxLanguage==0)
							{
						Lcd_Write_strreg(1, 2, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][1]);
						

						Lcd_Write_charreg(1, 31,(logdptr->chardata.RecordLoop%3)+0x30);
					
							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
						Lcd_Write_strreg(1, 1, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][1]);
						

						Lcd_Write_charreg(1, 38,(logdptr->chardata.RecordLoop%3)+0x30);
							}
						else if(dispptr->LanguageType%maxLanguage==2)
							{
						Lcd_Write_strreg(1, 4, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][1]);
						

						Lcd_Write_charreg(1, 34,(logdptr->chardata.RecordLoop%3)+0x30);
				
							}
						break;
		    case EARTHFAULT:
		    case	RepeaterEarthfault:
						Lcd_ClearReg();
						
							if(logdptr->chardata.RecordType==EARTHFAULT)
							{
						/*
							if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}
							*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logdptr->chardata.RecordType==RepeaterEarthfault)
							{
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
						/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=9;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=14;

							}
							*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][4]);
						Address=(40-charlength-3)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][4]);
						

						break;
			

			case LoopPowerFault:
						Lcd_ClearReg();
						
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}
						
						Lcd_Write_str(0, Address,(const unsigned char	*) &mainpanelfault[dispptr->LanguageType%maxLanguage][0]);*/
						/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}*/
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=12;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=10;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][5]);
						Address=(40-charlength-3)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][5]);
						
				
						break;
			case convpowerfault:
			case Repeaterconvpowerfault:
							Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==convpowerfault)
							{
							/*
						if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=Address+charlength+1;
						//Address=Address+12;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;

							}
						else if(logdptr->chardata.RecordType==Repeaterconvpowerfault)
							{
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address,(const unsigned char	*) &Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+1+charlength;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

							}
						
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=1;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=0;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][6]);
						Address=(40-charlength-3)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][6]);
						
						break;
			case aux1powerFault:
			case RepeaterAUX1powerFault:
							Lcd_ClearReg();
						
							if(logdptr->chardata.RecordType==aux1powerFault)
							{
						
						/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=logDispdata.chardata.RecordDev%100;*/
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logdptr->chardata.RecordType==RepeaterAUX1powerFault)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=3;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=0;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][7]);
						Address=(40-charlength-3)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][7]);
					
						break;
				
			case loopshortopen:

						Lcd_ClearReg();
						
						/*	if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=50;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=49;

							}*/
						/*	if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}*/
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);	
						Address=40+(40-charlength-3)/2+charlength+1;

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
					/*
					if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=6;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=13;

							}
					Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][3]);
					
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=33;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=28;

							}
					Lcd_Write_charreg(1, Address,(logdptr->chardata.RecordLoop/10)+0x30);
					Address++;
					Lcd_Write_charreg(1, Address,(logdptr->chardata.RecordLoop%10)+0x30);
					*/
					Address=strlen((const char *)mainpanelfault[dispptr->LanguageType%maxLanguage][3]);
					Address=(40-(Address+3))/2;
					/*if(disptr->LanguageType%maxLanguage==0)
							{
										Address=6;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=13;

							}*/
					Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][3]);
						Address=Address+strlen((const char *)mainpanelfault[dispptr->LanguageType%maxLanguage][3])+1;
						/*	if(disptr->LanguageType%maxLanguage==0)
							{
										Address=33;

							}
						else if(disptr->LanguageType%maxLanguage==1)
							{
										Address=28;

							}*/
					Lcd_Write_charreg(1, Address,(logdptr->chardata.RecordLoop/10)+0x30);
					Address++;
					Lcd_Write_charreg(1, Address,(logdptr->chardata.RecordLoop%10)+0x30);
					
					break;
			case mainpowerfault:
			case Repeaterpowerfault:
						Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==mainpowerfault)
							{
							/*
						if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);*/
						/*
							if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}
							*/
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);

							}
						else if(logdptr->chardata.RecordType==Repeaterpowerfault)
							{
						/*	if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						Address=(40-strlen((const char *)Repeaterlab[dispptr->LanguageType%maxLanguage])-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+strlen((const char *)Repeaterlab[dispptr->LanguageType%maxLanguage])+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

							}
						
						/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=7;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=5;

							}*/
						Address=(40-strlen((const char	*)mainpanelfault[dispptr->LanguageType%maxLanguage][8]))/2;
						Lcd_Write_strreg(1, Address,(const unsigned char	*) &mainpanelfault[dispptr->LanguageType%maxLanguage][8]);
					
						break;
			case   batterypowerfault:
			case   RepeaterBatFaul:
						Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==batterypowerfault)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}
						Lcd_Write_str(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);*/
							/*if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}*/
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							
						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logdptr->chardata.RecordType==RepeaterBatFaul)
							{
					/*	if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2+charlength+1;
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
						/*	if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=9;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=4;

							}*/
						Address=(40-strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][9]))/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][9]);
						
						break;
			case  batterychargerfault:
			case Repeaterbatterychargerfault:
						Lcd_ClearReg();
						
						
							if(logdptr->chardata.RecordType==batterychargerfault)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}
						Lcd_Write_str(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);*/
						/*
							if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}*/
								charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);

						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logdptr->chardata.RecordType==Repeaterbatterychargerfault)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2+charlength+1;
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=4;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=1;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][10]);
						Address=(40-charlength-3)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][10]);
					
						break;
			case  CustomerCheckSumError:
			case RepeaterCustomerCheckSumError:
						Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==CustomerCheckSumError)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/



						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;

						
						Lcd_Write_strreg(0, Address,(const unsigned char	*) &mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
					//	Address=Address+12;
					Address=Address+charlength+1;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
							}
						else if(logdptr->chardata.RecordType==RepeaterCustomerCheckSumError)
							{
							
							/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
								charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						
						/*	if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=6;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=12;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][12]);
						Address=(40-charlength-3)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][12]);
						
						break;
		
			case  ProCheckSumError:
			case RepeaterProCheckSumError:
						Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==ProCheckSumError)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=55;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=52;

							}*/
										charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
								//	Address=Address+12;
								Address=Address+charlength+1;
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
							}
						else if(logdptr->chardata.RecordType==RepeaterProCheckSumError)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][11]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][11]);
						
						break;
				case  mainpowerremove:
				case Repeatermainpowerremove:
						
						Lcd_ClearReg();
						
						if(logdptr->chardata.RecordType==mainpowerremove)
							{
							
						/*if(logDispdata.chardata.RecordDev==0)
							{
							charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
							}
						else {

						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][2]);

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2+charlength+1;
						

					Datatempint=(logDispdata.chardata.RecordDev+1)%100;
					Datatemp=Datatempint/10;
					Datatempint=Datatempint%10;
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
						Address++;
					Datatemp=Datatempint;
					Datatempint=Datatempint%10;	
					Lcd_Write_charreg(0, Address,Datatemp+0x30);
							}
						else if(logDispdata.chardata.RecordType==Repeatermainpowerremove)
							{
							/*
							if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
							}


						
						/*if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=2;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=0;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][13]);
						Address=(40-charlength)/2;
						Lcd_Write_strreg(1, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][13]);
						
						break;
				case  RepeatercomFault:
						Lcd_ClearReg();
						/*
						if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=52;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=51;

							}*/
						charlength=strlen((const char *)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&Repeaterlab[dispptr->LanguageType%maxLanguage]);
						Datatempint=logdptr->chardata.RecordLoop;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						charlength=strlen((const char *)&RepComFault[dispptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;

						Lcd_Write_strreg(1, Address, (const unsigned char	*)&RepComFault[dispptr->LanguageType%maxLanguage]);
						break;
				case SUBPANELcomFault:
					Lcd_ClearReg();
					/*
						if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=50;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=49;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=Address+charlength+1;
						Datatempint=logdptr->chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Address++;
						Datatemp=Datatempint%10;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);

						charlength=strlen((const char *)&RepComFault[dispptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;

						Lcd_Write_strreg(1, Address, (const unsigned char	*)&RepComFault[dispptr->LanguageType%maxLanguage]);
						break;
					case NetWorkComFault:

							Lcd_ClearReg();
							/*
						if(dispptr->LanguageType%maxLanguage==0)
							{
										Address=50;

							}
						else if(dispptr->LanguageType%maxLanguage==1)
							{
										Address=45;

							}*/
						charlength=strlen((const char *)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Address=40+(40-charlength-3)/2;
						
						Lcd_Write_strreg(0, Address, (const unsigned char	*)&mainpanelfault[dispptr->LanguageType%maxLanguage][0]);
						Datatempint=logDispdata.chardata.RecordDev+1;
						Datatempint=Datatempint%100;
						Datatemp=Datatempint/10;
						Address=Address+charlength+1;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						Datatemp=Datatempint%10;
						Address++;
						Lcd_Write_charreg(0, Address, Datatemp+0x30);
						charlength=strlen((const char *)&NetworkComFault[dispptr->LanguageType%maxLanguage]);
						Address=(40-charlength)/2;

						Lcd_Write_strreg(1, Address, (const unsigned char	*)&NetworkComFault[dispptr->LanguageType%maxLanguage]);

						break;
					
						

		}








}

void		LcdinputReturnTimeCacl(unsigned long	*timereset,lcdstruct *lcdptr)
{
			static unsigned long	DeltaTime;
		if(lcdptr->dispcase1==1)
			{

			DeltaTime=5000;
			}
		else if(lcdptr->dispcase1==2){
				DeltaTime=15000;

			}
		if((lcdptr->dispcase1==1)||(lcdptr->dispcase1==2))
			{
		if(*timereset>HAL_GetTick())
			{
				*timereset=HAL_GetTick();


			}
		else if((HAL_GetTick()-*timereset)>DeltaTime)
			{

						 lcdptr->displayfault1 = 0;
         				  	 lcdptr->displayfire1 = 0;
           					 *timereset = HAL_GetTick();
          					 lcdptr->dispfaulteventnum = 0;
           					 lcdptr->dispfireeventnum = 0;

						lcdptr->dispcase1=11;
						lcdptr->LcdRunFlag.Bit.DispLayLcd=1;
						lcdptr->LcdRunFlag.Bit.DispLayLcdrx=1;
						SysTemFlag.Bit.ToEnterCode=0;




			}
			}
		else{

				*timereset=HAL_GetTick();

			}







}
unsigned long	lcdbacktime;
void		LcdBackLightTimeCacl(void)
{
	
	if(lcdbacktime>HAL_GetTick())
		{

		lcdbacktime=HAL_GetTick();

		}

	if(LcdDispLayData.LcdRunFlag.Bit.LcdBackLight)
		{

			if((HAL_GetTick()-lcdbacktime)>LCDONTime)
				{

				LcdDispLayData.LcdRunFlag.Bit.LcdBackLight=0;	
				lcdbacktime=HAL_GetTick();


				}
		


		}
	else{
		lcdbacktime=HAL_GetTick();




		}






}
extern	unsigned long	DispTestTime;
extern	unsigned long delayquedisable_time;
extern	unsigned long delayquetest_time;
extern	unsigned long faultquedisable_time;
extern	unsigned long	firequedisable_time;
extern	unsigned long	KdelayTime;
void		lcddispcal(lcdstruct * disptr)
{
static unsigned long	time7;
 if ((disptr->Setlevel1 == 8)&&(disptr->Setlevel2 == 3)&&(disptr->Setlevel3 == 1)&&(SysTemFlag.Bit.RunLevel == 4)) 
 	{

	if(time7>HAL_GetTick())
		{

		time7=HAL_GetTick();

		}
     else if((HAL_GetTick()-time7)>=1000)
		{
       	    time7=HAL_GetTick();
            disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;


     	   }
    }

	   if ((LastMin != DS1302TIME.systemmin)&&(disptr->dispcase1 == 11))
	   	{
    		    if ((disptr->FaultEventNum == 0)&&(disptr->FireEventNum == 0)&&((HAL_GetTick()-DispTestTime)>15000)&&(!SysTemFlag.Bit.CaretakerTest)) 
				{
       		  disptr->LcdRunFlag.Bit.DispLayLcd = 1;
			 disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;
		

      			  }
 		   }

	 if(disptr->Setlevel1==0)
	 	{
			/*if(DispTestTime>HAL_GetTick())
				{
				DispTestTime=HAL_GetTick();


				}*/
			if(((HAL_GetTick()-DispTestTime)>15000)&&(HAL_GetTick()>DispTestTime))
				{

						     delayquedisable_time =HAL_GetTick();
      	  					disptr->dispcase1 = 11;
        					disptr->displayfault1 = 0;
        					disptr->displayfire1 = 0;
        					disptr->dispfaulteventnum = 0;
        					disptr->dispfireeventnum = 0;
			
       				 if (disptr->FireEventNum == 0) {
           					 disptr->dispcase3 = 1;


      					  } 
					 else {

           				 disptr->dispcase3 = 2;

      					  }
        disptr->LcdRunFlag.Bit.DispLayLcd = 1;
	 disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;
      	//  DispTestTime =HAL_GetTick()+20000;
      	DispTestTime=0xffffffff-14999;


				}


	 	}
	 else{

			//DispTestTime=HAL_GetTick();
			DispTestTime=0xffffffff-14999;

	 	}
    if (disptr->dispcase1 == 12) {

      
		 if(((HAL_GetTick()-delayquedisable_time)>15000)&&(HAL_GetTick()>delayquedisable_time)) {

            delayquedisable_time = HAL_GetTick();
            disptr->dispcase1 = 11;
            disptr->displayfault1 = 0;
            disptr->displayfire1 = 0;
            disptr->dispfaulteventnum = 0;
            disptr->dispfireeventnum = 0;
            if (disptr->FireEventNum == 0) {
                disptr->dispcase3 = 1;


            } else {

                disptr->dispcase3 = 2;

            }
             disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		 disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;

        }


    }
	else{
			  delayquedisable_time = HAL_GetTick();

		}
    if (disptr->dispcase1 == 13) {

	 if(((HAL_GetTick()-delayquetest_time)>15000)&&(HAL_GetTick()>delayquetest_time)) {

            delayquetest_time = HAL_GetTick();
            disptr->dispcase1 = 11;
            disptr->displayfault1 = 0;
           disptr->displayfire1 = 0;
            disptr->dispfaulteventnum = 0;
            disptr->dispfireeventnum = 0;
            if (disptr->FireEventNum == 0) {
                disptr->dispcase3 = 1;


            } else {

                disptr->dispcase3 = 2;

            }
             disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		 disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;

        }


    }
	else{
		 delayquetest_time = HAL_GetTick();

		}
    if (disptr->dispcase1 == 11) {

        if (disptr->dispcase3 == 1) {

	 
          if(((HAL_GetTick()-faultquedisable_time)>20000)&&(HAL_GetTick()>faultquedisable_time)) {
                faultquedisable_time = 0xffffffff;
                disptr->dispfaulteventnum = 0;
                disptr->dispfireeventnum = 0;

                if (disptr->FireEventNum == 0) {

                    disptr->displayfault1 = 0;
                } else {

                    disptr->dispcase3 = 2;
                    disptr->displayfire1 = 0;

                }
                 disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		 disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;
            }


        } else {
           
              if (((HAL_GetTick()-firequedisable_time)>20000)&&(HAL_GetTick()>firequedisable_time)) {
                disptr->dispfaulteventnum = 0;
                disptr->dispfireeventnum = 0;
                firequedisable_time = 0xffffffff;
               disptr->dispcase3 = 2;
               disptr->displayfire1 = 0;
                 disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		 disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;


            }



        }

    }
    if (disptr->dispcase1 != 11) {
        firequedisable_time = HAL_GetTick();
        faultquedisable_time = HAL_GetTick();


    }

   if (disptr->lastfaultDisp >HAL_GetTick())//?í?ó???????±?????????ü???í?ó????
    {
       disptr->lastfaultDisp=HAL_GetTick();

    }
   else if((HAL_GetTick()-disptr->lastfaultDisp)>=1000)
   	{
    if ((disptr->dispcase1 == 11)&&(disptr->faultnodisp >= 1)&&(disptr->dispcase3 == 1)&&((HAL_GetTick()-DispTestTime )> 15000)&&(disptr->faultnodisp != disptr->FaultEventNum))
		{
        disptr->LcdRunFlag.Bit.DispLayLcd = 1;
	disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;
        disptr->faultnodisp = disptr->FaultEventNum;


  		  }

   	}
	  if (SysTemFlag.Bit.ToEnterCode) {
	
       	 if (KdelayTime >HAL_GetTick())
				{

            KdelayTime = HAL_GetTick();

     		   } 
		 else if((HAL_GetTick()-KdelayTime)>15000){
            SysTemFlag.Bit.ToEnterCode = 0;
            SysTemFlag.Bit.ToEnterSuper = 0;
         	 disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;
            disptr->dispcase1 = 0;
            disptr->dispcase2 = 0;
            KdelayTime = HAL_GetTick();


        }
       else  if ((KeyCodeNum > 0)&&((HAL_GetTick()-KdelayTime)>15000)){

            disptr->LcdRunFlag.Bit.DispLayLcd = 1;
		disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;
            disptr->dispcase1 = 0;
            disptr->dispcase2 = 0;
            SysTemFlag.Bit.ToEnterCode = 0;
            KdelayTime = 0;
            SysTemFlag.Bit.ToEnterSuper = 0;

        }

    }
	  else {

        KeyCodeNum = 0;
        KdelayTime =  HAL_GetTick();

   	 }


}
void		delay1(unsigned long i)
{
	static volatile  unsigned  long j,k;
	for(j=0;j<i;j++){for(k=0;k<4;k++);}
	
		

}
void		GetSoundStatustoShuzu1_Conv(consnden	* p, unsigned char * Statustemp)
{

	
			
		Statustemp[0]=p->ConSounder0;
		Statustemp[1]=p->ConSounder1;
		Statustemp[2]=p->ConSounder2;
		Statustemp[3]=p->ConSounder3;
		Statustemp[4]=p->ConSounder4;
		Statustemp[5]=p->ConSounder5;
		Statustemp[6]=p->ConSounder6;
		Statustemp[7]=p->ConSounder7;
		Statustemp[8]=p->ConSounder8;
		Statustemp[9]=p->ConSounder9;
		Statustemp[10]=p->ConSounder10;
		Statustemp[11]=p->ConSounder11;
		Statustemp[12]=p->ConSounder12;
		Statustemp[13]=p->ConSounder13;
		Statustemp[14]=p->ConSounder14;
		Statustemp[15]=p->ConSounder15;
		Statustemp[16]=p->ConSounder16;
		Statustemp[17]=p->ConSounder17;
		Statustemp[18]=p->ConSounder18;
		Statustemp[19]=p->ConSounder19;
		Statustemp[20]=p->ConSounder20;
		Statustemp[21]=p->ConSounder21;
		Statustemp[22]=p->ConSounder22;
		Statustemp[23]=p->ConSounder23;
	







}
void		GetSoundStatustoShuzu1_1(loopsnden* p, unsigned char * Statustemp)
{

			static uint16_t	i;
	i=0;
			
		Statustemp[2+i*32+22]=p->LoopSounder0;
		Statustemp[3+i*32+22]=p->LoopSounder1;
		Statustemp[4+i*32+22]=p->LoopSounder2;
		Statustemp[5+i*32+22]=p->LoopSounder3;
		Statustemp[6+i*32+22]=p->LoopSounder4;
		Statustemp[7+i*32+22]=p->LoopSounder5;
		Statustemp[8+i*32+22]=p->LoopSounder6;
		Statustemp[9+i*32+22]=p->LoopSounder7;
		Statustemp[10+i*32+22]=p->LoopSounder8;
		Statustemp[11+i*32+22]=p->LoopSounder9;
		Statustemp[12+i*32+22]=p->LoopSounder10;
		Statustemp[13+i*32+22]=p->LoopSounder11;
		Statustemp[14+i*32+22]=p->LoopSounder12;
		Statustemp[15+i*32+22]=p->LoopSounder13;
		Statustemp[16+i*32+22]=p->LoopSounder14;
		Statustemp[17+i*32+22]=p->LoopSounder15;
		Statustemp[18+i*32+22]=p->LoopSounder16;
		Statustemp[19+i*32+22]=p->LoopSounder17;
		Statustemp[20+i*32+22]=p->LoopSounder18;
		Statustemp[21+i*32+22]=p->LoopSounder19;
		Statustemp[22+i*32+22]=p->LoopSounder20;
		Statustemp[23+i*32+22]=p->LoopSounder21;
		Statustemp[24+i*32+22]=p->LoopSounder22;
		Statustemp[25+i*32+22]=p->LoopSounder23;
		Statustemp[26+i*32+22]=p->LoopSounder24;
		Statustemp[27+i*32+22]=p->LoopSounder25;
		Statustemp[28+i*32+22]=p->LoopSounder26;
		Statustemp[29+i*32+22]=p->LoopSounder27;
		Statustemp[30+i*32+22]=p->LoopSounder28;
		Statustemp[31+i*32+22]=p->LoopSounder29;
		Statustemp[32+i*32+22]=p->LoopSounder30;
		Statustemp[33+i*32+22]=p->LoopSounder31;









}
void		PutSoundStatustobitfeild_Conv(consndgr	* p, unsigned char * Statustemp)
{

		

		p->ConSounder0=Statustemp[0];
		p->ConSounder1=Statustemp[1];
		p->ConSounder2=Statustemp[2];
		p->ConSounder3=Statustemp[3];
		p->ConSounder4=Statustemp[4];
		p->ConSounder5=Statustemp[5];
		p->ConSounder6=Statustemp[6];
		p->ConSounder7=Statustemp[7];
		p->ConSounder8=Statustemp[8];
		p->ConSounder9=Statustemp[9];
		p->ConSounder10=Statustemp[10];
		p->ConSounder11=Statustemp[11];
		p->ConSounder12=Statustemp[12];
		p->ConSounder13=Statustemp[13];
		p->ConSounder14=Statustemp[14];
		p->ConSounder15=Statustemp[15];
		p->ConSounder16=Statustemp[16];
		p->ConSounder17=Statustemp[17];
		p->ConSounder18=Statustemp[18];
		p->ConSounder19=Statustemp[19];
		p->ConSounder20=Statustemp[20];
		p->ConSounder21=Statustemp[21];
		p->ConSounder22=Statustemp[22];
		p->ConSounder23=Statustemp[23];
		

		

		




}
void		PutSoundStatustobitfeild_1(loopsndgr		* p, unsigned char * Statustemp)
{

			static uint16_t	i;
			i=0;
			
		p->LoopSounder0=Statustemp[2+i*32+22];
		p->LoopSounder1=Statustemp[3+i*32+22];
		p->LoopSounder2=Statustemp[4+i*32+22];
		p->LoopSounder3=Statustemp[5+i*32+22];
		p->LoopSounder4=Statustemp[6+i*32+22];
		p->LoopSounder5=Statustemp[7+i*32+22];
		p->LoopSounder6=Statustemp[8+i*32+22];
		p->LoopSounder7=Statustemp[9+i*32+22];
		p->LoopSounder8=Statustemp[10+i*32+22];
		p->LoopSounder9=Statustemp[11+i*32+22];
		p->LoopSounder10=Statustemp[12+i*32+22];
		p->LoopSounder11=Statustemp[13+i*32+22];
		p->LoopSounder12=Statustemp[14+i*32+22];
		p->LoopSounder13=Statustemp[15+i*32+22];
		p->LoopSounder14=Statustemp[16+i*32+22];
		p->LoopSounder15=Statustemp[17+i*32+22];
		p->LoopSounder16=Statustemp[18+i*32+22];
		p->LoopSounder17=Statustemp[19+i*32+22];
		p->LoopSounder18=Statustemp[20+i*32+22];
		p->LoopSounder19=Statustemp[21+i*32+22];
		p->LoopSounder20=Statustemp[22+i*32+22];
		p->LoopSounder21=Statustemp[23+i*32+22];
		p->LoopSounder22=Statustemp[24+i*32+22];
		p->LoopSounder23=Statustemp[25+i*32+22];
		p->LoopSounder24=Statustemp[26+i*32+22];
		p->LoopSounder25=Statustemp[27+i*32+22];
		p->LoopSounder26=Statustemp[28+i*32+22];
		p->LoopSounder27=Statustemp[29+i*32+22];
		p->LoopSounder28=Statustemp[30+i*32+22];
		p->LoopSounder29=Statustemp[31+i*32+22];
		p->LoopSounder30=Statustemp[32+i*32+22];
		p->LoopSounder31=Statustemp[33+i*32+22];









}
void		GetSoundStatustoShuzu(soundgr *p,unsigned char	*Statustemp)
{		
		static uint16_t	i;
		//Statustemp[0]=p->ConSounder0;
	//	Statustemp[1]=p->ConSounder1;

		Statustemp[0]=p->ConSounder.ConSounder0;
		Statustemp[1]=p->ConSounder.ConSounder1;
		Statustemp[2]=p->ConSounder.ConSounder2;
		Statustemp[3]=p->ConSounder.ConSounder3;
		Statustemp[4]=p->ConSounder.ConSounder4;
		Statustemp[5]=p->ConSounder.ConSounder5;
		Statustemp[6]=p->ConSounder.ConSounder6;
		Statustemp[7]=p->ConSounder.ConSounder7;
		Statustemp[8]=p->ConSounder.ConSounder8;
		Statustemp[9]=p->ConSounder.ConSounder9;
		Statustemp[10]=p->ConSounder.ConSounder10;
		Statustemp[11]=p->ConSounder.ConSounder11;
		Statustemp[12]=p->ConSounder.ConSounder12;
		Statustemp[13]=p->ConSounder.ConSounder13;
		Statustemp[14]=p->ConSounder.ConSounder14;
		Statustemp[15]=p->ConSounder.ConSounder15;
		Statustemp[16]=p->ConSounder.ConSounder16;
		Statustemp[17]=p->ConSounder.ConSounder17;
		Statustemp[18]=p->ConSounder.ConSounder18;
		Statustemp[19]=p->ConSounder.ConSounder19;
		Statustemp[20]=p->ConSounder.ConSounder20;
		Statustemp[21]=p->ConSounder.ConSounder21;
		Statustemp[22]=p->ConSounder.ConSounder22;
		Statustemp[23]=p->ConSounder.ConSounder23;
		for(i=0;i<LoopNum;i++)
			{
		Statustemp[2+i*32+22]=p->LoopSounder[i].LoopSounder0;
		Statustemp[3+i*32+22]=p->LoopSounder[i].LoopSounder1;
		Statustemp[4+i*32+22]=p->LoopSounder[i].LoopSounder2;
		Statustemp[5+i*32+22]=p->LoopSounder[i].LoopSounder3;
		Statustemp[6+i*32+22]=p->LoopSounder[i].LoopSounder4;
		Statustemp[7+i*32+22]=p->LoopSounder[i].LoopSounder5;
		Statustemp[8+i*32+22]=p->LoopSounder[i].LoopSounder6;
		Statustemp[9+i*32+22]=p->LoopSounder[i].LoopSounder7;
		Statustemp[10+i*32+22]=p->LoopSounder[i].LoopSounder8;
		Statustemp[11+i*32+22]=p->LoopSounder[i].LoopSounder9;
		Statustemp[12+i*32+22]=p->LoopSounder[i].LoopSounder10;
		Statustemp[13+i*32+22]=p->LoopSounder[i].LoopSounder11;
		Statustemp[14+i*32+22]=p->LoopSounder[i].LoopSounder12;
		Statustemp[15+i*32+22]=p->LoopSounder[i].LoopSounder13;
		Statustemp[16+i*32+22]=p->LoopSounder[i].LoopSounder14;
		Statustemp[17+i*32+22]=p->LoopSounder[i].LoopSounder15;
		Statustemp[18+i*32+22]=p->LoopSounder[i].LoopSounder16;
		Statustemp[19+i*32+22]=p->LoopSounder[i].LoopSounder17;
		Statustemp[20+i*32+22]=p->LoopSounder[i].LoopSounder18;
		Statustemp[21+i*32+22]=p->LoopSounder[i].LoopSounder19;
		Statustemp[22+i*32+22]=p->LoopSounder[i].LoopSounder20;
		Statustemp[23+i*32+22]=p->LoopSounder[i].LoopSounder21;
		Statustemp[24+i*32+22]=p->LoopSounder[i].LoopSounder22;
		Statustemp[25+i*32+22]=p->LoopSounder[i].LoopSounder23;
		Statustemp[26+i*32+22]=p->LoopSounder[i].LoopSounder24;
		Statustemp[27+i*32+22]=p->LoopSounder[i].LoopSounder25;
		Statustemp[28+i*32+22]=p->LoopSounder[i].LoopSounder26;
		Statustemp[29+i*32+22]=p->LoopSounder[i].LoopSounder27;
		Statustemp[30+i*32+22]=p->LoopSounder[i].LoopSounder28;
		Statustemp[31+i*32+22]=p->LoopSounder[i].LoopSounder29;
		Statustemp[32+i*32+22]=p->LoopSounder[i].LoopSounder30;
		Statustemp[33+i*32+22]=p->LoopSounder[i].LoopSounder31;
			}
		/*
		Statustemp[34]=p->LoopSounder32;
		Statustemp[35]=p->LoopSounder33;
		Statustemp[36]=p->LoopSounder34;
		Statustemp[37]=p->LoopSounder35;
		Statustemp[38]=p->LoopSounder36;
		Statustemp[39]=p->LoopSounder37;
		Statustemp[40]=p->LoopSounder38;
		Statustemp[41]=p->LoopSounder39;
		Statustemp[42]=p->LoopSounder40;
		Statustemp[43]=p->LoopSounder41;
		Statustemp[44]=p->LoopSounder42;
		Statustemp[45]=p->LoopSounder43;
		Statustemp[46]=p->LoopSounder44;
		Statustemp[47]=p->LoopSounder45;
		Statustemp[48]=p->LoopSounder46;
		Statustemp[49]=p->LoopSounder47;
		Statustemp[50]=p->LoopSounder48;
		Statustemp[51]=p->LoopSounder49;
		Statustemp[52]=p->LoopSounder50;
		Statustemp[53]=p->LoopSounder51;
		Statustemp[54]=p->LoopSounder52;
		Statustemp[55]=p->LoopSounder53;
		Statustemp[56]=p->LoopSounder54;
		Statustemp[57]=p->LoopSounder55;
		Statustemp[58]=p->LoopSounder56;
		Statustemp[59]=p->LoopSounder57;
		Statustemp[60]=p->LoopSounder58;
		Statustemp[61]=p->LoopSounder59;
		Statustemp[62]=p->LoopSounder60;
		Statustemp[63]=p->LoopSounder61;
		Statustemp[64]=p->LoopSounder62;
		Statustemp[65]=p->LoopSounder63;
		*/
		










}

void		GetSoundStatustoShuzu_Conv(consndgr	* p, unsigned char * Statustemp)
{

		Statustemp[0]=p->ConSounder0;
		Statustemp[1]=p->ConSounder1;
		Statustemp[2]=p->ConSounder2;
		Statustemp[3]=p->ConSounder3;
		Statustemp[4]=p->ConSounder4;
		Statustemp[5]=p->ConSounder5;
		Statustemp[6]=p->ConSounder6;
		Statustemp[7]=p->ConSounder7;
		Statustemp[8]=p->ConSounder8;
		Statustemp[9]=p->ConSounder9;
		Statustemp[10]=p->ConSounder10;
		Statustemp[11]=p->ConSounder11;
		Statustemp[12]=p->ConSounder12;
		Statustemp[13]=p->ConSounder13;
		Statustemp[14]=p->ConSounder14;
		Statustemp[15]=p->ConSounder15;
		Statustemp[16]=p->ConSounder16;
		Statustemp[17]=p->ConSounder17;
		Statustemp[18]=p->ConSounder18;
		Statustemp[19]=p->ConSounder19;
		Statustemp[20]=p->ConSounder20;
		Statustemp[21]=p->ConSounder21;
		Statustemp[22]=p->ConSounder22;
		Statustemp[23]=p->ConSounder23;
		





}
void		GetSoundStatustoShuzu_1(loopsndgr * p, unsigned char * Statustemp)
{

			static uint16_t	i;
			i=0;
		
		Statustemp[2+i*32+22]=p->LoopSounder0;
		Statustemp[3+i*32+22]=p->LoopSounder1;
		Statustemp[4+i*32+22]=p->LoopSounder2;
		Statustemp[5+i*32+22]=p->LoopSounder3;
		Statustemp[6+i*32+22]=p->LoopSounder4;
		Statustemp[7+i*32+22]=p->LoopSounder5;
		Statustemp[8+i*32+22]=p->LoopSounder6;
		Statustemp[9+i*32+22]=p->LoopSounder7;
		Statustemp[10+i*32+22]=p->LoopSounder8;
		Statustemp[11+i*32+22]=p->LoopSounder9;
		Statustemp[12+i*32+22]=p->LoopSounder10;
		Statustemp[13+i*32+22]=p->LoopSounder11;
		Statustemp[14+i*32+22]=p->LoopSounder12;
		Statustemp[15+i*32+22]=p->LoopSounder13;
		Statustemp[16+i*32+22]=p->LoopSounder14;
		Statustemp[17+i*32+22]=p->LoopSounder15;
		Statustemp[18+i*32+22]=p->LoopSounder16;
		Statustemp[19+i*32+22]=p->LoopSounder17;
		Statustemp[20+i*32+22]=p->LoopSounder18;
		Statustemp[21+i*32+22]=p->LoopSounder19;
		Statustemp[22+i*32+22]=p->LoopSounder20;
		Statustemp[23+i*32+22]=p->LoopSounder21;
		Statustemp[24+i*32+22]=p->LoopSounder22;
		Statustemp[25+i*32+22]=p->LoopSounder23;
		Statustemp[26+i*32+22]=p->LoopSounder24;
		Statustemp[27+i*32+22]=p->LoopSounder25;
		Statustemp[28+i*32+22]=p->LoopSounder26;
		Statustemp[29+i*32+22]=p->LoopSounder27;
		Statustemp[30+i*32+22]=p->LoopSounder28;
		Statustemp[31+i*32+22]=p->LoopSounder29;
		Statustemp[32+i*32+22]=p->LoopSounder30;
		Statustemp[33+i*32+22]=p->LoopSounder31;









}
void		GetSoundStatustoShuzu1(sounden *p,unsigned char	*Statustemp)
{		
		static uint16_t	i;
		Statustemp[0]=p->ConSounder.ConSounder0;
		Statustemp[1]=p->ConSounder.ConSounder1;
		Statustemp[2]=p->ConSounder.ConSounder2;
		Statustemp[3]=p->ConSounder.ConSounder3;
		Statustemp[4]=p->ConSounder.ConSounder4;
		Statustemp[5]=p->ConSounder.ConSounder5;
		Statustemp[6]=p->ConSounder.ConSounder6;
		Statustemp[7]=p->ConSounder.ConSounder7;
		Statustemp[8]=p->ConSounder.ConSounder8;
		Statustemp[9]=p->ConSounder.ConSounder9;
		Statustemp[10]=p->ConSounder.ConSounder10;
		Statustemp[11]=p->ConSounder.ConSounder11;
		Statustemp[12]=p->ConSounder.ConSounder12;
		Statustemp[13]=p->ConSounder.ConSounder13;
		Statustemp[14]=p->ConSounder.ConSounder14;
		Statustemp[15]=p->ConSounder.ConSounder15;
		Statustemp[16]=p->ConSounder.ConSounder16;
		Statustemp[17]=p->ConSounder.ConSounder17;
		Statustemp[18]=p->ConSounder.ConSounder18;
		Statustemp[19]=p->ConSounder.ConSounder19;
		Statustemp[20]=p->ConSounder.ConSounder20;
		Statustemp[21]=p->ConSounder.ConSounder21;
		Statustemp[22]=p->ConSounder.ConSounder22;
		Statustemp[23]=p->ConSounder.ConSounder23;
		for(i=0;i<LoopNum;i++)
			{
		Statustemp[2+i*32+22]=p->LoopSounder[i].LoopSounder0;
		Statustemp[3+i*32+22]=p->LoopSounder[i].LoopSounder1;
		Statustemp[4+i*32+22]=p->LoopSounder[i].LoopSounder2;
		Statustemp[5+i*32+22]=p->LoopSounder[i].LoopSounder3;
		Statustemp[6+i*32+22]=p->LoopSounder[i].LoopSounder4;
		Statustemp[7+i*32+22]=p->LoopSounder[i].LoopSounder5;
		Statustemp[8+i*32+22]=p->LoopSounder[i].LoopSounder6;
		Statustemp[9+i*32+22]=p->LoopSounder[i].LoopSounder7;
		Statustemp[10+i*32+22]=p->LoopSounder[i].LoopSounder8;
		Statustemp[11+i*32+22]=p->LoopSounder[i].LoopSounder9;
		Statustemp[12+i*32+22]=p->LoopSounder[i].LoopSounder10;
		Statustemp[13+i*32+22]=p->LoopSounder[i].LoopSounder11;
		Statustemp[14+i*32+22]=p->LoopSounder[i].LoopSounder12;
		Statustemp[15+i*32+22]=p->LoopSounder[i].LoopSounder13;
		Statustemp[16+i*32+22]=p->LoopSounder[i].LoopSounder14;
		Statustemp[17+i*32+22]=p->LoopSounder[i].LoopSounder15;
		Statustemp[18+i*32+22]=p->LoopSounder[i].LoopSounder16;
		Statustemp[19+i*32+22]=p->LoopSounder[i].LoopSounder17;
		Statustemp[20+i*32+22]=p->LoopSounder[i].LoopSounder18;
		Statustemp[21+i*32+22]=p->LoopSounder[i].LoopSounder19;
		Statustemp[22+i*32+22]=p->LoopSounder[i].LoopSounder20;
		Statustemp[23+i*32+22]=p->LoopSounder[i].LoopSounder21;
		Statustemp[24+i*32+22]=p->LoopSounder[i].LoopSounder22;
		Statustemp[25+i*32+22]=p->LoopSounder[i].LoopSounder23;
		Statustemp[26+i*32+22]=p->LoopSounder[i].LoopSounder24;
		Statustemp[27+i*32+22]=p->LoopSounder[i].LoopSounder25;
		Statustemp[28+i*32+22]=p->LoopSounder[i].LoopSounder26;
		Statustemp[29+i*32+22]=p->LoopSounder[i].LoopSounder27;
		Statustemp[30+i*32+22]=p->LoopSounder[i].LoopSounder28;
		Statustemp[31+i*32+22]=p->LoopSounder[i].LoopSounder29;
		Statustemp[32+i*32+22]=p->LoopSounder[i].LoopSounder30;
		Statustemp[33+i*32+22]=p->LoopSounder[i].LoopSounder31;
			}
		/*
		Statustemp[34]=p->LoopSounder32;
		Statustemp[35]=p->LoopSounder33;
		Statustemp[36]=p->LoopSounder34;
		Statustemp[37]=p->LoopSounder35;
		Statustemp[38]=p->LoopSounder36;
		Statustemp[39]=p->LoopSounder37;
		Statustemp[40]=p->LoopSounder38;
		Statustemp[41]=p->LoopSounder39;
		Statustemp[42]=p->LoopSounder40;
		Statustemp[43]=p->LoopSounder41;
		Statustemp[44]=p->LoopSounder42;
		Statustemp[45]=p->LoopSounder43;
		Statustemp[46]=p->LoopSounder44;
		Statustemp[47]=p->LoopSounder45;
		Statustemp[48]=p->LoopSounder46;
		Statustemp[49]=p->LoopSounder47;
		Statustemp[50]=p->LoopSounder48;
		Statustemp[51]=p->LoopSounder49;
		Statustemp[52]=p->LoopSounder50;
		Statustemp[53]=p->LoopSounder51;
		Statustemp[54]=p->LoopSounder52;
		Statustemp[55]=p->LoopSounder53;
		Statustemp[56]=p->LoopSounder54;
		Statustemp[57]=p->LoopSounder55;
		Statustemp[58]=p->LoopSounder56;
		Statustemp[59]=p->LoopSounder57;
		Statustemp[60]=p->LoopSounder58;
		Statustemp[61]=p->LoopSounder59;
		Statustemp[62]=p->LoopSounder60;
		Statustemp[63]=p->LoopSounder61;
		Statustemp[64]=p->LoopSounder62;
		Statustemp[65]=p->LoopSounder63;
		*/
		










}
/*
void		GetSoundStatustoShuzu1(sounden *p,unsigned char	*Statustemp)
{		
		static uint16_t	i;
		Statustemp[0]=p->ConSounder0;
		Statustemp[1]=p->ConSounder1;
		for(i=0;i<24;i++)
			{
		Statustemp[2+i*32]=p->LoopSounder[i].LoopSounder0;
		Statustemp[3+i*32]=p->LoopSounder[i].LoopSounder1;
		Statustemp[4+i*32]=p->LoopSounder[i].LoopSounder2;
		Statustemp[5+i*32]=p->LoopSounder[i].LoopSounder3;
		Statustemp[6+i*32]=p->LoopSounder[i].LoopSounder4;
		Statustemp[7+i*32]=p->LoopSounder[i].LoopSounder5;
		Statustemp[8+i*32]=p->LoopSounder[i].LoopSounder6;
		Statustemp[9+i*32]=p->LoopSounder[i].LoopSounder7;
		Statustemp[10+i*32]=p->LoopSounder[i].LoopSounder8;
		Statustemp[11+i*32]=p->LoopSounder[i].LoopSounder9;
		Statustemp[12+i*32]=p->LoopSounder[i].LoopSounder10;
		Statustemp[13+i*32]=p->LoopSounder[i].LoopSounder11;
		Statustemp[14+i*32]=p->LoopSounder[i].LoopSounder12;
		Statustemp[15+i*32]=p->LoopSounder[i].LoopSounder13;
		Statustemp[16+i*32]=p->LoopSounder[i].LoopSounder14;
		Statustemp[17+i*32]=p->LoopSounder[i].LoopSounder15;
		Statustemp[18+i*32]=p->LoopSounder[i].LoopSounder16;
		Statustemp[19+i*32]=p->LoopSounder[i].LoopSounder17;
		Statustemp[20+i*32]=p->LoopSounder[i].LoopSounder18;
		Statustemp[21+i*32]=p->LoopSounder[i].LoopSounder19;
		Statustemp[22+i*32]=p->LoopSounder[i].LoopSounder20;
		Statustemp[23+i*32]=p->LoopSounder[i].LoopSounder21;
		Statustemp[24+i*32]=p->LoopSounder[i].LoopSounder22;
		Statustemp[25+i*32]=p->LoopSounder[i].LoopSounder23;
		Statustemp[26+i*32]=p->LoopSounder[i].LoopSounder24;
		Statustemp[27+i*32]=p->LoopSounder[i].LoopSounder25;
		Statustemp[28+i*32]=p->LoopSounder[i].LoopSounder26;
		Statustemp[29+i*32]=p->LoopSounder[i].LoopSounder27;
		Statustemp[30+i*32]=p->LoopSounder[i].LoopSounder28;
		Statustemp[31+i*32]=p->LoopSounder[i].LoopSounder29;
		Statustemp[32+i*32]=p->LoopSounder[i].LoopSounder30;
		Statustemp[33+i*32]=p->LoopSounder[i].LoopSounder31;
			}
		








}
*/
void		GetSoundStatustoShuzu2(soundover *p,unsigned char	*Statustemp)
{		
		static uint16_t	i;
		
			Statustemp[0]=p->ConSounder.ConSounder0;
		Statustemp[1]=p->ConSounder.ConSounder1;
		Statustemp[2]=p->ConSounder.ConSounder2;
		Statustemp[3]=p->ConSounder.ConSounder3;
		Statustemp[4]=p->ConSounder.ConSounder4;
		Statustemp[5]=p->ConSounder.ConSounder5;
		Statustemp[6]=p->ConSounder.ConSounder6;
		Statustemp[7]=p->ConSounder.ConSounder7;
		Statustemp[8]=p->ConSounder.ConSounder8;
		Statustemp[9]=p->ConSounder.ConSounder9;
		Statustemp[10]=p->ConSounder.ConSounder10;
		Statustemp[11]=p->ConSounder.ConSounder11;
		Statustemp[12]=p->ConSounder.ConSounder12;
		Statustemp[13]=p->ConSounder.ConSounder13;
		Statustemp[14]=p->ConSounder.ConSounder14;
		Statustemp[15]=p->ConSounder.ConSounder15;
		Statustemp[16]=p->ConSounder.ConSounder16;
		Statustemp[17]=p->ConSounder.ConSounder17;
		Statustemp[18]=p->ConSounder.ConSounder18;
		Statustemp[19]=p->ConSounder.ConSounder19;
		Statustemp[20]=p->ConSounder.ConSounder20;
		Statustemp[21]=p->ConSounder.ConSounder21;
		Statustemp[22]=p->ConSounder.ConSounder22;
		Statustemp[23]=p->ConSounder.ConSounder23;
		for(i=0;i<LoopNum;i++)
			{
		Statustemp[2+i*32+22]=p->LoopSounder[i].LoopSounder0;
		Statustemp[3+i*32+22]=p->LoopSounder[i].LoopSounder1;
		Statustemp[4+i*32+22]=p->LoopSounder[i].LoopSounder2;
		Statustemp[5+i*32+22]=p->LoopSounder[i].LoopSounder3;
		Statustemp[6+i*32+22]=p->LoopSounder[i].LoopSounder4;
		Statustemp[7+i*32+22]=p->LoopSounder[i].LoopSounder5;
		Statustemp[8+i*32+22]=p->LoopSounder[i].LoopSounder6;
		Statustemp[9+i*32+22]=p->LoopSounder[i].LoopSounder7;
		Statustemp[10+i*32+22]=p->LoopSounder[i].LoopSounder8;
		Statustemp[11+i*32+22]=p->LoopSounder[i].LoopSounder9;
		Statustemp[12+i*32+22]=p->LoopSounder[i].LoopSounder10;
		Statustemp[13+i*32+22]=p->LoopSounder[i].LoopSounder11;
		Statustemp[14+i*32+22]=p->LoopSounder[i].LoopSounder12;
		Statustemp[15+i*32+22]=p->LoopSounder[i].LoopSounder13;
		Statustemp[16+i*32+22]=p->LoopSounder[i].LoopSounder14;
		Statustemp[17+i*32+22]=p->LoopSounder[i].LoopSounder15;
		Statustemp[18+i*32+22]=p->LoopSounder[i].LoopSounder16;
		Statustemp[19+i*32+22]=p->LoopSounder[i].LoopSounder17;
		Statustemp[20+i*32+22]=p->LoopSounder[i].LoopSounder18;
		Statustemp[21+i*32+22]=p->LoopSounder[i].LoopSounder19;
		Statustemp[22+i*32+22]=p->LoopSounder[i].LoopSounder20;
		Statustemp[23+i*32+22]=p->LoopSounder[i].LoopSounder21;
		Statustemp[24+i*32+22]=p->LoopSounder[i].LoopSounder22;
		Statustemp[25+i*32+22]=p->LoopSounder[i].LoopSounder23;
		Statustemp[26+i*32+22]=p->LoopSounder[i].LoopSounder24;
		Statustemp[27+i*32+22]=p->LoopSounder[i].LoopSounder25;
		Statustemp[28+i*32+22]=p->LoopSounder[i].LoopSounder26;
		Statustemp[29+i*32+22]=p->LoopSounder[i].LoopSounder27;
		Statustemp[30+i*32+22]=p->LoopSounder[i].LoopSounder28;
		Statustemp[31+i*32+22]=p->LoopSounder[i].LoopSounder29;
		Statustemp[32+i*32+22]=p->LoopSounder[i].LoopSounder30;
		Statustemp[33+i*32+22]=p->LoopSounder[i].LoopSounder31;
			}
		/*
		Statustemp[2]=p->LoopSounder0;
		Statustemp[3]=p->LoopSounder1;
		Statustemp[4]=p->LoopSounder2;
		Statustemp[5]=p->LoopSounder3;
		Statustemp[6]=p->LoopSounder4;
		Statustemp[7]=p->LoopSounder5;
		Statustemp[8]=p->LoopSounder6;
		Statustemp[9]=p->LoopSounder7;
		Statustemp[10]=p->LoopSounder8;
		Statustemp[11]=p->LoopSounder9;
		Statustemp[12]=p->LoopSounder10;
		Statustemp[13]=p->LoopSounder11;
		Statustemp[14]=p->LoopSounder12;
		Statustemp[15]=p->LoopSounder13;
		Statustemp[16]=p->LoopSounder14;
		Statustemp[17]=p->LoopSounder15;
		Statustemp[18]=p->LoopSounder16;
		Statustemp[19]=p->LoopSounder17;
		Statustemp[20]=p->LoopSounder18;
		Statustemp[21]=p->LoopSounder19;
		Statustemp[22]=p->LoopSounder20;
		Statustemp[23]=p->LoopSounder21;
		Statustemp[24]=p->LoopSounder22;
		Statustemp[25]=p->LoopSounder23;
		Statustemp[26]=p->LoopSounder24;
		Statustemp[27]=p->LoopSounder25;
		Statustemp[28]=p->LoopSounder26;
		Statustemp[29]=p->LoopSounder27;
		Statustemp[30]=p->LoopSounder28;
		Statustemp[31]=p->LoopSounder29;
		Statustemp[32]=p->LoopSounder30;
		Statustemp[33]=p->LoopSounder31;
		Statustemp[34]=p->LoopSounder32;
		Statustemp[35]=p->LoopSounder33;
		Statustemp[36]=p->LoopSounder34;
		Statustemp[37]=p->LoopSounder35;
		Statustemp[38]=p->LoopSounder36;
		Statustemp[39]=p->LoopSounder37;
		Statustemp[40]=p->LoopSounder38;
		Statustemp[41]=p->LoopSounder39;
		Statustemp[42]=p->LoopSounder40;
		Statustemp[43]=p->LoopSounder41;
		Statustemp[44]=p->LoopSounder42;
		Statustemp[45]=p->LoopSounder43;
		Statustemp[46]=p->LoopSounder44;
		Statustemp[47]=p->LoopSounder45;
		Statustemp[48]=p->LoopSounder46;
		Statustemp[49]=p->LoopSounder47;
		Statustemp[50]=p->LoopSounder48;
		Statustemp[51]=p->LoopSounder49;
		Statustemp[52]=p->LoopSounder50;
		Statustemp[53]=p->LoopSounder51;
		Statustemp[54]=p->LoopSounder52;
		Statustemp[55]=p->LoopSounder53;
		Statustemp[56]=p->LoopSounder54;
		Statustemp[57]=p->LoopSounder55;
		Statustemp[58]=p->LoopSounder56;
		Statustemp[59]=p->LoopSounder57;
		Statustemp[60]=p->LoopSounder58;
		Statustemp[61]=p->LoopSounder59;
		Statustemp[62]=p->LoopSounder60;
		Statustemp[63]=p->LoopSounder61;
		Statustemp[64]=p->LoopSounder62;
		Statustemp[65]=p->LoopSounder63;
		*/
		










}
void		GetSoundStatustoShuzu2_Conv(consndover	* p, unsigned char * Statustemp)
{

			
		Statustemp[0]=p->ConSounder0;
		Statustemp[1]=p->ConSounder1;
		Statustemp[2]=p->ConSounder2;
		Statustemp[3]=p->ConSounder3;
		Statustemp[4]=p->ConSounder4;
		Statustemp[5]=p->ConSounder5;
		Statustemp[6]=p->ConSounder6;
		Statustemp[7]=p->ConSounder7;
		Statustemp[8]=p->ConSounder8;
		Statustemp[9]=p->ConSounder9;
		Statustemp[10]=p->ConSounder10;
		Statustemp[11]=p->ConSounder11;
		Statustemp[12]=p->ConSounder12;
		Statustemp[13]=p->ConSounder13;
		Statustemp[14]=p->ConSounder14;
		Statustemp[15]=p->ConSounder15;
		Statustemp[16]=p->ConSounder16;
		Statustemp[17]=p->ConSounder17;
		Statustemp[18]=p->ConSounder18;
		Statustemp[19]=p->ConSounder19;
		Statustemp[20]=p->ConSounder20;
		Statustemp[21]=p->ConSounder21;
		Statustemp[22]=p->ConSounder22;
		Statustemp[23]=p->ConSounder23;
	







}
void		GetSoundStatustoShuzu2_1(loopsndover* p, unsigned char * Statustemp)
{

			static uint16_t	i;
			i=0;
			
		Statustemp[2+i*32+22]=p->LoopSounder0;
		Statustemp[3+i*32+22]=p->LoopSounder1;
		Statustemp[4+i*32+22]=p->LoopSounder2;
		Statustemp[5+i*32+22]=p->LoopSounder3;
		Statustemp[6+i*32+22]=p->LoopSounder4;
		Statustemp[7+i*32+22]=p->LoopSounder5;
		Statustemp[8+i*32+22]=p->LoopSounder6;
		Statustemp[9+i*32+22]=p->LoopSounder7;
		Statustemp[10+i*32+22]=p->LoopSounder8;
		Statustemp[11+i*32+22]=p->LoopSounder9;
		Statustemp[12+i*32+22]=p->LoopSounder10;
		Statustemp[13+i*32+22]=p->LoopSounder11;
		Statustemp[14+i*32+22]=p->LoopSounder12;
		Statustemp[15+i*32+22]=p->LoopSounder13;
		Statustemp[16+i*32+22]=p->LoopSounder14;
		Statustemp[17+i*32+22]=p->LoopSounder15;
		Statustemp[18+i*32+22]=p->LoopSounder16;
		Statustemp[19+i*32+22]=p->LoopSounder17;
		Statustemp[20+i*32+22]=p->LoopSounder18;
		Statustemp[21+i*32+22]=p->LoopSounder19;
		Statustemp[22+i*32+22]=p->LoopSounder20;
		Statustemp[23+i*32+22]=p->LoopSounder21;
		Statustemp[24+i*32+22]=p->LoopSounder22;
		Statustemp[25+i*32+22]=p->LoopSounder23;
		Statustemp[26+i*32+22]=p->LoopSounder24;
		Statustemp[27+i*32+22]=p->LoopSounder25;
		Statustemp[28+i*32+22]=p->LoopSounder26;
		Statustemp[29+i*32+22]=p->LoopSounder27;
		Statustemp[30+i*32+22]=p->LoopSounder28;
		Statustemp[31+i*32+22]=p->LoopSounder29;
		Statustemp[32+i*32+22]=p->LoopSounder30;
		Statustemp[33+i*32+22]=p->LoopSounder31;









}
void		PutSoundStatustobitfeild2(soundover *p,unsigned char	*Statustemp)
{
		static unsigned char	i;
		p->ConSounder.ConSounder0=Statustemp[0];
		p->ConSounder.ConSounder1=Statustemp[1];
		p->ConSounder.ConSounder2=Statustemp[2];
		p->ConSounder.ConSounder3=Statustemp[3];
		p->ConSounder.ConSounder4=Statustemp[4];
		p->ConSounder.ConSounder5=Statustemp[5];
		p->ConSounder.ConSounder6=Statustemp[6];
		p->ConSounder.ConSounder7=Statustemp[7];
		p->ConSounder.ConSounder8=Statustemp[8];
		p->ConSounder.ConSounder9=Statustemp[9];
		p->ConSounder.ConSounder10=Statustemp[10];
		p->ConSounder.ConSounder11=Statustemp[11];
		p->ConSounder.ConSounder12=Statustemp[12];
		p->ConSounder.ConSounder13=Statustemp[13];
		p->ConSounder.ConSounder14=Statustemp[14];
		p->ConSounder.ConSounder15=Statustemp[15];
		p->ConSounder.ConSounder16=Statustemp[16];
		p->ConSounder.ConSounder17=Statustemp[17];
		p->ConSounder.ConSounder18=Statustemp[18];
		p->ConSounder.ConSounder19=Statustemp[19];
		p->ConSounder.ConSounder20=Statustemp[20];
		p->ConSounder.ConSounder21=Statustemp[21];
		p->ConSounder.ConSounder22=Statustemp[22];
		p->ConSounder.ConSounder23=Statustemp[23];
		for(i=0;i<LoopNum;i++)
			{
		p->LoopSounder[i].LoopSounder0=Statustemp[2+i*32+22];
		p->LoopSounder[i].LoopSounder1=Statustemp[3+i*32+22];
		p->LoopSounder[i].LoopSounder2=Statustemp[4+i*32+22];
		p->LoopSounder[i].LoopSounder3=Statustemp[5+i*32+22];
		p->LoopSounder[i].LoopSounder4=Statustemp[6+i*32+22];
		p->LoopSounder[i].LoopSounder5=Statustemp[7+i*32+22];
		p->LoopSounder[i].LoopSounder6=Statustemp[8+i*32+22];
		p->LoopSounder[i].LoopSounder7=Statustemp[9+i*32+22];
		p->LoopSounder[i].LoopSounder8=Statustemp[10+i*32+22];
		p->LoopSounder[i].LoopSounder9=Statustemp[11+i*32+22];
		p->LoopSounder[i].LoopSounder10=Statustemp[12+i*32+22];
		p->LoopSounder[i].LoopSounder11=Statustemp[13+i*32+22];
		p->LoopSounder[i].LoopSounder12=Statustemp[14+i*32+22];
		p->LoopSounder[i].LoopSounder13=Statustemp[15+i*32+22];
		p->LoopSounder[i].LoopSounder14=Statustemp[16+i*32+22];
		p->LoopSounder[i].LoopSounder15=Statustemp[17+i*32+22];
		p->LoopSounder[i].LoopSounder16=Statustemp[18+i*32+22];
		p->LoopSounder[i].LoopSounder17=Statustemp[19+i*32+22];
		p->LoopSounder[i].LoopSounder18=Statustemp[20+i*32+22];
		p->LoopSounder[i].LoopSounder19=Statustemp[21+i*32+22];
		p->LoopSounder[i].LoopSounder20=Statustemp[22+i*32+22];
		p->LoopSounder[i].LoopSounder21=Statustemp[23+i*32+22];
		p->LoopSounder[i].LoopSounder22=Statustemp[24+i*32+22];
		p->LoopSounder[i].LoopSounder23=Statustemp[25+i*32+22];
		p->LoopSounder[i].LoopSounder24=Statustemp[26+i*32+22];
		p->LoopSounder[i].LoopSounder25=Statustemp[27+i*32+22];
		p->LoopSounder[i].LoopSounder26=Statustemp[28+i*32+22];
		p->LoopSounder[i].LoopSounder27=Statustemp[29+i*32+22];
		p->LoopSounder[i].LoopSounder28=Statustemp[30+i*32+22];
		p->LoopSounder[i].LoopSounder29=Statustemp[31+i*32+22];
		p->LoopSounder[i].LoopSounder30=Statustemp[32+i*32+22];
		p->LoopSounder[i].LoopSounder31=Statustemp[33+i*32+22];
			}
		/*
		p->LoopSounder32=Statustemp[34];
		p->LoopSounder33=Statustemp[35];
		p->LoopSounder34=Statustemp[36];
		p->LoopSounder35=Statustemp[37];
		p->LoopSounder36=Statustemp[38];
		p->LoopSounder37=Statustemp[39];
		p->LoopSounder38=Statustemp[40];
		p->LoopSounder39=Statustemp[41];
		p->LoopSounder40=Statustemp[42];
		p->LoopSounder41=Statustemp[43];
		p->LoopSounder42=Statustemp[44];
		p->LoopSounder43=Statustemp[45];
		p->LoopSounder44=Statustemp[46];
		p->LoopSounder45=Statustemp[47];
		p->LoopSounder46=Statustemp[48];
		p->LoopSounder47=Statustemp[49];
		p->LoopSounder48=Statustemp[50];
		p->LoopSounder49=Statustemp[51];
		p->LoopSounder50=Statustemp[52];
		p->LoopSounder51=Statustemp[53];
		p->LoopSounder52=Statustemp[54];
		p->LoopSounder53=Statustemp[55];
		p->LoopSounder54=Statustemp[56];
		p->LoopSounder55=Statustemp[57];
		p->LoopSounder56=Statustemp[58];
		p->LoopSounder57=Statustemp[59];
		p->LoopSounder58=Statustemp[60];
		p->LoopSounder59=Statustemp[61];
		p->LoopSounder60=Statustemp[62];
		p->LoopSounder61=Statustemp[63];
		p->LoopSounder62=Statustemp[64];
		p->LoopSounder63=Statustemp[65];
		*/









}
void		PutSoundStatustobitfeild2_Conv(consndover	*p,unsigned char	*Statustemp)
{


		p->ConSounder0=Statustemp[0];
		p->ConSounder1=Statustemp[1];
		p->ConSounder2=Statustemp[2];
		p->ConSounder3=Statustemp[3];
		p->ConSounder4=Statustemp[4];
		p->ConSounder5=Statustemp[5];
		p->ConSounder6=Statustemp[6];
		p->ConSounder7=Statustemp[7];
		p->ConSounder8=Statustemp[8];
		p->ConSounder9=Statustemp[9];
		p->ConSounder10=Statustemp[10];
		p->ConSounder11=Statustemp[11];
		p->ConSounder12=Statustemp[12];
		p->ConSounder13=Statustemp[13];
		p->ConSounder14=Statustemp[14];
		p->ConSounder15=Statustemp[15];
		p->ConSounder16=Statustemp[16];
		p->ConSounder17=Statustemp[17];
		p->ConSounder18=Statustemp[18];
		p->ConSounder19=Statustemp[19];
		p->ConSounder20=Statustemp[20];
		p->ConSounder21=Statustemp[21];
		p->ConSounder22=Statustemp[22];
		p->ConSounder23=Statustemp[23];
		





}
void		PutSoundStatustobitfeild2_1(loopsndover	*p,unsigned char	*Statustemp)
{
	
		static unsigned char	i;
		
		i=0;
		p->LoopSounder0=Statustemp[2+i*32+22];
		p->LoopSounder1=Statustemp[3+i*32+22];
		p->LoopSounder2=Statustemp[4+i*32+22];
		p->LoopSounder3=Statustemp[5+i*32+22];
		p->LoopSounder4=Statustemp[6+i*32+22];
		p->LoopSounder5=Statustemp[7+i*32+22];
		p->LoopSounder6=Statustemp[8+i*32+22];
		p->LoopSounder7=Statustemp[9+i*32+22];
		p->LoopSounder8=Statustemp[10+i*32+22];
		p->LoopSounder9=Statustemp[11+i*32+22];
		p->LoopSounder10=Statustemp[12+i*32+22];
		p->LoopSounder11=Statustemp[13+i*32+22];
		p->LoopSounder12=Statustemp[14+i*32+22];
		p->LoopSounder13=Statustemp[15+i*32+22];
		p->LoopSounder14=Statustemp[16+i*32+22];
		p->LoopSounder15=Statustemp[17+i*32+22];
		p->LoopSounder16=Statustemp[18+i*32+22];
		p->LoopSounder17=Statustemp[19+i*32+22];
		p->LoopSounder18=Statustemp[20+i*32+22];
		p->LoopSounder19=Statustemp[21+i*32+22];
		p->LoopSounder20=Statustemp[22+i*32+22];
		p->LoopSounder21=Statustemp[23+i*32+22];
		p->LoopSounder22=Statustemp[24+i*32+22];
		p->LoopSounder23=Statustemp[25+i*32+22];
		p->LoopSounder24=Statustemp[26+i*32+22];
		p->LoopSounder25=Statustemp[27+i*32+22];
		p->LoopSounder26=Statustemp[28+i*32+22];
		p->LoopSounder27=Statustemp[29+i*32+22];
		p->LoopSounder28=Statustemp[30+i*32+22];
		p->LoopSounder29=Statustemp[31+i*32+22];
		p->LoopSounder30=Statustemp[32+i*32+22];
		p->LoopSounder31=Statustemp[33+i*32+22];
		


}
void		PutSoundStatustobitfeild1(sounden*p,unsigned char	*Statustemp)
{
	
		static unsigned char	i;
		
		p->ConSounder.ConSounder0=Statustemp[0];
		p->ConSounder.ConSounder1=Statustemp[1];
		p->ConSounder.ConSounder2=Statustemp[2];
		p->ConSounder.ConSounder3=Statustemp[3];
		p->ConSounder.ConSounder4=Statustemp[4];
		p->ConSounder.ConSounder5=Statustemp[5];
		p->ConSounder.ConSounder6=Statustemp[6];
		p->ConSounder.ConSounder7=Statustemp[7];
		p->ConSounder.ConSounder8=Statustemp[8];
		p->ConSounder.ConSounder9=Statustemp[9];
		p->ConSounder.ConSounder10=Statustemp[10];
		p->ConSounder.ConSounder11=Statustemp[11];
		p->ConSounder.ConSounder12=Statustemp[12];
		p->ConSounder.ConSounder13=Statustemp[13];
		p->ConSounder.ConSounder14=Statustemp[14];
		p->ConSounder.ConSounder15=Statustemp[15];
		p->ConSounder.ConSounder16=Statustemp[16];
		p->ConSounder.ConSounder17=Statustemp[17];
		p->ConSounder.ConSounder18=Statustemp[18];
		p->ConSounder.ConSounder19=Statustemp[19];
		p->ConSounder.ConSounder20=Statustemp[20];
		p->ConSounder.ConSounder21=Statustemp[21];
		p->ConSounder.ConSounder22=Statustemp[22];
		p->ConSounder.ConSounder23=Statustemp[23];
		for(i=0;i<LoopNum;i++)
			{
		p->LoopSounder[i].LoopSounder0=Statustemp[2+i*32+22];
		p->LoopSounder[i].LoopSounder1=Statustemp[3+i*32+22];
		p->LoopSounder[i].LoopSounder2=Statustemp[4+i*32+22];
		p->LoopSounder[i].LoopSounder3=Statustemp[5+i*32+22];
		p->LoopSounder[i].LoopSounder4=Statustemp[6+i*32+22];
		p->LoopSounder[i].LoopSounder5=Statustemp[7+i*32+22];
		p->LoopSounder[i].LoopSounder6=Statustemp[8+i*32+22];
		p->LoopSounder[i].LoopSounder7=Statustemp[9+i*32+22];
		p->LoopSounder[i].LoopSounder8=Statustemp[10+i*32+22];
		p->LoopSounder[i].LoopSounder9=Statustemp[11+i*32+22];
		p->LoopSounder[i].LoopSounder10=Statustemp[12+i*32+22];
		p->LoopSounder[i].LoopSounder11=Statustemp[13+i*32+22];
		p->LoopSounder[i].LoopSounder12=Statustemp[14+i*32+22];
		p->LoopSounder[i].LoopSounder13=Statustemp[15+i*32+22];
		p->LoopSounder[i].LoopSounder14=Statustemp[16+i*32+22];
		p->LoopSounder[i].LoopSounder15=Statustemp[17+i*32+22];
		p->LoopSounder[i].LoopSounder16=Statustemp[18+i*32+22];
		p->LoopSounder[i].LoopSounder17=Statustemp[19+i*32+22];
		p->LoopSounder[i].LoopSounder18=Statustemp[20+i*32+22];
		p->LoopSounder[i].LoopSounder19=Statustemp[21+i*32+22];
		p->LoopSounder[i].LoopSounder20=Statustemp[22+i*32+22];
		p->LoopSounder[i].LoopSounder21=Statustemp[23+i*32+22];
		p->LoopSounder[i].LoopSounder22=Statustemp[24+i*32+22];
		p->LoopSounder[i].LoopSounder23=Statustemp[25+i*32+22];
		p->LoopSounder[i].LoopSounder24=Statustemp[26+i*32+22];
		p->LoopSounder[i].LoopSounder25=Statustemp[27+i*32+22];
		p->LoopSounder[i].LoopSounder26=Statustemp[28+i*32+22];
		p->LoopSounder[i].LoopSounder27=Statustemp[29+i*32+22];
		p->LoopSounder[i].LoopSounder28=Statustemp[30+i*32+22];
		p->LoopSounder[i].LoopSounder29=Statustemp[31+i*32+22];
		p->LoopSounder[i].LoopSounder30=Statustemp[32+i*32+22];
		p->LoopSounder[i].LoopSounder31=Statustemp[33+i*32+22];
			
			}
	









}
void		PutSoundStatustobitfeild1_Conv(consnden	*p,unsigned char	*Statustemp)
{
		p->ConSounder0=Statustemp[0];
		p->ConSounder1=Statustemp[1];
		p->ConSounder2=Statustemp[2];
		p->ConSounder3=Statustemp[3];
		p->ConSounder4=Statustemp[4];
		p->ConSounder5=Statustemp[5];
		p->ConSounder6=Statustemp[6];
		p->ConSounder7=Statustemp[7];
		p->ConSounder8=Statustemp[8];
		p->ConSounder9=Statustemp[9];
		p->ConSounder10=Statustemp[10];
		p->ConSounder11=Statustemp[11];
		p->ConSounder12=Statustemp[12];
		p->ConSounder13=Statustemp[13];
		p->ConSounder14=Statustemp[14];
		p->ConSounder15=Statustemp[15];
		p->ConSounder16=Statustemp[16];
		p->ConSounder17=Statustemp[17];
		p->ConSounder18=Statustemp[18];
		p->ConSounder19=Statustemp[19];
		p->ConSounder20=Statustemp[20];
		p->ConSounder21=Statustemp[21];
		p->ConSounder22=Statustemp[22];
		p->ConSounder23=Statustemp[23];
		

		
	



}
void		PutSoundStatustobitfeild1_1(loopsnden*p,unsigned char	*Statustemp)
{
	
		static unsigned char	i;
		
		i=0;
		p->LoopSounder0=Statustemp[2+i*32+22];
		p->LoopSounder1=Statustemp[3+i*32+22];
		p->LoopSounder2=Statustemp[4+i*32+22];
		p->LoopSounder3=Statustemp[5+i*32+22];
		p->LoopSounder4=Statustemp[6+i*32+22];
		p->LoopSounder5=Statustemp[7+i*32+22];
		p->LoopSounder6=Statustemp[8+i*32+22];
		p->LoopSounder7=Statustemp[9+i*32+22];
		p->LoopSounder8=Statustemp[10+i*32+22];
		p->LoopSounder9=Statustemp[11+i*32+22];
		p->LoopSounder10=Statustemp[12+i*32+22];
		p->LoopSounder11=Statustemp[13+i*32+22];
		p->LoopSounder12=Statustemp[14+i*32+22];
		p->LoopSounder13=Statustemp[15+i*32+22];
		p->LoopSounder14=Statustemp[16+i*32+22];
		p->LoopSounder15=Statustemp[17+i*32+22];
		p->LoopSounder16=Statustemp[18+i*32+22];
		p->LoopSounder17=Statustemp[19+i*32+22];
		p->LoopSounder18=Statustemp[20+i*32+22];
		p->LoopSounder19=Statustemp[21+i*32+22];
		p->LoopSounder20=Statustemp[22+i*32+22];
		p->LoopSounder21=Statustemp[23+i*32+22];
		p->LoopSounder22=Statustemp[24+i*32+22];
		p->LoopSounder23=Statustemp[25+i*32+22];
		p->LoopSounder24=Statustemp[26+i*32+22];
		p->LoopSounder25=Statustemp[27+i*32+22];
		p->LoopSounder26=Statustemp[28+i*32+22];
		p->LoopSounder27=Statustemp[29+i*32+22];
		p->LoopSounder28=Statustemp[30+i*32+22];
		p->LoopSounder29=Statustemp[31+i*32+22];
		p->LoopSounder30=Statustemp[32+i*32+22];
		p->LoopSounder31=Statustemp[33+i*32+22];
		


}
void		PutSoundStatustobitfeild(soundgr *p,unsigned char	*Statustemp)
{
	static uint16_t	i;
			
		p->ConSounder.ConSounder0=Statustemp[0];
		p->ConSounder.ConSounder1=Statustemp[1];
		p->ConSounder.ConSounder2=Statustemp[2];
		p->ConSounder.ConSounder3=Statustemp[3];
		p->ConSounder.ConSounder4=Statustemp[4];
		p->ConSounder.ConSounder5=Statustemp[5];
		p->ConSounder.ConSounder6=Statustemp[6];
		p->ConSounder.ConSounder7=Statustemp[7];
		p->ConSounder.ConSounder8=Statustemp[8];
		p->ConSounder.ConSounder9=Statustemp[9];
		p->ConSounder.ConSounder10=Statustemp[10];
		p->ConSounder.ConSounder11=Statustemp[11];
		p->ConSounder.ConSounder12=Statustemp[12];
		p->ConSounder.ConSounder13=Statustemp[13];
		p->ConSounder.ConSounder14=Statustemp[14];
		p->ConSounder.ConSounder15=Statustemp[15];
		p->ConSounder.ConSounder16=Statustemp[16];
		p->ConSounder.ConSounder17=Statustemp[17];
		p->ConSounder.ConSounder18=Statustemp[18];
		p->ConSounder.ConSounder19=Statustemp[19];
		p->ConSounder.ConSounder20=Statustemp[20];
		p->ConSounder.ConSounder21=Statustemp[21];
		p->ConSounder.ConSounder22=Statustemp[22];
		p->ConSounder.ConSounder23=Statustemp[23];
		for(i=0;i<LoopNum;i++)
			{
		p->LoopSounder[i].LoopSounder0=Statustemp[2+i*32+22];
		p->LoopSounder[i].LoopSounder1=Statustemp[3+i*32+22];
		p->LoopSounder[i].LoopSounder2=Statustemp[4+i*32+22];
		p->LoopSounder[i].LoopSounder3=Statustemp[5+i*32+22];
		p->LoopSounder[i].LoopSounder4=Statustemp[6+i*32+22];
		p->LoopSounder[i].LoopSounder5=Statustemp[7+i*32+22];
		p->LoopSounder[i].LoopSounder6=Statustemp[8+i*32+22];
		p->LoopSounder[i].LoopSounder7=Statustemp[9+i*32+22];
		p->LoopSounder[i].LoopSounder8=Statustemp[10+i*32+22];
		p->LoopSounder[i].LoopSounder9=Statustemp[11+i*32+22];
		p->LoopSounder[i].LoopSounder10=Statustemp[12+i*32+22];
		p->LoopSounder[i].LoopSounder11=Statustemp[13+i*32+22];
		p->LoopSounder[i].LoopSounder12=Statustemp[14+i*32+22];
		p->LoopSounder[i].LoopSounder13=Statustemp[15+i*32+22];
		p->LoopSounder[i].LoopSounder14=Statustemp[16+i*32+22];
		p->LoopSounder[i].LoopSounder15=Statustemp[17+i*32+22];
		p->LoopSounder[i].LoopSounder16=Statustemp[18+i*32+22];
		p->LoopSounder[i].LoopSounder17=Statustemp[19+i*32+22];
		p->LoopSounder[i].LoopSounder18=Statustemp[20+i*32+22];
		p->LoopSounder[i].LoopSounder19=Statustemp[21+i*32+22];
		p->LoopSounder[i].LoopSounder20=Statustemp[22+i*32+22];
		p->LoopSounder[i].LoopSounder21=Statustemp[23+i*32+22];
		p->LoopSounder[i].LoopSounder22=Statustemp[24+i*32+22];
		p->LoopSounder[i].LoopSounder23=Statustemp[25+i*32+22];
		p->LoopSounder[i].LoopSounder24=Statustemp[26+i*32+22];
		p->LoopSounder[i].LoopSounder25=Statustemp[27+i*32+22];
		p->LoopSounder[i].LoopSounder26=Statustemp[28+i*32+22];
		p->LoopSounder[i].LoopSounder27=Statustemp[29+i*32+22];
		p->LoopSounder[i].LoopSounder28=Statustemp[30+i*32+22];
		p->LoopSounder[i].LoopSounder29=Statustemp[31+i*32+22];
		p->LoopSounder[i].LoopSounder30=Statustemp[32+i*32+22];
		p->LoopSounder[i].LoopSounder31=Statustemp[33+i*32+22];
			}
	









}

extern	keybit		keybitflag;
void		updowncaltime(lcdstruct * disptr)
{

	    if (disptr->DispdelayChange.dispdelay1_1) 
			{
			/*
      			  if (disptr->DispdelayTime1_1 < 30)
		  		{
        	   	 disptr->DispdelayTime1_1++;

       			 }
			 else {
         		   SysTemFlag.Bit.DispLayLcd = 1;
           			 disptr->Setlevel3 = 2;
           			 disptr->DispdelayChange.dispdelay1_1 = 0;

       			 }*/
       			 if((HAL_GetTick()>disptr->DispdelayTime.DispdelayTime1_1)&&((HAL_GetTick()-disptr->DispdelayTime.DispdelayTime1_1)>3000))
       			 	{

						disptr->LcdRunFlag.Bit.DispLayLcd=1;
						disptr->LcdRunFlag.Bit.DispLayLcdrx=1;
					 // SysTemFlag.Bit.DispLayLcd = 1;
           				 disptr->Setlevel3 = 2;
           				 disptr->DispdelayChange.dispdelay1_1 = 0;




       			 	}
   			 } 
		else {

      			  disptr->DispdelayTime.DispdelayTime1_1 = HAL_GetTick();


   			 }
		//-----------1-1 的长安
		  if (disptr->DispdelayChange.dispdelay1_1_1 && keybitflag.Bit.UpK) {


			if((HAL_GetTick()>disptr->DispDelayTimeSet.DispdelayTime1_1_1)&&((HAL_GetTick()-disptr->DispDelayTimeSet.DispdelayTime1_1_1)>=1500))
				{
         			   if (LogEntryNo > 0) {
              
             
                if (disptr->NowDispNum < LogEntryNo) {

                   disptr->NowDispNum = disptr->NowDispNum + 5;
                    if (disptr->NowDispNum >= LogEntryNo) {
                        disptr->NowDispNum = LogEntryNo;

                    }

                } else {
                    if ( disptr->NowDispNum <= MaxLogNum) {

                         disptr->NowDispNum = 1;

                    } else {

                         disptr->NowDispNum =  LogEntryNo - MaxLogNum + 1;
                    }

                }
			disptr->DispDelayTimeSet.DispdelayTime1_1_1=HAL_GetTick()-100;

            }
            disptr->LcdRunFlag.Bit.DispLayLcd = 1;
	disptr->LcdRunFlag.Bit.DispLayLcdrx = 1;

        }
       
    

        



    } else {
    

        disptr->DispDelayTimeSet.DispdelayTime1_1_1 = HAL_GetTick();
          disptr->DispdelayChange.dispdelay1_1_1 = 0;


    }






}

void		ReadLcdStatus(void)
{
static unsigned long	readstatustime;

static unsigned char	i,j,ERRORTIME;
if(readstatustime>HAL_GetTick())
{


	readstatustime=HAL_GetTick();



}

if((HAL_GetTick()-readstatustime)>500)
{


	LcdDispLayData.LcdRunFlag.Bit.ToReadPos=1;
	if(!(Lcd_StatusRead(0)|Lcd_StatusRead(1)))
		{
			LcdDispLayData.LcdRunFlag.Bit.ToReadPos=0;
			LcdPosaddress[0]=Lcdramaddress[0];
			LcdPosaddress[1]=Lcdramaddress[1];
			readstatustime=HAL_GetTick();
		
			ReadRamDataFun(0,&readdatatemp[0][0]);
			ReadRamDataFun(1,&readdatatemp[1][0]);
			ReadRamDataFun(2,&readdatatemp[2][0]);
			ReadRamDataFun(3,&readdatatemp[3][0]);
			if(cursoraddress<80)
				{
				Lcd_pos(0, cursoraddress);
				if(flashcmd==0x0d)
				Lcd_CmdWtite(0, 0x0d);


				}
			else{

				Lcd_pos(1, cursoraddress-80);	
				if(flashcmd==0x0d)
				Lcd_CmdWtite(1, 0x0d);

				}
		//	Lcd_CmdWtite(0, LcdPosaddress[0]);
		//	Lcd_CmdWtite(1, LcdPosaddress[1]);


	
			for(i=0;i<4;i++)
				{
					
				for(j=0;j<40;j++)
					{
					if(pinlcddata[i][j]!=readdatatemp[i][j])
						{
									if(ERRORTIME<2)
										{
											ERRORTIME++;
												break;

										}
									else{
										ERRORTIME=0;
									LcdDispLayData.LcdRunFlag.Bit.Lcdrwdiffent=1;
									break;

										}

						}

					}
			}
			if(i==4)
				{
					ERRORTIME=0;


				}
			

		}

			LcdDispLayData.LcdRunFlag.Bit.ToReadPos=0;




}












}


void		ReadRamDataFun(unsigned char	row,unsigned char	*datatemp)
{
			static unsigned char	internaladdress,i;
			if(row%2==0)
				{

					internaladdress=0x80;

				}
			else {

					internaladdress=0xC0;

				}
			Lcd_CmdWtite(row/2, internaladdress);
			delay1(20);
			for(i=0;i<40;i++)
				{
				
					*datatemp=Readaddressram(row/2);
					datatemp++;



				}
			







}

unsigned char		Readaddressram(unsigned char	chipcs)
{
	 static  GPIO_InitTypeDef GPIO_InitStruct;
	  	static Datac		databit;
	  GPIO_InitStruct.Pin = LD2_Pin|LD3_Pin|LD4_Pin|LD5_Pin  |LD6_Pin
                          |LD0_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LD7_Pin;
                         
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			Readlcd;
		delay1(10);

		HAL_GPIO_WritePin(L_RS_GPIO_Port, L_RS_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(L_RW_GPIO_Port, L_RW_Pin, GPIO_PIN_RESET);
	//L_RS=!0;
	//L_RW=!0;
	delay1(10);
		if(chipcs==0)
		{
		//	L_E1=!1;
		HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);

		}
	else{

	//L_E2=!1;
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);

		}
	delay1(10);

	

	
	databit.Bit.data7=HAL_GPIO_ReadPin(LD0_GPIO_Port, LD0_Pin);
	databit.Bit.data6=HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin);
	databit.Bit.data5=HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
	databit.Bit.data4=HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin);
	databit.Bit.data3=HAL_GPIO_ReadPin(LD4_GPIO_Port, LD4_Pin);
	databit.Bit.data2=HAL_GPIO_ReadPin(LD5_GPIO_Port, LD5_Pin);
	databit.Bit.data1=HAL_GPIO_ReadPin(LD6_GPIO_Port, LD6_Pin);
	databit.Bit.data0=HAL_GPIO_ReadPin(LD7_GPIO_Port, LD7_Pin);
	HAL_GPIO_WritePin(LE_2_GPIO_Port,LE_2_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(L_E1_GPIO_Port,L_E1_Pin,GPIO_PIN_SET);
	delay1(10);

	//for(i=0;i<100;i++);
	
		  GPIO_InitStruct.Pin = LD2_Pin|LD3_Pin|LD4_Pin|LD5_Pin  |LD6_Pin
                          |LD0_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LD7_Pin;
                         
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
delay1(10);

	return	databit.Value;







}


void		LcdReWrite(void)
{


if(LcdDispLayData.LcdRunFlag.Bit.Lcdrwdiffent)
{
	LcdDispLayData.LcdRunFlag.Bit.NoreadCmd=1;
	 Lcd_Init();
	 LcdDispLayData.LcdRunFlag.Bit.NoreadCmd=0;
	Lcd_Write_strlabel(0, 0,(const unsigned char	*)&pinlcddata[0][0],40);
	Lcd_Write_strlabel(0, 40,(const unsigned char	*)&pinlcddata[1][0],40);
	Lcd_Write_strlabel(1, 0,(const unsigned char	*)&pinlcddata[2][0],40);
	Lcd_Write_strlabel(1, 40,(const unsigned char	*)&pinlcddata[3][0],40);
	LcdDispLayData.LcdRunFlag.Bit.Lcdrwdiffent=0;
	//Lcd_CmdWtite(0, LcdPosaddress[0]);
	//Lcd_CmdWtite(1, LcdPosaddress[1]);
		if(cursoraddress<80)
				{
				Lcd_pos(0, cursoraddress);
				if(flashcmd==0x0d)
				Lcd_CmdWtite(0, 0x0d);


				}
			else{

				Lcd_pos(1, cursoraddress-80);	
				if(flashcmd==0x0d)
				Lcd_CmdWtite(1, 0x0d);

				}


}




}
/*
void		delay(unsigned long i)
{
//	static  unsigned  long j;
	//for(j=0;j<i;j++){Nop();Nop();}
	;
		

}
*/
