// Snake.cpp : Define el punto de entrada de la aplicación.
// Carlos Eugenio Cabrera Guerra    

/*
     Proceso
    1) El servidor se levante y queda esperando una conexión
    2) el cliente se conecta
    3) el cliente escribe en el socket el buffer con los elementos en forma de arreglo de caracteres
    4) el servidor recibe he interpreta los elementos para así visualizarlos
    5) de igual manera si la app con rol servidor envía elementos en buffer, el servidor de la app con rol cliente recibe he interpreta la información
    6) el envió por parte del cliente cierra el socket una vez enviada la información
    7) el cliente escribe a pantalla el resultado en formato numérico



    NOTAS DE LA APLICACIÓN
    - Elementos de la interfaz (100% cubierto)
        > la comida solo se crea en sv, por lo tanto el cliente siempre la recibe del buffer
    - Lógica del protocolo de comunicación (100% cubierto)
        > ponderación totalmente a criterio del profesor
    - Funcionalidad de la aplicación (75% cubierto)
        > la comida se visualiza en ambos lados simultáneamente
        > si la comida es comida o reposicionada se refleja en ambos lados
        > debido a la poca sincronización las serpientes tienden a desfasarse en una posición
        > se implementó la colisión entre serpientes
        > en el caso de que ambas snakes quieran comer una misma comida esta resultaria en
          que las snakes terminarian chocando una a la otra

*/

#include "framework.h"
#include "Snake.h"
#include <stdio.h>

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
#define SO      2

#define SERP1   1
#define SERP2   2
#define COMID   0
#define INIT    -1

#define IDT_TIMER1  1

#define TABLERO 400

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

// Banderas Globales
static BOOL Inicio_b = false;
static int CrearSala_b = 0;
static int UnirseSala_b = 0;
static BOOL band = true;
static int Modo = -1;


char szMiIP[17] = "127.0.0.1";              // Dirección IP
char szUsuario[32] = "Carlos";              // Nombre actual de usuario 

char ip_cliente[17];
char ip_servidor[17];

// Snake y Tamaño de Snake Globales
static PEDACITOS* serpiente = NULL;
static PEDACITOS* serpiente2 = NULL;
static int tams = 5;
static int tams2 = 5;

static BOOL INICIO = false;
static BOOL primer_msgSV = true;
static BOOL primer_msgCL = true;

DWORD WINAPI Servidor(LPVOID argumento);
int Cliente(char* szDirIP, PSTR pstrMensaje);

int EnviarMensaje(char*, HWND);

PEDACITOS* NuevaSerpiente(int, int, int);
void DibujarSerpiente(HDC, const PEDACITOS*);
int MoverSerpiente(PEDACITOS*, int, RECT, int);
PEDACITOS* AjustarSerpiente(PEDACITOS*, int*, int, RECT);

int Colisionar(const PEDACITOS*, int);
int Colisionar2Serps(PEDACITOS*, PEDACITOS*, int, int);
int Comer(const PEDACITOS*, int, RECT);
void CrearComida(RECT rect);


void CrearBuffer(int,int,int,int,int,int);
void ColocarComida(int, int,int);

int MovVal(PEDACITOS*, int, int);
void Procesar(char*);

char* NumChar(int);

static HANDLE hHiloServidor;
static DWORD idHiloServidor;
static HANDLE hHiloServidor2;
static DWORD idHiloServidor2;

char buffer[256];

