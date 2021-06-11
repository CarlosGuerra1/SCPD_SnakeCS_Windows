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

struct Comida {
    POS pos;
    int tipo;
}; typedef struct Comida COMIDA;

COMIDA com = { {0,0}, NADA };
static int cuenta = 0; 
static BOOL Inicio_b = false;
static int CrearSala_b = 0;

static BOOL UnirseSala_b = false;
static int Modo = NULL;


char szMiIP[17] = "127.0.0.1";              // Dirección IP
char szUsuario[32] = "Carlos";              // Nombre actual de usuario 

char IP[17];
char ip[17] = "192.168.1.67";

static PEDACITOS* serpiente = NULL;
static PEDACITOS* serpiente2 = NULL;
static int tams = 5;
static int tams2 = 5;

static BOOL band = true;

void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color);

DWORD WINAPI Servidor(LPVOID argumento);
DWORD WINAPI Servidor2(LPVOID argumento);
int Cliente(char* szDirIP, PSTR pstrMensaje);

int EnviarMensaje(char*, HWND);
int EnviarMensaje2(char*, char*);

PEDACITOS* NuevaSerpiente(int,int,int);
void DibujarSerpiente(HDC, const PEDACITOS*);
int MoverSerpiente(PEDACITOS*, int, RECT, int);
PEDACITOS* AjustarSerpiente(PEDACITOS*, int *, int, RECT);

int Colisionar(const PEDACITOS*, int);
int Comer(const PEDACITOS *, int, RECT);
void CrearComida(RECT rect);



int server(char* szDirIP, LPSTR pstrMensaje);
int sv(char* szDirIP, LPSTR pstrMensaje);



int MovVal(PEDACITOS*, int,int);
void CrearSerpientes(HWND);
void Procesar( char* );
char* NumChar(int);

static HANDLE hHiloServidor;
static DWORD idHiloServidor;
static HANDLE hHiloServidor2;
static DWORD idHiloServidor2;

