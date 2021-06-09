// Snake.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#include "Snake.h"

// Librerias para sockets
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "comctl32.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 		512
#define DEFAULT_PORT		"4200"

#define MAX_LOADSTRING 100

#define TAMSERP 20

#define CUERPO  1
#define CABEZA  2
#define COLA    3

#define IZQ     1
#define DER     2
#define ARRIBA  3
#define ABAJO   4

#define CRECE   1
#define ACHICA  2
#define NADA    3

#define SV      0
#define CL      1

#define IDT_TIMER1  1

#define TABLERO 500

struct pos {
    int x;
    int y;
}; typedef struct pos POS;

struct PedacitoS {
    POS pos;
    int tipo;
    int dir;
    int id;
}; typedef struct PedacitoS PEDACITOS;
/*
struct Snakes {
    PEDACITOS* peds;
    int id;
}; typedef struct Snakes SNAKES;
*/

struct Comida {
    POS pos;
    int tipo;
}; typedef struct Comida COMIDA;

COMIDA com = { {0,0}, NADA };
static int cuenta = 0; 
static BOOL Inicio_b = false;
static BOOL CrearSala_b = false;
static int Cliente_b = 0;
static BOOL UnirseSala_b = false;
static int Modo = SV;


char szMiIP[17] = "127.0.0.1";              // Dirección IP
char szUsuario[32] = "Carlos";              // Nombre actual de usuario 

static PEDACITOS* serpiente = NULL;
static PEDACITOS* serpiente2 = NULL;
static int tams = 5;
static int tams2 = 5;
static BOOL band = true;

DWORD WINAPI Servidor(LPVOID argumento);
DWORD WINAPI Servidor_Snake(LPVOID argumento);
int Cliente(char* szDirIP, PSTR pstrMensaje);
int Cliente_Snake(char* szDirIP, PSTR pstrMensaje, RECT rect);

void EnviarMensaje(char*, HWND, RECT, HWND);


PEDACITOS* NuevaSerpiente(int,int,int);
//SNAKES* NS(int,int);

void DibujarSerpiente(HDC, const PEDACITOS*);
void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color);
int MoverSerpiente(PEDACITOS*, int, RECT, int);
PEDACITOS* AjustarSerpiente(PEDACITOS*, int *, int, RECT);
int Colisionar(const PEDACITOS*, int);
int Comer(const PEDACITOS *, int, RECT);

void CrearComida(RECT rect);
int MovVal(PEDACITOS*, int,int);
void M_Serp(char*, RECT);
void CrearSerpientes(HWND);

char* NumChar(int);

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Colocar código aquí.

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SNAKE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SNAKE));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SNAKE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Almacenar identificador de instancia en una variable global

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       300, 100, TABLERO +300, TABLERO +100, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - publicar un mensaje de salida y volver
//
//

