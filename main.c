
#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#define XWIDTH 384
#define YWIDTH 384
#define PARTS 9
#define SPEED 1.002
#define ISP for(i=0;i<PARTS;i++)
#define PADDING 20

BOOL active = FALSE;
float x,y,xwidth,ywidth;
float xGrid[XWIDTH*2];
float yGrid[YWIDTH*2];

typedef struct draw_group 
{
    HANDLE thread;
    HDC memdc;
    DWORD threadID;
    HBITMAP bitmap;
    BYTE screen[0x10000]; 
    DWORD id; 
} DrawGroup;

DrawGroup sp[PARTS];

//display
HDC memdc=NULL;
HBITMAP bitmap=NULL;
LPBITMAPINFO info;
BYTE gsLPBITMAPINFO[sizeof(BITMAPINFO)+sizeof(RGBQUAD)*256];
char szWinName[]="MandelbrotViewer";

//functions
LRESULT CALLBACK WindowFunc(HWND,UINT,WPARAM,LPARAM);
void mandelbrot(BYTE* screen,float* gridx, float* gridy) __attribute__ ((regparm (3)));
void delGroup(DrawGroup*);
void updateGrids();
DWORD WINAPI drawScreen(LPVOID param);

int WINAPI WinMain(HINSTANCE hThisInst,HINSTANCE hPrevInst,LPSTR lpszArgs,int nWinMode)
{
    HWND hwnd;
    MSG msg;
    WNDCLASSEX wcl;
    wcl.cbSize=sizeof(WNDCLASSEX);
    wcl.hInstance=hThisInst;  
    wcl.lpszClassName=szWinName;   
    wcl.lpfnWndProc =WindowFunc;  
    wcl.style=0;  
    wcl.hIcon=LoadIcon(NULL,IDI_APPLICATION);  
    wcl.hIconSm=NULL;  
    wcl.hCursor=LoadCursor(NULL,IDC_ARROW);
    wcl.lpszMenuName=NULL; 
    wcl.cbClsExtra = 0;
    wcl.cbWndExtra = 0; 
    wcl.hbrBackground=(HBRUSH) GetStockObject(WHITE_BRUSH);
    
    if(!RegisterClassEx(&wcl))
    {
        return 0;
    }
    
    hwnd=CreateWindow(
        szWinName, 
        " The Mandelbrot Set", 
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT, 
        CW_USEDEFAULT,  
        XWIDTH*2+PADDING,      
        YWIDTH*2+PADDING,  
        NULL,  
        NULL,  
        hThisInst, 
        NULL
    );
    
    ShowWindow(hwnd,nWinMode);
    UpdateWindow(hwnd);
    
    while(GetMessage(&msg,NULL,0,0))
    {
        TranslateMessage(&msg);  
        DispatchMessage(&msg);  
    }
    
    return msg.wParam;
}

LRESULT CALLBACK WindowFunc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    HDC hdc;
    int i;
    PAINTSTRUCT ps;
    switch(message)
    {
        case WM_CREATE:
        {
            int c;
            hdc=GetDC(hwnd);
            x=y=0;
            xwidth=3;
            ywidth=3;
            updateGrids();
            ISP
            {
                sp[i].memdc=CreateCompatibleDC(hdc);
                sp[i].bitmap=CreateCompatibleBitmap(hdc,0x100,0x100);
                SelectObject(sp[i].memdc,sp[i].bitmap);  
                sp[i].id=i;
            }
            memdc=CreateCompatibleDC(hdc);
            bitmap=CreateCompatibleBitmap(hdc,XWIDTH*2,YWIDTH*2);
            SelectObject(memdc,bitmap);
            info=(LPBITMAPINFO)((void*)gsLPBITMAPINFO);
            info->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
            info->bmiHeader.biWidth=0x100;
            info->bmiHeader.biHeight=-0x100;
            info->bmiHeader.biPlanes=1;
            info->bmiHeader.biBitCount=8;
            info->bmiHeader.biCompression=BI_RGB;
            info->bmiHeader.biSizeImage=0;
            info->bmiHeader.biXPelsPerMeter=0;           
            info->bmiHeader.biYPelsPerMeter=0;           
            info->bmiHeader.biClrUsed=0;
            info->bmiHeader.biClrImportant=0;
            for(c=255;c>0;c--)
            {
                info->bmiColors[c].rgbGreen=255-c;
                info->bmiColors[c].rgbRed=0;
                info->bmiColors[c].rgbBlue=0;
            }
            info->bmiColors[0].rgbRed=255;
            info->bmiColors[0].rgbBlue=255;
            info->bmiColors[0].rgbGreen=255;
            ISP
            {
                for(c=0;c < 0x10000;c++)
                {
                    sp[i].screen[c]=255;
                }
            }
            ReleaseDC(hwnd,hdc);
        }
        break;
        case WM_PAINT:                                                                                          
            hdc=BeginPaint(hwnd,&ps); 
            ISP
            {        
                SetDIBits(sp[i].memdc,sp[i].bitmap,0,0x100,sp[i].screen,info,DIB_RGB_COLORS);
                BitBlt(memdc,(i%3)*0x100,(i/3)*0x100,0x100,0x100,sp[i].memdc,0,0,SRCCOPY);
            }
            BitBlt(hdc,0,0,XWIDTH*2,YWIDTH*2,memdc,0,0,SRCCOPY);	// final
            EndPaint(hwnd,&ps);
        break;
        case WM_TIMER:
            xwidth/=SPEED;
            ywidth/=SPEED;
            updateGrids();
            InvalidateRect(hwnd,NULL,0);
        break;    
        case WM_KEYDOWN:
            active = !active;
            if(active)
            {
                ISP
                {
                    sp[i].thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)drawScreen,&sp[i].id,0,&sp[i].threadID);
                }
                SetTimer(hwnd,1,0,NULL);
            }
            else 
            {   
                KillTimer(hwnd,1);
            }
        break;
        case WM_LBUTTONDOWN:
            x=LOWORD(lParam)*xwidth/XWIDTH+x-xwidth;
            y=y+ywidth-HIWORD(lParam)*ywidth/YWIDTH;	
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;		
        case WM_DESTROY:
            active = FALSE; 
            KillTimer(hwnd,1);
            DeleteDC(memdc);
            DeleteObject(bitmap);
            ISP
            {
                delGroup(&sp[i]);                   
            }
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd,message,wParam,lParam);
    }
    return 0;
}

void delGroup(DrawGroup* g)
{  
    DeleteDC(g->memdc);
    DeleteObject(g->bitmap);  
}

void updateGrids()
{
    int c;
    float* xTraveler = xGrid;
    float* yTraveler = yGrid;
    float xStart = x - xwidth;
    float yStart = y + ywidth;
    float xInc = xwidth/XWIDTH;
    float yInc = ywidth/YWIDTH;
    for(c=0;c < XWIDTH*2;xTraveler++)
    {
        *xTraveler = xStart + c*xInc;
        c++;
    }
    for(c=0;c < YWIDTH*2;yTraveler++)
    {
        *yTraveler = yStart - c*yInc;
        c++;
    }    
}

DWORD WINAPI drawScreen(LPVOID param)
{
    int i = *((int*)param);
    float* xg = ((i%3) << 8)+ xGrid;
    float* yg = ((i/3) << 8)+ yGrid;
    while(active) mandelbrot(sp[i].screen,xg,yg);
}


