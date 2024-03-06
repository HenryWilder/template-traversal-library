#pragma once
#include <type_traits>
#include <cassert>
#include <unordered_set>
#include <queue>
#include <stack>
#include <vector>
#include <tuple>

// graph traversal
namespace trav
{
    template<class _Ty>
    concept non_void = !std::is_void_v<_Ty>;

    template<class T, class... TN>
    concept one_type_of = std::disjunction_v<std::is_same<T, TN>...>;

    template<class _VTy, class _ETy = void> class vert;
    template<class _VTy, class _ETy = void> class edge;

    template<class _VTy, class _ETy>
    class vert
    {
    public:
        using edge = edge<_VTy, _ETy>;

        vert(
            _In_ const _VTy &data
            ) :
            _data(data)
        { }

              std::vector<edge *> &prev( )       { return _prev; }
        const std::vector<edge *> &prev( ) const { return _prev; }
              std::vector<edge *> &next( )       { return _next; }
        const std::vector<edge *> &next( ) const { return _next; }

        operator       _VTy &( )       { return _data; }
        operator const _VTy &( ) const { return _data; }

    private:
        std::vector<edge *> _prev, _next;
        _VTy _data;
    };

    namespace
    {
        template<class _VTy, class _ETy = void>
        class _edge_base
        {
        public:
            using vert = vert<_VTy, _ETy>;

            _edge_base(
                _In_ vert *prev,
                _In_ vert *next
                ) :
                _prev(prev),
                _next(next)
            { }

            vert *prev( ) const { return _prev; }
            vert *next( ) const { return _next; }

        private:
            vert *_prev, *_next;
        };
    }

    template<class _VTy>
    class edge<_VTy, void>
        : public _edge_base<_VTy, void>
    {
        using base = _edge_base<_VTy, void>;
    public:
        using vert = base::vert;

        edge(
            _In_ vert *prev,
            _In_ vert *next
            ) :
            base(prev, next)
        { }
    };

    template<class _VTy, non_void _ETy>
    class edge<_VTy, _ETy>
        : public _edge_base<_VTy, _ETy>
    {
        using base = _edge_base<_VTy, _ETy>;
    public:
        using vert = base::vert;

        edge(
            _In_ vert *prev,
            _In_ vert *next,
            _In_ _ETy data
            ) :
            base(prev, next),
            _data(data)
        { }

        operator       _ETy &( )       { return _data; }
        operator const _ETy &( ) const { return _data; }

    private:
        _ETy _data;
    };

    namespace
    {
        template<class _VTy, class _ETy = void>
        class _graph_base
        {
        public:
            using vert = vert<_VTy, _ETy>;
            using edge = edge<_VTy, _ETy>;

            ~_graph_base( )
            {
                for (vert *v : verts) delete v;
                for (edge *e : edges) delete e;
            }

            void push(
                _In_ const _VTy &value
            )
            {
                verts.push_back(new vert(value));
            }

            vert &at(
                _In_range_(0, vert_count()) size_t index
            )
            {
                return *verts[index];
            }

            const vert &at(
                _In_range_(0, vert_count()) size_t index
            ) const
            {
                return *verts[index];
            }

        protected:
            void _link(
                _In_ vert *prev,
                _In_ vert *next,
                _In_ edge *e
            )
            {
                assert(prev != next);

                edges.push_back(e);
                prev->next( ).push_back(e);
                next->prev( ).push_back(e);
            }

        public:
            edge &edge_at(
                _In_range_(0, edge_count()) size_t index
            )
            {
                return *edges[index];
            }

            const edge &edge_at(
                _In_range_(0, edge_count()) size_t index
            ) const
            {
                return *edges[index];
            }

            _Ret_maybenull_ edge *edge_connecting(
                _In_ const vert *from_vert,
                _In_ const vert *  to_vert
            )
            {
                auto &from_vert_edges = from_vert->next( );
                auto &  to_vert_edges =   to_vert->prev( );

                if (from_vert_edges.size( ) < to_vert_edges.size( ))
                {
                    // Look for 'to' in 'from'
                    for (edge *e : from_vert_edges)
                    {
                        if (e->next( ) == to_vert) return e;
                    }
                }
                else
                {
                    // Look for 'from' in 'to'
                    for (edge *e : to_vert_edges)
                    {
                        if (e->prev( ) == from_vert) return e;
                    }
                }

                return nullptr;
            }

