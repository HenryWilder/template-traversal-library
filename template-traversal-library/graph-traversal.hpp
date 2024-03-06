#pragma once
#include <type_traits>
#include <cassert>
#include <queue>
#include <vector>
#include <functional>
#include <tuple>
#include <optional>

#define _In_vector_index_(vec) _In_range_(0, vec.size( ) - 1)

// graph traversal
namespace trav
{
    template<class _Ty> concept non_void = !std::is_void_v<_Ty>;

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

            template<traversal kind, direction dir>
            class walk
            {
            public:
                walk(
                    _In_ _graph_base &g,
                    _In_ std::initializer_list<size_t> rootIndices
                )
                    : g(g)
                {
                    std::queue<size_t> q;
                    std::vector<bool> visited(g.verts.size( ), false);

                    auto _visit = [&](
                        _In_vector_index_(g.verts) size_t vIndex,
                        _In_opt_ edge *e
                        )
                    {
                        visited[vIndex] = true;
                        q.push(vIndex);
                        _push_to_walk(e, g.verts[vIndex]);
                    };

                    for (size_t rootIndex : rootIndices)
                    {
                        _visit(rootIndex, nullptr);
                    }

                    constexpr bool is_traversal_forward = dir == direction::FORWARD;

                    while (!q.empty( ))
                    {
                        const size_t vIndex = q.front( );
                        const vert *v = g.verts[vIndex];
                        q.pop( );

                        if constexpr (kind == traversal::BREADTH_FIRST)
                        {
                            for (const size_t eIndex : (is_traversal_forward ? v->next( ) : v->prev( )))
                            {
                                edge *e = g.edges[eIndex];
                                const size_t wIndex = (is_traversal_forward ? e->next( ) : e->prev( ));

                                if (!visited[wIndex])
                                {
                                    _visit(wIndex, e);
                                }
                            }
                        }
                    }
                }

                auto begin( ) const { return _walk.begin( ); }
                auto end  ( ) const { return _walk.end  ( ); }

            private:
                void _push_to_walk(
                    _In_opt_ edge *e,
                    _In_     vert *v
                )
                {
                    _walk.push_back({ e, *v });
                }

                _graph_base &g;
                std::vector<std::tuple<edge *, vert &>> _walk;
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
