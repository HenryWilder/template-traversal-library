#include <iostream>
#include <raylib.hpp>
#include <graph-traversal.hpp>

struct Node
{
    Vector2 position;
    static constexpr float radius = 5.0f;
};

struct Wire
{
    Color color;
};

using graph = trav::graph<Node, Wire>;
using vert = graph::vert;
using edge = graph::edge;
using step_vector = graph::step_vector;

static bool IsNodeOverlapping(const Node &n, Vector2 position)
{
    return CheckCollisionPointCircle(position, n.position, Node::radius);
}

static bool IsVertInputless(const vert &v)
{
    return v.prev_count( ) == 0;
}

int main()
{
    constexpr Color colorOptions[ ] = {
        ORANGE,
        GREEN,
        YELLOW,
        BLUE,
        RED,
        VIOLET,
    };
    size_t colorIndex = 0;
    auto currentColor = [&colorOptions, &colorIndex]( )
    {
        Color color = colorOptions[colorIndex];
        colorIndex = (colorIndex + 1) % _countof(colorOptions);
        return color;
    };

    InitWindow(1280, 720, "Test");
    SetTargetFPS(30);

    graph g;

    bool isDirty = true;
    vert *activeVert = nullptr;
    vert *hoveredVert = nullptr;

    while (!WindowShouldClose())
    {
        Vector2 mousePos = GetMousePosition( );

        hoveredVert = g.find(IsNodeOverlapping, mousePos);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (hoveredVert) // add an edge
            {
                if (activeVert && activeVert != hoveredVert)
                {
                    g.link(*activeVert, *hoveredVert, { currentColor( ) });
                    isDirty = true;
                }
                activeVert = hoveredVert;
            }
            else // add a vertex
            {
                g.push({ mousePos });
                isDirty = true;
            }
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            if (!hoveredVert || activeVert == hoveredVert)
            {
                activeVert = nullptr;
            }

            if (hoveredVert)
            {
                g.erase(*hoveredVert);
                isDirty = true;
                hoveredVert = nullptr;
            }
        }

        BeginDrawing( );
        {
            ClearBackground(BLACK);

            for (edge &e : g.all_edges( ))
            {
                Wire &w = e;
                Node &prev = e.prev( );
                Node &next = e.next( );
                DrawLineV(prev.position, next.position, hoveredVert ? GRAY : w.color);
            }

            for (vert &v : g.all_verts( ))
            {
                Node &n = v;
                DrawCircleV(n.position, Node::radius, &v == activeVert ? BLUE : GRAY);
            }

            if (hoveredVert)
            {
                auto inputs = g.walk_dfs_r(*hoveredVert);
                auto outputs = g.walk_dfs(*hoveredVert);
                decltype(inputs) io[2] = { inputs, outputs };
                constexpr Color ioCol[2] = { PURPLE, ORANGE };

                for (size_t i = 0; i < 2; ++i)
                {
                    Color color = ioCol[i];
                    for (auto &[e, v] : io[i])
                    {
                        if (e)
                        {
                            Wire &w = *e;
                            Node &prev = e->prev( );
                            Node &next = e->next( );
                            DrawLineEx(prev.position, next.position, 2.0f, color);
                        }

                        if (v)
                        {
                            Node &n = *v;
                            DrawCircleV(n.position, Node::radius, color);
                        }
                    }
                }
            }
        }
        EndDrawing( );
    }

    CloseWindow( );
}