static HWND hEnviar, hIP, hIPCliente;

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
    if (!InitInstance(hInstance, nCmdShow))
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

    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNAKE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SNAKE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
        300, 100, TABLERO + 300, TABLERO + 61, nullptr, nullptr, hInstance, nullptr);

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
    
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect{};
    static HPEN hpen1, hpen2, hpenC1, hpenC2, hpenN;

    switch (message)
    {
    case WM_CREATE:
    {
        serpiente = NuevaSerpiente(tams, 0, 1);
        serpiente2 = NuevaSerpiente(tams2, 0, 5);

        hpen1 = CreatePen(PS_SOLID, 3, RGB(255, 80, 80));
        hpen2 = CreatePen(PS_SOLID, 3, RGB(60, 200, 60));
        hpenC1 = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
        hpenC2 = CreatePen(PS_SOLID, 3, RGB(0, 0, 255));
        hpenN = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));

        SetTimer(hWnd, IDT_TIMER1, 1000, NULL);

        hIP = CreateWindowEx(0, L"EDIT", L"", ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, TABLERO + 10, 70, 200, 30, hWnd, (HMENU)IDC_EDITIP, hInst, NULL);
        hIPCliente = CreateWindowEx(0, L"EDIT", L"", ES_LEFT | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, TABLERO + 10, 120, 200, 30, hWnd, (HMENU)IDC_EDITIP, hInst, NULL);
        hEnviar = CreateWindowEx(0, L"BUTTON", L"Conectar", BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP, TABLERO + 215, 70, 65, 30, hWnd, (HMENU)IDC_BOTONENVIAR, hInst, NULL);

        ShowWindow(hEnviar, FALSE);
        ShowWindow(hIPCliente, FALSE);
        ShowWindow(hIP, FALSE);
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

                cuenta++;
                if (cuenta == 10) {
                    if (Modo == SV) {
                        CrearComida(rect);

                        strcpy(buffer, "");
                        CrearBuffer(COMID, 0, 0, com.pos.x, com.pos.y, com.tipo);
                        EnviarMensaje(buffer, hIP);


                    }                   
                    cuenta = 0;
                }
                if (Comer(serpiente, tams, rect)) {
                    serpiente = AjustarSerpiente(serpiente, &tams, com.tipo, rect);
                }
                if (Comer(serpiente2, tams2, rect)) {
                    serpiente2 = AjustarSerpiente(serpiente2, &tams2, com.tipo, rect);
                }

                SetFocus(hWnd);
            }

            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        default:
            break;
        }
    }


    /*
    En el apartado de WM_KEYDOWN en caso de estar en modo solitario, los movimientos
    de la serpiente son directos, utilizando la funcion MovVal se valida y se cambia de dirección
    la serpiente, pero no se cambia la posicion, esto es para que solo el timer tenga la capacidad de
    mover las serpientes y actualizar el area de juego.

    En caso de no estar en modo solitario, se tomas la direccion pulsada, el modo actual (SV o CL) y el tamaño,
    estos datos se pasan como parametros al buffer.

    El buffer tiene la funcionalidad de contener los datos que se quieren mandar por el hilo ya sea de Cliente a Servidor
    o viceversa como parte de su protocolo de comunicación.
    Se utiliza la funcion strcat() para concatenar el dato que se quiere agregar al buffer.

    Una vez finalizada la construccion de la cadena, se envia en la funcion EnviarMensaje(), junto con el HWND
    que contiene la dirección ip para su posterior extracción.

    */
    case WM_KEYDOWN: {
        GetClientRect(hWnd, &rect);

        switch (wParam)
        {
        case VK_UP: {
            strcpy(buffer, "");
            if (Modo == SO) {
                MovVal(serpiente, ARRIBA, tams);
            }
            else {
                if (Modo == SV) {
                    CrearBuffer(SERP1, ARRIBA, tams, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }
                if (Modo == CL) {
                    CrearBuffer(SERP2, ARRIBA, tams2, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }

            }

            break;
        }
        case VK_DOWN: {
            strcpy(buffer, "");
            if (Modo == SO) {
                MovVal(serpiente, ABAJO, tams);
            }
            else {
                if (Modo == SV) {
                    CrearBuffer(SERP1, ABAJO, tams, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }
                if (Modo == CL) {
                    CrearBuffer(SERP2, ABAJO, tams2, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }

            }
            

            break;
        }
        case VK_LEFT: {
            strcpy(buffer, "");
            if (Modo == SO) {
                MovVal(serpiente, IZQ, tams);
            }
            else {
                if (Modo == SV) {
                    CrearBuffer(SERP1,IZQ,tams,com.pos.x,com.pos.y,com.tipo);
                    EnviarMensaje(buffer, hIP);
                }
                if (Modo == CL) {
                    CrearBuffer(SERP2, IZQ, tams2, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }
            }

            break;
        }
        case VK_RIGHT: {
            strcpy(buffer, "");
            if (Modo == SO) {
                MovVal(serpiente, DER, tams);
            }
            else {
                if (Modo == SV) {
                    CrearBuffer(SERP1, DER, tams, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }
                if (Modo == CL) {
                    CrearBuffer(SERP2, DER, tams2, com.pos.x, com.pos.y, com.tipo);
                    EnviarMensaje(buffer, hIP);
                }

            }

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
            if (Modo == CL) {
                CrearBuffer(INIT, 0, 0, 0,0, 0);
                EnviarMensaje(buffer, hIP);
            }

            SetFocus(hWnd);
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        }
        case ID_NUEVOJUEGO_SOLO: {                                                                          // Se crea una sola serpiente en caso de estar en modo Solo
            if (serpiente != NULL) {

                KillTimer(hWnd, IDT_TIMER1);
                free(serpiente);
                free(serpiente2);
                tams = 5;
                cuenta = 0;
                serpiente = NuevaSerpiente(tams, 0, 1);
                SetTimer(hWnd, IDT_TIMER1, 1000, NULL);
                InvalidateRect(hWnd, NULL, TRUE);
                Inicio_b = true;
            }
            Modo = SO;
            UnirseSala_b = 0;
            CrearSala_b = 0;
            break;
        }
        case ID_NUEVOJUEGO_CREARSALA: {                                                                     // Se crea un hilo correspondiente al servidor para crear las conexiones 
            hHiloServidor = CreateThread(NULL, 0, Servidor, (LPVOID)hWnd, 0, &idHiloServidor);                  // con el cliente, en la función CreateThread se introducen los parametos:
                                                                                                            // Tipo de seguridad, tamaño de pila, funcion inicializadora, argumento de función
                                                                                                            // y el identificador del hilo.
            if (hHiloServidor == NULL) {
                MessageBox(hWnd, L"Error al crear el hilo servidor", L"Error", MB_OK | MB_ICONERROR);       // Validación de creacion de hilo
            }
            Modo = SV;                                                                                      // Se configuran banderas
            UnirseSala_b = 0;
            CrearSala_b = 1;
            SetFocus(hIP);
            break;
        }

        case ID_NUEVOJUEGO_UNIRSEASALA: {

            hHiloServidor2 = CreateThread(NULL, 0, Servidor, (LPVOID)hWnd, 0, &idHiloServidor2);            // Se crea un hilo correspondiente al cliente, esto es debido a que 
                                                                                                            // se necesita de un servidor en cada ventana para poder realizar la
                                                                                                            // comunicación de datos.
            if (hHiloServidor2 == NULL) {
                MessageBox(hWnd, L"Error al crear el hilo servidor", L"Error", MB_OK | MB_ICONERROR);       // Validación de creacion de hilo
            }

            UnirseSala_b = 1;                                                                              // Se configuran banderas
            CrearSala_b = 0;
            Modo = CL;
            SetFocus(hIP);
            break;
        }

        case IDM_ABOUT:
            MessageBox(hWnd, L"Snake. Cliente/Servidor. Windows\nCarlos Eugenio Cabrera Guerra\nGrupo 804, Ingenieria en Computacion", L"Acerca de", MB_OK );
            //DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
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

        /*
        Las serpientes no se mostraran en pantalla hasta que la bandera Inicio_b
        sea true, hasta entonces, el area de juego no mostrara ninguna sepiente
        */

        if (Inicio_b) {

            /*
            En caso de que sea en modo solitario (SO), solo se dibujara una serpiente
            */
            hpenTemp = (HPEN)SelectObject(hdc, hpen1);
            DibujarSerpiente(hdc, serpiente);
            SelectObject(hdc, hpenTemp);
            if (Modo != SO) {
                hpenTemp = (HPEN)SelectObject(hdc, hpen2);
                DibujarSerpiente(hdc, serpiente2);
                SelectObject(hdc, hpenTemp);
            }

        }

        if (com.tipo == CRECE) {
            hpenTemp = (HPEN)SelectObject(hdc, hpenC1);
            RoundRect(hdc, com.pos.x * TAMSERP,
                com.pos.y * TAMSERP,
                com.pos.x * TAMSERP + TAMSERP,
                com.pos.y * TAMSERP + TAMSERP,
                7, 7);
            SelectObject(hdc, hpenTemp);
        }
        else if (com.tipo == ACHICA) {
            hpenTemp = (HPEN)SelectObject(hdc, hpenC2);
            Ellipse(hdc, com.pos.x * TAMSERP,
                com.pos.y * TAMSERP,
                com.pos.x * TAMSERP + TAMSERP,
                com.pos.y * TAMSERP + TAMSERP);
            SelectObject(hdc, hpenTemp);
        }

        hpenTemp = (HPEN)SelectObject(hdc, hpenN);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, TABLERO, 0);
        MoveToEx(hdc, 0, 0, NULL);
        LineTo(hdc, 0, TABLERO);
        MoveToEx(hdc, TABLERO, 0, NULL);
        LineTo(hdc, TABLERO, TABLERO);
        MoveToEx(hdc, TABLERO, TABLERO, NULL);
        LineTo(hdc, 0, TABLERO);
        SelectObject(hdc, hpenTemp);



        /*
        Se utilizan banderas para representar estados del programa, tambien definen que información
        se debe mostrar en pantalla
        */
        if (Modo == SO) {
            TextOut(hdc, TABLERO + 10, 20, L"------------ MODO DE JUEGO SOLO------------", sizeof("------------ MODO DE JUEGO SOLO------------"));
            ShowWindow(hEnviar, FALSE);
            ShowWindow(hIP, FALSE);
        }
        else if (Modo == SV) {
            if (CrearSala_b == 1) {
                TextOut(hdc, TABLERO + 10, 20, L"------------ CREANDO UNA SALA ------------", sizeof("------------ CREANDO UNA SALA ------------"));
                TextOut(hdc, TABLERO + 10, 50, L"Introduce la IP del Cliente:", sizeof("Introduce la IP del Cliente:"));
                TextOut(hdc, TABLERO + 10, 120, L"Esperando Conexion...", sizeof("Esperando Conexion..."));
                //ShowWindow(hEnviar, TRUE);
                ShowWindow(hIP, TRUE);
            }
            if (CrearSala_b == 2) {
                TextOut(hdc, TABLERO + 10, 20, L"------------ CREANDO UNA SALA ------------", sizeof("------------ CREANDO UNA SALA ------------"));
                TextOut(hdc, TABLERO + 10, 120, L"Conectado", sizeof("Conectado"));
            }
            if (CrearSala_b == 3) {
                TextOut(hdc, TABLERO + 10, 20, L"------------ CREANDO UNA SALA ------------", sizeof("------------ CREANDO UNA SALA ------------"));
                TextOut(hdc, TABLERO + 10, 120, L"JUEGO INICIADO", sizeof("JUEGO INICIADO"));
            }
            TextOut(hdc, TABLERO + 10, TABLERO - 25, L"Rol: SERVIDOR.", sizeof("Rol: SERVIDOR."));
        }
        else if (Modo == CL) {
            if (UnirseSala_b == 1) {
                TextOut(hdc, TABLERO + 10, 20, L"------------ CONECTARSE A UNA SALA ----------", sizeof("------------ CONECTARSE A UNA SALA ----------"));
                TextOut(hdc, TABLERO + 10, 52, L"Introduce la IP del Servidor:", sizeof("Introduce la IP del Servidor:"));
                ShowWindow(hEnviar, TRUE);
                ShowWindow(hIP, TRUE);
            }
            if (UnirseSala_b == 2) {
                TextOut(hdc, TABLERO + 10, 20, L"------------ CONECTARSE A UNA SALA ----------", sizeof("------------ CONECTARSE A UNA SALA ----------"));
                TextOut(hdc, TABLERO + 10, 50, L"Introduce la IP del Servidor:", sizeof("Introduce la IP del Servidor:"));
                TextOut(hdc, TABLERO + 10, 120, L"Conectado.", sizeof("Conectado."));
            }
            TextOut(hdc, TABLERO + 10, TABLERO - 25, L"Rol: CLIENTE.", sizeof("Rol: CLIENTE."));
        }


        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        DeleteObject(hpen1);
        DeleteObject(hpen2);
        DeleteObject(hpenC1);
        DeleteObject(hpenC2);
        CloseHandle(hHiloServidor);     // Se cierran los hilos
        CloseHandle(hHiloServidor2);
        free(serpiente);                // Se librera la memoria ocupada por la serpiente
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

/*Función ya conocida*/
PEDACITOS* NuevaSerpiente(int tams, int x, int y) {
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
    serpiente[i].pos.x = tams + x - 1;
    serpiente[i].pos.y = y;
    serpiente[i].dir = DER;


    return serpiente;
}

/*
Función ya conocida, pero se agregaron elementos adicionales como la lengua
de la serpiente
*/
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

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2 - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2 + TAMSERP / 4);

        //TextOutA(hdc, serpiente[i].pos.x * TAMSERP , serpiente[i].pos.y * TAMSERP- TAMSERP, "s1", 2);
        /// /////////////////////////////////////////////////////////////


        break;
    case IZQ:
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP,
            serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2
        );
        Ellipse(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.x * TAMSERP + TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP
        );

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP - TAMSERP / 4 - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2 - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP - TAMSERP / 4 - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP + TAMSERP / 2 + TAMSERP / 4);

        //TextOutA(hdc, serpiente[i].pos.x* TAMSERP, serpiente[i].pos.y* TAMSERP - TAMSERP, "s1", 2);

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

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2 + TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4 - TAMSERP / 4);

        MoveToEx(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4, NULL);

        LineTo(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP / 2 - TAMSERP / 4,
            serpiente[i].pos.y * TAMSERP - TAMSERP / 4 - TAMSERP / 4);

        //TextOutA(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP+ TAMSERP/4, serpiente[i].pos.y * TAMSERP , "s1", 2);

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


        //TextOutA(hdc, serpiente[i].pos.x * TAMSERP + TAMSERP + TAMSERP / 4, serpiente[i].pos.y * TAMSERP, "s1", 2);

        break;
    default:
        break;
    }

    SelectObject(hdc, hpenTemp);
    SelectObject(hdc, hbrTemp);

    DeleteObject(hpenN);
    DeleteObject(hbrB);
}

/*Función ya conocida*/
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
            serpiente[i].pos.x = (TABLERO / TAMSERP) - 1;
        break;
    case ARRIBA:
        serpiente[i].pos.y = serpiente[i].pos.y - 1;
        if (serpiente[i].pos.y < 0)
            serpiente[i].pos.y = (TABLERO / TAMSERP) - 1;
        break;
    case ABAJO:
        serpiente[i].pos.y = serpiente[i].pos.y + 1;
        if (serpiente[i].pos.y >= TABLERO / TAMSERP)
            serpiente[i].pos.y = 0;
        break;
    default:
        break;
    }
    if (Colisionar(serpiente, tams) || Colisionar2Serps(serpiente,serpiente2,tams,tams2)) { 
        return 0; 
    }
    else {
        return 1; 
    }
}

