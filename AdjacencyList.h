#include <iterator>
#include <list>

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/thread/mutex.hpp>

#ifndef AdjacencyList_h
#define AdjacencyList_h

/**
 * Convenience wrapper for a thread-safe directed unweighted graph with vertices labeled with a specific type.
 *
 * @param <T> The vertex type.  Must be default-constructible, copy-constructible, assignable, and hashable.
 */
template <class T>
class Graph {
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, T> internalGraph;
	
	typedef typename boost::graph_traits<internalGraph>::vertex_descriptor VertexDescriptor;
	typedef typename boost::graph_traits<internalGraph>::edge_descriptor EdgeDescriptor;
	
	typedef typename boost::graph_traits<internalGraph>::vertex_iterator VertexIterator;
	typedef typename boost::graph_traits<internalGraph>::edge_iterator EdgeIterator;
	
	typedef boost::shared_ptr<boost::shared_mutex> mutex_ptr;
	typedef boost::shared_lock<boost::shared_mutex> ReadLock;
	typedef boost::upgrade_lock<boost::shared_mutex> RereadLock;
	typedef boost::upgrade_to_unique_lock<boost::shared_mutex> WriteLock;
	
	struct Transformer {
		const internalGraph& G;
		Transformer(const internalGraph& G_) : G(G_) {}
		
		T operator()(const VertexDescriptor& v) { return G[v]; }
	};
public:
	typedef typename boost::graph_traits<internalGraph>::vertices_size_type VertexSizeType;
	typedef typename boost::graph_traits<internalGraph>::edges_size_type EdgeSizeType;
	
	Graph() : G() {}
	
	/**
	 * Adds a vertex to the graph.  Does nothing if the vertex already exists.
	 */
	void addVertex(const T& vertex) {
		if (containsVertex(vertex)) return;
		
		RereadLock rereadLock(mutex);				// Reread Lock
		if (descriptorMap.count(vertex) == 0) {		// Check again to see if vertex was inserted while waiting
			WriteLock writeLock(rereadLock);		// Write Lock
			
			// Add the vertex
			VertexDescriptor vd = boost::add_vertex(vertex, G);
			descriptorMap[vertex] = vd;
			
			// Create the new vertex mutex
			vertexMutexes.insert(std::pair<T, mutex_ptr>(vertex, mutex_ptr(new boost::shared_mutex())));
		}
	}
	
	/**
	 * Determines if the vertex is present in the graph.
	 */
	bool containsVertex(const T& vertex) const {
		ReadLock readLock(mutex);					// Read Lock
		return descriptorMap.count(vertex) != 0;
	}
	
	/**
	 * Adds an edge to the graph.  Creates the vertices if they do not exist.
	 */
	void addEdge(const T& from, const T& to) {
		if (!containsVertex(from)) addVertex(from);
		if (!containsVertex(to)) addVertex(to);
		
		// Lock only the from vertex
		boost::shared_mutex& fromMutex = *(vertexMutexes[from]);
		RereadLock rereadLock(fromMutex);				// Reread Lock
		
		// TODO - move the following two into the read lock section?
		VertexDescriptor from_d = getDescriptor(from);
		VertexDescriptor to_d = getDescriptor(to);
		
		if (!boost::edge(from_d, to_d, G).second) {		// If the vertex wasn't created while waiting
			WriteLock writeLock(rereadLock);			// Write Lock
			
			boost::add_edge(from_d, to_d, G);
		}
	}
	
	bool containsEdge(const T& from, const T& to) const {
		if (!containsVertex(from)) return false;
		if (!containsVertex(to)) return false;
		
		ReadLock readLock(mutex);					// Previous checks had read locks, so now we have to have their own.
		VertexDescriptor from_d = getDescriptor(from);
		VertexDescriptor to_d = getDescriptor(to);
		
		return boost::edge(from_d, to_d, G).second;
	}
	
	VertexSizeType numVertices() const {
		ReadLock readLock(mutex);						// Read Lock
		return boost::num_vertices(G);
	}
	EdgeSizeType numEdges() const {
		ReadLock readLock(mutex);						// Read Lock
		return boost::num_edges(G);
	}
	
	std::list<T> getVertices() const {
		ReadLock readLock(mutex);						// Read Lock
		std::pair<VertexIterator, VertexIterator> its = vertices();
		
		std::list<T> result;
		std::transform(its.first(), its.second(), std::back_inserter(result), Transformer(G));
		return result;
	}
	
	std::list<T> topological_sort() const {
		ReadLock readLock(mutex);						// Read Lock
		std::deque<VertexDescriptor> descriptors;
		boost::topological_sort(G, std::front_inserter(descriptors));
		
		std::list<T> result;
		std::transform(descriptors.begin(), descriptors.end(), std::back_inserter(result), Transformer(G));
		
		return result;
	}
private:
	internalGraph G;
	// BGL does not track the T-VertexDescriptor association, so we do so manually.
	boost::unordered_map<T, VertexDescriptor> descriptorMap;
	
	mutable boost::shared_mutex mutex;					// mutable so that a const Graph can get locks!
	boost::unordered_map<T, mutex_ptr> vertexMutexes;	// boost::shared_mutex is not copyable, so heap allocate
	
	std::pair<VertexIterator, VertexIterator> vertices() const { return boost::vertices(G); }
	std::pair<EdgeIterator, EdgeIterator> edges() const { return boost::edges(G); }
	
	/**
	 * Returns the vertex descriptor for a vertex.
	 *
	 * Precondition: the vertex must already be in the graph.
	 */
	VertexDescriptor getDescriptor(const T& vertex) { return descriptorMap[vertex]; }
	
	template <class PropertyTag>
	typename boost::property_map<internalGraph, PropertyTag>::type get(PropertyTag tag) { return boost::get(tag, G); }
	
	template <class PropertyTag>
	typename boost::property_map<internalGraph, PropertyTag>::const_type get(PropertyTag tag) const { return boost::get(tag, G); }
};

#endif
