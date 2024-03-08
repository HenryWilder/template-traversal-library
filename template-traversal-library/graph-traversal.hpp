#pragma once
#pragma warning( push )
#pragma warning( disable: 4365 )
#include <type_traits>
#include <cassert>
#include <unordered_set>
#include <queue>
#include <stack>
#include <vector>
#include <tuple>
#include <thread>
#pragma warning( pop )

// graph traversal
namespace trav
{
    template<class _Ty>
    concept non_void = !std::is_void_v<_Ty>;

    template<class T, class... TN>
    concept one_type_of = std::disjunction_v<std::is_same<T, TN>...>;

    // Assumes that every element is valid
    template<class T>
    struct deref_interface
    {
        deref_interface(const deref_interface &other)
            : data(other.data)
        { }

        deref_interface(const std::vector<T *> &data)
            : data(data)
        { }

        struct deref_iter;

        struct const_deref_iter
        {
            using const_iterator = typename std::vector<T *>::const_iterator;

            const_deref_iter(const_iterator it)
                : it(it)
            { }

                  T &operator*( )       { return **it; }
            const T &operator*( ) const { return **it; }

            bool operator!=(const const_deref_iter &other)
            {
                return it != other.it;
            }

            const_deref_iter &operator++()
            {
                ++it;
                return *this;
            }

            const_iterator it;
        };

        const_deref_iter begin( ) const { return const_deref_iter(data.begin( )); }
        const_deref_iter end  ( ) const { return const_deref_iter(data.end  ( )); }

        const std::vector<T *> &data;
    };

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

        size_t prev_count() const { return _prev.size(); }
        size_t next_count() const { return _next.size(); }