/*
La funcion MovVal es una variabte de MoverSerpiente, ya que su funcion
es solamente validar y asignar la direccion que se le envia.
A diferencia de MoverSerpiente, esta no aumenta en uno la posición
de la serpiente.
Ya que se esta utilizando el timer como principalmetodo de movimiento
de todo el area cliente, MoverSerpiente solo se utiliza en ese apartado.
MovVal permite modificar la dirección sin mover la serpiente, hasta que
el timer se actualize es cuando se vera el movimiento.
*/
int MovVal(PEDACITOS* serpiente, int dir, int tams) {
    int i = 0, a = 0;
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

/*Función ya conocida*/
int Colisionar(const PEDACITOS* serpiente, int tams) {
    int i = 0;
    while (serpiente[i].tipo != CABEZA) {
        if (serpiente[i].pos.x == serpiente[tams - 1].pos.x &&
            serpiente[i].pos.y == serpiente[tams - 1].pos.y) {
            return 1;
        }
        i++;
    }
    return 0;
}

/*
   Colisionar2Serps sirve para detectar en que momento alguna serpiente choca con otra.
*/

int Colisionar2Serps(PEDACITOS* serpiente, PEDACITOS* serpiente2, int tams, int tams2) {
    int i = 0;
    while (serpiente[i].tipo != CABEZA) {
        if (serpiente[i].pos.x == serpiente2[tams2 - 1].pos.x && 
            serpiente[i].pos.y == serpiente2[tams2 - 1].pos.y) {
            return 1;
        }
        i++;
    }
    return 0;
}

/*Función ya conocida*/
PEDACITOS* AjustarSerpiente(PEDACITOS* serpiente, int* tams, int comida, RECT rect) {
    int i;
    PEDACITOS cabeza = serpiente[*tams - 1];
    switch (comida)
    {
    case CRECE: {
        (*tams)++;
        serpiente = (PEDACITOS*)realloc(serpiente, sizeof(PEDACITOS) * (*tams));
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
    case ACHICA: {
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

/*
La funcion Comer valida si la posicion actual de la cabeza de una serpiente
coincide con el del alguna comida, en caso de ser así, se llama a la función
CrearComida, para que actualize la posición de una nueva comida en el tablero de juego.

En esta funcion tambien se envia un mensaje al socket, argumentando que tiene que
actualizar la posicion x, y y el tipo de la comida, la cual se refleja en el cliente tambien.

*/
int Comer(const PEDACITOS* serpiente, int tams, RECT rect) {
    if (serpiente[tams - 1].pos.x == com.pos.x &&
        serpiente[tams - 1].pos.y == com.pos.y) {
        com.tipo = NADA;
        CrearComida(rect);

        strcpy(buffer, "");
        CrearBuffer(COMID, 0, 0, com.pos.x, com.pos.y, com.tipo);
        EnviarMensaje(buffer, hIP);
        return 1;
    }
    return 0;
}

/*
CrearComida crea en el rango del tablero de juego la comida para las
serpientes. En teoria utiliza la funcion rand() de windows que devuelve un
numero aleatorio.
Y digo en teoria porque no lo hace, si se crean dos ventanas y se inicia el
juego, la comida saldra en las mismas posicones consecutivas.
*/
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


/*
La función EnviarMensaje() recibe el buffer y el Hwnd que contiene la dirección ip
Su funcion en general extrae la ip de su HWND, luego utiliza la función malloc() que
sirve para la asignación de memoria dinamica. Luego de eso se copia el mensaje al buffer ptch,
despues ptchBuffer a pstrBuffer, este sera el que se enviara por medio de la funcion Cliente()
al igual que la direccion IP.

*/

int EnviarMensaje(char* hEscribir, HWND hIP) {
    TCHAR tchDirIP[16];
    char szDirIP[16];
    int tam = 0;
    size_t i;

    GetWindowText(hIP, tchDirIP, 16);                   // Copiar el contenido de la caja de texto
    tam = GetWindowTextLength(hIP);                     // Tamaño de la cadena
    wcstombs(szDirIP, tchDirIP, tam);                   // Conversion de TCHAR a char
    szDirIP[tam] = '\0';                                // Fin de cadena



    long iLength;
    PSTR pstrBuffer;
    TCHAR* ptchBuffer;

    //iLength = GetWindowTextLength(hEscribir);
    iLength = strlen(hEscribir);

    if (NULL == (pstrBuffer = (PSTR)malloc(sizeof(char) * (iLength + 2))) ||
        NULL == (ptchBuffer = (TCHAR*)malloc(sizeof(TCHAR) * (iLength + 2))))
        MessageBox(NULL, L"Error al reservar memoria", L"Error", MB_OK | MB_ICONERROR);
    else {

        //GetWindowText(hEscribir, ptchBuffer, iLength + 1);     
        swprintf(ptchBuffer, 30, L"%hs", hEscribir);            // Copiamos lo que tiene la caja de mensjes a enviar

        wcstombs_s(&i, pstrBuffer, (iLength + 1), ptchBuffer, (iLength + 1));
        pstrBuffer[iLength + 1] = '\0';

        strcpy(ip_cliente, szDirIP);
        Cliente(szDirIP, pstrBuffer);

        free(pstrBuffer);
        free(ptchBuffer);
    }
    return 1;
}


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

    //CrearSala_b = 1;
    

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

        ClientSocket = accept(ListenSocket, NULL, NULL);
        //Inicio_b = true;

        CrearSala_b = 2;

        if (ClientSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"acept failed with error: %d", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en servidor", MB_OK | MB_ICONERROR);
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        /*****      Enviar mensajes al cliente     ******/

        // Recibir hasta que el par cierra la conexión

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);//2
        sscanf(szBuffer, "%s %s", szIP, szNN);
        sprintf_s(szBuffer, "Ok");

        // Enviar el búfer al remitente

        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);//3

        iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);//6

        iResult = shutdown(ClientSocket, SD_SEND);
        iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);//7

        Procesar(szBuffer);  
    }

    // Limpiar

    closesocket(ClientSocket);
    WSACleanup();

    return 1;
}

