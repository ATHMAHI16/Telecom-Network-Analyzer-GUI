// Telecom Communication Network Connectivity Analyzer
// Fundamentals of Data Structures - course project
//
// Data structures used (all implemented here, no STL containers for the core logic):
//   - Singly linked list  : adjacency list of each node (graph edges)
//   - Queue (linked list) : frontier for BFS
//   - Stack (linked list) : frontier for DFS
//   - Graph               : undirected communication network
//
// Compile: g++ -std=c++17 -O2 -o analyzer main.cpp
// Run    : ./analyzer

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
using namespace std;

// ---------------------------------------------------------------
// Linked list node used for adjacency lists
// ---------------------------------------------------------------
struct EdgeNode {
    int to;
    EdgeNode* next;
    EdgeNode(int t, EdgeNode* n) : to(t), next(n) {}
};

// ---------------------------------------------------------------
// Generic linked list node for Queue and Stack
// ---------------------------------------------------------------
struct ListNode {
    int val;
    ListNode* next;
    ListNode(int v) : val(v), next(nullptr) {}
};

class Queue {
    ListNode *head, *tail;
    int count, peak;
public:
    Queue() : head(nullptr), tail(nullptr), count(0), peak(0) {}
    ~Queue() { while (!empty()) pop(); }
    bool empty() const { return head == nullptr; }
    int peakSize() const { return peak; }
    void push(int v) {
        ListNode* n = new ListNode(v);
        if (tail) tail->next = n; else head = n;
        tail = n;
        count++;
        if (count > peak) peak = count;
    }
    int pop() {
        ListNode* t = head;
        int v = t->val;
        head = head->next;
        if (!head) tail = nullptr;
        delete t;
        count--;
        return v;
    }
};

class Stack {
    ListNode* top;
    int count, peak;
public:
    Stack() : top(nullptr), count(0), peak(0) {}
    ~Stack() { while (!empty()) pop(); }
    bool empty() const { return top == nullptr; }
    int peakSize() const { return peak; }
    void push(int v) {
        ListNode* n = new ListNode(v);
        n->next = top;
        top = n;
        count++;
        if (count > peak) peak = count;
    }
    int pop() {
        ListNode* t = top;
        int v = t->val;
        top = top->next;
        delete t;
        count--;
        return v;
    }
};

// Counters collected during a traversal, used for the complexity comparison
struct TraversalStats {
    long long verticesVisited = 0;
    long long edgesExamined = 0;
    int peakFrontier = 0;
    double micros = 0;
};

// ---------------------------------------------------------------
// Graph: undirected network stored as adjacency lists
// ---------------------------------------------------------------
class Graph {
public:
    vector<string> name;
    vector<EdgeNode*> adj;
    vector<bool> active;   // false means the node has failed / is switched off
    int edgeCount = 0;

    Graph() {}
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    ~Graph() { clear(); }

    int size() const { return (int)name.size(); }

    void clear() {
        for (EdgeNode* h : adj) {
            while (h) { EdgeNode* t = h; h = h->next; delete t; }
        }
        adj.clear(); name.clear(); active.clear(); edgeCount = 0;
    }

    int addNode(const string& nm) {
        name.push_back(nm);
        adj.push_back(nullptr);
        active.push_back(true);
        return size() - 1;
    }

    int findNode(const string& nm) const {
        for (int i = 0; i < size(); i++) if (name[i] == nm) return i;
        return -1;
    }

    bool hasEdge(int u, int v) const {
        for (EdgeNode* p = adj[u]; p; p = p->next) if (p->to == v) return true;
        return false;
    }

    bool addEdge(int u, int v) {
        if (u == v || hasEdge(u, v)) return false;
        adj[u] = new EdgeNode(v, adj[u]);
        adj[v] = new EdgeNode(u, adj[v]);
        edgeCount++;
        return true;
    }

    bool removeEdge(int u, int v) {
        if (!hasEdge(u, v)) return false;
        removeOne(u, v);
        removeOne(v, u);
        edgeCount--;
        return true;
    }

    // degree counts only links to nodes that are still active
    int degree(int u) const {
        int d = 0;
        for (EdgeNode* p = adj[u]; p; p = p->next) if (active[p->to]) d++;
        return d;
    }

    // ---------------- Traversals ----------------