        bool bypassable( ) const
        {
            size_t numPrev = _prev.size( ), numNext = _next.size( );
            return numPrev == 1ull || numNext == 1ull || numPrev == numNext;
        }

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
                _In_ vert &prev,
                _In_ vert &next
                ) :
                _prev(prev),
                _next(next)
            { }

            vert &prev( ) const { return _prev; }
            vert &next( ) const { return _next; }

        private:
            vert &_prev, &_next;
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
            _In_ vert &prev,
            _In_ vert &next
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
            _In_ vert &prev,
            _In_ vert &next,
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

        private:
            void _delete_verts( )
            {
                for (vert *v : verts)
                    delete v;
            };
            void _delete_edges( )
            {
                for (edge *e : edges)
                    delete e;
            };

        public:
            ~_graph_base( )
            {
                constexpr size_t mtMinElements = 512, mtMinDifference = 256;

                long long numVerts = verts.size( );
                long long numEdges = edges.size( );
                if ((numVerts + numEdges) > mtMinElements && abs(numVerts - numEdges) > mtMinDifference)
                {
                    std::thread vthread(&_graph_base::_delete_verts, this);
                    _delete_edges( );
                    vthread.join( );
                }
                else
                {
                    _delete_verts( );
                    _delete_edges( );
                }
            }

            deref_interface<vert> all_verts( ) const { return deref_interface(verts); }
            deref_interface<edge> all_edges( ) const { return deref_interface(edges); }

            void push(
                _In_ const _VTy &value
            )
            {
                verts.push_back(new vert(value));
            }

            bool empty( )
            {
                return verts.empty( );
            }

            vert &at(
                _In_range_(<, verts.size()) size_t index
            )
            {
                assert(index < verts.size( ));
                return *verts.at(index);
            }

            const vert &at(
                _In_range_(<, verts.size( )) size_t index
            ) const
            {
                assert(index < verts.size( ));
                return *verts.at(index);
            }

        protected:
            void _link(
                _In_ vert &prev,
                _In_ vert &next,
                _In_ edge *e
            )
            {
                assert(&prev != &next);

                edges.push_back(e);
                prev.next( ).push_back(e);
                next.prev( ).push_back(e);
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

            _Ret_maybenull_ edge *edge_between(
                _In_ const vert &from_vert,
                _In_ const vert &  to_vert
            )
            {
                auto &from_vert_edges = from_vert.next( );
                auto &  to_vert_edges =   to_vert.prev( );

                if (from_vert_edges.size( ) < to_vert_edges.size( ))
                {
                    // Look for 'to' in 'from'
                    for (edge *e : from_vert_edges)
                    {
                        if (&e->next( ) == &to_vert) return e;
                    }
                }
                else
                {
                    // Look for 'from' in 'to'
                    for (edge *e : to_vert_edges)
                    {
                        if (&e->prev( ) == &from_vert) return e;
                    }
                }

                return nullptr;
            }

            template<class _Func, class... _Args>
            _Ret_maybenull_ vert *find(_In_ _Func fn, _Args... args)
            {
                for (vert *v : verts)
                {
                    if (fn(*v, args...)) return v;
                }
                return nullptr;
            }

            template<class _Func, class... _Args>
            std::vector<vert *> find_all(_In_ _Func fn, _Args... args)
            {
                std::vector<vert *> results;
                for (vert *v : verts)
                {
                    if (fn(*v, args...)) results.push_back(v);
                }
                return results;
            }

            void unlink(
                _Inout_ vert &from_vert,
                _Inout_ vert &to_vert,
                _Inout_ _Post_invalid_ edge &between_edge
            )
            {
                auto &from = from_vert.next( );
                auto &to = to_vert.prev( );

                auto edgesEnd = edges.end( );
                auto it_full = std::find(edges.begin( ), edgesEnd, &between_edge);

                auto fromEnd = from.end( );
                auto it_from = std::find(from.begin( ), fromEnd, &between_edge);

                auto toEnd = to.end( );
                auto it_to = std::find(to.begin( ), toEnd, &between_edge);

                assert(it_full != edgesEnd);
                assert(it_from != fromEnd);
                assert(it_to != toEnd);

                edges.erase(it_full);
                from.erase(it_from);
                to.erase(it_to);
                delete &between_edge;
            }

            void unlink(
                _Inout_ vert &from_vert,
                _Inout_ vert &  to_vert
            )
            {
                edge *between = edge_between(from_vert, to_vert);
                assert(between);
                unlink(from_vert, to_vert, *between);
            }

            void erase(
                _In_ _Post_invalid_ vert &erase_vert
            )
            {
                // Erase reference from list of all verts
                auto it = std::find(verts.begin( ), verts.end( ), &erase_vert);
                assert(it != verts.end( )); // It SHOULD BE in the graph.
                verts.erase(it);

                // Erase references from inputs
                for (edge *e : erase_vert.prev( ))
                {
                    unlink(e->prev(), erase_vert, *e);
                }

                // Erase references from outputs
                for (edge *e : erase_vert.next( ))
                {
                    unlink(erase_vert, e->next( ), *e);
                }

                // Free memory
                delete &erase_vert;
            }

            template<std::integral T>
            void erase(
                _In_range_(0, vert_count( )) T erase_vert_index
            )
            {
                erase(&at(static_cast<size_t>(erase_vert_index)));
            }

        protected:
            // removes links but doesn't destroy the vertex
            template<class _Fn = decltype([ ](vert &, edge &, vert &, edge &, vert &){ })>
            void _bypass(
                _Inout_ vert &bypass_vert,
                _In_ _Fn _apply_linkage
            )
            {
                // bypassing n<1<n --> n<1<n is undefined.
                // bypassing n --> n is defined, but may be unpredictable if edges aren't ordered.
                assert(bypass_vert->bypassable( ));

                auto &prev = bypass_vert->prev( );
                auto &next = bypass_vert->next( );

                size_t num_prev = prev.size( );
                size_t num_next = next.size( );
                size_t num_more = std::max(num_prev, num_next);

                for (size_t i = 0; i < num_more; ++i)
                {
                    size_t p_index = std::min(i, num_prev - 1);
                    size_t n_index = std::min(i, num_next - 1);

                    if (num_prev < num_next)
                    {
                        assert(p_index == 0);
                        assert(n_index == i);
                    }
                    else if (num_prev > num_next)
                    {
                        assert(p_index == i);
                        assert(n_index == 0);
                    }
                    else // num_prev == num_next
                    {
                        assert(p_index == i);
                        assert(n_index == i);
                    }

                    edge &pe = *prev.at(p_index);
                    edge &ne = *next.at(n_index);
                    vert &p = pe.prev( );
                    vert &n = ne.next( );
                    this->unlink(p, bypass_vert, pe);
                    this->unlink(bypass_vert, n, ne);
                    _apply_linkage(p, pe, bypass_vert, ne, n);
                }
            }

        public:
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
            _In_ vert &prev,
            _In_ vert &next
        )
        {
            this->_link(prev, next, new edge(prev, next));
        }

        // removes links but doesn't destroy the vertex
        void bypass(
            _Inout_ vert *bypass_vert
        )
        {
            auto apply_linkage = [this](vert *pv, edge *, vert *, edge *, vert *nv)
            {
                this->link(pv, nv);
            };
            this->_bypass<decltype(apply_linkage)>(bypass_vert, apply_linkage);
        }
    };

    template<class _VTy, non_void _ETy>
    class graph<_VTy, _ETy>
        : public _graph_base<_VTy, _ETy>
    {
        using base = _graph_base<_VTy, _ETy>;
    public:
        using vert = base::vert;
        using edge = base::edge;

        void link(
            _In_ vert &prev,
            _In_ vert &next,
            _In_ const _ETy &value
        )
        {
            this->_link(prev, next, new edge(prev, next, value));
        }

        struct bypass_combine_params
        {
            const _VTy &vert_prev;
            const _ETy &edge_prev;
            const _VTy &vert_bypassing;
            const _ETy &edge_next;
            const _VTy &vert_next;
        };

        // removes links but doesn't destroy the vertex
        template<class _Fn>
        void bypass(
            _Inout_ vert *bypass_vert,
            _In_ _Fn combine_func
        )
        {
            auto apply_linkage = [&](vert *pv, edge *pe, vert *v, edge *ne, vert *nv)
            {
                _ETy data = combine_func(bypass_combine_params{ *pv, *pe, *v, *ne, *nv });
                link(pv, nv, data);
            };
            this->_bypass(bypass_vert, apply_linkage);
        }
    };

    // ---

    template<class T>
    concept stepper_class = requires(const typename T::vert & v, const typename T::edge & e)
    {
        { T::step(v) } -> std::same_as<const std::vector<typename T::edge *> &>;
        { T::step(e) } -> std::same_as<typename T::vert &>;
    };

    template<class _VTy, class _ETy>
    struct step_forward
    {
        using vert = vert<_VTy, _ETy>;
        using edge = edge<_VTy, _ETy>;

        static const std::vector<edge *> &step(_In_ const vert &v)
        {
            return v.next( );
        }

        static vert &step(_In_ const edge &e)
        {
            return e.next( );
        }

        static_assert(stepper_class<step_forward>);
    };

    template<class _VTy, class _ETy>
    struct step_backward
    {
        using vert = vert<_VTy, _ETy>;
        using edge = edge<_VTy, _ETy>;

        static const std::vector<edge *> &step(_In_ const vert &v)
        {
            return v.prev( );
        }

        static vert &step(_In_ const edge &e)
        {
            return e.prev( );
        }

        static_assert(stepper_class<step_backward>);
    };

    template<class _VTy, class _ETy>
    struct walk
    {
        using vert = vert<_VTy, _ETy>;
        using edge = edge<_VTy, _ETy>;

        using single_step = std::tuple<edge *, vert *>;
        using step_vector = std::vector<single_step>;

    private:
        // see https://en.wikipedia.org/wiki/Breadth-first_search
        template<stepper_class stepper>
        class _walk_bfs
        {
        private:
            void handle_root(
                _In_ std::queue<vert *> &q,
                _In_ std::unordered_set<vert *> &visited,
                _In_ vert &root)
            {
                // 3  label root as explored
                visited.insert(&root);

                // 4  Q.enqueue(root)
                q.push(&root);
                this->steps.emplace_back(nullptr, &root);
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
                    for (edge *const e : stepper::step(v))
                    {
                        vert &w = stepper::step(e);

                        // 10  if w is not labeled as explored then
                        if (!visited.contains(&w))
                        {
                            // 11  label w as explored
                            visited.insert(&w);

                            // 13  Q.enqueue(w)
                            q.push(&w);
                            this->steps.emplace_back(&e, &w);
                        }
                    }
                }
            }

        public:
            // 1  procedure BFS(G, root) is
            _walk_bfs(
                _In_ std::vector<vert *> roots
            )
            {
                // 2  let Q be a queue
                std::queue<vert *> q;
                std::unordered_set<vert *> visited;

                // All roots at breadth 0
                for (vert *root : roots)
                {
                    handle_root(q, visited, *root);
                }

                handle_walk(q, visited);
            }

            // 1  procedure BFS(G, root) is
            _walk_bfs(
                _In_ vert &root
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
        template<stepper_class stepper>
        class _walk_dfs
        {
        private:
            // 1  procedure DFS_iterative(G, v) is
            void _walk_util(
                _Inout_ step_vector &_steps,
                _Inout_ std::unordered_set<vert *> &visited,
                _In_opt_ edge *eTaken,
                _In_ vert &v)
            {
                // 2  label v as discovered
                visited.insert(&v);
                _steps.emplace_back(eTaken, &v);

                // 3  for all directed edges from v to w that are in G.adjacentEdges(v) do
                for (edge *const e : stepper::step(v))
                {
                    vert &w = stepper::step(*e);

                    // 4  if vertex w is not labeled as discovered then
                    if (!visited.contains(&w))
                    {
                        // 5  recursively call DFS(G, w)
                        _walk_util(_steps, visited, e, w);
                    }
                }
            }

        public:
            _walk_dfs(
                _In_ std::vector<vert *> roots
            )
            {
                size_t numRoots = roots.size( );

                std::vector<std::unordered_set<vert *>> visitedVec;
                visitedVec.reserve(numRoots);

                std::vector<step_vector> stepsVec;
                stepsVec.reserve(numRoots);

                std::vector<std::thread> tasks;
                tasks.reserve(numRoots);

                for (size_t i = 0; i < numRoots; ++i)
                {
                    tasks.emplace_back(_walk_util, std::ref(stepsVec.at(i)), std::ref(visitedVec.at(i)), nullptr, *roots.at(i));
                }

                for (std::thread &task : tasks)
                {
                    task.join( );
                }

                // Combine steps

                size_t totalSteps = 0;
                for (const step_vector &s : stepsVec)
                {
                    totalSteps += s.size( );
                }

                this->steps.reserve(totalSteps);

                for (const step_vector &s : stepsVec)
                {
                    this->steps.insert(this->steps.end( ), s.begin( ), s.end( ));
                }
            }

            _walk_dfs(
                _In_ vert &root
            )
            {
                std::unordered_set<vert *> visited;

                _walk_util(this->steps, visited, nullptr, root);
            }
        };
    
        // see https://en.wikipedia.org/wiki/Depth-first_search
        template<stepper_class stepper>
        class _walk_dfs_stack
        {
        private:
            void _walk_route( )
            {

            }

        public:
            // 1  procedure DFS_iterative(G, v) is
            _walk_dfs_stack(
                _In_ std::vector<vert *> roots
            )
            {
                std::unordered_set<vert *> visited;

                // 2  let S be a stack
                std::stack<vert *> s;

                // 3  S.push(v)
                s.push( );

                // 4  while S is not empty do
            }

            // 1  procedure DFS_iterative(G, v) is
            _walk_dfs_stack(
                _In_ vert &root
            )
            {
                std::unordered_set<vert *> visited;

                // 2  let S be a stack
                std::stack<vert *> s;

                // 3  S.push(v)
                s.push(&root);
                this->steps.emplace_back(nullptr, &root);

                // 4  while S is not empty do
                while (!s.empty( ))
                {
                    // 5  v = S.pop()
                    vert &v = *s.top( );
                    s.pop( );

                    // 6  if v is not labeled as discovered then
                    if (!visited.contains(&v))
                    {
                        // 7  label v as discovered
                        visited.insert(&v);

                        // 8  for all edges from v to w in G.adjacentEdges(v) do 
                        for (edge *const e : stepper::step(v))
                        {
                            vert &w = stepper::step(*e);

                            // 9  S.push(w)
                            s.push(w);
                            this->steps.emplace_back(e, &w);
                        }
                    }
                }
            }
        };

    public:
        auto walk_bfs        (_In_ std::vector<vert *> roots) { return _walk_bfs      <step_forward >(roots); }
        auto walk_bfs_r      (_In_ std::vector<vert *> roots) { return _walk_bfs      <step_backward>(roots); }
        auto walk_dfs        (_In_ std::vector<vert *> roots) { return _walk_dfs      <step_forward >(roots); }
        auto walk_dfs_r      (_In_ std::vector<vert *> roots) { return _walk_dfs      <step_backward>(roots); }
        auto walk_dfs_stack  (_In_ std::vector<vert *> roots) { return _walk_dfs_stack<step_forward >(roots); }
        auto walk_dfs_stack_r(_In_ std::vector<vert *> roots) { return _walk_dfs_stack<step_backward>(roots); }
    
        auto walk_bfs        (_In_ vert &root) { return _walk_bfs      <step_forward >(root); }
        auto walk_bfs_r      (_In_ vert &root) { return _walk_bfs      <step_backward>(root); }
        auto walk_dfs        (_In_ vert &root) { return _walk_dfs      <step_forward >(root); }
        auto walk_dfs_r      (_In_ vert &root) { return _walk_dfs      <step_backward>(root); }
        auto walk_dfs_stack  (_In_ vert &root) { return _walk_dfs_stack<step_forward >(root); }
        auto walk_dfs_stack_r(_In_ vert &root) { return _walk_dfs_stack<step_backward>(root); }
    };
}
