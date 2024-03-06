#pragma once
#include <type_traits>
#include <cassert>
#include <queue>
#include <stack>
#include <vector>
#include <functional>
#include <tuple>
#include <optional>

#define _In_vector_index_(vec) _In_range_(0, vec.size( ) - 1)

// graph traversal
namespace trav
{
    template<class _Ty>
    concept non_void = !std::is_void_v<_Ty>;

    template<class _VTy>
    class vert
    {
    public:
        vert(
            _In_ size_t index,
            _In_ const _VTy &data
            ) :
            _index(index),
            _data(data)
        { }

        size_t index( ) const { return _index; }

        using edge_container = std::vector<size_t>;

              edge_container &prev( )       { return _prev; }
        const edge_container &prev( ) const { return _prev; }
              edge_container &next( )       { return _next; }
        const edge_container &next( ) const { return _next; }

        operator       _VTy &( )       { return _data; }
        operator const _VTy &( ) const { return _data; }

    private:
        size_t _index; // verts
        edge_container _prev, _next; // edges
        _VTy _data;
    };

    template<class _ETy = void> class edge;

    template<>
    class edge<void>
    {
    public:
        edge(
            _In_ size_t index,
            _In_ size_t prev,
            _In_ size_t next
            ) :
            _index(index),
            _prev(prev),
            _next(next)
        { }

        size_t index( ) const { return _index; }
        size_t prev ( ) const { return _prev;  }
        size_t next ( ) const { return _next;  }

    private:
        size_t _index; // edges
        size_t _prev, _next; // verts
    };

    template<non_void _ETy>
    class edge<_ETy>
        : public edge<void>
    {
    public:
        edge(
            _In_ size_t index,
            _In_ size_t prev,
            _In_ size_t next,
            _In_ const _ETy &data
            ) :
            edge<void>(index, prev, next),
            _data(data)
        { }

        operator       _ETy &( )       { return _data; }
        operator const _ETy &( ) const { return _data; }

    private:
        _ETy _data;
    };

    enum class traversal { BREADTH_FIRST, DEPTH_FIRST, };

    enum class direction { BACKWARD, FORWARD, };

    namespace
    {
        template<class _VTy, class _ETy = void>
        class _graph_base
        {
        public:
            using vert = vert<_VTy>;
            using edge = edge<_ETy>;

            ~_graph_base( )
            {
                for (vert *v : verts) delete v;
                for (edge *e : edges) delete e;
            }

            void push(
                _In_ const _VTy &value
            )
            {
                verts.push_back(new vert(verts.size( ), value));
            }

            vert &at(
                _In_vector_index_(verts) size_t index
            )
            {
                return *verts[index];
            }

            const vert &at(
                _In_vector_index_(verts) size_t index
            ) const
            {
                return *verts[index];
            }

        protected:
            void _link(
                _In_ size_t prev,
                _In_ size_t next,
                _In_ edge *_edge
            )
            {
                assert(prev < this->verts.size( ));
                assert(next < this->verts.size( ));
                assert(prev != next);

                size_t newEdgeIndex = edges.size( );
                verts[prev]->next( ).push_back(newEdgeIndex);
                verts[next]->prev( ).push_back(newEdgeIndex);
                edges.push_back(_edge);
            }

        public:
            edge &edge_at(
                _In_vector_index_(edges) size_t index
            )
            {
                return *edges[index];
            }

            const edge &edge_at(
                _In_vector_index_(edges) size_t index
            ) const
            {
                return *edges[index];
            }

            _Ret_maybenull_ edge *edge_connecting(
                _In_vector_index_(verts) size_t from_vert,
                _In_vector_index_(verts) size_t   to_vert
            )
            {
                auto &from_vert_edges = verts[from_vert]->next( );
                auto &to_vert_edges = verts[to_vert]->prev( );

                if (from_vert_edges.size( ) < to_vert_edges.size( ))
                {
                    // Look for 'to' in 'from'
                    for (size_t edgeIndex : from_vert_edges)
                    {
                        edge *e = edges[edgeIndex];
                        if (e->next( ) == to_vert) return e;
                    }
                }
                else
                {
                    // Look for 'from' in 'to'
                    for (size_t edgeIndex : to_vert_edges)
                    {
                        edge *e = edges[edgeIndex];
                        if (e->prev( ) == from_vert) return e;
                    }
                }

                return nullptr;
            }

        private:
            class _walk
            {
            public:
                auto begin( ) const { return steps.begin( ); }
                auto end  ( ) const { return steps.end  ( ); }

            protected:
                void _push_step(
                    _In_opt_ edge *e,
                    _In_     vert *v
                )
                {
                    steps.push_back({ e, *v });
                }

            private:
                std::vector<std::tuple<edge *, vert &>> steps;
            };

        public:
            template<traversal kind, direction dir> class walk;

