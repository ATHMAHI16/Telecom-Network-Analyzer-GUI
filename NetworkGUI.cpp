#define main telecom_console_main
#include "Network.cpp"
#undef main
#include <windows.h>
#include <sstream>
#include <cmath>

static Graph g;
static HWND hInput=nullptr,hOutput=nullptr,hStatus=nullptr,hCanvas=nullptr;
static HFONT hFont=nullptr;

static std::wstring W(const std::string&s){if(s.empty())return L"";int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);std::wstring v(n,0);MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,&v[0],n);if(!v.empty())v.pop_back();return v;}
static std::string U(const std::wstring&s){if(s.empty())return "";int n=WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,nullptr,0,nullptr,nullptr);std::string v(n,0);WideCharToMultiByte(CP_UTF8,0,s.c_str(),-1,&v[0],n,nullptr,nullptr);if(!v.empty())v.pop_back();return v;}
static std::string input(){wchar_t b[4096];GetWindowTextW(hInput,b,4096);return U(b);}
static void writeOut(const std::string&s){int n=GetWindowTextLengthW(hOutput);SendMessageW(hOutput,EM_SETSEL,n,n);auto v=W(s);SendMessageW(hOutput,EM_REPLACESEL,FALSE,(LPARAM)v.c_str());}
static void status(){std::wstring s=L"Network: "+std::to_wstring(g.size())+L" towers | "+std::to_wstring(g.edgeCount)+L" links";SetWindowTextW(hStatus,s.c_str());InvalidateRect(hCanvas,nullptr,TRUE);}

struct BNode{int id;std::string name;BNode*l,*r;BNode(int i,const std::string&n):id(i),name(n),l(nullptr),r(nullptr){}};
static void insert(BNode*&p,int id,const std::string&name){if(!p){p=new BNode(id,name);return;}if(id<p->id)insert(p->l,id,name);else if(id>p->id)insert(p->r,id,name);}
static void destroy(BNode*p){if(!p)return;destroy(p->l);destroy(p->r);delete p;}
static void inorder(BNode*p,std::ostringstream&o){if(!p)return;inorder(p->l,o);o<<p->id<<" - "<<p->name<<"\\n";inorder(p->r,o);}
static void preorder(BNode*p,std::ostringstream&o){if(!p)return;o<<p->id<<" - "<<p->name<<"\\n";preorder(p->l,o);preorder(p->r,o);}
static void postorder(BNode*p,std::ostringstream&o){if(!p)return;postorder(p->l,o);postorder(p->r,o);o<<p->id<<" - "<<p->name<<"\\n";}
static int height(BNode*p){return p?1+(std::max)(height(p->l),height(p->r)):0;}
static BNode* buildTree(){BNode*root=nullptr;for(int i=0;i<g.size();++i)insert(root,i+1,g.name[i]);return root;}
static void doBST(int op,std::istringstream&in,std::ostringstream&out){BNode*r=buildTree();if(op==17){int id=0;if(!(in>>id))out<<"Enter a one-based tower ID in the input box.\\n";else{BNode*p=r;while(p&&p->id!=id)p=id<p->id?p->l:p->r;if(p)out<<"Found: "<<p->id<<" - "<<p->name<<"\\n";else out<<"No tower with ID "<<id<<". Valid IDs: 1-"<<g.size()<<"\\n";}}else if(op==18){out<<"BST INORDER (ascending ID)\\n";inorder(r,out);}else if(op==19){out<<"BST PREORDER\\n";preorder(r,out);}else if(op==20){out<<"BST POSTORDER\\n";postorder(r,out);}else out<<"Records: "<<g.size()<<"\\nHeight: "<<height(r)<<" levels\\n";destroy(r);}

static void execute(int op){std::istringstream in(input());std::ostringstream out;auto*oldIn=cin.rdbuf(in.rdbuf());auto*oldOut=cout.rdbuf(out.rdbuf());try{switch(op){
case 1:loadSample(g);break;case 2:doAddNode(g);break;case 3:doAddLink(g);break;case 4:doRemoveLink(g);break;case 5:g.printAdjacency();break;case 6:doBFS(g);break;case 7:doDFS(g);break;case 8:doComponents(g);break;case 9:doDegree(g);break;case 10:doIsolated(g);break;case 11:doCritical(g);break;case 12:doShortest(g);break;case 13:doAlternate(g);break;case 14:doFailure(g);break;case 15:doCompare(g);break;case 16:doBenchmark();break;default:break;}}
catch(const std::exception&e){out<<"Error: "<<e.what()<<"\\n";}cout.rdbuf(oldOut);cin.rdbuf(oldIn);if(op>=17&&op<=21)doBST(op,in,out);std::string result=out.str();if(result.empty())result="Operation completed.\\n";writeOut("\\r\\n> "+result);status();}

