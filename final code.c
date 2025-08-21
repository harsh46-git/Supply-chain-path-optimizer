#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_NODES 300
#define MAX_PRODUCTS 100
#define INF 100000  // Large value to represent no direct path

typedef struct {
    char name[50];
    int quantity;
    double price;
} Product;

Product inventory[MAX_PRODUCTS];
int productCount = 0;
int orders[MAX_NODES];    // Stores the customer locations where products will be delivered
int orderCount = 0;

typedef struct ListNode {
    int vertex;
    int weight;
    struct ListNode* next;
} ListNode;

typedef struct {
    int numNodes;
    ListNode* adjList[MAX_NODES];
} Graph;

// Priority queue node for Dijkstra's algorithm
typedef struct {
    int vertex;
    int distance;
} MinHeapNode;

typedef struct {
    MinHeapNode* array[MAX_NODES];
    int size;
} MinHeap;

// Function to create a new adjacency list node
void addEdge(Graph* graph, int u, int v, int weight) {
    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    newNode->vertex = v;
    newNode->weight = weight;
    newNode->next = graph->adjList[u];
    graph->adjList[u] = newNode;

    newNode = (ListNode*)malloc(sizeof(ListNode));
    newNode->vertex = u;
    newNode->weight = weight;
    newNode->next = graph->adjList[v];
    graph->adjList[v] = newNode;
}

// Function to create a new min heap node for Dijkstra
MinHeapNode* newMinHeapNode(int vertex, int distance) {
    MinHeapNode* minHeapNode = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    minHeapNode->vertex = vertex;
    minHeapNode->distance = distance;
    return minHeapNode;
}

// Function to initialize a min heap
MinHeap* createMinHeap() {
    MinHeap* minHeap = (MinHeap*)malloc(sizeof(MinHeap));
    minHeap->size = 0;
    return minHeap;
}

// Function to swap two nodes in a min heap
void swapMinHeapNode(MinHeapNode** a, MinHeapNode** b) {
    MinHeapNode* temp = *a;
    *a = *b;
    *b = temp;
}

// Function to heapify a node
void minHeapify(MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->distance < minHeap->array[smallest]->distance) {
        smallest = left;
    }

    if (right < minHeap->size && minHeap->array[right]->distance < minHeap->array[smallest]->distance) {
        smallest = right;
    }

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// Function to extract the node with the minimum distance
MinHeapNode* extractMin(MinHeap* minHeap) {
    if (minHeap->size == 0) {
        return NULL;
    }

    MinHeapNode* root = minHeap->array[0];
    MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;
    minHeap->size--;
    minHeapify(minHeap, 0);

    return root;
}