            // see https://en.wikipedia.org/wiki/Breadth-first_search
            // (skips 7 and 8 because there is no goal)
            // (skips 12 because parents are already known)
            //  1  procedure BFS(G, root) is
            //  2      let Q be a queue
            //  3      label root as explored
            //  4      Q.enqueue(root)
            //  5      while Q is not empty do
            //  6          v := Q.dequeue()
            //  9          for all edges from v to w in G.adjacentEdges(v) do
            // 10              if w is not labeled as explored then
            // 11                  label w as explored
            // 13                  Q.enqueue(w)
            template<direction dir>
            class walk<traversal::BREADTH_FIRST, dir>
                : public _walk
            {
                static constexpr bool is_traversal_forward = dir == direction::FORWARD;

            public:
                // 1  procedure BFS(G, root) is
                walk(
                    _In_ _graph_base &g,
                    _In_ std::initializer_list<size_t> rootIndices
                )
                {
                    // 2  let Q be a queue
                    std::queue<size_t> q;
                    std::vector<bool> visited(g.verts.size( ), false);

                    // All roots at breadth 0
                    for (size_t rootIndex : rootIndices)
                    {
                        // 3  label root as explored
                        visited[rootIndex] = true;

                        // 4  Q.enqueue(root)
                        q.push(rootIndex);
                        this->_push_step(nullptr, g.verts[rootIndex]);
                    }

                    // 5  while Q is not empty do
                    while (!q.empty( ))
                    {
                        // 6  v := Q.dequeue()
                        const size_t vIndex = q.front( );
                        const vert *const v = g.verts[vIndex];
                        q.pop( );

                        // 9  for all edges from v to w in G.adjacentEdges(v) do
                        for (const size_t eIndex : (is_traversal_forward ? v->next( ) : v->prev( )))
                        {
                            edge *const e = g.edges[eIndex];
                            const size_t wIndex = (is_traversal_forward ? e->next( ) : e->prev( ));

                            // 10  if w is not labeled as explored then
                            if (!visited[wIndex])
                            {
                                // 11  label w as explored
                                visited[wIndex] = true;

                                // 13  Q.enqueue(w)
                                q.push(wIndex);
                                this->_push_step(e, g.verts[wIndex]);
                            }
                        }
                    }
                }
            };

            // see https://en.wikipedia.org/wiki/Depth-first_search
            // 1  procedure DFS(G, v) is
            // 2  label v as discovered
            // 3      for all directed edges from v to w that are in G.adjacentEdges(v) do
            // 4          if vertex w is not labeled as discovered then
            // 5              recursively call DFS(G, w)
            template<direction dir>
            class walk<traversal::DEPTH_FIRST, dir>
                : public _walk
            {
                static constexpr bool is_traversal_forward = dir == direction::FORWARD;

            private:
                // 1  procedure DFS_iterative(G, v) is
                void _walk_util(
                    _In_ _graph_base &g,
                    _Inout_ std::vector<bool> &visited,
                    _In_opt_ edge *eTaken,
                    _In_ size_t vIndex)
                {
                    vert *const v = g.verts[vIndex];

                    // 2  label v as discovered
                    visited[vIndex] = true;
                    this->_push_step(eTaken, g.verts[vIndex]);

                    // 3  for all directed edges from v to w that are in G.adjacentEdges(v) do
                    for (const size_t eIndex : (is_traversal_forward ? v->next( ) : v->prev( )))
                    {
                        edge *const e = g.edges[eIndex];
                        const size_t wIndex = (is_traversal_forward ? e->next( ) : e->prev( ));

                        // 4  if vertex w is not labeled as discovered then
                        if (!visited[wIndex])
                        {
                            // 5  recursively call DFS(G, w)
                            _walk_util(g, visited, e, wIndex);
                        }
                    }

                }

            public:
                walk(
                    _In_ _graph_base &g,
                    _In_ std::initializer_list<size_t> rootIndices
                )
                {
                    std::vector<bool> visited(g.verts.size( ), false);

                    for (const size_t rootIndex : rootIndices)
                    {
                        _walk_util(g, visited, nullptr, rootIndex);
                    }
                }
            };

            auto walk_bfs  (_In_ std::initializer_list<size_t> roots) { return walk<traversal::BREADTH_FIRST, direction:: FORWARD>(*this, roots); }
            auto walk_bfs_r(_In_ std::initializer_list<size_t> roots) { return walk<traversal::BREADTH_FIRST, direction::BACKWARD>(*this, roots); }
            auto walk_dfs  (_In_ std::initializer_list<size_t> roots) { return walk<traversal::  DEPTH_FIRST, direction:: FORWARD>(*this, roots); }
            auto walk_dfs_r(_In_ std::initializer_list<size_t> roots) { return walk<traversal::  DEPTH_FIRST, direction::BACKWARD>(*this, roots); }
    
            auto walk_bfs  (_In_vector_index_(verts) size_t root) { return walk_bfs  ({ root }); }
            auto walk_bfs_r(_In_vector_index_(verts) size_t root) { return walk_bfs_r({ root }); }
            auto walk_dfs  (_In_vector_index_(verts) size_t root) { return walk_dfs  ({ root }); }
            auto walk_dfs_r(_In_vector_index_(verts) size_t root) { return walk_dfs_r({ root }); }

        protected:
            std::vector<vert *> verts;
            std::vector<edge *> edges;
        };
    }

    template<class _VTy, class _ETy = void> class graph;

    template<class _VTy>
    class graph<_VTy, void>
        : public _graph_base<_VTy, void>
    {
    public:
        void link(
            _In_ size_t prev,
            _In_ size_t next
        )
        {
            this->_link(prev, next, new edge<void>(this->edges.size( ), prev, next));
        }
    };

    template<class _VTy, non_void _ETy>
    class graph<_VTy, _ETy>
        : public _graph_base<_VTy, _ETy>
    {
    public:
        void link(
            _In_ size_t prev,
            _In_ size_t next,
            _In_ const _ETy &value
        )
        {
            this->_link(prev, next, new edge<_ETy>(this->edges.size(), prev, next, value));
        }
    };
}

#undef _In_vector_index_