/*
    [0] Serpiente
    [1] Dirección
    [2] Tamaño

    [3] Comida x
    [4] Comida y

    Ejemplo...

    Cadena = "0 3 3 100 50"
             [0][1][2][3][4]
*/


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEnviar, hIP;
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect{};

      
    //static SNAKES* snakes = NULL;


    char buffer[20] = "";

    static HANDLE hHiloServidor;
    static DWORD idHiloServidor;
    
    switch (message)
    {
    case WM_CREATE:
        {   
       
        serpiente = NuevaSerpiente(tams,3,1);
        serpiente2 = NuevaSerpiente(tams2, 3, 1);

        //snakes = NS(2, tams);
        SetTimer(hWnd, IDT_TIMER1, 500, NULL);

        hIP = CreateWindowEx(0,
            L"EDIT",
            L"",
            ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER |
            WS_TABSTOP,
            510, 70,
            200, 30,
            hWnd,
            (HMENU)IDC_EDITIP,
            hInst,
            NULL);

        hEnviar = CreateWindowEx(0, L"BUTTON",
            L"Enviar",
            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE |
            WS_BORDER | WS_TABSTOP,
            715, 70,
            60, 30,
            hWnd, (HMENU)IDC_BOTONENVIAR,
            hInst,
            NULL);

        }
        break;

    case WM_TIMER: {
        switch (wParam)
        {
        case IDT_TIMER1: {
            GetClientRect(hWnd, &rect);
            if (Inicio_b) {
                if (!MoverSerpiente(serpiente, serpiente[tams - 1].dir, rect, tams)) {
                    KillTimer(hWnd, IDT_TIMER1);
                    MessageBox(hWnd, L"Ya se murió", L"Fin del juego", MB_OK | MB_ICONINFORMATION);
                }

                if (!MoverSerpiente(serpiente2, serpiente2[tams2 - 1].dir, rect, tams2)) {
                    KillTimer(hWnd, IDT_TIMER1);
                    MessageBox(hWnd, L"Ya se murió", L"Fin del juego", MB_OK | MB_ICONINFORMATION);
                }
            }
            cuenta++;
            if (cuenta == 30) {
                CrearComida(rect);

                cuenta = 0;
            }
            if (Comer(serpiente, tams, rect)) {
                serpiente = AjustarSerpiente(serpiente, &tams, com.tipo, rect);
            }
            if (Comer(serpiente2, tams2, rect)) {
                serpiente2 = AjustarSerpiente(serpiente2, &tams2, com.tipo, rect);
            }
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        default:
            break;
        }
    }



    case WM_KEYDOWN:{
        GetClientRect(hWnd, &rect);
      
        switch (wParam)
        {
        case VK_UP: {
            if (MovVal(serpiente,ARRIBA, tams)) {
                serpiente[tams - 1].dir = ARRIBA;
                strcat(buffer, NumChar(0));
                strcat(buffer, NumChar(ARRIBA));
                strcat(buffer, NumChar(tams));

                
                /*
                strcat(strcpy(buffer, "0"), " 3 ");
                strcat(buffer, tams + "0");*/
                
                /*
                char buffer[6] = "0 3 3";
                EnviarMensaje(buffer, hIP, rect);*/
            }
            if (MovVal(serpiente2, ARRIBA, tams2)) {
                serpiente2[tams2 - 1].dir = ARRIBA;
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(ARRIBA));
                strcat(buffer, NumChar(tams));
            }

            
            /*
            if (!MoverSerpiente(serpiente, ARRIBA, rect, tams)) {
                KillTimer(hWnd, IDT_TIMER1);
                MessageBox(hWnd, L"Ya se murió", L"Fin del juego", MB_OK | MB_ICONINFORMATION);
            }*/
            EnviarMensaje(buffer, hIP, rect,hWnd);
            break;
        }
        case VK_DOWN: {
            if (MovVal(serpiente, ABAJO,tams)) {
                serpiente[tams - 1].dir = ABAJO;
                strcat(buffer, NumChar(0));
                strcat(buffer, NumChar(ABAJO));
                strcat(buffer, NumChar(tams));

                
            }
            if (MovVal(serpiente2, ABAJO, tams2)) {
                serpiente2[tams2 - 1].dir = ABAJO;
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(ABAJO));
                strcat(buffer, NumChar(tams));
            }
            /*
            char buffer[6] = "1 3 3";
            EnviarMensaje(buffer, hIP, rect);*/

            /*
            if (!MoverSerpiente(serpiente, ABAJO, rect, tams)) {
                KillTimer(hWnd, IDT_TIMER1);
                MessageBox(hWnd, L"Ya se murió", L"Fin del juego", MB_OK | MB_ICONINFORMATION);
            }*/
            
            EnviarMensaje(buffer, hIP, rect,hWnd);
            break;
        }
        case VK_LEFT: {
            if (MovVal(serpiente, IZQ, tams)) {
                serpiente[tams - 1].dir = IZQ;
                strcat(buffer, NumChar(0));
                strcat(buffer, NumChar(IZQ));
                strcat(buffer, NumChar(tams));

                
            }
            if (MovVal(serpiente2, IZQ, tams2)) {   
                serpiente2[tams2 - 1].dir = IZQ;
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(IZQ));
                strcat(buffer, NumChar(tams));
            }

            //serpiente[tams - 1].dir = IZQ;
            /*
            if (!MoverSerpiente(serpiente, IZQ, rect, tams)) {
                KillTimer(hWnd, IDT_TIMER1);
                MessageBox(hWnd, L"Ya se murió", L"Fin del juego", MB_OK | MB_ICONINFORMATION);
            }*/
            EnviarMensaje(buffer, hIP, rect,hWnd);
            break;
        } 
        case VK_RIGHT: {
            if (MovVal(serpiente, DER, tams) && Modo == SV) {
                serpiente[tams - 1].dir = DER;
                strcat(buffer, NumChar(0));
                strcat(buffer, NumChar(DER));
                strcat(buffer, NumChar(tams));


            }
            if (MovVal(serpiente2, DER, tams2) && Modo == CL) {
                serpiente2[tams - 1].dir = DER;
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(DER));
                strcat(buffer, NumChar(tams));
            }
            //serpiente[tams - 1].dir = DER;
            /*
            if (!MoverSerpiente(serpiente, DER, rect, tams)) {
                KillTimer(hWnd, IDT_TIMER1);
                MessageBox(hWnd, L"Ya se murió", L"Fin del juego", MB_OK | MB_ICONINFORMATION);
            }*/

            EnviarMensaje(buffer, hIP, rect,hWnd);           
            break;
        }
        default:
            break;
        }

    }

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizar las selecciones de menú:
            switch (wmId)
            {
            case IDC_BOTONENVIAR: {
                                
                /*
                strcat(buffer, NumChar(0));
                strcat(buffer, NumChar(3));
                strcat(buffer, NumChar(3));
                
                strcat(buffer, "100 ");
                strcat(buffer, "50");
                */
                EnviarMensaje(buffer, hIP, rect,hWnd);


                Modo = CL;
                //EnviarMensaje((char *)"Hola", hIP,rect);
                //Cliente_Snake(szDirIP, (LPSTR)"Holas", rect);
                break;
            }   
            case ID_NUEVOJUEGO_SOLO: {
                if (serpiente != NULL) {
                    KillTimer(hWnd, IDT_TIMER1);
                    free(serpiente);
                    tams = 5;
                    cuenta = 0;
                    serpiente = NuevaSerpiente(tams,5,5);                   
                    SetTimer(hWnd, IDT_TIMER1, 500, NULL);
                    Inicio_b = true;
                    InvalidateRect(hWnd, NULL, TRUE);
                }

                if (serpiente2 != NULL) {
                    KillTimer(hWnd, IDT_TIMER1);
                    free(serpiente2);
                    tams2 = 5;
                    cuenta = 0;
                    serpiente2 = NuevaSerpiente(tams2, 10, 10);
                    SetTimer(hWnd, IDT_TIMER1, 500, NULL);
                    Inicio_b = true;
                    InvalidateRect(hWnd, NULL, TRUE);
                }


                UnirseSala_b = false;
                CrearSala_b = false;
                break;
            }
            case ID_NUEVOJUEGO_CREARSALA: {

                hHiloServidor = CreateThread(NULL,0,Servidor,(LPVOID)hWnd, 0,&idHiloServidor);
                if (hHiloServidor == NULL) {
                    MessageBox(hWnd, L"Error al crear el hilo servidor", L"Error", MB_OK | MB_ICONERROR);
                }
                Modo = SV;
                
                UnirseSala_b = false;
                break;
            }
            case ID_NUEVOJUEGO_UNIRSEASALA: {
                UnirseSala_b = true;
                CrearSala_b = false;


                //Cliente_Snake(char* szDirIP, LPSTR pstrMensaje, RECT rect)
                break;
            }

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            HPEN hpenTemp;
            // TODO: Agregar cualquier código de dibujo que use hDC aquí...

            if (Inicio_b) {
                DibujarSerpiente(hdc, serpiente);
                DibujarSerpiente(hdc, serpiente2);
            }
            
            if (com.tipo == CRECE) {
                RoundRect(hdc, com.pos.x * TAMSERP,
                    com.pos.y * TAMSERP,
                    com.pos.x * TAMSERP + TAMSERP,
                    com.pos.y * TAMSERP + TAMSERP,
                    7, 7);
            }
            else if (com.tipo == ACHICA) {
                Ellipse(hdc, com.pos.x* TAMSERP,
                    com.pos.y* TAMSERP,
                    com.pos.x* TAMSERP + TAMSERP,
                    com.pos.y* TAMSERP + TAMSERP);
            }
            

            MoveToEx(hdc, 0, 0, NULL);
            LineTo(hdc, TABLERO, 0);
            MoveToEx(hdc, 0, 0, NULL);
            LineTo(hdc, 0, TABLERO);
            MoveToEx(hdc, TABLERO,0, NULL);
            LineTo(hdc, TABLERO, TABLERO);
            MoveToEx(hdc, TABLERO, TABLERO, NULL);
            LineTo(hdc, 0, TABLERO);

            

            if (CrearSala_b) {
                TextOut(hdc, 510, 20, L"------------ Creando una Sala ------------", sizeof("------------ Creando una Sala ------------"));
                TextOut(hdc, 510, 50, L"Esperando Conexion...", sizeof("Esperando Conexion..."));
            }
                
            if (UnirseSala_b) {
                TextOut(hdc, 510, 20, L"------------Conectarse a una Sala ------------", sizeof("------------Conectarse a una Sala ------------"));
                TextOut(hdc, 510, 50, L"Introduce la IP del Servidor:", sizeof("Introduce la IP del Servidor:"));
                ShowWindow(hEnviar, TRUE);
                ShowWindow(hIP, TRUE);
            }
            else {
                ShowWindow(hEnviar, FALSE);
                ShowWindow(hIP, FALSE);
            }

            
                
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        free(serpiente);
        free(serpiente2);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


/*
SNAKES* NS(int num,int tams) {
    SNAKES* snake = NULL;

    snake = (SNAKES*)malloc(sizeof(SNAKES) * num);
    if (snake == NULL) {
        MessageBox(NULL, L"Sin memoria", L"Error", MB_OK | MB_ICONERROR);
        exit(0);
    }

    for (int i = 0; i < num; i++) {
        snake[i].id = i;
        snake[i].peds = NuevaSerpiente(tams);
    }

}*/

PEDACITOS * NuevaSerpiente(int tams,int x,int y) {
    PEDACITOS* serpiente = NULL;
    int i;

    if (tams < 2)
        tams = 2;
    serpiente = (PEDACITOS*)malloc(sizeof(PEDACITOS) * tams);
    if (serpiente == NULL) {
        MessageBox(NULL, L"Sin memoria", L"Error", MB_OK | MB_ICONERROR);
        exit(0);
    }
    serpiente[0].tipo = COLA;
    serpiente[0].pos.x = x;
    serpiente[0].pos.y = y;
    serpiente[0].dir = DER;
    for (i = 1; i < tams - 1; i++) {
        serpiente[i].tipo = CUERPO;
        serpiente[i].pos.x = i + x;
        serpiente[i].pos.y = y;
        serpiente[i].dir = DER;
    }
    serpiente[i].tipo = CABEZA;
    serpiente[i].pos.x = tams + x -1;
    serpiente[i].pos.y = y;
    serpiente[i].dir = DER;


    return serpiente;
}

void DibujarSerpiente(HDC hdc, const PEDACITOS* serpiente) {
    int i = 1;
    switch (serpiente[0].dir)
    {
    case DER:
        MoveToEx(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP, 
                      serpiente[0].pos.y * TAMSERP, NULL);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP,
                    serpiente[0].pos.y * TAMSERP + TAMSERP / 2);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP,
                    serpiente[0].pos.y * TAMSERP + TAMSERP);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP,
                    serpiente[0].pos.y * TAMSERP);

        
        break;
    case IZQ:
        MoveToEx(hdc, serpiente[0].pos.x * TAMSERP,
                      serpiente[0].pos.y * TAMSERP, NULL);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP,
                    serpiente[0].pos.y * TAMSERP + TAMSERP / 2);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP,
                    serpiente[0].pos.y * TAMSERP + TAMSERP);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP,
                    serpiente[0].pos.y * TAMSERP);
        break;
    case ARRIBA:
        MoveToEx(hdc, serpiente[0].pos.x * TAMSERP,
                      serpiente[0].pos.y * TAMSERP, NULL);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP / 2,
                    serpiente[0].pos.y * TAMSERP + TAMSERP);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP,
                    serpiente[0].pos.y * TAMSERP);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP,
                    serpiente[0].pos.y * TAMSERP);
        break;
    case ABAJO:
        MoveToEx(hdc, serpiente[0].pos.x * TAMSERP,
                      serpiente[0].pos.y * TAMSERP + TAMSERP, NULL);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP / 2,
                    serpiente[0].pos.y * TAMSERP);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP + TAMSERP,
                    serpiente[0].pos.y * TAMSERP + TAMSERP);
        LineTo(hdc, serpiente[0].pos.x * TAMSERP,
                    serpiente[0].pos.y * TAMSERP + TAMSERP);
        break;
    default:
        break;
    }
    while (serpiente[i].tipo != CABEZA) {
        RoundRect(hdc, serpiente[i].pos.x * TAMSERP,
            serpiente[i].pos.y * TAMSERP,
            serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP,
            5, 5);
        i++;
    }
    RoundRect(hdc, serpiente[i].pos.x * TAMSERP,
        serpiente[i].pos.y * TAMSERP,
        serpiente[i].pos.x * TAMSERP + TAMSERP,
        serpiente[i].pos.y * TAMSERP + TAMSERP,
        5, 5);

    switch (serpiente[i].dir)
    {
    case DER:
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP,
            serpiente[i].pos.y * TAMSERP,
            serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2
            );
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP
        );
        /// /////////////////////////////////////////////////////////////
 
        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP+ TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP/2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP/4,
            serpiente[i].pos.y * TAMSERP + TAMSERP /2);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2 - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2 + TAMSERP / 4);

        TextOutA(hdc, serpiente[i].pos.x * TAMSERP , serpiente[i].pos.y * TAMSERP- TAMSERP, "s1", 2);
        /// /////////////////////////////////////////////////////////////


        break;
    case IZQ:
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP,
            serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2
            );
        Ellipse(hdc, serpiente[i].pos.x* TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2,
            serpiente[i].pos.x* TAMSERP + TAMSERP,
            serpiente[i].pos.y* TAMSERP + TAMSERP
        );

        MoveToEx(hdc, serpiente[i].pos.x* TAMSERP ,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x* TAMSERP  - TAMSERP / 4,
            serpiente[i].pos.y* TAMSERP + TAMSERP/2);

        MoveToEx(hdc, serpiente[i].pos.x* TAMSERP - TAMSERP / 4,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x* TAMSERP - TAMSERP / 4 - TAMSERP / 4,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2 - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x* TAMSERP - TAMSERP / 4,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x* TAMSERP - TAMSERP / 4 - TAMSERP / 4,
            serpiente[i].pos.y* TAMSERP + TAMSERP / 2 + TAMSERP / 4);

        TextOutA(hdc, serpiente[i].pos.x* TAMSERP, serpiente[i].pos.y* TAMSERP - TAMSERP, "s1", 2);

        break;
    case ARRIBA:
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP
        );
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP
        );

        MoveToEx(hdc, serpiente[i].pos.x* TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y* TAMSERP , NULL);

        LineTo(hdc, serpiente[i].pos.x* TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y* TAMSERP - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4 - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x* TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y* TAMSERP - TAMSERP / 4, NULL);

        LineTo(hdc, serpiente[i].pos.x* TAMSERP + TAMSERP / 2 - TAMSERP / 4,
            serpiente[i].pos.y* TAMSERP - TAMSERP / 4 - TAMSERP / 4);

        TextOutA(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP+ TAMSERP/4, serpiente[i].pos.y * TAMSERP , "s1", 2);

        break;
    case ABAJO:
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP,
            serpiente[i].pos.y * TAMSERP,
            serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2
        );
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP,
            serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2
        );

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP + TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP + TAMSERP / 4, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP + TAMSERP / 4 + TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP + TAMSERP / 4, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2 - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP + TAMSERP / 4 + TAMSERP / 4);


        TextOutA(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4, serpiente[i].pos.y * TAMSERP, "s1", 2);

        break;
    default:
        break;
    }
}