            _Ret_maybenull_ edge *edge_connecting(
                _In_ size_t from_vert_index,
                _In_ size_t   to_vert_index
            )
            {
                return edge_connecting(verts[from_vert_index], verts[to_vert_index]);
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

            class _step_forward
            {
            public:
                static const std::vector<edge *> &step(_In_ const vert *v) { return v->next( ); }
                static                   vert *   step(_In_ const edge *e) { return e->next( ); }
            };

            class _step_backward
            {
            public:
                static const std::vector<edge *> &step(_In_ const vert *v) { return v->prev( ); }
                static                   vert *   step(_In_ const edge *e) { return e->prev( ); }
            };

            // see https://en.wikipedia.org/wiki/Breadth-first_search
            template<one_type_of<_step_forward, _step_backward> _stepper>
            class _walk_bfs
                : public _walk, public _stepper
            {
            private:
                void handle_root(
                    _In_ std::queue<vert *> &q,
                    _In_ std::unordered_set<vert *> &visited,
                    _In_ vert *root)
                {
                    // 3  label root as explored
                    visited.insert(root);

                    // 4  Q.enqueue(root)
                    q.push(root);
                    this->_push_step(nullptr, root);
                }

                void handle_walk(
                    _In_ std::queue<vert *> &q,
                    _In_ std::unordered_set<vert *> &visited)
                {
                    // 5  while Q is not empty do
                    while (!q.empty( ))
                    {
                        // 6  v := Q.dequeue()
                        const vert *const v = q.front( ); q.pop( );

                        // 9  for all edges from v to w in G.adjacentEdges(v) do
                        for (edge *const e : _stepper::step(v))
                        {
                            vert *const w = _stepper::step(e);

                            // 10  if w is not labeled as explored then
                            if (!visited.contains(w))
                            {
                                // 11  label w as explored
                                visited.insert(w);

                                // 13  Q.enqueue(w)
                                q.push(w);
                                this->_push_step(e, w);
                            }
                        }
                    }
                }

            public:
                // 1  procedure BFS(G, root) is
                _walk_bfs(
                    _In_ const _graph_base &g,
                    _In_ std::vector<vert *> roots
                )
                {
                    // 2  let Q be a queue
                    std::queue<vert *> q;
                    std::unordered_set<vert *> visited;

                    // All roots at breadth 0
                    for (vert *root : roots)
                    {
                        handle_root(q, visited, root);
                    }

                    handle_walk(q, visited);
                }

                // 1  procedure BFS(G, root) is
                _walk_bfs(
                    _In_ const _graph_base &g,
                    _In_ vert *root
                )
                {
                    // 2  let Q be a queue
                    std::queue<vert *> q;
                    std::unordered_set<vert *> visited;

                    handle_root(q, visited, root);
                    handle_walk(q, visited);
                }
            };

            // see https://en.wikipedia.org/wiki/Depth-first_search
            template<one_type_of<_step_forward, _step_backward> _stepper>
            class _walk_dfs
                : public _walk, public _stepper
            {
            private:
                // 1  procedure DFS_iterative(G, v) is
                void _walk_util(
                    _In_ const _graph_base &g,
                    _Inout_ std::unordered_set<vert *> &visited,
                    _In_opt_ edge *eTaken,
                    _In_ vert *v)
                {
                    // 2  label v as discovered
                    visited.insert(v);
                    this->_push_step(eTaken, v);

                    // 3  for all directed edges from v to w that are in G.adjacentEdges(v) do
                    for (edge *const e : _stepper::step(v))
                    {
                        vert *const w = _stepper::step(e);

                        // 4  if vertex w is not labeled as discovered then
                        if (!visited.contains(w))
                        {
                            // 5  recursively call DFS(G, w)
                            _walk_util(g, visited, e, w);
                        }
                    }

                }

            public:
                _walk_dfs(
                    _In_ const _graph_base &g,
                    _In_ std::vector<vert *> roots
                )
                {
                    std::unordered_set<vert *> visited;

                    for (vert *const root : roots)
                    {
                        _walk_util(g, visited, nullptr, root);
                    }
                }

                _walk_dfs(
                    _In_ const _graph_base &g,
                    _In_ vert *root
                )
                {
                    std::unordered_set<vert *> visited;

                    _walk_util(g, visited, nullptr, root);
                }
            };

            using _walk_bfs_f = _walk_bfs<_step_forward >;
            using _walk_bfs_r = _walk_bfs<_step_backward>;
            using _walk_dfs_f = _walk_dfs<_step_forward >;
            using _walk_dfs_r = _walk_dfs<_step_backward>;

        public:
            _walk_bfs_f walk_bfs  (_In_ std::vector<vert *> roots) { return _walk_bfs_f(*this, roots); }
            _walk_bfs_r walk_bfs_r(_In_ std::vector<vert *> roots) { return _walk_bfs_r(*this, roots); }
            _walk_dfs_f walk_dfs  (_In_ std::vector<vert *> roots) { return _walk_dfs_f(*this, roots); }
            _walk_dfs_r walk_dfs_r(_In_ std::vector<vert *> roots) { return _walk_dfs_r(*this, roots); }
    
            _walk_bfs_f walk_bfs  (_In_ vert *root) { return _walk_bfs_f(*this, root); }
            _walk_bfs_r walk_bfs_r(_In_ vert *root) { return _walk_bfs_r(*this, root); }
            _walk_dfs_f walk_dfs  (_In_ vert *root) { return _walk_dfs_f(*this, root); }
            _walk_dfs_r walk_dfs_r(_In_ vert *root) { return _walk_dfs_r(*this, root); }
    
            _walk_bfs_f walk_bfs  (_In_ vert &root) { return walk_bfs  (&root); }
            _walk_bfs_r walk_bfs_r(_In_ vert &root) { return walk_bfs_r(&root); }
            _walk_dfs_f walk_dfs  (_In_ vert &root) { return walk_dfs  (&root); }
            _walk_dfs_r walk_dfs_r(_In_ vert &root) { return walk_dfs_r(&root); }
    
            template<std::integral T> _walk_bfs_f walk_bfs  (_In_range_(0, vert_count()) T root_index) { return walk_bfs  (at(root_index)); }
            template<std::integral T> _walk_bfs_r walk_bfs_r(_In_range_(0, vert_count()) T root_index) { return walk_bfs_r(at(root_index)); }
            template<std::integral T> _walk_dfs_f walk_dfs  (_In_range_(0, vert_count()) T root_index) { return walk_dfs  (at(root_index)); }
            template<std::integral T> _walk_dfs_r walk_dfs_r(_In_range_(0, vert_count()) T root_index) { return walk_dfs_r(at(root_index)); }

            size_t vert_count() const { return verts.size(); }
            size_t edge_count() const { return edges.size(); }

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
        using base = _graph_base<_VTy, void>;
    public:
        using vert = base::vert;
        using edge = base::edge;

        void link(
            _In_ vert *prev,
            _In_ vert *next
        )
        {
            this->_link(prev, next, new edge(prev, next));
        }

        void link(
            _In_ vert &prev,
            _In_ vert &next
        )
        {
            link(&prev, &next);
        }

        template<std::integral T>
        void link(
            _In_range_(0, this->vert_count()) T prev,
            _In_range_(0, this->vert_count()) T next
        )
        {
            assert(prev < this->vert_count( ));
            assert(next < this->vert_count( ));
            link(this->verts[prev], this->verts[next]);
        }
    };

    template<class _VTy, non_void _ETy>
    class graph<_VTy, _ETy>
        : public _graph_base<_VTy, _ETy>
    {
        using base = _graph_base<_VTy, void>;
    public:
        using vert = base::vert;
        using edge = base::edge;

        void link(
            _In_ vert *prev,
            _In_ vert *next,
            _In_ const _ETy &value
        )
        {
            this->_link(prev, next, new edge(prev, next, value));
        }

        void link(
            _In_ vert &prev,
            _In_ vert &next,
            _In_ const _ETy &value
        )
        {
            link(&prev, &next, value);
        }

        template<std::integral T>
        void link(
            _In_range_(0, this->vert_count()) T prev,
            _In_range_(0, this->vert_count()) T next,
            _In_ const _ETy &value
        )
        {
            assert(prev < this->vert_count( ));
            assert(next < this->vert_count( ));
            link(this->verts[prev], this->verts[next], value);
        }
    };
}
