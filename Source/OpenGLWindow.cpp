#include "OpenGLWindow.h"

OpenGLWindow::OpenGLWindow() {
    setSize(900, 700);
}


OpenGLWindow::~OpenGLWindow() {
    shutdownOpenGL();
}

void OpenGLWindow::setInitializeCallback(void(*cb)()) {
    initializeCallback = cb;
};


void OpenGLWindow::initialise() {
    createShaders();
    DBG("--- OpenGL initialized ---");
    if (initializeCallback) {
        initializeCallback();
    }
}



void OpenGLWindow::shutdown() {
    shader.reset();
    shapes.clear();
    attributes.reset();
    uniforms.reset();
}

void OpenGLWindow::render(){
    using namespace ::juce::gl;

    jassert(juce::OpenGLHelpers::isContextActive());

    auto desktopScale = (float)openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear(Colour());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0,
        0,
        juce::roundToInt(desktopScale * (float)getWidth()),
        juce::roundToInt(desktopScale * (float)getHeight()));

    shader->use();

    if (uniforms->projectionMatrix.get() != nullptr)
        uniforms->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);

    if (uniforms->viewMatrix.get() != nullptr)
        uniforms->viewMatrix->setMatrix4(getViewMatrix().mat, 1, false);

    for (auto& shape : shapes) {
        shape.draw(*attributes);
    }

    // Reset the element buffers so child Components draw correctly
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 
}

void OpenGLWindow::paint(juce::Graphics& g) {
    // You can add your component specific drawing code here!
    // This will draw over the top of the openGL background.

    g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
    g.setFont(20);
    g.drawText("OpenGL Test", 25, 20, 300, 30, juce::Justification::left);
    g.drawLine(20, 20, 140, 20);
    g.drawLine(20, 50, 140, 50);
}

/*
*   Attributes
*/
OpenGLWindow::Attributes::Attributes(juce::OpenGLShaderProgram& shaderProgram) {
    position.reset(createAttribute(shaderProgram, "position"));
    normal.reset(createAttribute(shaderProgram, "normal"));
    sourceColour.reset(createAttribute(shaderProgram, "sourceColour"));
}

void OpenGLWindow::Attributes::enable() {
    using namespace ::juce::gl;

    if (position.get() != nullptr) {
        glVertexAttribPointer(position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);
        glEnableVertexAttribArray(position->attributeID);
    }

    if (normal.get() != nullptr) {
        glVertexAttribPointer(normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 3));
        glEnableVertexAttribArray(normal->attributeID);
    }

    if (sourceColour.get() != nullptr) {
        glVertexAttribPointer(sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(sizeof(float) * 6));
        glEnableVertexAttribArray(sourceColour->attributeID);
    }


}

void OpenGLWindow::Attributes::disable() {
    using namespace ::juce::gl;

    if (position.get() != nullptr)       glDisableVertexAttribArray(position->attributeID);
    if (normal.get() != nullptr)         glDisableVertexAttribArray(normal->attributeID);
    if (sourceColour.get() != nullptr)   glDisableVertexAttribArray(sourceColour->attributeID);
}

juce::OpenGLShaderProgram::Attribute* OpenGLWindow::Attributes::createAttribute(juce::OpenGLShaderProgram& shader,
    const juce::String& attributeName) {
    using namespace ::juce::gl;

    if (glGetAttribLocation(shader.getProgramID(), attributeName.toRawUTF8()) < 0)
        return nullptr;

    return new juce::OpenGLShaderProgram::Attribute(shader, attributeName.toRawUTF8());
}


/*
*   Uniforms
*/
OpenGLWindow::Uniforms::Uniforms(juce::OpenGLShaderProgram& shaderProgram) {
    projectionMatrix.reset(createUniform(shaderProgram, "projectionMatrix"));
    viewMatrix.reset(createUniform(shaderProgram, "viewMatrix"));
}

