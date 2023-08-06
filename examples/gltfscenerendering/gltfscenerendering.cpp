/*
* Vulkan Example - Scene rendering
*
* Copyright (C) 2020-202- by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*
* Summary:
* Render a complete scene loaded from an glTF file. The sample is based on the glTF model loading sample, 
* and adds data structures, functions and shaders required to render a more complex scene using Crytek's Sponza model.
*
* This sample comes with a tutorial, see the README.md in this folder
*/

#include "gltfscenerendering.h"

/*
	Vulkan Example class
*/

VulkanExample::VulkanExample() : VulkanExampleBase(ENABLE_VALIDATION)
{
	title = "glTF scene rendering";
	camera.type = Camera::CameraType::firstperson;
	camera.flipY = true;
	camera.setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
	camera.setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
	camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
}

VulkanExample::~VulkanExample()
{
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	shaderData.buffer.destroy();
}

void VulkanExample::getEnabledFeatures()
{
	enabledFeatures.samplerAnisotropy = deviceFeatures.samplerAnisotropy;
}

void VulkanExample::buildCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = defaultClearColor;
	clearValues[0].color = { { 0.25f, 0.25f, 0.25f, 1.0f } };;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	const VkViewport viewport = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
	const VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);

	for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = frameBuffers[i];
		VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));
		vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
		// Bind scene matrices descriptor to set 0
		vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		// POI: Draw the glTF scene
		//glTFScene.draw(drawCmdBuffers[i], pipelineLayout);
		scene.draw(drawCmdBuffers[i], 0, pipelineLayout);

		drawUI(drawCmdBuffers[i]);
		vkCmdEndRenderPass(drawCmdBuffers[i]);
		VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
	}
}

void VulkanExample::loadAssets()
{
	//const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
	const uint32_t glTFLoadingFlags = 0;
	//scene.loadFromFile(getAssetPath() + "models/sponza/sponza.gltf", vulkanDevice, queue, glTFLoadingFlags);
	scene.loadFromFile("C:\\Users\\bingk\\Desktop\\Now\\Sponza_Grouped.glb", vulkanDevice, queue, glTFLoadingFlags);
}

void VulkanExample::setupDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = {
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8),
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 9)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 3);
	VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &descriptorPool));
}

void VulkanExample::setupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
		// Binding 0 : Vertex shader uniform buffer
		vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
		//// Binding 1 : Position texture target / Scene colormap
		//vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		//// Binding 2 : Normals texture target
		//vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
		//// Binding 3 : Albedo texture target
		//vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
		//// Binding 4 : Fragment shader uniform buffer
		//vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorLayout, nullptr, &descriptorSetLayout));

	// Pipeline layout using both descriptor sets (set 0 = matrices, set 1 = material)
	std::array<VkDescriptorSetLayout, 2> setLayouts                = { descriptorSetLayout, vkglTF::descriptorSetLayoutImage };
	VkPipelineLayoutCreateInfo           pPipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(setLayouts.data(), static_cast<uint32_t>(setLayouts.size()));
	// We will use push constants to push the local matrices of a primitive to the vertex shader
	VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);
	// Push constant ranges are part of the pipeline layout
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pPipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;
	VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void VulkanExample::setupDescriptors()
{
	// Descriptor set for scene matrices
	VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(descriptorPool, &descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
	VkWriteDescriptorSet writeDescriptorSet = vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &shaderData.buffer.descriptor);
	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void VulkanExample::preparePipelines()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationStateCI = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
	VkPipelineColorBlendAttachmentState blendAttachmentStateCI = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
	VkPipelineColorBlendStateCreateInfo colorBlendStateCI = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentStateCI);
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	VkPipelineViewportStateCreateInfo viewportStateCI = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
	VkPipelineMultisampleStateCreateInfo multisampleStateCI = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
	const std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCI = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables.data(), static_cast<uint32_t>(dynamicStateEnables.size()), 0);
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCI = *vkglTF::Vertex::getPipelineVertexInputState(
		{ vkglTF::VertexComponent::Position,
		  vkglTF::VertexComponent::Normal,
		  vkglTF::VertexComponent::UV,
		  vkglTF::VertexComponent::Color,
		  vkglTF::VertexComponent::Tangent });

	VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass, 0);
	pipelineCI.pVertexInputState = &vertexInputStateCI;
	pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
	pipelineCI.pRasterizationState = &rasterizationStateCI;
	pipelineCI.pColorBlendState = &colorBlendStateCI;
	pipelineCI.pMultisampleState = &multisampleStateCI;
	pipelineCI.pViewportState = &viewportStateCI;
	pipelineCI.pDepthStencilState = &depthStencilStateCI;
	pipelineCI.pDynamicState = &dynamicStateCI;
	pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCI.pStages = shaderStages.data();

	shaderStages[0] = loadShader(getShadersPath() + "gltfscenerendering/scene.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = loadShader(getShadersPath() + "gltfscenerendering/scene.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// POI: Instead if using a few fixed pipelines, we create one pipeline for each material using the properties of that material
	for (auto &material : scene.materials) {

		struct MaterialSpecializationData {
			VkBool32 alphaMask;
			float alphaMaskCutoff;
		} materialSpecializationData;

		materialSpecializationData.alphaMask       = material.alphaMode == vkglTF::Material::ALPHAMODE_MASK;
		materialSpecializationData.alphaMaskCutoff = material.alphaCutoff;

		// POI: Constant fragment shader material parameters will be set using specialization constants
		std::vector<VkSpecializationMapEntry> specializationMapEntries = {
			vks::initializers::specializationMapEntry(0, offsetof(MaterialSpecializationData, alphaMask), sizeof(MaterialSpecializationData::alphaMask)),
			vks::initializers::specializationMapEntry(1, offsetof(MaterialSpecializationData, alphaMaskCutoff), sizeof(MaterialSpecializationData::alphaMaskCutoff)),
		};
		VkSpecializationInfo specializationInfo = vks::initializers::specializationInfo(specializationMapEntries, sizeof(materialSpecializationData), &materialSpecializationData);
		shaderStages[1].pSpecializationInfo = &specializationInfo;

		// For double sided materials, culling will be disabled
		rasterizationStateCI.cullMode = material.doubleSided ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &material.pipeline));
	}
}

