#include <wx/wx.h>
#include <wx/listctrl.h>
#include <vector>

#include <wx/imagpng.h>
#include <wx/gdicmn.h>

using namespace std;


struct ItemData {

    int         id;
    wxString    name;
    wxString    description;
};

class ListModel {

public:
    vector<ItemData> items;
};



// Virtual list subclass - virtual lists special case of report view
class VirtualList : public wxListCtrl {

private:

    // Store reference to model
    ListModel* hostModel{ nullptr };

public:

    VirtualList(wxWindow* parent, const wxWindowID id, const wxPoint& pos, const wxSize& size, ListModel *model) : wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_VIRTUAL | wxLC_EDIT_LABELS) {

        // Link to model
        this->hostModel = model;

        // Setup list columns
        AppendColumn("ID");
        AppendColumn("Name");
        AppendColumn("Description");

        SetColumnWidth(0, 80);
        SetColumnWidth(1, 120);
        SetColumnWidth(2, 600);
    }

    // Override method to query data for list element
    virtual wxString OnGetItemText(long index, long column) const override {

        ItemData item = hostModel->items[index];

        switch (column) {
        case 0: return std::to_string(item.id);
        case 1: return item.name;
        case 2: return item.description;
        default: return wxString("");
        }
    }

    // Refresh list count and update list itself once changes made
    void RefreshAfterUpdate() {

        SetItemCount(hostModel->items.size());
        Refresh();
    }

};


class ListFrame : public wxFrame {

private:

    ListModel* model;

    VirtualList* listView{ nullptr };

#ifdef _DEBUG
    wxLog* logger = nullptr;
#endif

public:

    ListFrame(const wxString& title, int init_width, int init_height, ListModel *model) :
        wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(init_width, init_height)) {

        wxImage::AddHandler(new wxPNGHandler);
        
        SetIcon(wxIcon(wxT("FRAMEICON1")));
        
        auto buttonIcon = wxBITMAP_PNG(COFFEE64);
        auto toolIcon = wxBITMAP(COFFEE16);

        auto toolbar = CreateToolBar(wxTB_TOP);
        toolbar->SetToolBitmapSize(wxSize(64, 64));

        auto tool = toolbar->AddTool(wxID_ANY, "Coffee", wxBitmapBundle::FromBitmap(toolIcon));

        toolbar->Realize();

#ifdef _DEBUG
        logger = new wxLogWindow(this, "Debug Log", true, true); // cleaner
        wxLog::SetActiveTarget(logger);
#endif

        // Link frame to model data
        this->model = model;

        // Setup views
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        panel->SetSizer(sizer);

        listView = new VirtualList(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, this->model);
        sizer->Add(listView, 1, wxALL | wxEXPAND, 0);

        // Populate model
        this->model->items.push_back({20, "A-Some Item------------", "foo"});
        this->model->items.push_back({25, "B-Another Item----", "bar" });
        this->model->items.push_back({10, "D-some item", "blob" });
        this->model->items.push_back({8, "C-big", "max power" });

        // Update list based on new data
        listView->RefreshAfterUpdate();

        // Setup event handler for list column clicks
        listView->Bind(wxEVT_LIST_COL_CLICK, [this](wxListEvent event) {
            this->sortByColumn(event.GetColumn());
            });

        // Test other events
        listView->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent event) {
            wxLogDebug("item %d selected: column %d", event.GetIndex(), event.GetColumn());
            });

        listView->Bind(wxEVT_LIST_BEGIN_LABEL_EDIT, [this](wxListEvent event) {
            wxLogDebug("edit label %d", event.GetIndex());
            });

        auto button = new wxButton(panel, wxID_ANY);
        button->SetBitmap(wxBitmapBundle::FromBitmap(buttonIcon));

        sizer->Add(button);
    }

    // Sort model and update list
    void sortByColumn(int column) {

        switch (column) {

        case 0: // id
            std::sort(model->items.begin(), model->items.end(), [](ItemData i1, ItemData i2)->bool {
                return i1.id < i2.id;
                });
            break;
        case 1: // name
            std::sort(model->items.begin(), model->items.end(), [](ItemData i1, ItemData i2)->bool {
                return i1.name < i2.name;
                });
            break;
        case 2: // description
            std::sort(model->items.begin(), model->items.end(), [](ItemData i1, ItemData i2)->bool {
                return i1.description < i2.description;
                });
            break;
        }

        // Once sorted refresh list
        listView->RefreshAfterUpdate();
    }
};

// Main app class declaration
class MyApp : public wxApp {

private:
    ListModel* model{ nullptr };

public:
    bool OnInit() override;
};


bool MyApp::OnInit()
{
    // Setup model
    model = new ListModel();
    
    // Setup top-level views and link model
    auto frame = new ListFrame("Virtual List Example", 1024, 768, model);

    frame->InitDialog();
    frame->Show();
    return true;
}

// Tell wxWidgets app class to use
IMPLEMENT_APP(MyApp);

