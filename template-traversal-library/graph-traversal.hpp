#pragma once
#include <type_traits>
#include <cassert>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>
#include <functional>
#include <tuple>

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

        _VTy &operator*( )
        {
            return data;
        }
        const _VTy &operator*( ) const
        {
            return data;
        }

        _VTy *operator->( )
        {
            return &data;
        }
        const _VTy *operator->( ) const
        {
            return &data;
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

        _ETy &operator*( )
        {
            return _data;
        }
        const _ETy &operator*( ) const
        {
            return _data;
        }

        _ETy *operator->( )
        {
            return &_data;
        }
        const _ETy *operator->( ) const
        {
            return &_data;
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
            friend class walk_iterator;

            class walk_iterator
            {
            public:
                friend class walker;

                bool operator!=(const walk_iterator &other)
                {
                    return &_walker != &other._walker || q.empty( ) != other.q.empty( );
                }

                walk_iterator &operator++( )
                {
                    graph &g = _walker.g;

                    const size_t vIndex = q.front( );
                    const vert *v = g.verts[vIndex];
                    q.pop( );

                    if constexpr (kind == traversal::BREADTH_FIRST)
                    {
                        for (const size_t eIndex : v->adj)
                        {
                            edge *e = g.edges[eIndex];
                            bool isForward = e->prev() == vIndex;
                            const size_t wIndex = isForward ? e->next() : e->prev();
                            vert *w = g.verts[wIndex];

                            if (isForward == (dir == direction::FORWARD) && !visited.contains(wIndex))
                            {
                                visited.insert(wIndex);
                                q.push(wIndex);
                                _vert = w;
                                _edge = e;
                            }
                        }
                    }
                    return *this;
                }

                std::tuple<vert *, edge *> operator*( ) const
                {
                    return { _vert, _edge };
                }

            private:
                walk_iterator(
                    walker &_walker,
                    vert *_vert,
                    size_t root)
                    : _walker(_walker), _vert(_vert), _edge(nullptr), q(), visited(root)
                {
                    q.push(root); // w h y
                }

                walk_iterator(walker &_walker)
                    : _walker(_walker), _vert(nullptr), _edge(nullptr), q(), visited()
                { }

                walker &_walker;
                vert *_vert;
                edge *_edge;
                std::queue<size_t> q;
                std::unordered_set<size_t> visited;
            };

            walk_iterator begin( )
            {
                walk_iterator it(*this, g.verts[rootIndex], rootIndex);
                return it;
            }

            walk_iterator end( )
            {
                walk_iterator it(*this);
                return it;
            }

            walker(_In_ graph &g, _In_ size_t rootIndex)
                : g(g), rootIndex(rootIndex)
            { }

        private:
            graph &g;
            size_t rootIndex;
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
