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
		std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, lightPosition, hasWireframe, wireframeColour;
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
		bool hasWireframe;
		juce::Colour wireframeColour;

		Shape(int numIndices, float vertexPositions[], float vertexNormals[], juce::uint32 indices[], juce::Colour colour, bool hasWireframe, juce::Colour wireframeColour);

		void draw(Attributes& glAttributes);
    };

	juce::String vertexShader;
	juce::String fragmentShader;

	std::unique_ptr<juce::OpenGLShaderProgram> shader;
	std::vector<Shape> shapes;
	std::unique_ptr<Attributes> attributes;
	std::unique_ptr<Uniforms> uniforms;

	Draggable3DOrientation camera;
	float cameraDistanceNext = 10.0f;
	float cameraDistance = 10.0f;

	float scrollSpeedFactor = 0.5;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLWindow)

	void(*initializeCallback)();

	OpenGLWindow();
	~OpenGLWindow() override;
	void initialise() override;
	void setInitializeCallback(void(*cb)());

	void shutdown() override;
	void render() override;

	void resized() override;
	void mouseDown(const MouseEvent& e) override;
	//void mouseUp(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	//void mouseDoubleClick(const MouseEvent& e) override;
	void mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& w) override;
	//void mouseMagnify(const MouseEvent& e, float scale) override;

	void paint(juce::Graphics& g) override;

	void createShaders();
	Matrix3D<float> getViewMatrix() const;
	Vector3D<float> getLightPosition() const;
	Matrix3D<float> getProjectionMatrix() const;
};
juce::Vector3D<float> applyTransformationMatrix(const juce::Matrix3D<float>& matrix, const juce::Vector3D<float>& vector);
juce::Matrix3D<float> transposeMatrix(const juce::Matrix3D<float>& matrix);