    // BFS from start. Fills order, parent and dist (dist = hops, -1 if unreachable).
    void bfs(int start, vector<int>& order, vector<int>& parent, vector<int>& dist,
             TraversalStats& st) const {
        int n = size();
        vector<bool> seen(n, false);
        parent.assign(n, -1);
        dist.assign(n, -1);
        order.clear();
        st = TraversalStats();
        auto t0 = chrono::high_resolution_clock::now();

        Queue q;
        q.push(start);
        seen[start] = true;
        dist[start] = 0;
        while (!q.empty()) {
            int u = q.pop();
            order.push_back(u);
            st.verticesVisited++;
            for (EdgeNode* p = adj[u]; p; p = p->next) {
                st.edgesExamined++;
                int v = p->to;
                if (!active[v] || seen[v]) continue;
                seen[v] = true;
                parent[v] = u;
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
        st.peakFrontier = q.peakSize();
        auto t1 = chrono::high_resolution_clock::now();
        st.micros = chrono::duration<double, micro>(t1 - t0).count();
    }

    // Iterative DFS using an explicit stack. A node is marked visited when popped.
    void dfs(int start, vector<int>& order, TraversalStats& st) const {
        int n = size();
        vector<bool> visited(n, false);
        order.clear();
        st = TraversalStats();
        auto t0 = chrono::high_resolution_clock::now();

        Stack s;
        s.push(start);
        while (!s.empty()) {
            int u = s.pop();
            if (visited[u]) continue;
            visited[u] = true;
            order.push_back(u);
            st.verticesVisited++;
            for (EdgeNode* p = adj[u]; p; p = p->next) {
                st.edgesExamined++;
                if (active[p->to] && !visited[p->to]) s.push(p->to);
            }
        }
        st.peakFrontier = s.peakSize();
        auto t1 = chrono::high_resolution_clock::now();
        st.micros = chrono::duration<double, micro>(t1 - t0).count();
    }

    // ---------------- Connected components ----------------
    // comp[i] = component id of node i, -1 for failed nodes. Returns number of components.
    int components(vector<int>& comp) const {
        int n = size();
        comp.assign(n, -1);
        int c = 0;
        vector<int> order, parent, dist;
        TraversalStats st;
        for (int i = 0; i < n; i++) {
            if (!active[i] || comp[i] != -1) continue;
            bfs(i, order, parent, dist, st);
            for (int v : order) comp[v] = c;
            c++;
        }
        return c;
    }

    // ---------------- Critical nodes and links (Tarjan) ----------------
    void criticalElements(vector<int>& cutNodes, vector<pair<int,int>>& bridges) const {
        int n = size();
        vector<int> disc(n, 0), low(n, 0), par(n, -1);
        vector<bool> isCut(n, false);
        int timer = 0;
        bridges.clear();
        for (int i = 0; i < n; i++)
            if (active[i] && disc[i] == 0) tarjan(i, disc, low, par, isCut, timer, bridges);
        cutNodes.clear();
        for (int i = 0; i < n; i++) if (isCut[i]) cutNodes.push_back(i);
    }

    // ---------------- Paths ----------------
    // Shortest path (fewest hops) as a list of node ids. Empty if none.
    vector<int> shortestPath(int s, int t) const {
        vector<int> order, parent, dist;
        TraversalStats st;
        bfs(s, order, parent, dist, st);
        vector<int> path;
        if (dist[t] == -1) return path;
        for (int v = t; v != -1; v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());
        return path;
    }

    // Enumerate simple paths from s to t by backtracking DFS, up to maxPaths.
    void allPaths(int s, int t, int maxPaths, vector<vector<int>>& result) const {
        result.clear();
        vector<bool> onPath(size(), false);
        vector<int> cur;
        walk(s, t, maxPaths, onPath, cur, result);
    }

    void printAdjacency() const {
        for (int i = 0; i < size(); i++) {
            cout << "  " << setw(2) << i << " " << left << setw(14) << name[i] << right
                 << (active[i] ? " [UP]   " : " [DOWN] ") << "-> ";
            if (!adj[i]) cout << "(no links)";
            for (EdgeNode* p = adj[i]; p; p = p->next)
                cout << name[p->to] << (p->next ? " -> " : "");
            cout << "\n";
        }
    }

private:
    void removeOne(int u, int v) {
        EdgeNode *p = adj[u], *prev = nullptr;
        while (p && p->to != v) { prev = p; p = p->next; }
        if (!p) return;
        if (prev) prev->next = p->next; else adj[u] = p->next;
        delete p;
    }

    void tarjan(int u, vector<int>& disc, vector<int>& low, vector<int>& par,
                vector<bool>& isCut, int& timer, vector<pair<int,int>>& bridges) const {
        disc[u] = low[u] = ++timer;
        int children = 0;
        for (EdgeNode* p = adj[u]; p; p = p->next) {
            int v = p->to;
            if (!active[v]) continue;
            if (disc[v] == 0) {
                children++;
                par[v] = u;
                tarjan(v, disc, low, par, isCut, timer, bridges);
                low[u] = min(low[u], low[v]);
                if (par[u] == -1 && children > 1) isCut[u] = true;
                if (par[u] != -1 && low[v] >= disc[u]) isCut[u] = true;
                if (low[v] > disc[u]) bridges.push_back({u, v});
            } else if (v != par[u]) {
                low[u] = min(low[u], disc[v]);
            }
        }
    }

    void walk(int u, int t, int maxPaths, vector<bool>& onPath, vector<int>& cur,
              vector<vector<int>>& result) const {
        if ((int)result.size() >= maxPaths) return;
        onPath[u] = true;
        cur.push_back(u);
        if (u == t) {
            result.push_back(cur);
        } else {
            for (EdgeNode* p = adj[u]; p; p = p->next)
                if (active[p->to] && !onPath[p->to]) walk(p->to, t, maxPaths, onPath, cur, result);
        }
        cur.pop_back();
        onPath[u] = false;
    }
};

// ---------------------------------------------------------------
// Helpers for input and output
// ---------------------------------------------------------------
int readInt(const string& prompt) {
    while (true) {
        cout << prompt;
        string line;
        if (!getline(cin, line)) exit(0);
        try { return stoi(line); } catch (...) { cout << "  Please enter a number.\n"; }
    }
}

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) exit(0);
    return line;
}