//sv                cl
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


    //SetWindowText(hChat, szUsuario);
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);                             // Se inicia el WinSock
    if (iResult != 0) {
        wsprintf(msgFalla, L"WSAStartup failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);

        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));                                                  // Se limpa la memoria que configura la estructura del
    hints.ai_family = AF_UNSPEC;                                                        // control de los sockets
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolver la dirección y el puerto del servidor, se utiliza la dirección IP
    iResult = getaddrinfo(szDirIP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        wsprintf(msgFalla, L"getaddrinfo failed with error: %d\n", iResult);
        MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
        WSACleanup();

        return 1;
    }

    // Se intenta conectarse a una dirección hasta que una tenga éxito

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        // Crear el SOCKET para conectar al servidor
        if (ConnectSocket == INVALID_SOCKET) {
            wsprintf(msgFalla, L"socket failed with error: %d\n", WSAGetLastError());
            MessageBox(NULL, msgFalla, L"Error en cliente", MB_OK | MB_ICONERROR);
            WSACleanup();

            return 1;
        }
        // Se conectar al server
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {                                                                  // Se valida llaamada de conexión al socket valido
        MessageBox(NULL, L"Unable to connect to server!\n", L"Error en  cliente", MB_OK | MB_ICONERROR);
        sprintf(szMsg, "Error en la llamada a connect\nla dirección %s no es válida", szDirIP);
        //Mostrar_Mensaje(hChat, localhost, chat, szMsg, RGB(255, 0, 0));
        WSACleanup();
        return 1;
    }

    UnirseSala_b = 2;


    /******     Envio de mensajes al servidor       *******/

    sprintf(szMsg, "%s %s", szDirIP, szUsuario);

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);//1            // Enviar IP y nombre de Usuario
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);//4            // Recibir confirmación de conexión "ok"

    strcpy_s(szMsg, pstrMensaje);                                             // Cargar Mensaje de chat

    iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);//5            // Enviar Mensaje
    iResult = shutdown(ConnectSocket, SD_SEND);
    iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);//8            // Recibir el mismo mensaje enviado

    Procesar(szMsg);                                
    

    closesocket(ConnectSocket);                                             // Cerrar el socket
    WSACleanup();

    return 1;
}

