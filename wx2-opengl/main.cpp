#include <glew.h>

#include <wx/wx.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/colordlg.h>
#include <wx/glcanvas.h>

#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "glew32s.lib")

using namespace std;

#if !wxUSE_GLCANVAS
#error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif


#pragma region DEPRICATED - GLFrame initial version declaration
class GLFrame;

class GLCanvas : public wxGLCanvas {

private:

    wxGLContext* glContext;
    bool isOpenGLInitialised{ false };
    unsigned int vao{ 0 }, vbo{ 0 }, shader{ 0 };

public:

    GLCanvas(GLFrame* parent, const wxGLAttributes& canvasAttrs);
    ~GLCanvas();

    bool InitializeOpenGLFunctions();
    bool InitializeOpenGL();

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);

    wxColour triangleColor{ wxColour(255, 128, 51) };

};

class GLFrame : public wxFrame {

private:

    GLCanvas* glCanvas{ nullptr };


public:

    GLFrame(const wxString& title);
 };
#pragma endregion


// Simple model encapsulating triangle model (colour for this example)
class TriangleModel {

private:
    wxColour triangleColour;

public:

    TriangleModel(wxColour initColour) {

        triangleColour = initColour;
    }

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

#ifdef _DEBUG
    wxLog* logger = nullptr;
#endif

    wxGLContext* gl_context{ nullptr };

    bool is_gl_initialised{ false };

    unsigned int vao{ 0 }, vbo{ 0 }, shader{ 0 };

    TriangleModel* model{ nullptr };

public:

