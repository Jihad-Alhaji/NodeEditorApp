#pragma once
#include<vector>
#include<functional>
#include"ResourceManagement/ISerializationInterface.h"
class AdjacencyMat : public ResourceManagement::Serialization::ISerializationInterface
{
public:
	//a package of 8 edges in one 8bit memory block
	struct Edge_Package
	{
		unsigned char E0 : 1;
		unsigned char E1 : 1;
		unsigned char E2 : 1;
		unsigned char E3 : 1;
		unsigned char E4 : 1;
		unsigned char E5 : 1;
		unsigned char E6 : 1;
		unsigned char E7 : 1;
		void SetEdge(unsigned char index, bool val)
		{
			switch (index)
			{
			case 0:
				E0 = val;
				break;
			case 1:
				E1 = val;
				break;
			case 2:
				E2 = val;
				break;
			case 3:
				E3 = val;
				break;
			case 4:
				E4 = val;
				break;
			case 5:
				E5 = val;
				break;
			case 6:
				E6 = val;
				break;
			case 7:
				E7 = val;
				break;
			default:
				break;
			}
		}
		bool GetEdge(unsigned char index)const
		{
			switch (index)
			{
			case 0:
				return E0;
			case 1:
				return E1;
			case 2:
				return E2;
			case 3:
				return E3;
			case 4:
				return E4;
			case 5:
				return E5;
			case 6:
				return E6;
			case 7:
			default:
				return E7;
			}
		}
		
		unsigned char GetEdges()const
		{
			return reinterpret_cast<const unsigned char&>(*this);
		}
		
		//allows setting the eight edges with one step!
		void SetEdges(unsigned char edges)
		{
			//this cast allows us to access the memory block of this class directly
			auto& e = reinterpret_cast<unsigned char&>(*this);
			e = edges;
		}
	};
	
	//create enouph pakages to hold the give size
	void Create(size_t num);
	void Clear();

	size_t GetSize()const { return TrueSize; };
	bool GetEdgeValue(size_t from, size_t to)const;

	void AddEdge(size_t from, size_t to);//sets the indexed edge to one
	void RemoveEdge(size_t from, size_t to);//sets the indexed edge to zero

	//iterate for each out going connection from the given vertex, iterates over the set edges only i.e. not for 0 edge values
	void foreach_outGoing(size_t from, std::function<void(size_t to)> reader)const;
	
	//iterate over each incomming connection to the given vertex, iterates over the set edges only i.e. not for 0 edge values
	void foreach_inComing(size_t to, std::function<void(size_t from)> reader)const;

	//iterates over the whole graph
	void foreach(std::function<void(size_t from, size_t to, bool val)> reader)const;

	bool Load(const ResourceManagement::Serialization::ISerializationInterface::Serialzer_t& archive)override;
	void Save(ResourceManagement::Serialization::ISerializationInterface::Serialzer_t& archive)override;
	void Unload()override;
private:
	void SetEdgeValue(size_t from, size_t to, bool val);
	size_t TrueSize;//the nember of elemnts
	std::vector<std::vector<Edge_Package>> Vertices;
};

//adjacency matrix with wights to represent the edges
class WeightedAdjacencyMat : public ResourceManagement::Serialization::ISerializationInterface
{
public:
	using Edge = double;
	//create enouph pakages to hold the give size
	void Create(size_t num);
	void Clear();

	size_t GetSize()const { return Vertices.size(); };
	Edge GetEdgeValue(size_t from, size_t to)const;

	std::vector<Edge>& operator[](size_t from) { return Vertices[from]; };
	const std::vector<Edge>& operator[](size_t from)const { return Vertices[from]; };

	void AddEdge(size_t from, size_t to, Edge e);
	void RemoveEdge(size_t from, size_t to);//sets the indexed edge to zero

	//iterate for each out going connection from the given vertex
	void foreach_outGoing(size_t from, std::function<void(size_t to, Edge e)> reader)const;

	//iterate over each incomming connection to the given vertex
	void foreach_inComing(size_t to, std::function<void(size_t from, Edge e)> reader)const;

	//iterates over the whole graph
	void foreach(std::function<void(size_t from, size_t to, Edge e)> reader)const;

	bool Load(const ResourceManagement::Serialization::ISerializationInterface::Serialzer_t& archive)override;
	void Save(ResourceManagement::Serialization::ISerializationInterface::Serialzer_t& archive)override;
	void Unload()override;
private:
	std::vector<std::vector<Edge>> Vertices;
};
