#include "OpenGLWindow.h"

OpenGLWindow::OpenGLWindow() {
    setSize(700, 700);
    camera.setViewport(getLocalBounds());
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

void OpenGLWindow::resized() {
    camera.setViewport(getLocalBounds());
}

void OpenGLWindow::mouseDown(const MouseEvent& e) {
    camera.mouseDown(e.getPosition());
}

void OpenGLWindow::mouseDrag(const MouseEvent& e) {
    camera.mouseDrag(e.getPosition());
}

void OpenGLWindow::mouseWheelMove(const MouseEvent& e, const MouseWheelDetails& w) {
    cameraDistanceNext = cameraDistanceNext * (1-w.deltaY*scrollSpeedFactor);
}

auto newTime = std::chrono::high_resolution_clock::now();
auto oldTime = std::chrono::high_resolution_clock::now();

void OpenGLWindow::render(){
    newTime = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double, std::milli>(newTime - oldTime).count()/1000;
    oldTime = newTime;
    //update
    cameraDistance += 15*dt*((double)cameraDistanceNext - cameraDistance);

    //render
    using namespace ::juce::gl;

    jassert(juce::OpenGLHelpers::isContextActive());

    auto desktopScale = (float)openGLContext.getRenderingScale();
    juce::OpenGLHelpers::clear(Colour());

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonOffset(0.1,1);

    glViewport(0, 0, roundToInt(desktopScale * (float)getWidth()), roundToInt(desktopScale * (float)getHeight()));

    shader->use();

    if (uniforms->projectionMatrix.get() != nullptr)
        uniforms->projectionMatrix->setMatrix4(getProjectionMatrix().mat, 1, false);

    if (uniforms->viewMatrix.get() != nullptr)
        uniforms->viewMatrix->setMatrix4(getViewMatrix().mat, 1, false);

    if (uniforms->lightPosition.get() != nullptr) {
        Vector3D<float> lightPos = (Vector3D<float>)applyTransformationMatrix(getViewMatrix(), getLightPosition());
        uniforms->lightPosition->set(lightPos.x, lightPos.y, lightPos.z, 1.0f);
    }
    
    uniforms->hasWireframe->set(0);
    uniforms->wireframeColour->set(0,0,0,1);

    glPolygonMode(GL_FRONT, GL_FILL);
    for (auto& shape : shapes) {
        shape.draw(*attributes);
    }


    glPolygonMode(GL_FRONT, GL_LINE);
    for (auto& shape : shapes) {
        uniforms->hasWireframe->set(shape.hasWireframe? 1: 0);
        uniforms->wireframeColour->set(shape.wireframeColour.getFloatRed(), shape.wireframeColour.getFloatGreen(), shape.wireframeColour.getFloatBlue(), shape.wireframeColour.getFloatAlpha());
        shape.draw(*attributes);
    }

    // Reset the element buffers so child Components draw correctly
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 
}


void OpenGLWindow::paint(juce::Graphics& g) {
    // You can add your component specific drawing code here!
    // This will draw over the top of the openGL background.

    //g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
    //g.setFont(30);
    //g.drawText("OpenGL Test", 25, 20, 300, 30, juce::Justification::left);
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
    lightPosition.reset(createUniform(shaderProgram, "lightPosition"));
    hasWireframe.reset(createUniform(shaderProgram, "hasWireframe"));
    wireframeColour.reset(createUniform(shaderProgram, "wireframeColour"));

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
OpenGLWindow::Shape::Shape(int numIndices, float vertexPositions[], float vertexNormals[], juce::uint32 indices[], juce::Colour colour, bool hasWireframe, juce::Colour wireframeColour): hasWireframe(hasWireframe), wireframeColour(wireframeColour) {
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
    auto w = .25f;
    auto h = w * getLocalBounds().toFloat().getAspectRatio(false);

    return Matrix3D<float>::fromFrustum(-w, w, -h, h, 1.0f, 200.0f);

}

Matrix3D<float> OpenGLWindow::getViewMatrix() const {

    auto viewMatrix = camera.getRotationMatrix()
        * Vector3D<float>(0.0f, 0.0f, -cameraDistance);

    auto rotationMatrix = Matrix3D<float>::rotation({ 0, 0, 0 });

    return rotationMatrix * viewMatrix;
}

Vector3D<float> OpenGLWindow::getLightPosition() const {
    Vector3D<float> lightPos(10.0f,10.0f,5.0f);

    return lightPos;
}


void OpenGLWindow::createShaders() {
    vertexShader = R"(
    attribute vec4 position;
    attribute vec4 normal;
    attribute vec4 sourceColour;

    uniform mat4 projectionMatrix;
    uniform mat4 viewMatrix;
    uniform vec4 lightPosition;

    uniform int hasWireframe;
    uniform vec4 wireframeColour;

    varying vec4 destinationColour;

    void main()
    {
        destinationColour = vec4( (hasWireframe > 0 ? wireframeColour.xyz : sourceColour.xyz)*min(1, .5+max(dot(normalize(normal), normalize(lightPosition)), 0.0)),1);
        gl_Position = projectionMatrix * viewMatrix *position ;
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
    std::unique_ptr<juce::OpenGLShaderProgram> newShader(new juce::OpenGLShaderProgram(openGLContext)); 
    juce::String statusText;

    if (newShader->addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(vertexShader))
        && newShader->addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(fragmentShader))
        && newShader->link()) {
        attributes.reset();
        uniforms.reset();

        shader.reset(newShader.release());
        shader->use();

        attributes.reset(new Attributes(*shader));
        uniforms.reset(new Uniforms(*shader));

        statusText = "GLSL: v" + juce::String(juce::OpenGLShaderProgram::getLanguageVersion(), 2);
    }
    else {
        statusText = newShader->getLastError();
    }
};



juce::Vector3D<float> applyTransformationMatrix(const juce::Matrix3D<float>& matrix, const juce::Vector3D<float>& vector) {
    juce::Vector3D<float> result;
    result.x = result.x = matrix.mat[4 * 0 + 0] * vector.x + matrix.mat[4 * 0 + 1] * vector.y + matrix.mat[4 * 0 + 2] * vector.z + matrix.mat[4 * 0 + 3];
    result.y = result.y = matrix.mat[4 * 1 + 0] * vector.x + matrix.mat[4 * 1 + 1] * vector.y + matrix.mat[4 * 1 + 2] * vector.z + matrix.mat[4 * 1 + 3];
    result.z = result.z = matrix.mat[4 * 2 + 0] * vector.x + matrix.mat[4 * 2 + 1] * vector.y + matrix.mat[4 * 2 + 2] * vector.z + matrix.mat[4 * 2 + 3];
    return result;
}

juce::Matrix3D<float> transposeMatrix(const juce::Matrix3D<float>& matrix) {
    juce::Matrix3D<float> result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.mat[4 * j + i] = matrix.mat[4 * j + i];
        }
    }
    return result;
}