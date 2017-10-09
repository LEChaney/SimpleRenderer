//
// Bachelor of Software Engineering
// Media Design School
// Auckland
// New Zealand
//
// (c) 2017 Media Design School
//
// Description  : A system which handles rendering each entity.
// Author       : Lance Chaney
// Mail         : lance.cha7337@mediadesign.school.nz
//

#include "RenderSystem.h"

#include "GLUtils.h"
#include "MaterialComponent.h"
#include "MeshComponent.h"
#include "Scene.h"
#include "UniformFormat.h"

#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

using glm::mat4;
using glm::vec3;

RenderSystem::RenderSystem(GLFWwindow* glContext, const Scene& scene)
	: m_glContext{ glContext }
	, m_scene{ scene }
	, m_uniformBindingPoint{ 0 }
	, m_shaderParamsBindingPoint{ 1 }
	, m_kUniformModelOffset{ 0 }
	, m_kUniformViewOffset{ sizeof(mat4) }
	, m_kUniformProjectionOffset{ sizeof(mat4) * 2 }
{
	// Create buffer for camera parameters
	glGenBuffers(1, &m_uboUniforms);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_uniformBindingPoint, m_uboUniforms);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformFormat), nullptr, GL_DYNAMIC_DRAW);

	// Create buffer for shader parameters
	glGenBuffers(1, &m_uboShaderParams);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_shaderParamsBindingPoint, m_uboShaderParams);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderParams), nullptr, GL_DYNAMIC_DRAW);
}

void RenderSystem::beginRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderSystem::endRender()
{
	glfwSwapBuffers(m_glContext);
}

void RenderSystem::update(size_t entityID)
{
	// Filter renderable entities
	const size_t kRenderableMask = COMPONENT_MESH | COMPONENT_MATERIAL | COMPONENT_TRANSFORM;
	if ((m_scene.componentMasks.at(entityID) & kRenderableMask) != kRenderableMask)
		return;

	// Get rendering components of entity
	const MaterialComponent& material = m_scene.materialComponents[entityID];
	const MeshComponent& mesh = m_scene.meshComponents.at(entityID);
	const mat4& transform = m_scene.transformComponents.at(entityID);

	// TODO: Add check that camera is a valid camera entity, throw error otherwise
	const mat4& cameraTransform = m_scene.transformComponents.at(m_cameraEntity);

	// Tell the gpu what material to use
	glUseProgram(material.shader);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(material.shader, "sampler"), 0);
	glBindTexture(GL_TEXTURE_2D, material.texture);

	// Send shader parameters to gpu
	GLuint blockIndex;
	blockIndex = glGetUniformBlockIndex(material.shader, "ShaderParams");
	glUniformBlockBinding(material.shader, blockIndex, m_shaderParamsBindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_shaderParamsBindingPoint, m_uboShaderParams);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderParams), &material.shaderParams);
		
	// Get Aspect ratio
	int width, height;
	glfwGetFramebufferSize(m_glContext, &width, &height);
	float aspectRatio = static_cast<float>(width) / height;

	// Get model, view and projection matrices
	UniformFormat uniforms;
	uniforms.model = transform;
	uniforms.view = glm::inverse(cameraTransform);
	uniforms.projection = glm::perspective(glm::radians(60.0f), aspectRatio, 0.5f, 100.0f);
	uniforms.cameraPos = cameraTransform[3];

	// Send the model view and projection matrices to the gpu
	blockIndex = glGetUniformBlockIndex(material.shader, "Uniforms");
	glUniformBlockBinding(material.shader, blockIndex, m_uniformBindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_uniformBindingPoint, m_uboUniforms);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformFormat), &uniforms);

	// Draw object
	glBindVertexArray(mesh.VAO);
	glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_INT, 0);
}

void RenderSystem::setCamera(size_t entityID)
{
	// TODO: Throw error if entity does not have a camera component
	m_cameraEntity = entityID;
}
