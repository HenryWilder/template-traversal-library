#include <iostream>
#include <graph-traversal.hpp>

using namespace trav;

int main()
{
	{
		graph<const char *> graph;
		graph.push("v0");
		graph.push("v1");
		graph.link(0, 1);
		graph.push("v2");
		graph.link(0, 2);
		graph.push("v3");
		graph.link(1, 3);
		graph.link(2, 3);

		// BFS
		// Expected: v0 v1 v2 v3
		for (auto &[e, v] : graph.walk_bfs(0))
		{
			std::cout << v << ' ';
		}
		std::cout << '\n';

		// DFS
		// Expected: v0 v1 v3 v2
		for (auto &[e, v] : graph.walk_dfs(0))
		{
			std::cout << v << ' ';
		}
		std::cout << '\n';
	}
}