static void addButton(HWND h,const wchar_t*t,int id,int x,int y){HWND b=CreateWindowW(L"BUTTON",t,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,x,y,150,30,h,(HMENU)(INT_PTR)id,GetModuleHandleW(nullptr),nullptr);SendMessageW(b,WM_SETFONT,(WPARAM)hFont,TRUE);}
static void paint(HWND h,HDC dc){RECT r;GetClientRect(h,&r);FillRect(dc,&r,(HBRUSH)(COLOR_WINDOW+1));int n=g.size();if(!n)return;int cx=r.right/2,cy=r.bottom/2,rad=(std::min)(cx-55,cy-45);if(rad<25)rad=25;std::vector<POINT>p(n);for(int i=0;i<n;++i){double a=6.283185307*i/n-1.5707963;p[i].x=cx+(int)(rad*cos(a));p[i].y=cy+(int)(rad*sin(a));}
HPEN pen=CreatePen(PS_SOLID,2,RGB(155,166,180));HGDIOBJ old=SelectObject(dc,pen);for(int u=0;u<n;++u)for(EdgeNode*e=g.adj[u];e;e=e->next)if(u<e->to){MoveToEx(dc,p[u].x,p[u].y,nullptr);LineTo(dc,p[e->to].x,p[e->to].y);}SelectObject(dc,old);DeleteObject(pen);
for(int i=0;i<n;++i){HBRUSH br=CreateSolidBrush(g.active[i]?RGB(28,105,170):RGB(190,65,65));old=SelectObject(dc,br);Ellipse(dc,p[i].x-16,p[i].y-16,p[i].x+16,p[i].y+16);SelectObject(dc,old);DeleteObject(br);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(255,255,255));wchar_t num[12];wsprintfW(num,L"%d",i);RECT a={p[i].x-14,p[i].y-9,p[i].x+14,p[i].y+9};DrawTextW(dc,num,-1,&a,DT_CENTER|DT_VCENTER|DT_SINGLELINE);SetTextColor(dc,RGB(30,40,50));RECT b={p[i].x-48,p[i].y+18,p[i].x+48,p[i].y+35};auto nm=W(g.name[i]);DrawTextW(dc,nm.c_str(),-1,&b,DT_CENTER|DT_SINGLELINE|DT_END_ELLIPSIS);}}
static LRESULT CALLBACK canvasProc(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);paint(h,dc);EndPaint(h,&ps);return 0;}return DefWindowProcW(h,m,w,l);}
static LRESULT CALLBACK wndProc(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_CREATE){hFont=CreateFontW(16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Segoe UI");
CreateWindowW(L"STATIC",L"TELECOM NETWORK CONNECTIVITY ANALYZER",WS_CHILD|WS_VISIBLE,18,12,700,28,h,nullptr,nullptr,nullptr);
CreateWindowW(L"STATIC",L"Input: tower name/index; for multi-step operations enter one value per line",WS_CHILD|WS_VISIBLE,18,45,760,22,h,nullptr,nullptr,nullptr);
hInput=CreateWindowW(L"EDIT",L"Pune\\r\\nMumbai",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,18,70,360,42,h,(HMENU)100,nullptr,nullptr);
hStatus=CreateWindowW(L"STATIC",L"Sample network loading...",WS_CHILD|WS_VISIBLE,395,80,500,22,h,(HMENU)102,nullptr,nullptr);
const wchar_t*labels[]={L"Load sample",L"Add tower",L"Add link",L"Remove link",L"Adjacency lists",L"BFS traversal",L"DFS traversal",L"Connected components",L"Degree / hubs",L"Isolated towers",L"Critical nodes / links",L"Shortest route",L"Alternate routes",L"Fail / restore node",L"Compare BFS / DFS",L"Benchmark",L"BST search ID",L"BST inorder",L"BST preorder",L"BST postorder",L"BST height / count"};
for(int i=0;i<21;++i)addButton(h,labels[i],200+i,18+(i%3)*158,124+(i/3)*37);
hCanvas=CreateWindowW(L"TelecomCanvas",L"",WS_CHILD|WS_VISIBLE|WS_BORDER,500,124,420,265,h,(HMENU)103,GetModuleHandleW(nullptr),nullptr);
hOutput=CreateWindowW(L"EDIT",L"Welcome! Sample network loads automatically.\\r\\nGraph node IDs are zero-based; BST tower IDs are one-based.\\r\\n",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL,18,390,902,245,h,(HMENU)101,nullptr,nullptr);
for(HWND c=GetWindow(h,GW_CHILD);c;c=GetWindow(c,GW_HWNDNEXT))SendMessageW(c,WM_SETFONT,(WPARAM)hFont,TRUE);loadSample(g);status();return 0;}
if(m==WM_COMMAND&&LOWORD(w)>=200&&LOWORD(w)<=220){execute(LOWORD(w)-199);return 0;}
if(m==WM_SIZE){int width=LOWORD(l),height=HIWORD(l);MoveWindow(hCanvas,width-440,124,420,265,TRUE);MoveWindow(hOutput,18,390,width-36,(std::max)(120,height-410),TRUE);return 0;}
if(m==WM_DESTROY){DeleteObject(hFont);PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,PWSTR,int show){WNDCLASSW c={};c.lpfnWndProc=canvasProc;c.hInstance=inst;c.lpszClassName=L"TelecomCanvas";c.hCursor=LoadCursor(nullptr,IDC_ARROW);c.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&c);WNDCLASSW wc={};wc.lpfnWndProc=wndProc;wc.hInstance=inst;wc.lpszClassName=L"TelecomAnalyzer";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&wc);HWND h=CreateWindowW(wc.lpszClassName,L"Telecom Network Analyzer | C++17",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,960,700,nullptr,nullptr,inst,nullptr);if(!h)return 1;ShowWindow(h,show);UpdateWindow(h);MSG msg;while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return (int)msg.wParam;}
