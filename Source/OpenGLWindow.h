#pragma once
#include <JuceHeader.h>
#include <iostream>

class OpenGLWindow : public juce::OpenGLAppComponent {
public:
	struct Vertex {
		float position[3];
		float normal[3];
		float colour[4];
	};

    struct Attributes {
			std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position, normal, sourceColour;
			Attributes(juce::OpenGLShaderProgram& shaderProgram);
			void enable();
			void disable();
	private:
		static juce::OpenGLShaderProgram::Attribute* createAttribute(juce::OpenGLShaderProgram& shader,
			const juce::String& attributeName);
    };

	struct Uniforms {
		std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, lightPosition;
		Uniforms(juce::OpenGLShaderProgram& shaderProgram);
	private:
		static juce::OpenGLShaderProgram::Uniform* createUniform(juce::OpenGLShaderProgram& shaderProgram, const juce::String& uniformName);
	};

    struct Shape {
        struct VertexBuffer {
            GLuint vertexBuffer, indexBuffer;
            int numIndices;

			JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VertexBuffer);

			explicit VertexBuffer(int numIndices, float positions[], float normals[], juce::uint32 indices[], juce::Colour colour);

			~VertexBuffer();

			void bind();
        };
        juce::OwnedArray<VertexBuffer> vertexBuffers;

		Shape(int numIndices, float vertexPositions[], float vertexNormals[], juce::uint32 indices[], juce::Colour colour);

		void draw(Attributes& glAttributes);
    };

	juce::String vertexShader;
	juce::String fragmentShader;

	std::unique_ptr<juce::OpenGLShaderProgram> shader;
	std::vector<Shape> shapes;
	std::unique_ptr<Attributes> attributes;
	std::unique_ptr<Uniforms> uniforms;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLWindow)

	void(*initializeCallback)();

	OpenGLWindow();
	~OpenGLWindow() override;
	void initialise() override;
	void setInitializeCallback(void(*cb)());

	void shutdown() override;
	void render() override;
	void paint(juce::Graphics& g) override;

	void createShaders();
	Matrix3D<float> getViewMatrix() const;
	Vector3D<float> getLightPosition() const;
	Matrix3D<float> getProjectionMatrix() const;
};
juce::Vector3D<float> applyTransformationMatrix(const juce::Matrix3D<float>& matrix, const juce::Vector3D<float>& vector);
