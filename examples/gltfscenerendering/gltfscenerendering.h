/*
* Vulkan Example - Scene rendering
*
* Copyright (C) 2020-2022 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*
* Summary:
* Render a complete scene loaded from an glTF file. The sample is based on the glTF model loading sample,
* and adds data structures, functions and shaders required to render a more complex scene using Crytek's Sponza model.
*
* This sample comes with a tutorial, see the README.md in this folder
*/

#include "vulkanexamplebase.h"
#include "VulkanglTFModel.h"

#define ENABLE_VALIDATION false

class VulkanExample : public VulkanExampleBase
{
public:
	//VulkanglTFScene glTFScene;
	vkglTF::Model scene;

	struct ShaderData {
		vks::Buffer buffer;
		struct Values {
			glm::mat4 projection;
			glm::mat4 view;
			glm::vec4 lightPos = glm::vec4(0.0f, 2.5f, 0.0f, 1.0f);
			glm::vec4 viewPos;
		} values;
	} shaderData;

	VkPipelineLayout      pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet       descriptorSet;

	VulkanExample();
	~VulkanExample();
	virtual void getEnabledFeatures();
	void buildCommandBuffers();
	void loadAssets();
	void setupDescriptorPool();
	void setupDescriptorSetLayout();
	void setupDescriptors();
	void preparePipelines();
	void prepareUniformBuffers();
	void updateUniformBuffers();
	void setupCallbacks();
	void prepare();
	

	bool onDrawNode(vkglTF::Node*    node,
					VkCommandBuffer  commandBuffer,
					VkPipelineLayout pipelineLayout);
	bool onDrawMesh(vkglTF::Mesh*    mesh,
					VkCommandBuffer  commandBuffer,
					VkPipelineLayout pipelineLayout);
	bool onDrawPrimitive(vkglTF::Primitive* primitive,
						 VkCommandBuffer    commandBuffer,
						 VkPipelineLayout   pipelineLayout);

	virtual void render();
	virtual void viewChanged();
	virtual void OnUpdateUIOverlay(vks::UIOverlay* overlay);
};
