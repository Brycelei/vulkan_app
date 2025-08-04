#include<mesh.h>


namespace lxh
{
	Mesh::Mesh()
		: m_name("default")
		, indexBuffer(nullptr)
		, vertexBuffer(nullptr)
		, vertexCount(0)
		, indexCount(0)
	{

	}
	Mesh::Mesh(const Mesh& mesh)
		: m_name(mesh.m_name)
		, indexBuffer(mesh.indexBuffer)
		, vertexBuffer(mesh.vertexBuffer)
		, vertexCount(mesh.vertexCount)
		, indexCount(mesh.indexCount)
	{
	}


	Mesh::Mesh(const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices)
		:m_name("default")
		, indexBuffer(nullptr)
		, vertexBuffer(nullptr)
		, vertexCount(static_cast<uint32_t>(vertices.size()))
		, indexCount(static_cast<uint32_t>(indices.size()))
	{

	}

	Mesh::Mesh(std::vector<uint32_t>& indices, std::vector<Vertex>& vertices)
		:m_name("default")
		, indexBuffer(nullptr)
		, vertexBuffer(nullptr)
		, vertexCount(static_cast<uint32_t>(vertices.size()))
		, indexCount(static_cast<uint32_t>(indices.size()))
	{

	}
	
	Mesh::~Mesh()
	{

	}

}