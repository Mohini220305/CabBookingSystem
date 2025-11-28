#include <stdio.h>
#include <math.h>
#include "map.h"

#define PI 3.14159265

void displayMap(Graph *g);

int main()
{
    printf("Content-type:text/html\n\n");
    printf("<html><head><title>City Map | Cabify-C</title>");
    printf("<style>");
    printf("body {font-family: 'Segoe UI', Arial; background:#f8fafc; text-align:center;}");
    printf("h2 {color:#2c3e50; margin-top:20px;}");
    printf(".container {display:inline-block; background:#fff; padding:20px; border-radius:12px; box-shadow:0 4px 12px rgba(0,0,0,0.1);}");
    printf("svg {border:1px solid #ccc; background:#fdfdfd; border-radius:10px;}");
    printf("circle:hover {fill:#2ecc71; cursor:pointer;}");
    printf("text {pointer-events:none; font-weight:bold;}");
    printf("</style></head><body>");
    printf("<h2>Dehradun City Map (Cabify-C Routes)</h2>");
    printf("<div class='container'>");

    Graph *city = initDehradunMap();
    displayMap(city);

    printf("</div></body></html>");
    return 0;
}

void displayMap(Graph *g)
{
    int centerX = 400, centerY = 300, radius = 220;
    double angleStep = 2 * PI / g->n;
    int nodeX[50], nodeY[50];

    printf("<svg width='800' height='600'>");

    // --- Compute circular layout positions ---
    for (int i = 0; i < g->n; i++)
    {
        double angle = i * angleStep;
        nodeX[i] = centerX + (int)(radius * cos(angle));
        nodeY[i] = centerY + (int)(radius * sin(angle));
    }

    // --- Draw edges first (so they appear behind nodes) ---
    for (int i = 0; i < g->n; i++)
    {
        Node *temp = g->list[i];
        while (temp)
        {
            int j = temp->dest;
            int midX = (nodeX[i] + nodeX[j]) / 2;
            int midY = (nodeY[i] + nodeY[j]) / 2;

            // Draw line for connection
            printf("<line x1='%d' y1='%d' x2='%d' y2='%d' stroke='#7f8c8d' stroke-width='2' marker-end='url(#arrow)' />",
                   nodeX[i], nodeY[i], nodeX[j], nodeY[j]);

            // Show distance label
            printf("<text x='%d' y='%d' font-size='11' fill='#2c3e50' text-anchor='middle' dominant-baseline='middle'>%d km</text>",
                   midX, midY, temp->distance);

            temp = temp->next;
        }
    }

    // --- Define arrow marker for edges ---
    printf("<defs>"
           "<marker id='arrow' markerWidth='10' markerHeight='10' refX='6' refY='3' orient='auto' markerUnits='strokeWidth'>"
           "<path d='M0,0 L0,6 L9,3 z' fill='#7f8c8d' />"
           "</marker>"
           "</defs>");

    // --- Draw nodes on top of edges ---
    for (int i = 0; i < g->n; i++)
    {
        printf("<circle cx='%d' cy='%d' r='25' fill='#3498db' stroke='#2c3e50' stroke-width='2'>", nodeX[i], nodeY[i]);
        printf("<title>%s</title>", g->placeNames[i]); // tooltip
        printf("</circle>");

        // Node label
        printf("<text x='%d' y='%d' font-family='Arial' font-size='13' fill='black' text-anchor='middle' dominant-baseline='middle'>%s</text>",
               nodeX[i], nodeY[i], g->placeNames[i]);
    }

    printf("</svg>");
}