char buffer[30] = "";

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
    static HPEN hpen1,hpen2;



    
    switch (message)
    {
    case WM_CREATE:
        {   
       
        serpiente = NuevaSerpiente(tams,5,5);
        serpiente2 = NuevaSerpiente(tams2, 10, 10);

        hpen1 = CreatePen(PS_SOLID, 2, RGB(80, 255, 80));
        hpen2 = CreatePen(PS_SOLID, 2, RGB(255, 80, 80));


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
            strcpy(buffer, "");

            if (Modo == SV) {
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(ARRIBA));
                strcat(buffer, NumChar(tams));
               
            }
            if (Modo == CL) {
                strcat(buffer, NumChar(2));
                strcat(buffer, NumChar(ARRIBA));
                strcat(buffer, NumChar(tams2));
                
            }

            EnviarMensaje(buffer, hIP);
            break;
        }
        case VK_DOWN: {
            strcpy(buffer, "");

            if (Modo == SV) {
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(ABAJO));
                strcat(buffer, NumChar(tams));
                
            }
            if (Modo == CL) {
                strcat(buffer, NumChar(2));
                strcat(buffer, NumChar(ABAJO));
                strcat(buffer, NumChar(tams2));
                
            }
            EnviarMensaje(buffer, hIP);
            
            break;
        }
        case VK_LEFT: {
            strcpy(buffer, "");

            if (Modo == SV) {
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(IZQ));
                strcat(buffer, NumChar(tams));
                
            }
            if (Modo == CL) {
                strcat(buffer, NumChar(2));
                strcat(buffer, NumChar(IZQ));
                strcat(buffer, NumChar(tams2));
                
            }
            EnviarMensaje(buffer, hIP);
            
            break;
        } 
        case VK_RIGHT: {
            strcpy(buffer, "");

            if (Modo == SV) {
                strcat(buffer, NumChar(1));
                strcat(buffer, NumChar(DER));
                strcat(buffer, NumChar(tams));
                
            }
            if (Modo == CL) {
                strcat(buffer, NumChar(2));
                strcat(buffer, NumChar(DER));
                strcat(buffer, NumChar(tams2));
                
            }
            EnviarMensaje(buffer, hIP);
                      
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
                strcpy(buffer, "0 0 0 ");
                if (EnviarMensaje(buffer, hIP)) {
                    /*
                    if (serpiente != NULL) {                       
                        tams = 5;                        
                        serpiente = NuevaSerpiente(tams, 5, 5);
                        Inicio_b = true;
                        
                    }
                    if (serpiente2 != NULL) {
                        tams = 5;
                        serpiente2 = NuevaSerpiente(tams2, 10, 10);
                        Inicio_b = true;                        
                    }*/

                    cuenta = 0;
                    InvalidateRect(hWnd, NULL, TRUE);
                }

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
                //MessageBox(NULL, L"Modo SV", L"MODO SV", MB_OK);
                UnirseSala_b = false;
                break;
            }
            case ID_NUEVOJUEGO_UNIRSEASALA: {

                UnirseSala_b = true;
                CrearSala_b = 0;
                Modo = CL;
                //MessageBox(NULL, L"Modo CL", L"MODO CL", MB_OK);
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
            //HBRUSH hbrTemp;

            // TODO: Agregar cualquier código de dibujo que use hDC aquí...

            if (Inicio_b) {
                hpenTemp = (HPEN)SelectObject(hdc, hpen1);
 

                DibujarSerpiente(hdc, serpiente);

                SelectObject(hdc, hpenTemp);
     

                hpenTemp = (HPEN)SelectObject(hdc, hpen2);
      

                DibujarSerpiente(hdc, serpiente2); 

                SelectObject(hdc, hpenTemp);
     
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

            if (CrearSala_b==1) {
                TextOut(hdc, 510, 20, L"------------ Creando una Sala ------------", sizeof("------------ Creando una Sala ------------"));
                TextOut(hdc, 510, 50, L"Esperando Conexion...", sizeof("Esperando Conexion..."));
            }
            if (CrearSala_b == 2) {
                TextOut(hdc, 510, 20, L"------------ Creando una Sala ------------", sizeof("------------ Creando una Sala ------------"));
                TextOut(hdc, 510, 50, L"Conectado...", sizeof("Conectado..."));
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
        DeleteObject(hpen1);
        DeleteObject(hpen2);

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

    HPEN hpenTemp, hpenN;
    HBRUSH hbrTemp, hbrB;

    hpenN = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    hbrB = CreateSolidBrush(RGB(255, 255, 255));

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


    hpenTemp = (HPEN)SelectObject(hdc, hpenN);
    hbrTemp = (HBRUSH)SelectObject(hdc, hbrB);
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

    SelectObject(hdc, hpenTemp);
    SelectObject(hdc, hbrTemp);

    DeleteObject(hpenN);
    DeleteObject(hbrB);
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

int EnviarMensaje(char* hEscribir, HWND hIP) {
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
        swprintf(ptchBuffer, 30, L"%hs", hEscribir);

        wcstombs_s(&i, pstrBuffer, (iLength + 1), ptchBuffer, (iLength + 1));
        pstrBuffer[iLength + 1] = '\0';
        /*
        if (Modo == SV) {
            Cliente(IP, pstrBuffer);
        }
        else if (Modo == CL) {
            Cliente(szDirIP, pstrBuffer);
        }*/

        
        Cliente(ip, pstrBuffer);

        //SetWindowText(hEscribir, L"");
        free(pstrBuffer);
        free(ptchBuffer);
    }
    return 1;
}



/// ///////////////////////////////////////////////////////////////////////
/*
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

    CrearSala_b = 1;
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

        ClientSocket = accept(ListenSocket, NULL, NULL);

        CrearSala_b = 2;

        if (ClientSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"acept failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        else {
            if (band) {
                if (serpiente != NULL) {
                    tams = 5;
                    cuenta = 0;
                    serpiente = NuevaSerpiente(tams, 5, 5);
                }
                if (serpiente2 != NULL) {
                    tams = 5;
                    cuenta = 0;
                    serpiente2 = NuevaSerpiente(tams2, 15, 15);
                }
                InvalidateRect(hWnd, NULL, TRUE);
                band = false;
                Inicio_b = true;
            }
        }

        /*****      Enviar mensajes al cliente     *****

        // Recibir hasta que el par cierra la conexión

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);//2
        sscanf(szBuffer, "%s %s", szIP, szNN);
        sprintf_s(szBuffer, "Conectado PC");

        // Enviar el búfer al remitente

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);//3

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);//6

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);//7

        Procesar(szBuffer);
        /*
        if (Modo = SV) {
            MessageBox(NULL, L"AEA", L"CL", MB_OK);
        }
        if (Modo = CL) {
            MessageBox(NULL, L"AEA", L"CL", MB_OK);
        }

        iResult = shutdown(ClientSocket, SD_SEND);
    }

    // Limpiar

    closesocket(ClientSocket);
    WSACleanup();

    return 1;
}
*/

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

    CrearSala_b = 1;
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

        ClientSocket = accept(ListenSocket, NULL, NULL);

        CrearSala_b = 2;

        if (ClientSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"acept failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        else {
            if (band) {
                if (serpiente != NULL) {
                    tams = 5;
                    cuenta = 0;
                    serpiente = NuevaSerpiente(tams, 1, 1);                    
                }
                if (serpiente2 != NULL) {
                    tams = 5;
                    cuenta = 0;
                    serpiente2 = NuevaSerpiente(tams2, 1, 5);                    
                }
                InvalidateRect(hWnd, NULL, TRUE);
                band = false;
                Inicio_b = true;
            }
        }

        /*****      Enviar mensajes al cliente     ******/

        // Recibir hasta que el par cierra la conexión

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);//2
        sscanf(szBuffer, "%s %s", szIP, szNN);
        
        strcpy(IP, szIP);

        sprintf_s(szBuffer, "Ok");

        // Enviar el búfer al remitente

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);//3

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);//6
        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);//7

        /*
        WCHAR    str[256];
        MultiByteToWideChar(0, 0, szBuffer, 256, str, 256);        
        MessageBox(NULL, str, L"SERVER BUFFER", MB_OK);
        */
        Procesar(szBuffer);

        

        iResult = shutdown(ClientSocket, SD_SEND);
    }

    // Limpiar

    closesocket(ClientSocket);
    WSACleanup();

    return 1;
}