int MovVal(PEDACITOS* serpiente, int dir,int tams) {
    int i=0,a=0;
    /*
    while (serpiente[i].tipo != CABEZA) {
        serpiente[i].dir = serpiente[i + 1].dir;
        serpiente[i].pos = serpiente[i + 1].pos;
        i++;
    }
    */
    switch (serpiente[tams - 1].dir)
    {

    case DER:
        if (dir != IZQ) {
            serpiente[tams - 1].dir = dir;            
            return 1;
        }
        break;
    case IZQ:
        if (dir != DER) {
            serpiente[tams - 1].dir = dir;
            return 1;
        }
        break;
    case ARRIBA:
        if (dir != ABAJO) {
            serpiente[tams - 1].dir = dir;
            return 1;
        }
        break;
    case ABAJO:
        if (dir != ARRIBA) {
            serpiente[tams - 1].dir = dir;
            return 1;
        }
        break;
    default:
        break;
    }


    return 0;
    

}

int MoverSerpiente(PEDACITOS* serpiente, int dir, RECT rect, int tams) {
    int i = 0;
    while (serpiente[i].tipo != CABEZA) {
        serpiente[i].dir = serpiente[i + 1].dir;
        serpiente[i].pos = serpiente[i + 1].pos;
        i++;
    }

    switch (serpiente[i].dir)
    {
    case DER:
        if (dir != IZQ)
            serpiente[i].dir = dir;
        break;
    case IZQ:
        if (dir != DER)
            serpiente[i].dir = dir;
        break;
    case ARRIBA:
        if (dir != ABAJO)
            serpiente[i].dir = dir;
        break;
    case ABAJO:
        if (dir != ARRIBA)
            serpiente[i].dir = dir;
        break;
    default:
        break;
    }

    switch (serpiente[i].dir)
    {

    case DER:
        serpiente[i].pos.x = serpiente[i].pos.x + 1;
        if (serpiente[i].pos.x >= TABLERO / TAMSERP)
            serpiente[i].pos.x = 0;
        break;
    case IZQ:
        serpiente[i].pos.x = serpiente[i].pos.x - 1;
        if (serpiente[i].pos.x < 0)
            serpiente[i].pos.x = (TABLERO / TAMSERP) -1;
        break;
    case ARRIBA:
        serpiente[i].pos.y = serpiente[i].pos.y - 1;
        if (serpiente[i].pos.y < 0)
            serpiente[i].pos.y = (TABLERO / TAMSERP) -1;
        break;
    case ABAJO:
        serpiente[i].pos.y = serpiente[i].pos.y + 1;
        if (serpiente[i].pos.y >= TABLERO / TAMSERP)
            serpiente[i].pos.y = 0;
        break; 
    default:
        break;
    }
    if (Colisionar(serpiente, tams)) { return 0; }
    else { return 1; }
}

