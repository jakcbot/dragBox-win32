#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <algorithm>
#define NOMINMAX

class DraggableBox {
public:
    int x, y, size;
    COLORREF color;  // Added color attribute

    DraggableBox(int x, int y, int size, COLORREF color) : x(x), y(y), size(size), color(color) {}

    void Draw(HDC hdc) const {
        HBRUSH hBrush = CreateSolidBrush(color);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        Rectangle(hdc, x, y, x + size, y + size);

        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    void Move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    bool Contains(int xPos, int yPos) const {
        return (xPos >= x && xPos <= x + size && yPos >= y && yPos <= y + size);
    }
};

std::vector<DraggableBox> boxes;
int selectedBoxIndex = -1; // Index of the currently selected box

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param);

int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int n_cmd_show) {
    const wchar_t class_name[] = L"DraggableBoxApp";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = h_instance;
    wc.lpszClassName = class_name;
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, class_name, L"Draggable Box App", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, h_instance, nullptr);

    ShowWindow(hwnd, n_cmd_show);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param) {
    static HDC hdcBuffer = nullptr;
    static HBITMAP hbmBuffer = nullptr;
    static int cxClient, cyClient;
    static bool isDragging = false; // Flag indicating whether dragging is in progress

    HDC hdc = nullptr;  // Declare hdc outside the switch statement

    switch (message) {
    case WM_CREATE:
        break;

    case WM_SIZE:
        cxClient = LOWORD(l_param);
        cyClient = HIWORD(l_param);

        if (hdcBuffer) {
            DeleteObject(hbmBuffer);
            DeleteDC(hdcBuffer);
        }

        hdc = GetDC(hwnd);
        hdcBuffer = CreateCompatibleDC(hdc);
        hbmBuffer = CreateCompatibleBitmap(hdc, cxClient, cyClient);
        ReleaseDC(hwnd, hdc);
        break;

    case WM_LBUTTONDOWN:
    {
        int xPos = GET_X_LPARAM(l_param);
        int yPos = GET_Y_LPARAM(l_param);

        // Check if clicking on an existing box
        for (size_t i = 0; i < boxes.size(); ++i) {
            if (boxes[i].Contains(xPos, yPos)) {
                selectedBoxIndex = static_cast<int>(i);
                isDragging = true;
                break;
            }
        }

        // If not clicking on an existing box, create a new one
        if (!isDragging) {
            int windowSize = (std::min)(800, 600);
            int boxSize = (std::min)(10 * windowSize / 100, windowSize);

            // Make the box blue
            COLORREF blueColor = RGB(0, 0, 255);

            DraggableBox newBox(xPos, yPos, boxSize, blueColor);
            boxes.push_back(newBox);
        }

        InvalidateRect(hwnd, nullptr, TRUE);
    }
    break;

    case WM_MOUSEMOVE:
    {
        if (w_param & MK_LBUTTON && isDragging) {
            int xPos = GET_X_LPARAM(l_param);
            int yPos = GET_Y_LPARAM(l_param);

            if (selectedBoxIndex != -1) {
                int dx = xPos - boxes[selectedBoxIndex].x;
                int dy = yPos - boxes[selectedBoxIndex].y;

                boxes[selectedBoxIndex].Move(dx, dy);
                InvalidateRect(hwnd, nullptr, TRUE);
            }
        }
    }
    break;

    case WM_LBUTTONUP:
        isDragging = false;
        selectedBoxIndex = -1; // Reset selected box index
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmBuffer);

        FillRect(hdcMem, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        for (const auto& box : boxes) {
            box.Draw(hdcMem);
        }

        BitBlt(hdc, 0, 0, cxClient, cyClient, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);

        EndPaint(hwnd, &ps);
    }
    break;

    case WM_DESTROY:
        if (hdcBuffer) {
            DeleteObject(hbmBuffer);
            DeleteDC(hdcBuffer);
        }

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, w_param, l_param);
    }

    return 0;
}