int Cliente(char* szDirIP, LPSTR pstrMensaje) {
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
                    wsprintf(msgFalla, L"socket invalido alv: %d\n", WSAGetLastError());
                    MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
                    ConnectSocket = INVALID_SOCKET;
                    continue;
                }
                
                break;
            }
        
        
        freeaddrinfo(result);
        //MessageBox(NULL, L"Despues de Free", L"Cliente", MB_OK);
        if (ConnectSocket == INVALID_SOCKET) {
            MessageBox(NULL, L"Unable to connect to server!\n", L"Error en  cliente", MB_OK | MB_ICONERROR);
            //sprintf(szMsg, "Error en la llamada a connect\nla dirección %s no es válida", szDirIP);
            //Mostrar_Mensaje(hChat, localhost, chat, szMsg, RGB(255, 0, 0));
            WSACleanup();

            return 1;
        }
        else {
            if (band) {
                if (serpiente != NULL) {
                    tams = 5;
                    cuenta = 0;
                    serpiente = NuevaSerpiente(tams, 1, 1);
                }
                if (serpiente2 != NULL) {
                    tams = 5;
                    cuenta = 0;
                    serpiente2 = NuevaSerpiente(tams2, 1, 5);
                }
                
                band = false;
                Inicio_b = true;
            }
            
        }

    }
    
    /******     Envio de mensajes al servidor       *******/

    sprintf(szMsg, "%s %s", szDirIP, szUsuario);

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);//1            // Enviar IP y nombre de Usuario
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);//4            // Recibir confirmación de conexión "ok"


    sprintf(szMsg, "%s%s", pstrMensaje, szDirIP);
    //strcpy_s(szMsg, pstrMensaje);                                             // Cargar Mensaje de chat


    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);//5            // Enviar Mensaje
 
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);//8            // Recibir el mismo mensaje enviado
    /*
    WCHAR    str[256];
    MultiByteToWideChar(0, 0, szMsg, 256, str, 256);        
    MessageBox(NULL, str, L"CLIENTE BUFFER", MB_OK);
    */
    Procesar(szMsg);
    iResult = shutdown(ConnectSocket, SD_SEND);
    closesocket(ConnectSocket);                                             // Cerrar el socket
    WSACleanup();

    return 1;
}


char* NumChar(int num) {    
    char NumChar[3] = {num + '0' };
    strcat(NumChar, " ");
    return NumChar;
}

void CrearSerpientes(HWND hWnd) {
    SetTimer(hWnd, IDT_TIMER1, 500, NULL);
    cuenta = 0;

    if (serpiente != NULL) {
        KillTimer(hWnd, IDT_TIMER1);
        free(serpiente);
        tams = 5;
        serpiente = NuevaSerpiente(tams, 5, 5);        
    }
    if (serpiente2 != NULL) {
        KillTimer(hWnd, IDT_TIMER1);
        free(serpiente2);
        tams2 = 5;        
        serpiente2 = NuevaSerpiente(tams2, 10, 10);        
    }

    Inicio_b = true;
    InvalidateRect(hWnd, NULL, TRUE);
}

void Procesar(char* szMsg) {

    char S[2];
    char D[2];
    char T[2];
    int Si =0, Di=0, Ti=0;

    sscanf(szMsg, "%s %s %s %s", S, D,T,IP);
    
    Si = atoi(S);
    Di = atoi(D);
    Ti = atoi(T);
    if (Si != 0) {

        if (Si == 1 ) {   

           //MovVal(serpiente, Di, Ti);
           MovVal(serpiente, Di, Ti);
           //MessageBox(NULL, L"s1", L"Si = 1", MB_OK);
        }
        if(Si == 2 ){

           MovVal(serpiente2, Di, Ti);
           //MessageBox(NULL, L"s2", L"Si=2", MB_OK);
        }
    }
   
    //WCHAR    str[256];
    //MultiByteToWideChar(0, 0, szMsg, 256, str, 256);
    /*
    if (Modo == CL) {
        MessageBox(NULL, str, L"CL", MB_OK);
    }
    if (Modo == SV) {
        MessageBox(NULL, str, L"SV", MB_OK);
    }*/
}

