#define UNICODE
#define _UNICODE
#define _USE_MATH_DEFINES  // Para constantes matemáticas

#include <Windows.h>
#include <commctrl.h>      // Controles comunes
#include <strsafe.h>
#include <cmath>           // Funciones matemáticas

// Enlazar con bibliotecas necesarias
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

// Variables globales para los controles
HWND hEdit1, hEdit2, hButtonAdd, hButtonSub, hButtonMul, hButtonDiv, hResult;

// Variables para la animación
const int CIRCLE_RADIUS = 30;
float circleAngle = 0.0f;

// Prototipos de funciones
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void PerformOperation(int operation);
void UpdateAnimation(HWND hwnd);
void DrawAnimatedCircle(HWND hwnd);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow
) {
    const wchar_t CLASS_NAME[] = L"InputWindowClass";
    
    // Inicializar controles comunes
    INITCOMMONCONTROLSEX icc = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Calculadora con Animación",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 
        500, 350,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Configurar temporizador para animación (16ms ≈ 60 FPS)
    SetTimer(hwnd, 1, 16, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateControls(hwnd);
            return 0;
            
        case WM_COMMAND:
            if ((HWND)lParam == hButtonAdd) PerformOperation(0);
            else if ((HWND)lParam == hButtonSub) PerformOperation(1);
            else if ((HWND)lParam == hButtonMul) PerformOperation(2);
            else if ((HWND)lParam == hButtonDiv) PerformOperation(3);
            return 0;
        
        case WM_TIMER:
            if (wParam == 1) {
                UpdateAnimation(hwnd);
                // Forzar repintado
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
            
        case WM_PAINT:
            DrawAnimatedCircle(hwnd);
            return 0;
            
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CreateControls(HWND hwnd) {
    // Crear etiquetas
    CreateWindowW(L"STATIC", L"Número 1:", 
                 WS_VISIBLE | WS_CHILD,
                 20, 30, 80, 25, hwnd, NULL, NULL, NULL);
                 
    CreateWindowW(L"STATIC", L"Número 2:", 
                 WS_VISIBLE | WS_CHILD,
                 20, 70, 80, 25, hwnd, NULL, NULL, NULL);
                 
    // Crear campos de entrada
    hEdit1 = CreateWindowW(L"EDIT", L"0", 
                          WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                          110, 30, 150, 25, hwnd, NULL, NULL, NULL);
                          
    hEdit2 = CreateWindowW(L"EDIT", L"0", 
                          WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                          110, 70, 150, 25, hwnd, NULL, NULL, NULL);
                          
    // Crear botones de operaciones
    int buttonY = 120;
    int buttonWidth = 80;
    int buttonSpacing = 90;
    
    hButtonAdd = CreateWindowW(L"BUTTON", L"Sumar", 
                              WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                              20, buttonY, buttonWidth, 35, hwnd, NULL, NULL, NULL);
                              
    hButtonSub = CreateWindowW(L"BUTTON", L"Restar", 
                              WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                              20 + buttonSpacing, buttonY, buttonWidth, 35, hwnd, NULL, NULL, NULL);
                              
    hButtonMul = CreateWindowW(L"BUTTON", L"Multiplicar", 
                              WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                              20 + buttonSpacing*2, buttonY, buttonWidth, 35, hwnd, NULL, NULL, NULL);
                              
    hButtonDiv = CreateWindowW(L"BUTTON", L"Dividir", 
                              WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                              20 + buttonSpacing*3, buttonY, buttonWidth, 35, hwnd, NULL, NULL, NULL);
                           
    // Crear área de resultados
    hResult = CreateWindowW(L"STATIC", L"Resultado: 0", 
                           WS_VISIBLE | WS_CHILD | SS_CENTER | SS_SUNKEN,
                           20, 180, 440, 80, hwnd, NULL, NULL, NULL);
}

void PerformOperation(int operation) {
    wchar_t text1[32] = {0};
    wchar_t text2[32] = {0};
    wchar_t result[100] = {0};
    
    GetWindowTextW(hEdit1, text1, 32);
    GetWindowTextW(hEdit2, text2, 32);
    
    double num1 = _wtof(text1);
    double num2 = _wtof(text2);
    double calculation = 0;
    
    const wchar_t* operationSymbol = L"";
    bool error = false;
    
    switch(operation) {
        case 0: // Suma
            calculation = num1 + num2;
            operationSymbol = L" + ";
            break;
        case 1: // Resta
            calculation = num1 - num2;
            operationSymbol = L" - ";
            break;
        case 2: // Multiplicación
            calculation = num1 * num2;
            operationSymbol = L" × ";
            break;
        case 3: // División
            if (num2 == 0) {
                StringCchCopyW(result, 100, L"Error: División por cero");
                error = true;
                break;
            }
            calculation = num1 / num2;
            operationSymbol = L" ÷ ";
            break;
    }
    
    if (!error) {
        swprintf_s(result, 100, L"%.2f%s%.2f = %.2f", num1, operationSymbol, num2, calculation);
    }
    SetWindowTextW(hResult, result);
}

void UpdateAnimation(HWND hwnd) {
    circleAngle += 0.05f;
    if (circleAngle > 2 * static_cast<float>(M_PI)) {
        circleAngle -= 2 * static_cast<float>(M_PI);
    }
}

void DrawAnimatedCircle(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    
    // Obtener dimensiones del área cliente
    RECT rect;
    GetClientRect(hwnd, &rect);
    
    // Calcular posición del círculo (trayectoria circular)
    int centerX = rect.right / 2;
    int centerY = rect.bottom / 2;
    
    // Calcular radio de la órbita (usando el mínimo entre ancho/alto)
    int orbitRadius = (centerX < centerY ? centerX : centerY) - CIRCLE_RADIUS - 50;
    
    int circleX = centerX + static_cast<int>(orbitRadius * cosf(circleAngle));
    int circleY = centerY + static_cast<int>(orbitRadius * sinf(circleAngle));
    
    // Crear pincel rojo
    HBRUSH hRedBrush = CreateSolidBrush(RGB(255, 0, 0));
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hRedBrush);
    
    // Dibujar círculo
    Ellipse(hdc, 
        circleX - CIRCLE_RADIUS, 
        circleY - CIRCLE_RADIUS,
        circleX + CIRCLE_RADIUS, 
        circleY + CIRCLE_RADIUS
    );
    
    // Restaurar pincel original y limpiar
    SelectObject(hdc, hOldBrush);
    DeleteObject(hRedBrush);
    
    EndPaint(hwnd, &ps);
}