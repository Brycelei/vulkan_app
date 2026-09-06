#include <lxh_model.h>

#include <stdexcept>

#ifndef ENGINE_DIR
#define ENGINE_DIR ""
#endif

namespace lxh
{
	void LxhModel::Builder::loadModel(const std::string& filepath, LxhDevice& device)
	{
		if (!parser.LoadModel(filepath, device, m_meshs))
		{
			throw std::runtime_error("Failed to load model: " + filepath);
		}

		m_directory = filepath.substr(0, filepath.find_last_of("/\\"));
		m_name = filepath.substr(filepath.find_last_of("/\\") + 1);
	}

	LxhModel::LxhModel(LxhDevice& device, const LxhModel::Builder& builder)
		: lxhDevice(device)
		, m_meshs(builder.m_meshs)
	{
		for (auto& mesh : m_meshs)
		{
			mesh->createVertexBuffers(device);
			mesh->createIndexBuffers(device);
		}
	}

	std::unique_ptr<LxhModel> LxhModel::createModelFromFile(LxhDevice& device, const std::string& filepath)
	{
		Builder builder{};
		builder.loadModel(ENGINE_DIR + filepath, device);
		return std::make_unique<LxhModel>(device, builder);
	}
}