// Function to decrease the distance of a vertex in the heap
void decreaseKey(MinHeap* minHeap, int vertex, int distance) {
    int i;
    for (i = 0; i < minHeap->size; i++) {
        if (minHeap->array[i]->vertex == vertex) {
            minHeap->array[i]->distance = distance;
            break;
        }
    }

    while (i && minHeap->array[i]->distance < minHeap->array[(i - 1) / 2]->distance) {
        swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

// Function to check if a vertex is in the heap
int isInMinHeap(MinHeap* minHeap, int vertex) {
    for (int i = 0; i < minHeap->size; i++) {
        if (minHeap->array[i]->vertex == vertex) {
            return 1;
        }
    }
    return 0;
}

// Function to print the path from the source to the target
void printPath(int parent[], int target) {
    if (parent[target] == -1) {
        printf("%d ", target);
        return;
    }
    printPath(parent, parent[target]);
    printf("-> %d ", target);
}

// Dijkstra's Algorithm to find the shortest path between two nodes, storing the path
int dijkstra(Graph* graph, int src, int target, int parent[]) {
    int dist[MAX_NODES];
    MinHeap* minHeap = createMinHeap();

    for (int i = 0; i < graph->numNodes; i++) {
        dist[i] = INF;
        minHeap->array[i] = newMinHeapNode(i, dist[i]);
        minHeap->size++;
        parent[i] = -1;  // Initialize parent of each node as -1
    }

    dist[src] = 0;
    decreaseKey(minHeap, src, dist[src]);

    while (minHeap->size) {
        MinHeapNode* minNode = extractMin(minHeap);
        int u = minNode->vertex;

        ListNode* temp = graph->adjList[u];
        while (temp != NULL) {
            int v = temp->vertex;

            if (isInMinHeap(minHeap, v) && dist[u] != INF && dist[u] + temp->weight < dist[v]) {
                dist[v] = dist[u] + temp->weight;
                parent[v] = u;  // Update the parent of v
                decreaseKey(minHeap, v, dist[v]);
            }

            temp = temp->next;
        }
    }

    free(minHeap);

    return dist[target];
}

// Updated getDistance function using Dijkstra's algorithm with path tracking
int getDistance(Graph* graph, int u, int v, int parent[]) {
    int distance = dijkstra(graph, u, v, parent);
    if (distance == INF) {
        return INF;  // No path exists between the nodes
    }
    return distance;
}

// Nearest Neighbor Heuristic for TSP, showing all nodes in the path
void nearestNeighborTSP(Graph* graph, int startNode, int* orderNodes, int n) {
    int visited[MAX_NODES] = {0};  // Track visited nodes
    int parent[MAX_NODES];         // To store the parent nodes for path tracking
    int totalDistance = 0;
    int currentNode = startNode;

    printf("Starting from warehouse (Node %d)\n", startNode);
    printf("Optimal Route: %d ", startNode);
    visited[startNode] = 1;

    for (int i = 0; i < n; i++) {
        int nextNode = -1;
        int shortestDistance = INF;

        // Find the nearest unvisited node
        for (int j = 0; j < n; j++) {
            if (!visited[orderNodes[j]]) {
                int distance = getDistance(graph, currentNode, orderNodes[j], parent);
                if (distance < shortestDistance) {
                    shortestDistance = distance;
                    nextNode = orderNodes[j];
                }
            }
        }

        if (nextNode != -1) {
            // Print the path from currentNode to nextNode, including intermediate nodes
            printf("-> ");
            printPath(parent, nextNode);
            visited[nextNode] = 1;  // Mark node as visited
            totalDistance += shortestDistance;
            currentNode = nextNode;
        }
    }

    // Return to the warehouse
    int returnDistance = getDistance(graph, currentNode, startNode, parent);
    printf("-> ");
    printPath(parent, startNode);

    totalDistance += returnDistance;
    printf("END\n");
    printf("Total Delivery Distance: %d\n", totalDistance);
}

// Add product to inventory (without location now)
void addProduct() {
    if (productCount < MAX_PRODUCTS) {
        char name[50];
        int quantity;
        double price;

        printf("\nEnter product name: ");
        scanf("%s", name);
        printf("Enter quantity: ");
        scanf("%d", &quantity);
        printf("Enter price: ");
        scanf("%lf", &price);

        strcpy(inventory[productCount].name, name);
        inventory[productCount].quantity = quantity;
        inventory[productCount].price = price;
        productCount++;

        printf("Product added: %s, Quantity: %d, Price: %.2f\n", name, quantity, price);
    } else {
        printf("Inventory is full! Cannot add more products.\n");
    }
}

// Display inventory
void displayInventory() {
    if (productCount == 0) {
        printf("Inventory is empty.\n");
    } else {
        printf("\nCurrent Inventory:\n");
        for (int i = 0; i < productCount; i++) {
            printf("Product: %s, Quantity: %d, Price: %.2f\n",
                   inventory[i].name, inventory[i].quantity, inventory[i].price);
        }
    }
}

// Process an order and set the delivery location (customer)
void processOrder(char productName[], int quantity, int location) {
    for (int i = 0; i < productCount; i++) {
        if (strcmp(inventory[i].name, productName) == 0) {
            if (inventory[i].quantity >= quantity) {
                // Reduce inventory
                inventory[i].quantity -= quantity;

                // Add the customer location where the product will be delivered
                orders[orderCount] = location;
                orderCount++;

                printf("Order placed for %d %s(s) to be delivered to customer location %d. Remaining stock: %d\n",
                       quantity, productName, location, inventory[i].quantity);
                return;
            } else {
                printf("Not enough stock for %s.\n", productName);
                return;
            }
        }
    }
    printf("Product %s not found in inventory.\n", productName);
}

int main() {
    // Initialize graph with 50 nodes
    Graph graph;
    graph.numNodes = MAX_NODES;
    for (int i = 0; i < MAX_NODES; i++) {
        graph.adjList[i] = NULL;
    }


    // Add edges to represent delivery paths between nodes
    addEdge(&graph, 0, 1, 145);       // First: Node 0 to Node 1
    addEdge(&graph, 1, 2, 110);       // Second: Node 1 to Node 2
    addEdge(&graph, 2, 3, 89);        // Third: Node 2 to Node 3
    addEdge(&graph, 3, 4, 49);        // Fourth: Node 3 to Node 4
    addEdge(&graph, 4, 5, 25);        // Fifth: Node 4 to Node 5
    addEdge(&graph, 5, 6, 17);        // Sixth: Node 5 to Node 6
    addEdge(&graph, 6, 7, 51);        // Seventh: Node 6 to Node 7
    addEdge(&graph, 7, 8, 22);        // Eighth: Node 7 to Node 8
    addEdge(&graph, 8, 9, 28);        // Ninth: Node 8 to Node 9
    addEdge(&graph, 5, 10, 66);       // Tenth: Node 5 to Node 10
    addEdge(&graph, 10, 11, 28);      // Eleventh: Node 10 to Node 11
    addEdge(&graph, 11, 12, 46);      // Twelfth: Node 11 to Node 12
    addEdge(&graph, 4, 11, 71);       // Thirteenth: Node 4 to Node 11
    addEdge(&graph, 3, 12, 71);       // Fourteenth: Node 3 to Node 12
    addEdge(&graph, 1, 13, 66);       // Fifteenth: Node 1 to Node 13
    addEdge(&graph, 13, 14, 175);     // Sixteenth: Node 13 to Node 14
    addEdge(&graph, 14, 15, 68);      // Seventeenth: Node 1 to Node 4 to Node 15
    addEdge(&graph, 15, 16, 63);      // Eighteenth: Node 15 to Node 16
    addEdge(&graph, 16, 17, 115);     // Nineteenth: Node 16 to Node 17
    addEdge(&graph, 17, 18, 53);      // Twentieth: Node 17 to Node 18
    addEdge(&graph, 17, 19, 41);      // Twenty-first: Node 17 to Node 19
    addEdge(&graph, 19, 20, 27);      // Twenty-second: Node 19 to Node 20
    addEdge(&graph, 20, 21, 32);      // Twenty-third: Node 20 to Node 21
    addEdge(&graph, 21, 22, 33);      // Twenty-fourth: Node 21 to Node 22
    addEdge(&graph, 22, 23, 34);      // Twenty-fifth: Node 22 to Node 23
    addEdge(&graph, 23, 24, 31);      // Twenty-sixth: Node 23 to Node 24
    addEdge(&graph, 22, 25, 36);      // Twenty-seventh: Node 22 to Node 25
    addEdge(&graph, 25, 26, 19);      // Twenty-eighth: Node 25 to Node 26
    addEdge(&graph, 26, 27, 28);      // Twenty-ninth: Node 26 to Node 27
    addEdge(&graph, 27, 28, 41);      // Thirtieth: Node 27 to Node 28
    addEdge(&graph, 28, 29, 24);      // Thirty-first: Node 28 to Node 29
    addEdge(&graph, 29, 30, 44);      // Thirty-second: Node 29 to Node 30
    addEdge(&graph, 30, 31, 26);      // Thirty-third: Node 30 to Node 31
    addEdge(&graph, 31, 32, 27);      // Thirty-fourth: Node 31 to Node 32
    addEdge(&graph, 16, 33, 105);     // Thirty-fifth: Node 16 to Node 33
    addEdge(&graph, 33, 34, 230);     // Thirty-sixth: Node 33 to Node 34
    addEdge(&graph, 33, 35, 54);      // Thirty-seventh: Node 33 to Node 35
    addEdge(&graph, 35, 36, 44);      // Thirty-eighth: Node 35 to Node 36
    addEdge(&graph, 36, 37, 109);     // Thirty-ninth: Node 36 to Node 37
    addEdge(&graph, 36, 38, 189);     // Fortieth: Node 36 to Node 38
    addEdge(&graph, 38, 39, 26);      // Forty-first: Node 38 to Node 39
    addEdge(&graph, 39, 40, 60);      // Forty-second: Node 39 to Node 40
    addEdge(&graph, 40, 41, 35);      // Forty-third: Node 40 to Node 41
    addEdge(&graph, 41, 42, 71);      // Forty-fourth: Node 41 to Node 42
    addEdge(&graph, 42, 43, 15);      // Forty-fifth: Node 42 to Node 43
    addEdge(&graph, 43, 44, 68);      // Forty-sixth: Node 43 to Node 44
    addEdge(&graph, 44, 45, 12);      // Forty-seventh: Node 44 to Node 45
    addEdge(&graph, 45, 46, 70);      // Forty-eighth: Node 45 to Node 46
    addEdge(&graph, 45, 47, 60);      // Forty-ninth: Node 45 to Node 47
    addEdge(&graph, 47, 48, 70);      // Fiftieth: Node 47 to Node 48
    addEdge(&graph, 44, 49, 52);      // Fifty-one: 
    addEdge(&graph, 49, 50, 30);      // Fifty-two: 
    addEdge(&graph, 50, 51, 40);      // Fifty-three: 
    addEdge(&graph, 51, 52, 40);      // Fifty-four: 
    addEdge(&graph, 52, 53, 50);      // Fifty-five: 
    addEdge(&graph, 53, 54, 60);      // Fifty-six: 
    addEdge(&graph, 54, 55, 20);      // Fifty-seven: 
    addEdge(&graph, 55, 56, 20);      // Fifty-eight: 
    addEdge(&graph, 56, 57, 30);      // Fifty-nine: 
    addEdge(&graph, 57, 58, 80);      // Sixty: 
    addEdge(&graph, 58, 59, 80);      // Sixty-one: 
    addEdge(&graph, 59, 60, 40);      // Sixty-two: 
    addEdge(&graph, 60, 61, 40);      // Sixty-three: 
    addEdge(&graph, 61, 62, 70);      // sixty-four: 
    addEdge(&graph, 62, 63, 70);      // sixty-five: 
    addEdge(&graph, 63, 64, 40);      // sixty-six: 
    addEdge(&graph, 64, 65, 50);      // sixty-seven: 
    addEdge(&graph, 65, 66, 50);      // sixty-eight: 
    addEdge(&graph, 66, 67, 50);     // 69
    addEdge(&graph, 67, 68, 50);     // 70
    addEdge(&graph, 68, 69, 130);      // 71
    addEdge(&graph, 69, 70, 30);      //72
    addEdge(&graph, 70, 71, 60);      //73
    addEdge(&graph, 70, 147, 30);     // 74
    addEdge(&graph, 147, 142, 30);    // 75
    addEdge(&graph, 69, 142, 50);     // 76
    addEdge(&graph, 142, 143, 70);    // 77
    addEdge(&graph, 143, 144, 60);    // 78
    addEdge(&graph, 144, 145, 30);    // 79
    addEdge(&graph, 145, 146, 30);    // 80
    addEdge(&graph, 146, 129, 30);    // 81
    addEdge(&graph, 129, 130, 50);    // 82
    addEdge(&graph, 130, 131, 20);    // 83
    addEdge(&graph, 145, 131, 20);    // 84
    addEdge(&graph, 131, 132, 40);    // 85
    addEdge(&graph, 132, 133, 20);    // Eighty-sixth: Node 132 to Node 133
    addEdge(&graph, 133, 148, 30);    // Eighty-sixth: Node 132 to Node 133
    addEdge(&graph, 133, 139, 110);    // Eighty-eighth: Node 133 to Node 139
    addEdge(&graph, 139, 140, 40);    // Eighty-ninth: Node 139 to Node 140
    addEdge(&graph, 140, 141, 40);    // Ninetieth: Node 140 to Node 141
    addEdge(&graph, 139, 138, 40);    // Ninety-first: Node 139 to Node 138
    addEdge(&graph, 138, 137, 50);    // Ninety-second: Node 138 to Node 137
    addEdge(&graph, 137, 136, 20);    // Ninety-third: Node 137 to Node 136
    addEdge(&graph, 132, 135, 20);    // Ninety-fourth: Node 132 to Node 135
    addEdge(&graph, 135, 136, 60);    // Ninety-fifth: Node 135 to Node 136
    addEdge(&graph, 135, 134, 30);    // Ninety-sixth: Node 135 to Node 134
    addEdge(&graph, 130, 129, 30);    // Ninety-seventh: Node 130 to Node 129
    addEdge(&graph, 129, 117, 40);    // Ninety-eighth: Node 129 to Node 117
    addEdge(&graph, 143, 115, 30);    // Ninety-ninth: Node 143 to Node 115
    addEdge(&graph, 115, 116, 90);    // One-hundredth: Node 115 to Node 116
    addEdge(&graph, 116, 117, 30);    // One-hundred-first: Node 116 to Node 117
    addEdge(&graph, 117, 118, 10);    // One-hundred-second: Node 117 to Node 118
    addEdge(&graph, 118, 119, 40);    // One-hundred-third: Node 118 to Node 119
    addEdge(&graph, 119, 124, 30);    // One-hundred-fourth: Node 119 to Node 124
    addEdge(&graph, 124, 123, 40);    // One-hundred-fifth: Node 124 to Node 123
    addEdge(&graph, 123, 120, 40);    // One-hundred-sixth: Node 123 to Node 120
    addEdge(&graph, 119, 120, 50);    // One-hundred-seventh: Node 119 to Node 120
    addEdge(&graph, 120, 121, 30);    // One-hundred-eighth: Node 120 to Node 121
    addEdge(&graph, 121, 122, 20);    // One-hundred-ninth: Node 121 to Node 122
    addEdge(&graph, 120, 123, 40);    // One-hundred-tenth: Node 120 to Node 123
    addEdge(&graph, 116, 125, 100);   // One-hundred-eleventh: Node 116 to Node 125
    addEdge(&graph, 125, 126, 30);    // One-hundred-twelfth: Node 125 to Node 126
    addEdge(&graph, 126, 127, 40);    // One-hundred-thirteenth: Node 126 to Node 127
    addEdge(&graph, 127, 128, 50);    // One-hundred-fourteenth: Node 127 to Node 128
    addEdge(&graph, 143, 115, 40);    // One-hundred-fifteenth: Node 143 to Node 115
    addEdge(&graph, 115, 113, 70);    // One-hundred-sixteenth: Node 115 to Node 113
    addEdge(&graph, 113, 114, 50);    // One-hundred-seventeenth: Node 113 to Node 114
    addEdge(&graph, 68, 113, 40);     // One-hundred-eighteenth: Node 68 to Node 113
    addEdge(&graph, 67, 112, 70);     // One-hundred-nineteenth: Node 67 to Node 112
    addEdge(&graph, 65, 110, 50);     // One-hundred-twentieth: Node 65 to Node 110
    addEdge(&graph, 110, 111, 50);    // One-hundred-twenty-first: Node 110 to Node 111
    addEdge(&graph, 62, 108, 70);     // One-hundred-twenty-second: Node 62 to Node 108
    addEdge(&graph, 108, 107, 20);    // One-hundred-twenty-third: Node 108 to Node 107
    addEdge(&graph, 107, 106, 70);    // One-hundred-twenty-fourth: Node 107 to Node 106
    addEdge(&graph, 106, 105, 60);    // One-hundred-twenty-fifth: Node 106 to Node 105
    addEdge(&graph, 105, 104, 50);    // One-hundred-twenty-sixth: Node 105 to Node 104
    addEdge(&graph, 104, 103, 50);    // One-hundred-twenty-seventh: Node 104 to Node 103
    addEdge(&graph, 103, 90, 80);     // One-hundred-twenty-eighth: Node 103 to Node 90
    addEdge(&graph, 104, 88, 60);     // One-hundred-twenty-ninth: Node 104 to Node 88
    addEdge(&graph, 105, 85, 60);     // One-hundred-thirtieth: Node 105 to Node 85    
    addEdge(&graph, 85, 88, 50);     // One-hundred-thirtyone: Node 105 to Node 85    
    addEdge(&graph, 88, 89, 20);     // 132   
    addEdge(&graph, 89, 90, 30);     // 133    
    addEdge(&graph, 90, 91, 60);     // 134    
    addEdge(&graph, 91, 102, 20);     // 135    
    addEdge(&graph, 102, 101, 30);     // 136    
    addEdge(&graph, 101, 100, 50);     // 137    
    addEdge(&graph, 100, 92, 30);     // 138    
    addEdge(&graph, 92, 93, 100);     // 139    
    addEdge(&graph, 93, 94, 50);     //  140
    addEdge(&graph, 102, 92, 60);     // 141    
    addEdge(&graph, 102, 92, 60);     // 142   
    addEdge(&graph, 91, 94, 100);     // 143   
    addEdge(&graph, 94, 95, 10);     // 144    
    addEdge(&graph, 95, 96, 20);     // 145   
    addEdge(&graph, 95, 97, 20);     // 146 
    addEdge(&graph, 97, 98, 30);     // 147
    addEdge(&graph, 98, 99, 50);     // 148   
    addEdge(&graph, 99, 89, 20);     // 149  
    addEdge(&graph, 85, 86, 20);     //  150  
    addEdge(&graph, 85, 84, 60);     //  160   
    addEdge(&graph, 83, 82, 30);     //  161
    addEdge(&graph, 82, 81, 40);     // 162
    addEdge(&graph, 87, 109, 30);     // 163   
    addEdge(&graph, 87, 75, 80);     //  164 
    addEdge(&graph, 75, 74, 100);     // 165  
    addEdge(&graph, 74, 73, 20);     //   166 
    addEdge(&graph, 73, 72, 150);     //  167   
    addEdge(&graph, 61, 76, 110);     // 169
    addEdge(&graph, 55, 77, 30);     // 170    
    addEdge(&graph, 77, 78, 70);     // 171   
    addEdge(&graph, 78, 79, 30);     //  172   
    addEdge(&graph, 79, 80, 90);     // 173    
    addEdge(&graph, 53, 72, 30);     //  174  
    addEdge(&graph, 53, 72, 30);     //  175  
    addEdge(&graph, 40, 149, 101);     // 176
    addEdge(&graph, 149, 150, 56);     // 177
    addEdge(&graph, 150, 151, 21);     // 178  
    addEdge(&graph, 151, 152, 33);     // 179  
    addEdge(&graph, 152, 153, 30);     // 180   
    addEdge(&graph, 150, 153, 37);     //  181 
    addEdge(&graph, 153, 154, 47);     // 182   
    addEdge(&graph, 154, 155, 40);     // 183  
    addEdge(&graph, 155, 156, 80);     // 184    
    addEdge(&graph, 155, 157, 50);     // 185 
    addEdge(&graph, 157, 43, 70);     //  186   
    addEdge(&graph, 157, 158, 40);     // 187   
    addEdge(&graph, 158, 159, 40);     // 188 
    addEdge(&graph, 159, 160, 60);     // 189  
    addEdge(&graph, 160, 161, 60);     //  190  
    addEdge(&graph, 160, 162, 60);     // 191  
    addEdge(&graph, 162, 163, 30);     //   192
    addEdge(&graph, 163, 164, 10);     //   193
    addEdge(&graph, 164, 165, 10);     //   194
    addEdge(&graph, 162, 166, 30);     //   195
    addEdge(&graph, 166, 167, 40);     //   196
    addEdge(&graph, 167, 168, 40);     //   197
    addEdge(&graph, 168, 169, 10);     //    198
    addEdge(&graph, 169, 170, 30);     //     199
    addEdge(&graph, 169, 171, 20);     //   200
    addEdge(&graph, 171, 172, 30);     //     201
    addEdge(&graph, 172, 180, 30);     //  202
    addEdge(&graph, 172, 230, 60);     //    203
    addEdge(&graph, 230, 173, 10);     //     204
    addEdge(&graph, 173, 174, 60);     // 205
    addEdge(&graph, 174, 180, 40);     //    206 
    addEdge(&graph, 180, 181, 30);     //   207
    addEdge(&graph, 181, 182, 20);     //    208
    addEdge(&graph, 174, 175, 30);     //   209
    addEdge(&graph, 175, 176, 10);     //     210
    addEdge(&graph, 176, 182, 50);     // 211
    addEdge(&graph, 182, 183, 30);     //    212 
    addEdge(&graph, 183, 184, 40);     //   213
    addEdge(&graph, 184, 185, 70);     //    214
    addEdge(&graph, 185, 186, 10);     //    215
    addEdge(&graph, 176, 177, 70);     //     216
    addEdge(&graph, 177, 184, 70);     //     217
    addEdge(&graph, 177, 178, 70);     //    218
    addEdge(&graph, 178, 186, 80);     //   219
    addEdge(&graph, 178, 179, 70);     //   220
    addEdge(&graph, 179, 53, 100);     //   221
    addEdge(&graph, 178, 193, 70);     //    222
    addEdge(&graph, 193, 54, 160);     //     223
    addEdge(&graph, 193, 196, 70);     //  224
    addEdge(&graph, 196, 57, 140);     //    225
    addEdge(&graph, 196, 202, 70);     //     226
    addEdge(&graph, 202, 203, 50);     //   227
    addEdge(&graph, 203, 58, 70);     //  228
    addEdge(&graph, 203, 204, 40);     //    229
    addEdge(&graph, 204, 205, 20);     //  230
    addEdge(&graph, 205, 206, 130);     //    231 
    addEdge(&graph, 204, 207, 130);     //    232
    addEdge(&graph, 202, 211, 100);     //     233
    addEdge(&graph, 211, 216, 90);     //     234
    addEdge(&graph, 216, 217, 70);     //     235
    addEdge(&graph, 217, 61, 100);     //    236
    addEdge(&graph, 217, 225, 90);     //     237
    addEdge(&graph, 225, 226, 40);     //     238
    addEdge(&graph, 226, 224, 40);     //    239
    addEdge(&graph, 224, 227, 70);     //     240
    addEdge(&graph, 224, 223, 20);     //    241
    addEdge(&graph, 225, 223, 40);     //   242
    addEdge(&graph, 223, 222, 40);     //    243
    addEdge(&graph, 222, 227, 50);     //    244
    addEdge(&graph, 222, 221, 20);     //     245
    addEdge(&graph, 221, 215, 100);     //     246
    addEdge(&graph, 216, 215, 50);     //     247
    addEdge(&graph, 227, 228, 20);     //    248
    addEdge(&graph, 228, 63, 170);     //     249
    addEdge(&graph, 215, 214, 10);     //     250
    addEdge(&graph, 214, 210, 90);     //   251
    addEdge(&graph, 211, 210, 40);     //    252
    addEdge(&graph, 210, 201, 100);     //   253
    addEdge(&graph, 202, 201, 40);     //   254
    addEdge(&graph, 201, 200, 50);     //255
    addEdge(&graph, 200, 199, 70);     //256
    addEdge(&graph, 200, 199, 70);     //  257
    addEdge(&graph, 199, 197, 40);     //   258
    addEdge(&graph, 197, 194, 30);     //   259
    addEdge(&graph, 194, 195, 90);     // 260
    addEdge(&graph, 194, 196, 50);     //    261 
    addEdge(&graph, 194, 191, 80);     //  262
    addEdge(&graph, 191, 192, 90);     //  263
    addEdge(&graph, 193, 191, 150);     // 264
    addEdge(&graph, 175, 191, 70);     //     265
    addEdge(&graph, 199, 198, 290);     //   266
    addEdge(&graph, 198, 208, 90);     //     267
    addEdge(&graph, 208, 212, 90);     //   268
    addEdge(&graph, 212, 218, 50);     //     269
    addEdge(&graph, 218, 219, 70);     //   270
    addEdge(&graph, 219, 220, 80);     //    271
    addEdge(&graph, 220, 221, 50);     //    272
    addEdge(&graph, 221, 222, 30);     //   273
    addEdge(&graph, 220, 213, 80);     //    274
    addEdge(&graph, 212, 213, 90);     //    275
    addEdge(&graph, 212, 213, 90);     //   276
    addEdge(&graph, 212, 237, 50);     //   277
    addEdge(&graph, 237, 236, 70);     //    278
    addEdge(&graph, 236, 243, 80);     //     279
    addEdge(&graph, 243, 248, 180);     //    280
    addEdge(&graph, 248, 247, 50);     // 281
    addEdge(&graph, 247, 246, 50);     //282
    addEdge(&graph, 246, 245, 90);     // 283
    addEdge(&graph, 245, 244, 60);     //    284 
    addEdge(&graph, 244, 241, 40);     //    285
    addEdge(&graph, 241, 242, 100);     //    286
    addEdge(&graph, 246, 242, 80);     //  287
    addEdge(&graph, 243, 242, 80);     //    288
    addEdge(&graph, 236, 235, 80);     //     289
    addEdge(&graph, 235, 208, 160);     //   290
    addEdge(&graph, 198, 190, 70);     //  291
    addEdge(&graph, 190, 234, 60);     //   292
    addEdge(&graph, 234, 233, 30);     //     293
    addEdge(&graph, 233, 241, 50);     //  294
    addEdge(&graph, 234, 235, 100);     //  295
    addEdge(&graph, 190, 189, 40);     //     296
    addEdge(&graph, 189, 197, 90);     // 297
    addEdge(&graph, 189, 180, 70);     //    298
    addEdge(&graph, 188, 187, 60);     //    299
    addEdge(&graph, 191, 187, 350);     //   300
    addEdge(&graph, 187, 229, 50);     //    301
    addEdge(&graph, 173, 187, 60);     // 302
    addEdge(&graph, 230, 231, 70);     //  303
    addEdge(&graph, 231, 239, 90);     //   304
    addEdge(&graph, 231, 232, 250);     //   305
    addEdge(&graph, 232, 233, 120);     //   306
    addEdge(&graph, 231, 240, 60);     //    307
    addEdge(&graph, 240, 241, 170);     //  308
    addEdge(&graph, 240, 249, 60);     //  309
    addEdge(&graph, 249, 250, 20);     //   310
    addEdge(&graph, 250, 251, 50);     // 311
    addEdge(&graph, 251, 252, 50);     //   312
    addEdge(&graph, 252, 245, 40);     // 313
    addEdge(&graph, 245, 259, 80);     //   314
    addEdge(&graph, 250, 253, 150);     //   315
    addEdge(&graph, 253, 254, 40);     // 316
    addEdge(&graph, 254, 255, 90);     // 317
    addEdge(&graph, 255, 256, 40);     //  318
    addEdge(&graph, 256, 257, 30);     // 319
    addEdge(&graph, 257, 258, 40);     // 320
    addEdge(&graph, 256, 285, 10);     // 321
    addEdge(&graph, 285, 286, 50);     // 322
    addEdge(&graph, 286, 287, 60);     //   323
    addEdge(&graph, 149, 260, 109);     //  324
    addEdge(&graph, 260, 261, 70);     // 325
    addEdge(&graph, 261, 262, 30);     // 326
    addEdge(&graph, 262, 263, 60);     // 327
    addEdge(&graph, 260, 264, 100);     // 328
    addEdge(&graph, 264, 265, 30);     //    329
    addEdge(&graph, 265, 266, 30);     // 330
    addEdge(&graph, 266, 267, 90);     // 331
    addEdge(&graph, 267, 268, 90);     // 332
    addEdge(&graph, 268, 269, 30);     // 333
    addEdge(&graph, 269, 270, 40);     // 334
    addEdge(&graph, 270, 271, 130);     // 335
    addEdge(&graph, 264, 272, 90);     //  336
    addEdge(&graph, 272, 275, 50);     //   337
    addEdge(&graph, 275, 274, 70);     //     338
    addEdge(&graph, 272, 273, 80);     //   339
    addEdge(&graph, 273, 274, 110);     //   340
    addEdge(&graph, 274, 276, 140);     //    341
    addEdge(&graph, 276, 277, 20);     //     342
    addEdge(&graph, 277, 278, 100);     //   343
    addEdge(&graph, 278, 281, 40);     //    344
    addEdge(&graph, 278, 279, 100);     //   345
    addEdge(&graph, 279, 280, 80);     //     346
    addEdge(&graph, 277, 282, 140);     //     347
    addEdge(&graph, 282, 283, 200);     //    348
    addEdge(&graph, 283, 284, 100);     //    349
    addEdge(&graph, 284, 285, 50);     //   350
    addEdge(&graph, 285, 286, 50);     //    351
    addEdge(&graph, 286, 287, 60);     // 352
    addEdge(&graph, 287, 259, 120);     //   353


    // Add products to the inventory
    int numProducts;
    printf("Enter the number of products to add to inventory: ");
    scanf("%d", &numProducts);
    for (int i = 0; i < numProducts; i++) {
        addProduct();
    }

    // Display the inventory
    displayInventory();

    // Test adding multiple orders
    char productName[50];
    int quantity;
    int location;
    char choice;

    do {
        printf("\nEnter product name to order: ");
        scanf("%s", productName);
        printf("Enter quantity: ");
        scanf("%d", &quantity);
        printf("Enter the customer location (node) where the product will be delivered: ");
        scanf("%d", &location);

        // Process the order
        processOrder(productName, quantity, location);

        printf("\nDo you want to add another order? (y/n): ");
        scanf(" %c", &choice);
    } while (choice == 'y' || choice == 'Y');

    // Check if any orders are placed
    if (orderCount == 0) {
        printf("\nNo orders have been placed. Optimized delivery route cannot be calculated.\n");
        if (productCount == 0) {
            printf("Inventory is also empty. Please add products and place orders.\n");
        }
    } else {
        // Print the optimized delivery route
        printf("\nFinding optimized delivery route...\n");
        nearestNeighborTSP(&graph, 0, orders, orderCount);
    }

    return 0;
}