/*
Esta funcion recibe como parametro un numero entero, para
despues convertirlo y retornar su variante char respectivamente.
*/
char* NumChar(int num) {
    char NumChar[3] = { num + '0' };
    strcat(NumChar, " ");
    return NumChar;
}

/*
    La funcion procesar se podria tomar como clave en el protocolo
    ya que descompone el mensaje recibido para asi interpretarlo
    de manera que permite mover una serpiente especifica, en una
    dirección especifica, junto con su tamaño actual o recibir informacion
    adicional como lo es actualizar la posicion de la comida en pantalla y 
    dar inicio al juego luego de haber creando las serpientes.

    [0] Identificador
    [1] Dirección de serpiente
    [2] Tamaño de serpiente
    [3] Comida x
    [4] Comida y
    [5] Comida tipo

    Ejemplo...

    Cadena = "0 3 3 100 50 1"                     // Protocolo pensado 
             [0][1][2][3][4][5]

*/


void Procesar(char* szMsg) {

        int id, dir, tam, cpx, cpy, ctipo;
        int aux;
        aux = sscanf(szMsg, "%d %d %d %d %d %d", &id, &dir, &tam, &cpx, &cpy, &ctipo);

        if (id == SERP1) {                              // Dependiendo de el valor identificador, ahora guardado en id, si el valor 
            MovVal(serpiente, dir, tam);                // es 1 o 2 movera una determinada serpiente en una dirección dir, con
        }                                               // su respectivo tamaño tam.
        if (id == SERP2) {
            MovVal(serpiente2, dir, tam);
        }
        if (id == COMID) {                              // En caso de coincidir con COMID, significara que se le da una nueva instruccion de
            ColocarComida(cpx, cpy, ctipo);             // actualizacion de la comida, tomando sus valores x,y y tipo.
        }
        
        if (id == INIT) {                                                   // En caso de que id coincida con INIT, significa que da inicio 
                                                                            // al juego, creando las serpientes y volviendo true la bandera
                if (serpiente != NULL) {                                    // de inicio.
                    tams = 5;
                    cuenta = 0;
                    serpiente = NuevaSerpiente(tams, 0, 1);
                }
                if (serpiente2 != NULL) {
                    tams2 = 5;
                    cuenta = 0;
                    serpiente2 = NuevaSerpiente(tams2, 0, 5);
                }
            
            Inicio_b = true;
          
        }
        
}
/*
    La funcion CrearBuffer crea una cadena de caracteres al buffer a enviar,
    se podria tomar como la herramienta principal para el funcionamiento de la
    conexión, ya que construye una cadena con la informacion de cliente o servidor.
 
*/
void CrearBuffer(int serp, int dir, int tam, int cpx, int cpy, int ctipo) {
    sprintf(buffer, "%d %d %d %d %d %d", serp, dir, tam, cpx, cpy, ctipo);   
}

/*
    La funcion recibe la información de la comida a insertar en pantalla,
    para el cliente esta seria la unica forma en que aparesca comida.
*/
void ColocarComida(int cpx,int cpy,int ctipo){    
    com.pos.x = cpx;
    com.pos.y = cpy;
    com.tipo = ctipo;
}