// Accepts either a node name or an index
int readNode(const Graph& g, const string& prompt) {
    while (true) {
        string s = readLine(prompt);
        int id = g.findNode(s);
        if (id != -1) return id;
        try {
            int k = stoi(s);
            if (k >= 0 && k < g.size()) return k;
        } catch (...) {}
        cout << "  Node not found. Use a name or an index from the adjacency list.\n";
    }
}

void printPath(const Graph& g, const vector<int>& p) {
    for (size_t i = 0; i < p.size(); i++) cout << g.name[p[i]] << (i + 1 < p.size() ? " -> " : "");
    cout << "  (" << p.size() - 1 << " hops)\n";
}

void printOrder(const Graph& g, const vector<int>& order) {
    for (size_t i = 0; i < order.size(); i++) cout << g.name[order[i]] << (i + 1 < order.size() ? ", " : "");
    cout << "\n";
}

// Sample regional network: one large component, one small component, one isolated tower
void loadSample(Graph& g) {
    g.clear();
    const char* names[] = {"Pune", "Mumbai", "Nashik", "Lonavala", "Thane", "Satara", "Kolhapur",
                           "Solapur", "Aurangabad", "Nagpur", "Amravati", "Wardha", "Ratnagiri"};
    for (const char* n : names) g.addNode(n);
    const char* links[][2] = {
        {"Pune","Lonavala"}, {"Lonavala","Mumbai"}, {"Mumbai","Thane"}, {"Thane","Nashik"},
        {"Nashik","Aurangabad"}, {"Pune","Nashik"}, {"Pune","Satara"}, {"Satara","Kolhapur"},
        {"Pune","Solapur"}, {"Solapur","Aurangabad"}, {"Nagpur","Amravati"}, {"Amravati","Wardha"}
    };
    for (auto& l : links) g.addEdge(g.findNode(l[0]), g.findNode(l[1]));
    cout << "Sample network loaded: " << g.size() << " nodes, " << g.edgeCount << " links.\n";
}

// ---------------------------------------------------------------
// Menu actions
// ---------------------------------------------------------------
void doBFS(const Graph& g) {
    int s = readNode(g, "Start node: ");
    vector<int> order, parent, dist; TraversalStats st;
    g.bfs(s, order, parent, dist, st);
    cout << "BFS order: "; printOrder(g, order);
    cout << "Hop distance from " << g.name[s] << ":\n";
    for (int i = 0; i < g.size(); i++) {
        cout << "  " << left << setw(14) << g.name[i] << right;
        if (dist[i] < 0) cout << "unreachable\n"; else cout << dist[i] << "\n";
    }
}

void doDFS(const Graph& g) {
    int s = readNode(g, "Start node: ");
    vector<int> order; TraversalStats st;
    g.dfs(s, order, st);
    cout << "DFS order: "; printOrder(g, order);
}

