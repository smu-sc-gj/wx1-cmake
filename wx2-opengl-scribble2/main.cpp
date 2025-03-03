//#define GLEW_STATIC 1
//#pragma comment(lib, "glew\\lib\\Release\\x64\\glew32s.lib") // static lib not dll!

// linker args: /SUBSYSTEM:WINDOWS

#include "GL/glew.h"

#include <wx/wx.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/colordlg.h>
#include <wx/glcanvas.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif


// Simple model encapsulating triangle model (colour for this example)
class TriangleModel {

private:
    wxColour triangleColour;

public:

    TriangleModel(wxColour initColour) {

        triangleColour = initColour;
    }

    // Accessor member functions

    wxColour getColour() {

        return triangleColour;
    }

    void setColour(wxColour newColour) {

        triangleColour = newColour;
    }
};


#pragma region GLFrame3 model 

// ** Use this ** split OpenGL state management into canvas while managing model at frame (controller) level

class GLCanvas3 : public wxGLCanvas {

private:

    wxGLContext* gl_context{ nullptr };

    bool is_gl_initialised{ false };

    unsigned int vao{ 0 }, vbo{ 0 }, shader{ 0 };

    TriangleModel* model{ nullptr };

public:

    GLCanvas3(wxFrame* parent, const wxGLAttributes& canvasAttrs, const wxGLContextAttrs& contextAttrs, TriangleModel* model) : wxGLCanvas(parent, canvasAttrs) {

        gl_context = new wxGLContext(this, NULL, &contextAttrs);

        if (!gl_context->IsOK()) {

            wxMessageBox("This sample needs an OpenGL 4.1 capable driver.",
                "OpenGL version error", wxOK | wxICON_INFORMATION, this);
            delete gl_context;
            gl_context = nullptr;
        }
        else
        {
            // First call SetCurrent or GL initialization will fail.
            SetCurrent(*gl_context);

            // Initialize GLEW.
            GLenum initStatus = glewInit();

            if (initStatus != GLEW_OK) {

                wxMessageBox("GLEW could not be initialised", "GLEW Error", wxOK | wxICON_INFORMATION, this);
                delete gl_context;
                gl_context = nullptr;
            }
            else {

                // All okay - setup model and OpenGL environment
                this->model = model;
                InitOpenGL();

                // Setup event handling for canvas
                Bind(wxEVT_PAINT, &GLCanvas3::OnPaint, this);
                Bind(wxEVT_SIZE, &GLCanvas3::OnSize, this);
                Bind(wxEVT_LEFT_DOWN, &GLCanvas3::OnMouseClick, this);
            }
        }
    }

    // Event handlers

    void OnMouseClick(wxMouseEvent& event) {

        wxLogDebug("Mouse Down!"); // test interaction
    }

    void OnPaint(wxPaintEvent& event) {

        if (!is_gl_initialised)
            return;

        wxPaintDC dc(this);
        SetCurrent(*gl_context);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        wxColour triangleColor = model->getColour();

        glUseProgram(shader);

        int colorLocation = glGetUniformLocation(shader, "triangleColor");
        glUniform4f(colorLocation, triangleColor.Red() / 255.0f, triangleColor.Green() / 255.0f, triangleColor.Blue() / 255.0f, 1.0f);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SwapBuffers();
    }

    void OnSize(wxSizeEvent& event) {

        if (is_gl_initialised) {

            auto viewPortSize = event.GetSize() * GetContentScaleFactor();
            glViewport(0, 0, viewPortSize.x, viewPortSize.y);
        }

        event.Skip();
    }

private:

    // Initialise OpenGL environment.  Assume valid context given single caller point for this method
    bool InitOpenGL() {

        SetCurrent(*gl_context);

        wxLogDebug("OpenGL version: %s", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
        wxLogDebug("OpenGL vendor: %s", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));

        constexpr auto vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";

        constexpr auto fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 triangleColor;
        void main()
        {
            FragColor = triangleColor;
        }
    )";

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            wxLogDebug("Vertex Shader Compilation Failed: %s", infoLog);
        }

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            wxLogDebug("Fragment Shader Compilation Failed: %s", infoLog);
        }

        shader = glCreateProgram();
        glAttachShader(shader, vertexShader);
        glAttachShader(shader, fragmentShader);
        glLinkProgram(shader);

        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(shader, 512, nullptr, infoLog);
            wxLogDebug("Shader Program Linking Failed: %s", infoLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        is_gl_initialised = true;

        return true;
    }
};


// Model everything at frame level for now - OpenGL config encapsulated within GLCanvas3
class GLFrame3 : public wxFrame {

private:

    GLCanvas3* gl_canvas{ nullptr };

    // Simple state to represent triangle colour
    TriangleModel* triModel{ nullptr };

#ifdef _DEBUG
    wxLog* logger = nullptr;
#endif


public:

    GLFrame3(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

        // If debug config setup logging window alongside main GUI and redirect
        // wxWidgets output
#ifdef _DEBUG
        logger = new wxLogWindow(this, "Debug Log", true, true);
        wxLog::SetActiveTarget(logger);
#endif

        //
        // Initialise model here for now
        //
        triModel = new TriangleModel(wxColour(255, 128, 51));


        //
        // Build UI
        //
        auto top_level_sizer = new wxBoxSizer(wxVERTICAL);

        wxGLAttributes canvasAttrs;
        canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().EndList();

        wxGLContextAttrs contextAttrs;
        contextAttrs.PlatformDefaults().CompatibilityProfile().OGLVersion(4, 1).EndList();

        // Ideally just canvas at this level to focus on layout and top-level event handling - need to encapsulate GL init stuff away
        gl_canvas = new GLCanvas3(this, canvasAttrs, contextAttrs, triModel);
        gl_canvas->SetMinSize(FromDIP(wxSize(640, 480)));
        top_level_sizer->Add(gl_canvas, 1, wxEXPAND);


        auto button_sizer = new wxBoxSizer(wxHORIZONTAL);
        auto colorButton = new wxButton(this, wxID_ANY, "Change Color");

        button_sizer->Add(colorButton, 0, wxALL | wxALIGN_CENTER, FromDIP(15));
        button_sizer->AddStretchSpacer(1);

        top_level_sizer->Add(button_sizer, 0, wxEXPAND);

        // attach top level sizer to frame and config layout
        SetSizerAndFit(top_level_sizer);

        // Setup event handling
        colorButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
            {
                wxColourData colorData;
                colorData.SetColour(this->triModel->getColour());
                wxColourDialog dialog(this, &colorData);

                if (dialog.ShowModal() == wxID_OK)
                {
                    this->triModel->setColour(dialog.GetColourData().GetColour());
                    gl_canvas->Refresh();
                } });
    };

};

#pragma endregion


// Main app class declaration
class MyApp : public wxApp {

public:

    MyApp();
    bool OnInit() override;
};


const int BaseID = wxID_HIGHEST + 1;


#pragma region App class implementation

MyApp::MyApp() {}

bool MyApp::OnInit()
{
    if (!wxApp::OnInit()) {

        return false;
    }

    auto frame = new GLFrame3("Hello, OpenGL!");
    frame->InitDialog();
    frame->Show();

    return true;
}

// Tell wxWidgets app class to use
IMPLEMENT_APP(MyApp);

#pragma endregion