int Colisionar(const PEDACITOS* serpiente, int tams) {
    int i = 0;
    while (serpiente[i].tipo != CABEZA) {
        if (serpiente[i].pos.x == serpiente[tams - 1].pos.x &&
            serpiente[i].pos.y == serpiente[tams - 1].pos.y){
            return 1;
        }
        i++;
    }
    return 0;
}

PEDACITOS* AjustarSerpiente(PEDACITOS *serpiente, int *tams, int comida, RECT rect) {
    int i;
    PEDACITOS cabeza = serpiente[*tams-1];
    switch (comida)
    {
    case CRECE: {
        (*tams)++;
        serpiente = (PEDACITOS*) realloc(serpiente, sizeof(PEDACITOS) * (*tams));
        serpiente[*tams - 2].tipo = CUERPO;
        //serpiente[*tams - 1].tipo = CABEZA;
        serpiente[*tams - 1] = cabeza;
        i = *tams - 1;
        switch (serpiente[i].dir)
        {

        case DER:
            serpiente[i].pos.x = serpiente[i].pos.x + 1;
            if (serpiente[i].pos.x >= rect.right / TAMSERP)
                serpiente[i].pos.x = 0;
            break;
        case IZQ:
            serpiente[i].pos.x = serpiente[i].pos.x - 1;
            if (serpiente[i].pos.x < 0)
                serpiente[i].pos.x = rect.right / TAMSERP;
            break;
        case ARRIBA:
            serpiente[i].pos.y = serpiente[i].pos.y - 1;
            if (serpiente[i].pos.y < 0)
                serpiente[i].pos.y = rect.bottom / TAMSERP;
            break;
        case ABAJO:
            serpiente[i].pos.y = serpiente[i].pos.y + 1;
            if (serpiente[i].pos.y >= rect.bottom / TAMSERP)
                serpiente[i].pos.y = 0;
            break;
        default:
            break;
        }

        break;
    }
    case ACHICA:{
        if (*tams > 2) {
            i = 0;
            while (serpiente[i].tipo != CABEZA) {
                serpiente[i] = serpiente[i + 1];
                i++;
            }
            (*tams)--;
            serpiente = (PEDACITOS*)realloc(serpiente, sizeof(PEDACITOS) * (*tams));
            serpiente[*tams - 1] = cabeza;

        }
    } 
    default:
        break;
    }
    return serpiente;
}

