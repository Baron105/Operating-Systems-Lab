#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
using namespace std;

// Function to perform topological sorting
void topologicalSort(vector<vector<int>>& graph) {
    int V = graph.size();
    vector<int> inDegree(V, 0);

    // Calculate in-degree for each vertex
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) {
            if (graph[i][j] == 1)
                inDegree[j]++;
        }
    }

    // Create a queue to store vertices with in-degree 0
    queue<int> q;
    for (int i = 0; i < V; ++i) {
        if (inDegree[i] == 0)
            q.push(i);
    }

    // Perform topological sorting
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        cout << u << " ";

        // Reduce in-degree of adjacent vertices
        for (int v = 0; v < V; ++v) {
            if (graph[u][v] == 1) {
                inDegree[v]--;
                if (inDegree[v] == 0)
                    q.push(v);
            }
        }
    }
}

int main() {
    int V;
    // first line of graph.txt contains number of vertices
    ifstream fin("graph.txt");
    fin >> V;

    vector<vector<int>> graph(V, vector<int>(V, 0));
    // take file input from graph.txt
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) {
            fin >> graph[i][j];
        }
    }

    cout << "Topological Sorting: ";
    topologicalSort(graph);
    cout << endl;

    return 0;
}
