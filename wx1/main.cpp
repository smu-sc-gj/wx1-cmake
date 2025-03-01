#include <wx/wx.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/glcanvas.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

template <class T>
inline int valueof(T x) {
    return static_cast<int>(x);
}


class Resettable {

public:

    virtual void reset() = 0;
};


class Observable : public Resettable {

private:
    vector<wxWindow*> observerList;

public: 

    void addObserver(wxWindow* observer) {

        observerList.push_back(observer);
    }

    const vector<wxWindow*>& getObservers() {

        return observerList;
    }

};


class MyState : public Observable {

public: // to be accessible to validators, attributes need to be public

    int x, min_value, max_value;

    MyState(int x, int min_value, int max_value) {

        this->x = x;
        this->min_value = min_value;
        this->max_value = max_value;
    }

    // Resettable implementation
    void reset() override {

        x = min_value;
    }
};



wxDEFINE_EVENT(CHANGE_EVENT, wxCommandEvent);


// Main app class declaration
class MyApp : public wxApp {

public:
	bool OnInit() override;
    int OnExit() override;
};

const int BaseID = wxID_HIGHEST + 1;

class SliderFrame : public wxFrame {

private:

    
#ifdef _DEBUG
    wxLog* logger = nullptr;
#endif

    // View
    wxSlider* slider;
    wxTextCtrl* textBox;
    wxButton* resetButton;
    wxGenericValidator* sliderValidator;
    wxIntegerValidator<int>* myValidator;

    // Model
    
    MyState* modelA = nullptr;

    //int x = 50;

    // Test - observer list
    //vector<wxWindow*> observerList;

public:
    SliderFrame();

    enum class SliderID {

        ValueSlider = BaseID,
        ValueTextbox = BaseID + 1,
        ValueReset = BaseID + 2
    };

    /*void addObserver(wxWindow* observer) {

        observerList.push_back(observer);
    }

    // return const reference - cannot change
    const vector<wxWindow*>& getObservers() {

        return observerList;
    }*/

private:

    // Event handlers
    void OnSliderChange(wxScrollEvent& event);
    void OnTextEnter(wxCommandEvent& event);
    void OnTextFocusChange(wxFocusEvent& event);
    void OnReset(wxCommandEvent& event);
    void OnChange(wxCommandEvent& event);
};

class ButtonFrame : public wxFrame {

private:

#ifdef _DEBUG
    wxLog* logger = nullptr;
#endif


public:
    ButtonFrame();
   
    enum class ButtonID {
        Hello = wxID_HIGHEST + 1
    };

private:

    // Event handlers
    void OnOkayButtonPress(wxCommandEvent& event);
    void OnHelloButtonPress(wxCommandEvent& event);
};

// Top-level frame declaration
class MyFrame : public wxFrame
{
public:
    MyFrame();
    ~MyFrame();

private:

    // When debugging, setup re-direct if we want more control over destination
    // of output, but this will have no effect on RELEASE build
    // Don't do this for every log call, just logger setup as these have no effect
    // when not debugging.
#ifdef _DEBUG
    wxLog* logger = nullptr;
#endif


private:
    void OnHello(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
};

enum
{
    ID_Hello = 1
};


#pragma region App class implementation

// Tell wxWidgets app class to use
IMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    auto frame = new SliderFrame();

    //auto frame = new ButtonFrame();
    //MyFrame* frame = new MyFrame();

    frame->InitDialog();
    frame->Show();
    return true;
}

int MyApp::OnExit() {

    // Called before MyApp destructor (which in turn is called)
    // after wxWidgets cleans itself up.  Therefore, clean-up
    // any app-level resources in overridden OnExit BEFORE
    // wxWidgets does it's internal clean-up
    //cout << "Exiting MyApp!!";
    wxLogDebug("MyApp::OnExit\n");
    return 0;
}

#pragma endregion

#pragma region SliderFrame implementation