    GLCanvas3(wxFrame* parent, const wxGLAttributes& canvasAttrs, const wxGLContextAttrs& contextAttrs, TriangleModel* model) : wxGLCanvas(parent, canvasAttrs) {

#ifdef _DEBUG
        logger = new wxLogWindow(this, "Debug Log", true, true); // cleaner
        wxLog::SetActiveTarget(logger);
#endif

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

    void OnMouseClick(wxMouseEvent& event) {

        wxLogDebug("Mouse Down!");
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


// Model everything at frame level for now - move to encapsulating gl witin a canvas object in GLFrame3
class GLFrame3 : public wxFrame {

private:

    GLCanvas3* gl_canvas{ nullptr };

    // Simple state to represent triangle colour
    TriangleModel* triModel{ nullptr };

public:

    GLFrame3(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

        // Initialise model
        triModel = new TriangleModel(wxColour(255, 128, 51));


        // Build UI around model

        auto sizer = new wxBoxSizer(wxVERTICAL);

        wxGLAttributes canvasAttrs;
        canvasAttrs.PlatformDefaults().RGBA().DoubleBuffer().EndList();

        wxGLContextAttrs contextAttrs;
        contextAttrs.PlatformDefaults().CompatibilityProfile().OGLVersion(4, 1).EndList();

        // Ideally just canvas at this level to focus on layout and top-level event handling - need to encapsulate GL init stuff away
        gl_canvas = new GLCanvas3(this, canvasAttrs, contextAttrs, triModel);
        gl_canvas->SetMinSize(FromDIP(wxSize(640, 480)));
        sizer->Add(gl_canvas, 1, wxEXPAND);


        auto bottomSizer = new wxBoxSizer(wxHORIZONTAL);
        auto colorButton = new wxButton(this, wxID_ANY, "Change Color");

        bottomSizer->Add(colorButton, 0, wxALL | wxALIGN_CENTER, FromDIP(15));
        bottomSizer->AddStretchSpacer(1);

        sizer->Add(bottomSizer, 0, wxEXPAND);

        SetSizerAndFit(sizer);

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


#pragma region GLFrame2 model

// Model everything at frame level for now - move to encapsulating gl witin a canvas object in GLFrame3
class GLFrame2 : public wxFrame {

private:

    wxGLCanvas* gl_canvas{ nullptr };
    wxGLContext* gl_context{ nullptr };
    
    bool is_gl_initialised{ false };
    
    unsigned int vao{ 0 }, vbo{ 0 }, shader{ 0 };
    
    // Simple state to represent triangle colour
    //wxColour triangleColor{ wxColour(255, 128, 51) };
    TriangleModel* triModel{ nullptr };

public:

    GLFrame2(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

        // Initialise model
        triModel = new TriangleModel(wxColour(255, 128, 51));


        // Build UI around model

        auto sizer = new wxBoxSizer(wxVERTICAL);

        wxGLAttributes dispAttrs;
        dispAttrs.PlatformDefaults().RGBA().DoubleBuffer().EndList();

        wxGLContextAttrs cxtAttrs;
        cxtAttrs.PlatformDefaults().CompatibilityProfile().OGLVersion(4, 1).EndList();

        // Ideally just canvas at this level to focus on layout and top-level event handling - need to encapsulate GL init stuff away
        gl_canvas = new wxGLCanvas(this, dispAttrs);
        gl_context = new wxGLContext(gl_canvas, NULL, &cxtAttrs);

        if (!gl_context->IsOK())
        {
            wxMessageBox("This sample needs an OpenGL 4.1 capable driver.",
                "OpenGL version error", wxOK | wxICON_INFORMATION, this);
            delete gl_context;
            gl_context = nullptr;
        }
        else
        {
            // First call SetCurrent or GL initialization will fail.
            //gl_context->SetCurrent(*gl_canvas);
            gl_canvas->SetCurrent(*gl_context);

            // Initialize GLEW.
            GLenum initStatus = glewInit();

            if (initStatus != GLEW_OK)
            {
                wxMessageBox("GLEW could not be initialised", "GLEW Error", wxOK | wxICON_INFORMATION, this);
                delete gl_context;
                gl_context = nullptr;
            }

            InitOpenGL();
        }


        gl_canvas->SetMinSize(FromDIP(wxSize(640, 480)));
        sizer->Add(gl_canvas, 1, wxEXPAND);


        auto bottomSizer = new wxBoxSizer(wxHORIZONTAL);
        auto colorButton = new wxButton(this, wxID_ANY, "Change Color");

        bottomSizer->Add(colorButton, 0, wxALL | wxALIGN_CENTER, FromDIP(15));
        bottomSizer->AddStretchSpacer(1);

        sizer->Add(bottomSizer, 0, wxEXPAND);

        SetSizerAndFit(sizer);

        // Setup event handling
        // 
        // mixing event handling - GLFrame3 should encapsulate better :)
        gl_canvas->Bind(wxEVT_PAINT, &GLFrame2::OnPaint, this);
        gl_canvas->Bind(wxEVT_SIZE, &GLFrame2::OnSize, this);

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


    void OnPaint(wxPaintEvent& event) {

        wxPaintDC dc(gl_canvas);

        if (!is_gl_initialised) {

            return;
        }

        //gl_canvas->SetCurrent(*gl_context);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        wxColour triangleColor = triModel->getColour();

        glUseProgram(shader);

        int colorLocation = glGetUniformLocation(shader, "triangleColor");
        glUniform4f(colorLocation, triangleColor.Red() / 255.0f, triangleColor.Green() / 255.0f, triangleColor.Blue() / 255.0f, 1.0f);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        gl_canvas->SwapBuffers();
    }


    void OnSize(wxSizeEvent& event) {

        //bool firstApperance = IsShownOnScreen() && !is_gl_initialised;

        //if (firstApperance) {

        //    InitOpenGL();
        //}

        if (is_gl_initialised) {

            auto viewPortSize = event.GetSize() * GetContentScaleFactor();
            glViewport(0, 0, viewPortSize.x, viewPortSize.y);
        }

        event.Skip();
    }

private:

    bool InitOpenGL() {

        if (!gl_context) {

            return false;
        }

        gl_canvas->SetCurrent(*gl_context);

        //if (!InitializeOpenGLFunctions())
        //{
        //    wxMessageBox("Error: Could not initialize OpenGL function pointers.",
        //        "OpenGL initialization error", wxOK | wxICON_INFORMATION, this);
        //    return false;
        //}

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

#pragma endregion


#pragma region Simple GL Window setup test
class wxGlewFrame : public wxFrame {

private:

    wxGLCanvas* m_canvas;
    wxGLContext* m_context;

public:

    wxGlewFrame(wxWindow *parent) : wxFrame(parent, wxID_ANY, wxString()) {

        // Create the canvas and context.
#if wxCHECK_VERSION(3,1,0)
    // These settings should work with any GPU from the last 10 years.
        wxGLAttributes dispAttrs;
        dispAttrs.PlatformDefaults().RGBA().DoubleBuffer().EndList();

        wxGLContextAttrs cxtAttrs;
        cxtAttrs.PlatformDefaults().CoreProfile().OGLVersion(4, 3).EndList();

        m_canvas = new wxGLCanvas(this, dispAttrs);
        m_context = new wxGLContext(m_canvas, NULL, &cxtAttrs);

        if (!m_context->IsOK())
        {
            SetTitle("Failed to create context.");
            return;
        }


#else
        int dispAttrs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_CORE_PROFILE,
                            WX_GL_MAJOR_VERSION ,3, WX_GL_MINOR_VERSION, 3, 0 };

        m_canvas = new wxGLCanvas(this, wxID_ANY, dispAttrs);
        m_context = new wxGLContext(m_canvas, NULL);

        // Unfortunately, there doesn't seem to be any way to check if the
        // context is ok prior to wxWidgets 3.1.0.
#endif // wxCHECK_VERSION


        // On Linux, we must delay delay initialization until the canvas has
    // been full created.  On windows, we can finish now.
#ifdef __WXMSW__
        InitGL();
#elif defined(__WXGTK__)
        m_canvas->Bind(wxEVT_CREATE, [this](wxWindowCreateEvent&) {InitGL(); });
#endif // defined
    }

    ~wxGlewFrame() {

        delete m_context;
    }

private:

    void OnCanvasSize(wxSizeEvent& event) {

        wxSize sz = event.GetSize();
        glViewport(0, 0, sz.GetWidth(), sz.GetHeight());
        event.Skip();
    }

    void OnCanvasPaint(wxPaintEvent&) {

        wxPaintDC dc(m_canvas);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_canvas->SwapBuffers();
    }

    void InitGL() {

        // First call SetCurrent or GL initialization will fail.
        //m_context->SetCurrent(*m_canvas);
        m_canvas->SetCurrent(*m_context);

        // Initialize GLEW.
        GLenum initStatus = glewInit();

        if (initStatus != GLEW_OK)
        {
            SetTitle("Failed it initialize GLEW.");
            return;
        }

        SetTitle("Context and GLEW initialized.");

        // Bind event handlers for the canvas. Binding was delayed until OpenGL was
        // initialized because these handlers will need to call OpenGL functions.
        m_canvas->Bind(wxEVT_SIZE, &wxGlewFrame::OnCanvasSize, this);
        m_canvas->Bind(wxEVT_PAINT, &wxGlewFrame::OnCanvasPaint, this);
    }
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


#pragma region DEPRICATED - GLFrame initial version implementation

// Layout of controls/views and frame functionality (events)
GLFrame::GLFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize)  {

    auto sizer = new wxBoxSizer(wxVERTICAL);

    wxGLAttributes vAttrs;
    vAttrs.PlatformDefaults().RGBA().DoubleBuffer().EndList();

    if (wxGLCanvas::IsDisplaySupported(vAttrs)) {

        glCanvas = new GLCanvas(this, vAttrs);
        glCanvas->SetMinSize(FromDIP(wxSize(640, 480)));
        sizer->Add(glCanvas, 1, wxEXPAND);
    }

    auto bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    auto colorButton = new wxButton(this, wxID_ANY, "Change Color");

    bottomSizer->Add(colorButton, 0, wxALL | wxALIGN_CENTER, FromDIP(15));
    bottomSizer->AddStretchSpacer(1);

    sizer->Add(bottomSizer, 0, wxEXPAND);

    SetSizerAndFit(sizer);

    colorButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event)
        {
            wxColourData colorData;
            colorData.SetColour(this->glCanvas->triangleColor);
            wxColourDialog dialog(this, &colorData);

            if (dialog.ShowModal() == wxID_OK)
            {
                glCanvas->triangleColor = dialog.GetColourData().GetColour();
                glCanvas->Refresh();
            } });
}

GLCanvas::GLCanvas(GLFrame* parent, const wxGLAttributes& canvasAttrs) : wxGLCanvas(parent, canvasAttrs) {

    wxGLContextAttrs ctxAttrs;
    
    ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(3, 3).EndList();// .CoreProfile().OGLVersion(3, 1).EndList();
    //ctxAttrs.CompatibilityProfile().MajorVersion(1).MinorVersion(1).DebugCtx().EndList();
    glContext = new wxGLContext(this, nullptr, &ctxAttrs);

    if (!glContext->IsOK())
    {
        wxMessageBox("This sample needs an OpenGL 3.3 capable driver.",
            "OpenGL version error", wxOK | wxICON_INFORMATION, this);
        delete glContext;
        glContext = nullptr;
    }

    Bind(wxEVT_PAINT, &GLCanvas::OnPaint, this);
    Bind(wxEVT_SIZE, &GLCanvas::OnSize, this);
}

GLCanvas::~GLCanvas() {

    delete glContext;
}

bool GLCanvas::InitializeOpenGLFunctions() {

    GLenum err = glewInit();

    if (GLEW_OK != err) {

        wxLogError("OpenGL GLEW initialization failed: %s", reinterpret_cast<const char*>(glewGetErrorString(err)));
        return false;
    }

    wxLogDebug("Status: Using GLEW %s", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)));

    return true;
}

bool GLCanvas::InitializeOpenGL() {

    if (!glContext)
    {
        return false;
    }

    SetCurrent(*glContext);

    if (!InitializeOpenGLFunctions())
    {
        wxMessageBox("Error: Could not initialize OpenGL function pointers.",
            "OpenGL initialization error", wxOK | wxICON_INFORMATION, this);
        return false;
    }

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

    isOpenGLInitialised = true;

    return true;
}

void GLCanvas::OnPaint(wxPaintEvent& event) {

    wxPaintDC dc(this);

    if (!isOpenGLInitialised)
    {
        return;
    }

    SetCurrent(*glContext);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader);

    int colorLocation = glGetUniformLocation(shader, "triangleColor");
    glUniform4f(colorLocation, triangleColor.Red() / 255.0f, triangleColor.Green() / 255.0f, triangleColor.Blue() / 255.0f, 1.0f);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SwapBuffers();
}

void GLCanvas::OnSize(wxSizeEvent& event) {

    bool firstApperance = IsShownOnScreen() && !isOpenGLInitialised;

    if (firstApperance) {

        InitializeOpenGL();
    }

    if (isOpenGLInitialised) {

        auto viewPortSize = event.GetSize() * GetContentScaleFactor();
        glViewport(0, 0, viewPortSize.x, viewPortSize.y);
    }

    event.Skip();
}

#pragma endregion