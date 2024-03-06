#pragma once
#include <type_traits>
#include <cassert>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>
#include <functional>
#include <tuple>
#include <optional>

// graph traversal
namespace trav
{
    template<class _VTy, class _ETy> class graph;

    template<class _VTy>
    class vert
    {
    public:
        template<class _VTy, class _ETy> friend class graph;

        vert(_In_ size_t index, _In_ const _VTy &data)
            : index(index), data(data)
        { }

        size_t GetIndex( ) const
        {
            return index;
        }

        operator _VTy &( )
        {
            return data;
        }
        operator const _VTy &( ) const
        {
            return data;
        }

    private:
        size_t index;
        std::set<size_t> adj;
        _VTy data;
    };

    template<class _ETy>
    class edge
    {
    public:
        template<class _VTy, class _ETy> friend class graph;

        edge(_In_ size_t index, _In_ size_t prev, _In_ size_t next, _In_ const _ETy &data)
            : _index(index), _prev(prev), _next(next), _data(data)
        { }

        size_t index( ) const
        {
            return _index;
        }
        size_t prev( ) const
        {
            return _prev;
        }
        size_t next( ) const
        {
            return _next;
        }

        operator _ETy &( )
        {
            return _data;
        }
        operator const _ETy &( ) const
        {
            return _data;
        }

    private:
        size_t _index, _prev, _next;
        _ETy _data;
    };

    enum traversal
    {
        BREADTH_FIRST,
        DEPTH_FIRST,
    };

    enum direction
    {
        BACKWARD,
        FORWARD,
    };

    template<class _VTy, class _ETy>
    class graph
    {
    public:
        using vert = vert<_VTy>;
        using edge = edge<_ETy>;

        using vert_process = std::function<void(vert &)>;
        using edge_process = std::function<void(edge &)>;

        static void ignore(vert &) { }
        static void ignore(edge &) { }

        ~graph( )
        {
            for (vert *v : verts) delete v;
            for (edge *e : edges) delete e;
        }

        void push(const _VTy &value)
        {
            verts.push_back(new vert(verts.size( ), value));
        }

        vert &at(_In_range_(0, verts.size( ) - 1) size_t index)
        {
            return *verts[index];
        }
        const vert &at(_In_range_(0, verts.size( ) - 1) size_t index) const
        {
            return *verts[index];
        }

        void link(_In_ size_t prev, _In_ size_t next, _In_ const _ETy &value)
        {
            assert(prev != next);

            size_t newEdgeIndex = edges.size( );
            edges.push_back(new edge(newEdgeIndex, prev, next, value));
            for (size_t vert : { prev, next })
            {
                verts.at(vert)->adj.insert(newEdgeIndex);
            }
        }

        edge &link_at(_In_range_(0, edges.size( ) - 1) size_t index)
        {
            return *edges[index];
        }
        const edge &link_at(_In_range_(0, edges.size( ) - 1) size_t index) const
        {
            return *edges[index];
        }

        template<traversal kind, direction dir>
        class walker
        {
        public:
            walker(_In_ graph &g, _In_ size_t rootIndex)
                : g(g), rootIndex(rootIndex)
            {
                std::queue<size_t> q;
                std::unordered_set<size_t> visited;

                vert *root = g.verts[rootIndex];
                visited.insert(rootIndex);
                q.push(rootIndex);
                _push_to_walk(nullptr, root);

                while (!q.empty())
                {
                    const size_t vIndex = q.front( );
                    const vert *v = g.verts[vIndex];
                    q.pop( );

                    if constexpr (kind == traversal::BREADTH_FIRST)
                    {
                        for (const size_t eIndex : v->adj)
                        {
                            edge *e = g.edges[eIndex];
                            bool isForward = e->prev( ) == vIndex;
                            const size_t wIndex = isForward ? e->next( ) : e->prev( );
                            vert *w = g.verts[wIndex];

                            if (isForward != (dir == direction::FORWARD)) continue;

                            if (!visited.contains(wIndex))
                            {
                                visited.insert(wIndex);
                                q.push(wIndex);
                                _push_to_walk(e, w);
                            }
                        }
                    }
                }
            }

            auto begin( ) const
            {
                return _walk.begin( );
            }

            auto end( ) const
            {
                return _walk.end( );
            }

        private:
            void _push_to_walk(_In_opt_ edge *e, _In_ vert *v)
            {
                _walk.push_back(std::tuple<edge *, vert &>{ e, *v });
            }

            graph &g;
            size_t rootIndex;
            std::vector<std::tuple<edge *, vert &>> _walk;
        };

        template<traversal kind, direction dir = FORWARD>
        walker<kind, dir> walk(_In_ std::initializer_list<size_t> roots)
        {
            return walker<kind, dir>(*this, roots);
        }

        template<traversal kind, direction dir = FORWARD>
        walker<kind, dir> walk(_In_ size_t root)
        {
            return walker<kind, dir>(*this, { root });
        }

    protected:
        std::vector<vert *> verts;
        std::vector<edge *> edges;
    };
}