void doComponents(const Graph& g) {
    vector<int> comp;
    int c = g.components(comp);
    cout << "Number of connected components: " << c << "\n";
    for (int k = 0; k < c; k++) {
        cout << "  Component " << k + 1 << ": ";
        bool first = true;
        for (int i = 0; i < g.size(); i++)
            if (comp[i] == k) { cout << (first ? "" : ", ") << g.name[i]; first = false; }
        cout << "\n";
    }
    cout << (c == 1 ? "The network is fully connected.\n" : "The network is split into separate parts.\n");
}

void doDegree(const Graph& g) {
    vector<int> idx;
    for (int i = 0; i < g.size(); i++) if (g.active[i]) idx.push_back(i);
    sort(idx.begin(), idx.end(), [&](int a, int b) { return g.degree(a) > g.degree(b); });
    double total = 0;
    for (int i : idx) total += g.degree(i);
    double avg = idx.empty() ? 0 : total / idx.size();
    cout << "Average degree: " << fixed << setprecision(2) << avg << "\n";
    cout << "Nodes ranked by degree (hub = degree above 1.5 times the average):\n";
    for (int i : idx) {
        int d = g.degree(i);
        cout << "  " << left << setw(14) << g.name[i] << right << " degree " << d
             << (d > 1.5 * avg ? "   <- high-degree hub" : "") << "\n";
    }
}

void doIsolated(const Graph& g) {
    bool found = false;
    cout << "Isolated nodes (no working links): ";
    for (int i = 0; i < g.size(); i++)
        if (g.active[i] && g.degree(i) == 0) { cout << (found ? ", " : "") << g.name[i]; found = true; }
    cout << (found ? "\n" : "none\n");
}

void doCritical(const Graph& g) {
    vector<int> cut; vector<pair<int,int>> br;
    g.criticalElements(cut, br);
    cout << "Critical nodes (removing one splits the network): ";
    if (cut.empty()) cout << "none";
    for (size_t i = 0; i < cut.size(); i++) cout << (i ? ", " : "") << g.name[cut[i]];
    cout << "\nCritical links (bridges): ";
    if (br.empty()) cout << "none";
    for (size_t i = 0; i < br.size(); i++)
        cout << (i ? ", " : "") << g.name[br[i].first] << "-" << g.name[br[i].second];
    cout << "\n";
}

void doShortest(const Graph& g) {
    int s = readNode(g, "Source node: ");
    int t = readNode(g, "Destination node: ");
    vector<int> p = g.shortestPath(s, t);
    if (p.empty()) cout << "No route exists between these nodes.\n";
    else { cout << "Shortest route: "; printPath(g, p); }
}

void doAlternate(const Graph& g) {
    int s = readNode(g, "Source node: ");
    int t = readNode(g, "Destination node: ");
    vector<vector<int>> paths;
    g.allPaths(s, t, 10, paths);
    if (paths.empty()) { cout << "No route exists between these nodes.\n"; return; }
    sort(paths.begin(), paths.end(), [](const vector<int>& a, const vector<int>& b) { return a.size() < b.size(); });
    cout << paths.size() << " route(s) found (showing up to 10, shortest first):\n";
    for (size_t i = 0; i < paths.size(); i++) { cout << "  " << i + 1 << ". "; printPath(g, paths[i]); }
    if (paths.size() == 1) cout << "Only one route exists, so this connection has no backup.\n";
}

void doFailure(Graph& g) {
    int f = readNode(g, "Node to fail or restore: ");
    vector<int> before, after;
    int cb = g.components(before);
    g.active[f] = !g.active[f];
    int ca = g.components(after);
    cout << g.name[f] << (g.active[f] ? " restored." : " is now DOWN.") << "\n";
    cout << "Components before: " << cb << ", after: " << ca << "\n";
    if (!g.active[f]) {
        cout << "Nodes cut off from the main part may need rerouting. Use option 12 or 13 to check routes.\n";
    }
}

void printStatsRow(const string& label, const TraversalStats& st) {
    cout << "  " << left << setw(6) << label << right
         << setw(10) << st.verticesVisited
         << setw(12) << st.edgesExamined
         << setw(14) << st.peakFrontier
         << setw(12) << fixed << setprecision(1) << st.micros << "\n";
}