int Comer(const PEDACITOS* serpiente, int tams, RECT rect) {
    if (serpiente[tams - 1].pos.x == com.pos.x &&
        serpiente[tams - 1].pos.y == com.pos.y) {
        CrearComida(rect);
        return 1;
    }
    return 0;
}

void CrearComida(RECT rect) {
    if (rand() % 100 < 80) {
        com.tipo = CRECE;
    }
    else {
        com.tipo = ACHICA;
    }
    com.pos.x = rand() % TABLERO / TAMSERP;
    com.pos.y = rand() % TABLERO / TAMSERP;


        //MessageBox(NULL, L"Esperando conexión", L"Depuración", MB_OK);

    cuenta = 0;
}


///////////////////////////////////////////////////////////////////////

DWORD WINAPI Servidor(LPVOID argumento) {
    HWND hChat = (HWND)argumento;
    HWND hWnd = (HWND)argumento;
    WSADATA wsaData;
    int iResult;
    TCHAR msgFalla[256];
    TCHAR msg[256];

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    char szBuffer[256], szIP[16], szNN[32];

    if (true) {

        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
            MessageBox(NULL, msgFalla, L"Error em servidore", MB_OK | MB_ICONERROR);

            return 1;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            wsprintf(msgFalla, L"getaddrinfo failed with error: %d", iResult);
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            WSACleanup();

            return 1;
        }

        // Crear SOCKET para la coneccion al server
        ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"socket failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            freeaddrinfo(result);
            WSACleanup();

            return 1;
        }

        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            wsprintf(msgFalla, L"listen failed with error: %d\n", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            freeaddrinfo(result);
            closesocket(ListenSocket);
            WSACleanup();

            return 1;
        }

        freeaddrinfo(result);

        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR) {
            wsprintf(msgFalla, L"listen failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();

            return 1;
        }
    }

    while (TRUE) {
        //MessageBox(NULL, L"Esperando conexión", L"Depuración", MB_OK);       
        CrearSala_b = true;
        
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"acept failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        /*****      Enviar mensajes al cliente     ******/

        // Recibir hasta que el par cierra la conexión

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        sscanf(szBuffer, "%s %s", szIP, szNN);
        sprintf_s(szBuffer, "Ok");

        // Enviar el búfer al remitente

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);

        //Mostrar_Mensaje(hChat, szIP, szNN, szBuffer, RGB(34, 177, 76));

        WCHAR    str[256];
        MultiByteToWideChar(0, 0, szBuffer, 256, str, 256);
        MessageBox(NULL, str, L"Servidor", MB_OK);


        if (band) {
            CrearSerpientes(hWnd);
            band = false;
        }

        iResult = shutdown(ClientSocket, SD_SEND);
    }

    // Limpiar

    closesocket(ClientSocket);
    WSACleanup();

    return 1;
}