SliderFrame::SliderFrame() : wxFrame(nullptr, wxID_ANY, "Slider Example") {

#ifdef _DEBUG
    logger = new wxLogWindow(this, "Debug Log", true, true); // cleaner
    wxLog::SetActiveTarget(logger);
#endif

    SetSize(wxSize(1024, 768));

    wxPanel* hostPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Setup example model and associated UI

    modelA = new MyState(50, 0, 100);

    sliderValidator = new wxGenericValidator(&(modelA->x));
    slider = new wxSlider(hostPanel, valueof(SliderFrame::SliderID::ValueSlider), 0, modelA->min_value, modelA->max_value, wxDefaultPosition, wxDefaultSize, 4L, *sliderValidator);
    slider->SetClientData(modelA);
    
    myValidator = new wxIntegerValidator<int>(&(modelA->x), modelA->min_value, modelA->max_value);
    textBox = new wxTextCtrl(hostPanel, valueof(SliderFrame::SliderID::ValueTextbox), "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, *myValidator);

    // low level redirection of event - kill focus not emitted so not picked-up by Bind.
    textBox->Connect(valueof(SliderFrame::SliderID::ValueTextbox), wxEVT_KILL_FOCUS, wxFocusEventHandler(SliderFrame::OnTextFocusChange), nullptr, this);
    textBox->SetClientData(modelA);

    resetButton = new wxButton(hostPanel, valueof(SliderFrame::SliderID::ValueReset), "Reset");
    resetButton->SetClientData(modelA);
    
    // test - setup observers for state 'x'
    modelA->addObserver(slider);
    modelA->addObserver(textBox);

    sizer->Add(slider);
    sizer->Add(textBox);
    sizer->Add(resetButton);

    hostPanel->SetSizer(sizer);
    // Sizer hint / fit after set to panel, which contains controls
    // If we did before, defaults to window??  But no reference to content
    // so don't get correct results.
    //sizer->SetSizeHints(this);
    //sizer->Fit(this);
    
    // id parameter distinguishes which control sent message.
    Bind(wxEVT_SCROLL_THUMBTRACK, &SliderFrame::OnSliderChange, this, valueof(SliderFrame::SliderID::ValueSlider));
    Bind(wxEVT_SCROLL_CHANGED, &SliderFrame::OnSliderChange, this, valueof(SliderFrame::SliderID::ValueSlider));
    Bind(wxEVT_TEXT_ENTER, &SliderFrame::OnTextEnter, this, valueof(SliderFrame::SliderID::ValueTextbox));
   // Bind(wxEVT_KILL_FOCUS, &SliderFrame::OnTextFocusChange, this, valueof(SliderFrame::SliderID::ValueTextbox));
    Bind(wxEVT_BUTTON, &SliderFrame::OnReset, this, valueof(SliderFrame::SliderID::ValueReset));

    // events for Observer implementation?  Need distinct event type for each data source???
    Bind(CHANGE_EVENT, &SliderFrame::OnChange, this, 0); // provide userdata to specify ctrl / data source?
    
}

void SliderFrame::OnSliderChange(wxScrollEvent& event) {

    // Update model
    auto slider_ctrl = dynamic_cast<wxSlider * > (event.GetEventObject());
    slider_ctrl->TransferDataFromWindow(); // decouples data (model) from control and event handler
    
    wxCommandEvent changeEvent = wxCommandEvent(CHANGE_EVENT, 0);
    changeEvent.SetClientData(slider_ctrl->GetClientData());

    // Update views - Explicit initiation of change as validators will only change state they're associated with
    wxPostEvent(this, changeEvent);
}

void SliderFrame::OnTextEnter(wxCommandEvent& event) {

    // Note: Validator not applied prior to calling this!

    // Update model
    //event.GetString().ToInt(&x);
    //x = std::min(std::max(x, 0), 100); // u postfix is literal unsigned int.  When min bound is 0 std::max is redundant for unsigned values, left here for generality

    auto textctrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    textctrl->Validate(); // if fails validation will not transfer - need to report this to user or modify
    // different behaviour from focus change, that forces value into correct range (how?);  call this OR provide meta/user data and handle correction here
    textctrl->TransferDataFromWindow();


    // Update views

    wxCommandEvent changeEvent = wxCommandEvent(CHANGE_EVENT, 0);
    changeEvent.SetClientData(textctrl->GetClientData());

    wxPostEvent(this, changeEvent);
     
    //slider->SetValue(x);
    //dynamic_cast<wxTextCtrl*>(event.GetEventObject())->SetValue(std::to_string(x)); // push to textbox in case above validation changes incoming value

    //wxLogDebug("%d\n", modelA->x);
}

void SliderFrame::OnTextFocusChange(wxFocusEvent& event) {

    // Update model (this data is validated by the IntegerValidator attached to the field)
    auto textctrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
    textctrl->Validate();
    textctrl->TransferDataFromWindow();
    
    // Update views
    wxCommandEvent changeEvent = wxCommandEvent(CHANGE_EVENT, 0);
    changeEvent.SetClientData(textctrl->GetClientData());

    wxPostEvent(this, changeEvent);

    //wxPostEvent(this, wxCommandEvent(CHANGE_EVENT, 0));

    //wxLogDebug("%d\n", modelA->x);
    event.Skip();
}

// Reset is scoped differently - can apply to model or individial element - let's do individual element here.
// Reset refers to a specific structure (even if scalar) - requires reset on the data type
void SliderFrame::OnReset(wxCommandEvent& event) {

    auto resetButton = dynamic_cast<wxButton*>(event.GetEventObject());

    // no validators so explicit call to model change method
    Resettable* model = (Resettable*)resetButton->GetClientData();
    model->reset();

    // There's no state to push from the button - no validator - so what do we change?
    // 
    // Update model
    //modelA->x = modelA->min_value; // EXPLICIT - SHOULD NOT BE!

    
    // Update views
    wxCommandEvent changeEvent = wxCommandEvent(CHANGE_EVENT, 0);
    changeEvent.SetClientData(model);

    wxPostEvent(this, changeEvent);


    //slider->SetValue(x);
    //textBox->SetValue(std::to_string(x));
}

void SliderFrame::OnChange(wxCommandEvent& event) {

    // Update model
    wxLogDebug("Change event fired!\n");

    Observable* data = (Observable*)event.GetClientData();

    const vector<wxWindow*> observerList = data->getObservers(); // EXPLICIT - SHOULD NOT BE!
    for (wxWindow* observer : observerList) {

        observer->TransferDataToWindow();
    }
}

#pragma endregion



#pragma region ButtonFrame implementation

ButtonFrame::ButtonFrame() : wxFrame(nullptr, wxID_ANY, "Button Example") {

#ifdef _DEBUG
    logger = new wxLogWindow(this, "Debug Log", true, true); // cleaner
    wxLog::SetActiveTarget(logger);
#endif

    SetSize(wxSize(1024, 768));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* hostPanel = new wxPanel(this, wxID_ANY);
    
    wxButton* helloButton = new wxButton(hostPanel, (int)(ButtonFrame::ButtonID::Hello), "Hello");
    wxButton* okayButton = new wxButton(hostPanel, wxID_OK);

    sizer->Add(helloButton, 2, wxALIGN_RIGHT, 40);
    sizer->Add(okayButton, 1, wxEXPAND);
    
    hostPanel->SetSizer(sizer);
    // Sizer hint / fit after set to panel, which contains controls
    // If we did before, defaults to window??  But no reference to content
    // so don't get correct results.
    //sizer->SetSizeHints(this);
    //sizer->Fit(this);

    Bind(wxEVT_BUTTON, &ButtonFrame::OnOkayButtonPress, this, wxID_OK);
    Bind(wxEVT_BUTTON, &ButtonFrame::OnHelloButtonPress, this, valueof(ButtonFrame::ButtonID::Hello));
}

void ButtonFrame::OnOkayButtonPress(wxCommandEvent& event) {

    wxLogDebug("Okay!\n");
}
void ButtonFrame::OnHelloButtonPress(wxCommandEvent& event) {

    wxLogDebug("Hello there from ButtonFrame\n");
}

#pragma endregion


#pragma region MyFrame implementation example

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, wxT("Hello World"))
{

#ifdef _DEBUG
    //logger = new wxLogStderr();
    logger = new wxLogWindow(this, "Debug Log", true, true); // cleaner

    wxLog::SetActiveTarget(logger);
#endif

    // Create file menu
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    // Create help menu
    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    // Create menu bar seen at top of window and add file and help menus created above
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    // Set frame's menu bar
    SetMenuBar(menuBar);


    // Create and set status bar
    CreateStatusBar();
    SetStatusText("Welcome to wxWidgets!");

    // Bind events to frame's methods (event handlers)
    Bind(wxEVT_MENU, &MyFrame::OnHello, this, ID_Hello);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

MyFrame::~MyFrame() {

#ifdef _DEBUG
    wxLog::SetActiveTarget(nullptr);

    if (logger)
        delete logger;
#endif

}

void MyFrame::OnExit(wxCommandEvent& event)
{
    wxLogDebug("MyFrame::OnExit\n");

    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox("This is a wxWidgets Hello World example",
        "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent& event)
{
    //wxLogMessage("Hello world from wxWidgets!");
    wxLogDebug("Hello :)\n"); // debug console output
}

#pragma endregion
