#include <iostream>
#include <algorithm>
#include <graph-traversal.hpp>

using namespace trav;

int main()
{
	{
		graph<const char *, char> g;
		g.push("v0");
		g.push("v1");
		g.link(0, 1, 'a');
		g.push("v2");
		g.link(0, 2, 'b');
		g.push("v3");
		g.link(1, 3, 'c');
		g.link(2, 3, 'd');

		if (g.at(2).bypassable())
		{
			g.bypass(2, +[ ](const decltype(g)::bypass_combine_params &data)
			{
				return data.edge_prev;
			});
		}

		// BFS
		// Expected: v0 v1 v2 v3
		for (auto &[e, v] : g.walk_bfs(0))
		{
			std::cout << v << ' ';
		}
		std::cout << '\n';

		// DFS
		// Expected: v0 v1 v3 v2
		for (auto &[e, v] : g.walk_dfs(0))
		{
			std::cout << v << ' ';
		}
		std::cout << '\n';
	}
}
