#define main telecom_console_main
#include "Network.cpp"
#undef main
#include <windows.h>
#include <commctrl.h>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

static Graph graph;
static HWND hInput, hOutput, hStatus, hCanvas;
static HFONT hFont;
static const int ID_INPUT=100, ID_OUTPUT=101, ID_STATUS=102, ID_CANVAS=103;
static std::wstring widen(const std::string& s){ if(s.empty()) return L""; int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0); std::wstring w(n,0); MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,&w[0],n); if(!w.empty()&&w.back()==0)w.pop_back(); return w; }
static std::string narrow(const std::wstring& w){ if(w.empty())return ""; int n=WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,nullptr,0,nullptr,nullptr); std::string s(n,0); WideCharToMultiByte(CP_UTF8,0,w.c_str(),-1,&s[0],n,nullptr,nullptr); if(!s.empty()&&s.back()==0)s.pop_back(); return s; }
static void appendOutput(const std::string& s){ int len=GetWindowTextLengthW(hOutput); SendMessageW(hOutput,EM_SETSEL,len,len); std::wstring w=widen(s); SendMessageW(hOutput,EM_REPLACESEL,FALSE,(LPARAM)w.c_str()); }
static std::string inputText(){ wchar_t b[2048]; GetWindowTextW(hInput,b,2048); return narrow(b); }
static void setStatus(const std::wstring& s){SetWindowTextW(hStatus,s.c_str());}
static void invoke(int action){
 std::string in=inputText(); std::istringstream iss(in); std::ostringstream oss; auto* oldIn=cin.rdbuf(iss.rdbuf()); auto* oldOut=cout.rdbuf(oss.rdbuf());
 try {
 switch(action){
 case 1: loadSample(graph); break;
 case 2: doAddNode(graph); break;
 case 3: doAddLink(graph); break;
 case 4: doRemoveLink(graph); break;
 case 5: graph.printAdjacency(); break;
 case 6: doBFS(graph); break;
 case 7: doDFS(graph); break;
 case 8: doComponents(graph); break;
 case 9: doDegree(graph); break;
 case 10: doIsolated(graph); break;
 case 11: doCritical(graph); break;
 case 12: doShortest(graph); break;
 case 13: doAlternate(graph); break;
 case 14: doFailure(graph); break;
 case 15: doCompare(graph); break;
 case 16: doBenchmark(); break;
 case 17: {
   if(graph.size()==0){cout<<"No towers available. Load or add towers first.\\n";break;}
   TowerBST bst; bst.buildFromNames(graph.name);
   int id=0; if(!(iss>>id)){cout<<"Enter a one-based tower ID in the input box.\\n";break;}
   const TowerNode* t=bst.find(id); if(t)cout<<"Tower found: ID "<<t->id<<" - "<<t->name<<"\\n"; else cout<<"No tower exists with ID "<<id<<". Valid range: 1-"<<graph.size()<<"\\n";
   break;
 }
 case 18: {TowerBST bst;bst.buildFromNames(graph.name);cout<<"BST INORDER (ascending ID)\\n";bst.showInorder();break;}
 case 19: {TowerBST bst;bst.buildFromNames(graph.name);cout<<"BST PREORDER\\n";bst.showPreorder();break;}
 case 20: {TowerBST bst;bst.buildFromNames(graph.name);cout<<"BST POSTORDER\\n";bst.showPostorder();break;}
 case 21: {TowerBST bst;bst.buildFromNames(graph.name);cout<<"Tower records: "<<graph.size()<<"\\nBST height: "<<bst.getHeight()<<" level(s)\\n";break;}
 }
 } catch(const std::exception& e){cout<<"Operation error: "<<e.what()<<"\\n";}
 cout.rdbuf(oldOut); cin.rdbuf(oldIn);
 std::string result=oss.str(); if(result.empty())result="Operation completed.\\n"; appendOutput("\\r\\n> "+result);
 std::wstring st=L"Network: "+std::to_wstring(graph.size())+L" towers | "+std::to_wstring(graph.edgeCount)+L" links"; setStatus(st); InvalidateRect(hCanvas,nullptr,TRUE);
}
static void addButton(HWND parent,const wchar_t* label,int id,int x,int y,int w=148,int h=30){HWND b=CreateWindowW(L"BUTTON",label,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,x,y,w,h,parent,(HMENU)(INT_PTR)id,GetModuleHandleW(nullptr),nullptr);SendMessageW(b,WM_SETFONT,(WPARAM)hFont,TRUE);}
static void drawNetwork(HWND hwnd,HDC dc){RECT r;GetClientRect(hwnd,&r);FillRect(dc,&r,(HBRUSH)(COLOR_WINDOW+1));int n=graph.size();if(!n)return;int cx=(r.right-r.left)/2,cy=(r.bottom-r.top)/2,rad=(std::min)(cx-60,cy-50);if(rad<30)rad=30;std::vector<POINT> p(n);for(int i=0;i<n;i++){double a=6.283185307*i/(n?n:1)-1.5707963;p[i].x=cx+(int)(rad*cos(a));p[i].y=cy+(int)(rad*sin(a));}
 HPEN edge=CreatePen(PS_SOLID,2,RGB(145,160,178));HGDIOBJ old=SelectObject(dc,edge);for(int u=0;u<n;u++)for(EdgeNode* e=graph.adj[u];e;e=e->next)if(u<e->to){MoveToEx(dc,p[u].x,p[u].y,nullptr);LineTo(dc,p[e->to].x,p[e->to].y);}SelectObject(dc,old);DeleteObject(edge);
 for(int i=0;i<n;i++){HBRUSH br=CreateSolidBrush(graph.active[i]?RGB(28,105,170):RGB(190,65,65));HGDIOBJ ob=SelectObject(dc,br);Ellipse(dc,p[i].x-17,p[i].y-17,p[i].x+17,p[i].y+17);SelectObject(dc,ob);DeleteObject(br);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(255,255,255));wchar_t num[16];wsprintfW(num,L"%d",i);RECT tr={p[i].x-15,p[i].y-10,p[i].x+15,p[i].y+10};DrawTextW(dc,num,-1,&tr,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SetTextColor(dc,RGB(30,40,50));RECT nr={p[i].x-50,p[i].y+19,p[i].x+50,p[i].y+38};std::wstring nm=widen(graph.name[i]);DrawTextW(dc,nm.c_str(),-1,&nr,DT_CENTER|DT_TOP|DT_SINGLELINE|DT_END_ELLIPSIS);}
}
static LRESULT CALLBACK canvasProc(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);drawNetwork(h,dc);EndPaint(h,&ps);return 0;}return DefWindowProcW(h,m,w,l);}
static LRESULT CALLBACK wndProc(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){case WM_CREATE:{hFont=CreateFontW(16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
 CreateWindowW(L"STATIC",L"TELECOM NETWORK CONNECTIVITY ANALYZER",WS_CHILD|WS_VISIBLE,18,12,690,30,h,nullptr,nullptr,nullptr);
 CreateWindowW(L"STATIC",L"Input: node name / index; for links and routes enter two values on separate lines",WS_CHILD|WS_VISIBLE,18,48,760,22,h,nullptr,nullptr,nullptr);
 hInput=CreateWindowW(L"EDIT",L"Pune\\r\\nMumbai",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL,18,73,360,36,h,(HMENU)ID_INPUT,nullptr,nullptr);
 hStatus=CreateWindowW(L"STATIC",L"Loading sample network...",WS_CHILD|WS_VISIBLE,395,78,500,22,h,(HMENU)ID_STATUS,nullptr,nullptr);
 const wchar_t* labels[]={L"Load sample",L"Add tower",L"Add link",L"Remove link",L"Adjacency lists",L"BFS traversal",L"DFS traversal",L"Connected components",L"Degree / hubs",L"Isolated towers",L"Critical nodes / links",L"Shortest route",L"Alternate routes",L"Fail / restore node",L"Compare BFS / DFS",L"Benchmark",L"BST: search ID",L"BST: inorder",L"BST: preorder",L"BST: postorder",L"BST: height / count"};
 for(int i=0;i<21;i++){int col=i%3,row=i/3;addButton(h,labels[i],200+i,18+col*158,116+row*37,150,30);}
 hCanvas=CreateWindowW(L"TelecomNetworkCanvas",L"",WS_CHILD|WS_VISIBLE|WS_BORDER,500,116,420,265,h,(HMENU)ID_CANVAS,GetModuleHandleW(nullptr),nullptr);
 hOutput=CreateWindowW(L"EDIT",L"Welcome! The sample network is loaded automatically.\\r\\nUse the buttons to explore all graph and BST operations.\\r\\nGraph node IDs are zero-based; BST tower IDs are one-based.\\r\\n",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY|WS_VSCROLL,18,390,902,245,h,(HMENU)ID_OUTPUT,nullptr,nullptr);
 for(HWND c=GetWindow(h,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,(WPARAM)hFont,TRUE);loadSample(graph);setStatus(L"Network: 13 towers | 12 links");return 0;}
 case WM_COMMAND:if(LOWORD(w)>=200&&LOWORD(w)<=220){invoke(LOWORD(w)-199);return 0;}break;
 case WM_SIZE:{int width=LOWORD(l),height=HIWORD(l);MoveWindow(hCanvas,width-440,116,420,265,TRUE);MoveWindow(hOutput,18,390,width-36,height-410,TRUE);return 0;}
 case WM_DESTROY:DeleteObject(hFont);PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE, PWSTR,int show){INITCOMMONCONTROLSEX ic={sizeof(ic),ICC_STANDARD_CLASSES};InitCommonControlsEx(&ic);WNDCLASSW cc={};cc.lpfnWndProc=canvasProc;cc.hInstance=inst;cc.lpszClassName=L"TelecomNetworkCanvas";cc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);cc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClassW(&cc);WNDCLASSW wc={};wc.lpfnWndProc=wndProc;wc.hInstance=inst;wc.lpszClassName=L"TelecomNetworkAnalyzerWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);HWND h=CreateWindowW(wc.lpszClassName,L"Telecom Network Analyzer | C++17",WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,960,700,nullptr,nullptr,inst,nullptr);if(!h)return 1;ShowWindow(h,show);UpdateWindow(h);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;}
