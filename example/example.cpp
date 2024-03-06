#include <iostream>
#include <graph-traversal.hpp>

using namespace trav;

int main()
{
	graph<const char*, const char*> graph;
	graph.push("v0");
	graph.push("v1");
	graph.link(0, 1, "e0-1");
	graph.push("v2");
	graph.link(0, 2, "e0-2");
	graph.push("v3");
	graph.link(1, 3, "e1-3");
	graph.link(2, 3, "e2-3");

	// Forward first
	// "Hello, World!"

	for (auto &[e, v] : graph.walk_bfs(0))
	{
		std::cout << (e ? *e : "") << ' ' << v << ' ';
	}

	std::cout << '\n';

	// Backward second
	// "World!, Hello"

	for (auto &[e, v] : graph.walk_bfs_r(3))
	{
		std::cout << (e ? *e : "") << ' ' << v << ' ';
	}

	std::cout << '\n';
}