DWORD WINAPI Servidor_Snake(LPVOID argumento) {
    HWND hChat = (HWND)argumento;
    WSADATA wsaData;
    int iResult;
    TCHAR msgFalla[256];
    TCHAR msg[256];

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    int recvbuflen = DEFAULT_BUFLEN;
    char szBuffer[256], szIP[16], szNN[32];

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error em servidore", MB_OK | MB_ICONERROR);

        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        wsprintf(msgFalla, L"getaddrinfo failed with error: %d", iResult);
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        WSACleanup();

        return 1;
    }

    // Crear SOCKET para la coneccion al server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        wsprintf(msgFalla, L"socket failed with error: %d", WSAGetLastError());
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        freeaddrinfo(result);
        WSACleanup();

        return 1;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        wsprintf(msgFalla, L"listen failed with error: %d\n", WSAGetLastError());
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        wsprintf(msgFalla, L"listen failed with error: %d", WSAGetLastError());
        MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
        closesocket(ListenSocket);
        WSACleanup();

        return 1;
    }

    while (TRUE) {
        //MessageBox(NULL, L"Esperando conexión", L"Depuración", MB_OK);       
        CrearSala_b = true;

        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"acept failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        /*****      Enviar mensajes al cliente     ******/

        // Recibir hasta que el par cierra la conexión

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        sscanf_s(szBuffer, "%s %s", szIP, szNN);
        sprintf_s(szBuffer, "Ok");

        // Enviar el búfer al remitente

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);

        //Mostrar_Mensaje(hChat, szIP, szNN, szBuffer, RGB(34, 177, 76));

       
        wchar_t wtext[20];
        #pragma warning(suppress : 4996)
        mbstowcs(wtext, szBuffer, strlen(szBuffer) + 1);//Plus null
        LPWSTR ptr = wtext;
        MessageBox(NULL, ptr, L"Depuración", MB_OK);

        iResult = shutdown(ClientSocket, SD_SEND);
    }

    // Limpiar

    closesocket(ClientSocket);
    WSACleanup();

    return 1;
}

