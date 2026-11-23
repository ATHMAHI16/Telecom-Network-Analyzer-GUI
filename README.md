# Telecom Communication Network Connectivity Analyzer — Windows GUI

Native Windows desktop GUI written in C++17 using the Win32 API. The original graph and BST implementation is in Network.cpp; NetworkGUI.cpp provides the desktop interface. No Python, Qt, or external GUI runtime is required.

## Graph features
1. Load sample network  2. Add tower  3. Add link  4. Remove link
5. Adjacency lists  6. BFS  7. DFS  8. Connected components
9. Degree/hub analysis  10. Isolated towers  11. Tarjan critical nodes and bridges
12. Shortest route  13. Up to 10 alternate routes  14. Fail/restore tower
15. Compare BFS/DFS  16. Random-network benchmark

## BST features
Search tower by one-based ID; inorder, preorder and postorder traversals; tree height and record count.

The sample network loads at startup. Active towers are blue; failed towers are red.

## Input guide
Enter one value per line in the input field for multi-value operations:
- Add tower: new tower name
- Add/remove link: first tower, then second (e.g. Pune then Mumbai)
- BFS/DFS/fail/restore: tower name or zero-based graph index
- Shortest/alternate route: source then destination
- BST search: one-based ID (1 = Pune)

Graph IDs are zero-based; BST IDs are one-based.

## Local build (optional)
With MinGW-w64:
```powershell
g++ -std=c++17 -O2 -Wall -Wextra -municode -mwindows NetworkGUI.cpp -o TelecomNetworkAnalyzer.exe -lcomctl32 -lgdi32 -luser32
```

## Get the Windows executable
Open Actions → latest successful **Build Windows GUI** run → download the **Telecom-Network-Analyzer-Windows** artifact → extract and run TelecomNetworkAnalyzer.exe.

Target: Windows 10/11 x64. The executable is unsigned and Windows may display a security warning. Do not disable Windows security features to run an unfamiliar executable.