juce::OpenGLShaderProgram::Uniform* OpenGLWindow::Uniforms::createUniform(juce::OpenGLShaderProgram& shaderProgram,
    const juce::String& uniformName) {
    using namespace ::juce::gl;

    if (glGetUniformLocation(shaderProgram.getProgramID(), uniformName.toRawUTF8()) < 0)
        return nullptr;

    return new juce::OpenGLShaderProgram::Uniform(shaderProgram, uniformName.toRawUTF8());
}

/*
*   Shape
*/
OpenGLWindow::Shape::Shape(int numIndices, float vertexPositions[], float vertexNormals[], juce::uint32 indices[], juce::Colour colour) {
    vertexBuffers.add(new VertexBuffer(numIndices, vertexPositions, vertexNormals, indices, colour));
}

void OpenGLWindow::Shape::draw(Attributes& glAttributes) {
    using namespace ::juce::gl;

    for (auto* vertexBuffer : vertexBuffers) {
        vertexBuffer->bind();

        glAttributes.enable();
        glDrawElements(GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, nullptr);
        glAttributes.disable();
    }
}

OpenGLWindow::Shape::VertexBuffer::VertexBuffer(int nIndices, float positions[], float normals[], juce::uint32 indices[], juce::Colour colour): numIndices(nIndices) {
    using namespace ::juce::gl;

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    auto scale = 1.0f;

    juce::Array<Vertex> vertices;
    for (int v = 0; v < numIndices; v++) {
        vertices.add({ { scale * positions[v * 3], scale * positions[v * 3 + 1], scale * positions[v * 3 + 2], },
                { scale * normals[v * 3], scale * normals[v * 3 + 1], scale * normals[v * 3 + 2], },
                { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() }, });
    };



    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof(Vertex)),
        vertices.getRawDataPointer(), GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof(juce::uint32)), indices, GL_STATIC_DRAW);
}

OpenGLWindow::Shape::VertexBuffer::~VertexBuffer() {
    using namespace ::juce::gl;

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &indexBuffer);
}

void OpenGLWindow::Shape::VertexBuffer::bind() {
    using namespace ::juce::gl;

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
}

Matrix3D<float> OpenGLWindow::getProjectionMatrix() const {
    auto w = 1.0f / (0.5f + 0.1f);
    auto h = w * getLocalBounds().toFloat().getAspectRatio(false);

    return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.0f);
}

Matrix3D<float> OpenGLWindow::getViewMatrix() const {
    Matrix3D<float> viewMatrix({ 0.0f, 0.0f, -10.0f });
    Matrix3D<float> rotationMatrix = viewMatrix.rotation({ -0.3f, 5.0f * std::sin((float)getFrameCounter() * 0.01f), 0.0f });

    return rotationMatrix * viewMatrix;
}

void OpenGLWindow::createShaders() {
    vertexShader = R"(
    attribute vec4 position;
    attribute vec4 sourceColour;

    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;

    varying vec4 destinationColour;

    void main()
    {
        destinationColour = sourceColour;
        gl_Position = projectionMatrix * viewMatrix * position;
    })";

    fragmentShader =
#if JUCE_OPENGL_ES
        R"(varying lowp vec4 destinationColour;)"
#else
        R"(varying vec4 destinationColour;)"
#endif
        R"(
            void main()
            {)"
#if JUCE_OPENGL_ES
        R"(    lowp vec4 colour = destinationColour;
#else
            R"(    vec4 colour = destinationColour;)"
#endif
        R"(    gl_FragColor = colour;
            })";
    std::unique_ptr<juce::OpenGLShaderProgram> newShader(new juce::OpenGLShaderProgram(openGLContext));   // [1]
    juce::String statusText;

    if (newShader->addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(vertexShader))          // [2]
        && newShader->addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(fragmentShader))
        && newShader->link()) {
        attributes.reset();
        uniforms.reset();

        shader.reset(newShader.release());                                                                 // [3]
        shader->use();

        attributes.reset(new Attributes(*shader));
        uniforms.reset(new Uniforms(*shader));

        statusText = "GLSL: v" + juce::String(juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else {
        statusText = newShader->getLastError();                                                             // [4]
    }
};