int Cliente(char* szDirIP, LPSTR pstrMensaje,HWND hWnd) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    char szMsg[256];                                      // Guarda la cadena de mensajes para entrada y salida
    char localhost[] = "localhost";
    char chat[] = "chat";
    TCHAR msgFalla[256];

    if (true) {

        //SetWindowText(hChat, szUsuario);
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
            MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);

            return 1;
        }

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Resolver la dirección y el puerto del servidor
        iResult = getaddrinfo(szDirIP, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) {
            wsprintf(msgFalla, L"getaddrinfo failed with error: %d\n", iResult);
            MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
            WSACleanup();

            return 1;
        }

        // Intente conectarse a una dirección hasta que una tenga éxito
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

            // Crear el SOCKET para conectar al servidor
            if (ConnectSocket == INVALID_SOCKET) {
                wsprintf(msgFalla, L"socket failed with error: %d\n", WSAGetLastError());
                MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
                WSACleanup();

                return 1;
            }

            // Connectar al server
            iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult == SOCKET_ERROR) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
                continue;
            }

            break;
        }

        freeaddrinfo(result);

        if (ConnectSocket == INVALID_SOCKET) {
            MessageBox(NULL, L"Unable to connect to server!\n", L"Error en  cliente", MB_OK | MB_ICONERROR);
            //sprintf(szMsg, "Error en la llamada a connect\nla dirección %s no es válida", szDirIP);
            //Mostrar_Mensaje(hChat, localhost, chat, szMsg, RGB(255, 0, 0));
            WSACleanup();

            return 1;
        }

    }

    /******     Envio de mensajes al servidor       *******/

    sprintf(szMsg, "%s %s", szDirIP, szUsuario);

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Enviar IP y nombre de Usuario
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Recibir confirmación de conexión "ok"

    strcpy_s(szMsg, pstrMensaje);                                             // Cargar Mensaje de chat

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Enviar Mensaje
    iResult = shutdown(ConnectSocket, SD_SEND);
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Recibir el mismo mensaje enviado

    //sprintf(prueba, "ojo %s", szMsg);
    //SetWindowText(hChat, prueba);
    
    //Mostrar_Mensaje(hChat, szMiIP, szUsuario, szMsg, RGB(0, 0, 255));       // Imprimir Mensaje en el editor dersación

    WCHAR    str[256];
    const char* ab = "0 0 1";
    MultiByteToWideChar(0, 0, szMsg, 256, str, 256);
    MessageBox(NULL, str, L"Cliente", MB_OK);

    if (band) {
        CrearSerpientes(hWnd);
        band = false;
    }

    closesocket(ConnectSocket);                                             // Cerrar el socket
    WSACleanup();

    return 1;
}

int Cliente_Snake(char* szDirIP, LPSTR pstrMensaje, RECT rect) {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    char szMsg[256];                                      // Guarda la cadena de mensajes para entrada y salida
    char localhost[] = "localhost";
    char chat[] = "chat";
    TCHAR msgFalla[256];

    //SetWindowText(hChat, szUsuario);
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);

        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolver la dirección y el puerto del servidor
    iResult = getaddrinfo(szDirIP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        wsprintf(msgFalla, L"getaddrinfo failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
        WSACleanup();

        return 1;
    }

    // Intente conectarse a una dirección hasta que una tenga éxito
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        // Crear el SOCKET para conectar al servidor
        if (ConnectSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"socket failed with error: %d\n", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
            WSACleanup();

            return 1;
        }

        // Connectar al server
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Unable to connect to server!\n", L"Error en  cliente", MB_OK | MB_ICONERROR);
        //sprintf(szMsg, "Error en la llamada a connect\nla dirección %s no es válida", szDirIP);
        //Mostrar_Mensaje(hChat, localhost, chat, szMsg, RGB(255, 0, 0));
        WSACleanup();

        return 1;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    

    sprintf_s(szMsg, "%s %s", szDirIP, szUsuario);

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Enviar IP y nombre de Usuario
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Recibir confirmación de conexión "ok"

    strcpy_s(szMsg, pstrMensaje);                                             // Cargar Mensaje 

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Enviar Mensaje
    iResult = shutdown(ConnectSocket, SD_SEND);
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);            // Recibir el mismo mensaje enviado

    //sprintf(prueba, "ojo %s", szMsg);
    //SetWindowText(hChat, prueba);

    //M_Serp(szMsg,rect);
   

    //Mostrar_Mensaje(hChat, szMiIP, szUsuario, szMsg, RGB(0, 0, 255));       // Imprimir Mensaje en el editor dersación

    closesocket(ConnectSocket);                                             // Cerrar el socket
    WSACleanup();

    return 1;
}