void VulkanExample::prepareUniformBuffers()
{
	VK_CHECK_RESULT(vulkanDevice->createBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&shaderData.buffer,
		sizeof(shaderData.values)));
	VK_CHECK_RESULT(shaderData.buffer.map());
	updateUniformBuffers();
}

void VulkanExample::updateUniformBuffers()
{
	shaderData.values.projection = camera.matrices.perspective;
	shaderData.values.view = camera.matrices.view;
	shaderData.values.viewPos = camera.viewPos;
	memcpy(shaderData.buffer.mapped, &shaderData.values, sizeof(shaderData.values));
}

void VulkanExample::setupCallbacks()
{
	vkglTF::DrawCallCallback callback;
	callback.onDrawNode = [this](vkglTF::Node*    node,
								 VkCommandBuffer  commandBuffer,
								 VkPipelineLayout pipelineLayout)
	{ return onDrawNode(node, commandBuffer, pipelineLayout); };

	callback.onDrawMesh = [this](vkglTF::Mesh*    mesh,
								 VkCommandBuffer  commandBuffer,
								 VkPipelineLayout pipelineLayout)
	{ return onDrawMesh(mesh, commandBuffer, pipelineLayout); };

	callback.onDrawPrimitive = [this](vkglTF::Primitive* primitive,
									  VkCommandBuffer    commandBuffer,
									  VkPipelineLayout   pipelineLayout)
	{ return onDrawPrimitive(primitive, commandBuffer, pipelineLayout); };

	scene.setCallback(callback);
}

void VulkanExample::prepare()
{
	VulkanExampleBase::prepare();
	loadAssets();
	setupCallbacks();
	prepareUniformBuffers();
	setupDescriptorPool();
	setupDescriptorSetLayout();
	setupDescriptors();
	preparePipelines();
	buildCommandBuffers();
	prepared = true;
}

bool VulkanExample::onDrawNode(vkglTF::Node* node, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	return true;
}

bool VulkanExample::onDrawMesh(vkglTF::Mesh* mesh, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	const glm::mat4& nodeMatrix = mesh->uniformBlock.matrix;
	// Pass the final matrix to the vertex shader using push constants
	vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
	return true;
}

bool VulkanExample::onDrawPrimitive(vkglTF::Primitive* primitive, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	if (primitive->indexCount <= 0)
	{
		return false;
	}

	const vkglTF::Material& material = primitive->material;
	// POI: Bind the pipeline for the node's material
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &material.descriptorSet, 0, nullptr);
	return true;
}

void VulkanExample::render()
{
	renderFrame();
	if (camera.updated) {
		updateUniformBuffers();
	}
}

void VulkanExample::viewChanged()
{
	updateUniformBuffers();
}

void VulkanExample::OnUpdateUIOverlay(vks::UIOverlay* overlay)
{
	//if (overlay->header("Visibility")) {

	//	//if (overlay->button("All")) {
	//	//	std::for_each(glTFScene.nodes.begin(), glTFScene.nodes.end(), [](VulkanglTFScene::Node* node) { node->visible = true; });
	//	//	buildCommandBuffers();
	//	//}
	//	//ImGui::SameLine();
	//	//if (overlay->button("None")) {
	//	//	std::for_each(glTFScene.nodes.begin(), glTFScene.nodes.end(), [](VulkanglTFScene::Node* node) { node->visible = false; });
	//	//	buildCommandBuffers();
	//	//}
	//	//ImGui::NewLine();

	//	// POI: Create a list of glTF nodes for visibility toggle
	//	//ImGui::BeginChild("#nodelist", ImVec2(200.0f * overlay->scale, 340.0f * overlay->scale), false);
	//	//for (auto& node : glTFScene.nodes)
	//	//{		
	//	//	if (overlay->checkBox(node->name.c_str(), &node->visible))
	//	//	{
	//	//		buildCommandBuffers();
	//	//	}
	//	//}
	//	//ImGui::EndChild();
	//}
}

VULKAN_EXAMPLE_MAIN()
