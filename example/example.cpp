#include <iostream>
#include <graph-traversal.hpp>

using namespace trav;

int main()
{
	{
		graph<const char *> g;
		g.push("v0");
		g.push("v1");
		g.link(0, 1);
		g.push("v2");
		g.link(0, 2);
		g.push("v3");
		g.link(1, 3);
		g.link(2, 3);

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