void M_Serp(char* szMsg, RECT rect) {
    char dir[2];
    char snake[2];
    char tams[2];
    int dir_i, snake_i,tams_i;

    sscanf_s(szMsg, "%s %s %s",snake, dir,tams);
    dir_i = atoi(dir);
    snake_i = atoi(snake);
    tams_i = atoi(tams);

    if (snake_i == 0) {
        MoverSerpiente(serpiente, dir_i, rect, tams_i);
    }else if(snake_i == 1) {
        MoverSerpiente(serpiente2, dir_i, rect, tams_i);
        }    

}

void EnviarMensaje(char *hEscribir, HWND hIP, RECT rect, HWND hWnd) {
    TCHAR tchDirIP[16];
    char szDirIP[16];
    int tam = 0;
    size_t i;
    /*
        if ((tchDirIP = (LPSTR)malloc(sizeof(char) * (16 + 2))) == NULL)
            MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);
    */
    GetWindowText(hIP, tchDirIP, 16);           // Copiar el contenido de la caja de texto
    tam = GetWindowTextLength(hIP);             // Tamaño de la cadena
    wcstombs(szDirIP, tchDirIP, tam);          // Conversion de TCHAR a char
    szDirIP[tam] = '\0';                       // Fin de cadena

    long iLength;
    PSTR pstrBuffer;
    TCHAR* ptchBuffer;

    //iLength = GetWindowTextLength(hEscribir);
    iLength = strlen(hEscribir);

    if (NULL == (pstrBuffer = (PSTR)malloc(sizeof(char) * (iLength + 2))) ||
        NULL == (ptchBuffer = (TCHAR*)malloc(sizeof(TCHAR) * (iLength + 2))))
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);
    else {

        //GetWindowText(hEscribir, ptchBuffer, iLength + 1);     // Copiamos lo que tiene la caja de mensjes a enviar
        swprintf(ptchBuffer, 20, L"%hs", hEscribir);

        wcstombs_s(&i, pstrBuffer, (iLength + 1), ptchBuffer, (iLength + 1));
        pstrBuffer[iLength + 1] = '\0';

        Cliente(szDirIP, pstrBuffer,hWnd);

        //SetWindowText(hEscribir, L"");
        free(pstrBuffer);
        free(ptchBuffer);
    }
}

void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color) {
    CHARFORMAT2 cf;                                                                         // Formato del texto
    size_t i;

    // El siguiente segmento de codigo da formato reemplazando texto
    // normal por texto con formato

    memset(&cf, 0, sizeof cf);                  // Se limpia la estructura del formato
    cf.cbSize = sizeof(CHARFORMAT2);            // Se fija el tamaño de la esctructura
    cf.dwMask = CFM_COLOR;                      // Se establece la mascara para que sea posible aplicar color al texto
    cf.crTextColor = color;

    TCHAR auxiliar[35];

    /*
    if ((auxiliar = (LPSTR)malloc(sizeof(char) * (35 + 2))) == NULL)
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);*/

    int tam = strlen(szUsuario);
    mbstowcs_s(&i, auxiliar, szUsuario, sizeof(szUsuario) + 2);                 // Empatar(lexema)
    //strcpy(auxiliar, szUsuario);

    SendMessage(hChat, EM_SETSEL, (WPARAM)iLength, (LPARAM)iLength + tam);      // Se establece un rango de texto a selecionar
    SendMessage(hChat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);           // Se aplica el formato al rango seleccionado
    SendMessage(hChat, EM_REPLACESEL, FALSE, (LPARAM)auxiliar);                 //Se reemplaza el rango seleccionado con el nuevo texto
    cf.crTextColor = RGB(0, 0, 0);
}


char* NumChar(int num) {    
    char NumChar[3] = {num + '0' };
    strcat(NumChar, " ");
    return NumChar;
}


void CrearSerpientes(HWND hWnd) {

    if (serpiente != NULL) {
        KillTimer(hWnd, IDT_TIMER1);
        free(serpiente);
        tams = 5;
        cuenta = 0;
        serpiente = NuevaSerpiente(tams, 5, 5);
        SetTimer(hWnd, IDT_TIMER1, 500, NULL);
        Inicio_b = true;
        InvalidateRect(hWnd, NULL, TRUE);
    }

    if (serpiente2 != NULL) {
        KillTimer(hWnd, IDT_TIMER1);
        free(serpiente2);
        tams2 = 5;
        cuenta = 0;
        serpiente2 = NuevaSerpiente(tams2, 10, 10);
        SetTimer(hWnd, IDT_TIMER1, 500, NULL);
        Inicio_b = true;
        InvalidateRect(hWnd, NULL, TRUE);
    }

}