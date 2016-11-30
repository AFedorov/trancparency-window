#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <winerror.h>
#include <commctrl.h>
#include <cstdio>
#include <cstdlib>
/*
  mingw32-g++ -Wall ./main.cpp -o ./main -mwindows -L"C:/MinGW/lib" -lgdi32 -luser32 -lcomctl32
*/
//-------------------------------------------------------------------------
MSG msg;/* Сообщения, приходящие окну */
WNDCLASSEX wc;/* Класс окна */
HWND hWnd;/* HANDLE окна */
HWND hSlider;/* HANDLE слайдера */
HWND hButton;/* HANDLE checkbox-а */
LPCTSTR lpzClass=TEXT("WinAPI_window_opacity");/* Имя класса окна */
//-------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd,UINT message,     \
                            WPARAM wParam,LPARAM lParam \
                            )
{/* Обработчик сообщений главного окна */
  switch(message)
  {
  case WM_COMMAND:
    {
      if((HWND)lParam == hButton)
      {/* Проверка, checkbox прислал сообщение, или нет */
        WINDOWINFO wi;
        GetWindowInfo(hWnd,&wi);/* Получаем информацию о главном окне */
        /* Спрашиваем состояние checkbox-а */
        LPARAM lp=::SendMessage(hButton,BM_GETCHECK,(WPARAM)0,(LPARAM)0);
        if(lp == BST_CHECKED)/* Если взведён */
          SetWindowPos(hWnd,HWND_TOPMOST,                   \
                       wi.rcWindow.left,wi.rcWindow.top,    \
                       wi.rcWindow.right-wi.rcWindow.left,  \
                       wi.rcWindow.bottom-wi.rcWindow.top,  \
                       0                                    \
                       );/* Удерживаем окно поверх остальных */
        else if(lp == BST_UNCHECKED)/* Если сброшен */
          SetWindowPos(hWnd,HWND_NOTOPMOST,                 \
                       wi.rcWindow.left,wi.rcWindow.top,    \
                       wi.rcWindow.right-wi.rcWindow.left,  \
                       wi.rcWindow.bottom-wi.rcWindow.top,  \
                       0                                    \
                       );/* Обычное поведение окна */
      }
      break;
    }
  case WM_HSCROLL:
    {/* Сообщение от слайдера */
      if((LOWORD(wParam) >= TB_LINEUP) &&       \
         (LOWORD(wParam) <= TB_ENDTRACK)        \
         )
      {/* Если производилось изменение положения движка */
        COLORREF colorref=RGB(0xC8,0xD0,0xD4);
        /* Запрашиваем текущее положение движка */
        DWORD pos=::SendMessage(hSlider,TBM_GETPOS,(WPARAM)0,(LPARAM)0);
        /* На основании его устанавливаем прозрачность окна */
        SetLayeredWindowAttributes(hWnd,colorref,pos,LWA_ALPHA);
      }
      break;
    }
  case WM_DESTROY:
    {/* Когда закрывается главное окно */
      ::PostQuitMessage(0);
      break;
    }
  default:
    return ::DefWindowProc(hwnd,message,wParam,lParam);
  }
  return 0;
}
//-------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,       \
                     HINSTANCE hPrevInstance,   \
                     LPSTR lpCmdLine,           \
                     int nCmdShow               \
                     )
{
  /* Инициализируем движок стандартных компонент GDI */
  InitCommonControls();

  /* Инициализируем класс главного окна */
  wc.cbSize=sizeof(WNDCLASSEX);
  wc.style=CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc=(WNDPROC)WindowProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=sizeof(LONG);
  wc.hInstance=hInstance;
  wc.hIcon=NULL;
  wc.hCursor=::LoadCursor(0,IDC_ARROW);
  wc.hbrBackground=(HBRUSH)COLOR_WINDOW;
  wc.lpszMenuName=NULL;
  wc.lpszClassName=lpzClass;
  /* Регистрируем его в системе */
  if(::RegisterClassEx(&wc) == 0)
  { printf("Error code is %li\n",::GetLastError()); return 1; }

  /* Создаём главное окно приложения на основе
     зарегистрированного класса окна wc */
  hWnd=::CreateWindowEx(WS_EX_LAYERED | WS_EX_TOOLWINDOW, \
                        lpzClass,"Opacity window",        \
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE, \
                        10,10,385,150,                    \
                        0,0,NULL,NULL                     \
                        );

  /* Создаём checkbox управления поддержанием окна на переднем плане */
  hButton=::CreateWindowEx(0,"BUTTON","Foreground",                 \
                           WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, \
                           210,10,250,25,hWnd,0,NULL,NULL           \
                           );

  /* Создаём слайдер регулировки прозрачности окна */
  hSlider=::CreateWindowEx(0,TRACKBAR_CLASS,NULL,             \
                           WS_CHILD | WS_VISIBLE | TBS_HORZ,  \
                           10,10,200,25,hWnd,0,NULL,NULL      \
                           );
  /* Установка диапазона регулировки прозрачности */
  ::SendMessage(hSlider,TBM_SETRANGE,(WPARAM)TRUE,  \
                (LPARAM)MAKELONG(0x50,0xFF));
  /* Установка начального положения слайдера */
  ::SendMessage(hSlider,TBM_SETPOS,(WPARAM)TRUE,  \
                (LPARAM) (LONG)0xFF);

  COLORREF colorref=RGB(0xC8,0xD0,0xD4);
  /* Установка прозрачности окна */
  if(SetLayeredWindowAttributes(hWnd,colorref,0xFF,LWA_ALPHA))
    printf("failed set window opacity\n");

  ::ShowWindow(hWnd,SW_SHOWNORMAL);
  ::UpdateWindow(hWnd);

  /* Цикл выборки сообщений */
  int iGetOK;
  while((iGetOK=::GetMessage(&msg,NULL,0,0)))
  {
    if(iGetOK <= 0)
    { printf("Error code is %li\n",::GetLastError()); return 1; }
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
  return msg.wParam;
}