void doCompare(const Graph& g) {
    int s = readNode(g, "Start node: ");
    vector<int> order, parent, dist; TraversalStats sb, sd;
    g.bfs(s, order, parent, dist, sb);
    g.dfs(s, order, sd);
    cout << "\n  Algo   Vertices   Edges seen   Peak frontier   Time (us)\n";
    printStatsRow("BFS", sb);
    printStatsRow("DFS", sd);
    cout << "\nBoth visit every reachable vertex once and scan each adjacency list once,\n"
         << "so both run in O(V + E) time with an adjacency list. Extra space is O(V).\n"
         << "BFS keeps a queue whose size follows the widest level of the graph.\n"
         << "DFS keeps a stack that grows along the path being explored. This version may push\n"
         << "a node more than once before it is popped, so its peak can exceed BFS.\n"
         << "BFS also gives the fewest-hop route, which DFS does not guarantee.\n";
}

void doBenchmark() {
    cout << "\nRandom networks with about 3 links per node (BFS vs DFS from node 0):\n";
    cout << "  Nodes   Links   BFS peak   DFS peak   BFS us    DFS us\n";
    int sizes[] = {100, 1000, 5000, 20000};
    srand(42);
    for (int n : sizes) {
        Graph g;
        for (int i = 0; i < n; i++) g.addNode(to_string(i));
        for (int i = 1; i < n; i++) g.addEdge(i, rand() % i);   // keeps it connected
        int extra = 2 * n;
        while (extra--) g.addEdge(rand() % n, rand() % n);
        vector<int> order, parent, dist; TraversalStats sb, sd;
        g.bfs(0, order, parent, dist, sb);
        g.dfs(0, order, sd);
        cout << "  " << setw(5) << n << setw(8) << g.edgeCount << setw(11) << sb.peakFrontier
             << setw(11) << sd.peakFrontier << setw(9) << fixed << setprecision(0) << sb.micros
             << setw(10) << sd.micros << "\n";
    }
    cout << "Time grows roughly in step with V + E for both, matching O(V + E).\n";
}

void doAddNode(Graph& g) {
    string nm = readLine("New node name: ");
    if (nm.empty() || g.findNode(nm) != -1) { cout << "Name is empty or already used.\n"; return; }
    g.addNode(nm);
    cout << "Added " << nm << " as node " << g.size() - 1 << ".\n";
}

void doAddLink(Graph& g) {
    int u = readNode(g, "First node: ");
    int v = readNode(g, "Second node: ");
    cout << (g.addEdge(u, v) ? "Link added.\n" : "Link not added (same node or already exists).\n");
}

void doRemoveLink(Graph& g) {
    int u = readNode(g, "First node: ");
    int v = readNode(g, "Second node: ");
    cout << (g.removeEdge(u, v) ? "Link removed.\n" : "No such link.\n");
}

void menu() {
    cout << "\n===== Telecom Network Connectivity Analyzer =====\n"
         << " 1. Load sample network        9. Degree analysis / hubs\n"
         << " 2. Add node                  10. Detect isolated nodes\n"
         << " 3. Add link                  11. Critical nodes and links\n"
         << " 4. Remove link               12. Shortest route\n"
         << " 5. Show adjacency lists      13. Alternate routes\n"
         << " 6. BFS traversal             14. Fail / restore a node\n"
         << " 7. DFS traversal             15. Compare BFS and DFS\n"
         << " 8. Connected components      16. Benchmark on random networks\n"
         << " 0. Exit\n";
}

int main() {
    Graph g;
    loadSample(g);
    while (true) {
        menu();
        int ch = readInt("Choice: ");
        cout << "\n";
        if (ch == 0) break;
        bool needNodes = (ch >= 3 && ch != 16 && ch != 1 && ch != 2);
        if (needNodes && g.size() == 0) { cout << "The network is empty. Load the sample or add nodes first.\n"; continue; }
        switch (ch) {
            case 1:  loadSample(g); break;
            case 2:  doAddNode(g); break;
            case 3:  doAddLink(g); break;
            case 4:  doRemoveLink(g); break;
            case 5:  g.printAdjacency(); break;
            case 6:  doBFS(g); break;
            case 7:  doDFS(g); break;
            case 8:  doComponents(g); break;
            case 9:  doDegree(g); break;
            case 10: doIsolated(g); break;
            case 11: doCritical(g); break;
            case 12: doShortest(g); break;
            case 13: doAlternate(g); break;
            case 14: doFailure(g); break;
            case 15: doCompare(g); break;
            case 16: doBenchmark(); break;
            default: cout << "Invalid choice.\n";
        }
    }
    cout << "Goodbye.\n";
    return 0;
}