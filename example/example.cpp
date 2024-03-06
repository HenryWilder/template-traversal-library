#include <iostream>
#include <graph-traversal.hpp>

using namespace trav;

int main()
{
	graph<const char*, const char*> graph;
	graph.push("Hello");
	graph.push("big");
	graph.link(0, 1, " ");
	graph.push("cool");
	graph.link(0, 2, " ");
	graph.push("world");
	graph.link(1, 3, " ");
	graph.link(2, 3, " ");

	// Forward first
	// "Hello, World!"

	for (auto &[e, v] : graph.walk<BREADTH_FIRST>(0))
	{
		std::cout << (e ? *e : "") << v;
	}

	std::cout << '\n';

	// Backward second
	// "World!, Hello"

	for (auto &[e, v] : graph.walk<BREADTH_FIRST, BACKWARD>(3))
	{
		std::cout << (e ? *e : "") << v;
	}

	std::cout << '\n';
}
