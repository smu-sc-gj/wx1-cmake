#include <wx/wx.h>
#include <wx/listctrl.h>

using namespace std;

// Main app class declaration
class MyApp : public wxApp {

public:
	bool OnInit() override;
};


class ListFrame : public wxFrame {

private:

    wxListView* basicListView;
    int sortDirection  = 1;

public:

    ListFrame(const wxString& title, int init_width, int init_height) : wxFrame(nullptr, wxID_ANY, title) {

        // add views here
        SetSize(init_width, init_height);

        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        panel->SetSizer(sizer);


        // Create list
        basicListView = new wxListView(panel);

        // Setup columns
        basicListView->AppendColumn("ID");  // column 0
        basicListView->AppendColumn("Name"); // column 1
        basicListView->AppendColumn("Description"); // column 2

        // Configure column widths
        basicListView->SetColumnWidth(0, 80);
        basicListView->SetColumnWidth(1, 120);
        basicListView->SetColumnWidth(2, 600);

        populateListView(); // example build model and set in list

        sizer->Add(basicListView, 1, wxALL | wxEXPAND, 0);


        auto button = new wxButton(panel, wxID_ANY, "Sort By ID");
        button->Bind(wxEVT_BUTTON, &ListFrame::sortByID, this);
        sizer->Add(button, 0, wxALIGN_LEFT | wxTOP | wxBOTTOM | wxLEFT, 5);

    }

private:

    void populateListView() {

        // Insert - create item with col0 data
        // SetItem - add item data
        // If index > end of list then forces index to end of list
        
        //basicListView->InsertItem(0, "1");
        //basicListView->SetItem(0, 1, "Some Item");
        //basicListView->SetItem(0, 2, "foo");
        insertListItem(20, "Some Item", "foo");

        // do rest in convinience function
        insertListItem(25, "Another Item", "bar");
        insertListItem(10, "some item", "blob");
        insertListItem(8, "big item", "max power");
    }

    void insertListItem(int id, const wxString& name, const wxString& desc) {

        int index = basicListView->GetItemCount(); // insert at end of list (see above notes)

        basicListView->InsertItem(index, to_string(id));
        basicListView->SetItem(index, 1, name);
        basicListView->SetItem(index, 2, desc);

        basicListView->SetItemData(index, id); // meta data for sorting - wxListView doesn't sort on list contents
    }
    
    void sortByID(wxCommandEvent& event) {

        // Sort list based on client (meta) data set with SetItemData method
        basicListView->SortItems(
            [](wxIntPtr item1, wxIntPtr item2, wxIntPtr direction)->int {
                if (item1 == item2) {

                    return 0; // same
                }
                else if (item1 < item2) {

                    return -direction; // sign of value determines order 
                }
                else { // item1 > item2 

                    return direction;
                }
            },
            this->sortDirection
            );

        // invert direction so toggle order on button click
        sortDirection = -sortDirection;
    }
};

bool MyApp::OnInit()
{
    auto frame = new ListFrame("List Example", 1024, 768);

    frame->InitDialog();
    frame->Show();

    return true;
}

// Tell wxWidgets app class to use
IMPLEMENT_APP(MyApp);