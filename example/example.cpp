#include <iostream>
#include <graph-traversal.hpp>

using namespace trav;

int main()
{
	graph<const char*, const char *> graph;
	graph.push("Hello");
	graph.push("World!");
	graph.link(0, 1, ", ");

	for (auto [v,e] : graph.walk<BREADTH_FIRST>(0))
	{
		if (e) std::cout << **e;
		if (v) std::cout << **v;
	}
}
