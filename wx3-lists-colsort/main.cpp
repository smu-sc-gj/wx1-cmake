#include <wx/wx.h>
#include <wx/listctrl.h>
#include <unordered_set>
#include <memory>


using namespace std;

// Main app class declaration
class MyApp : public wxApp {

public:
    bool OnInit() override;
};


struct ItemData {

    int         id;
    wxString    name;
    wxString    description;
};


class ListFrame : public wxFrame {

private:

    wxListView* basicListView;
    int sortDirection = 1;

    // Our list model (preserves pointers linked to wxListView item's client/meta-data)
    unordered_set<unique_ptr<ItemData>> itemCollection;

public:

    ListFrame(const wxString& title, int init_width, int init_height) : wxFrame(nullptr, wxID_ANY, title) {

        // add views here
        SetSize(init_width, init_height);

        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        panel->SetSizer(sizer);


        // Create list - report style with columns (different approach for icon views etc - see online docs)
        basicListView = new wxListView(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);

        
        // Setup columns
        basicListView->AppendColumn("ID");  // column 0
        basicListView->AppendColumn("Name"); // column 1
        basicListView->AppendColumn("Description"); // column 2

        // Configure column widths
        basicListView->SetColumnWidth(0, 80);
        basicListView->SetColumnWidth(1, 120);
        basicListView->SetColumnWidth(2, 600);

        populateListView(); // example build model and set in list

        basicListView->Bind(wxEVT_LIST_COL_CLICK, &ListFrame::sortByColumn, this);

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
        insertListItem(20, "A-Some Item------------", "foo");

        // do rest in convinience function
        insertListItem(25, "B-Another Item----", "bar");
        insertListItem(10, "D-some item", "blob");
        insertListItem(8, "C-big", "max power");
    }

    // Build view and model from raw data
    void insertListItem(int id, const wxString& name, const wxString& desc) {

        int index = basicListView->GetItemCount(); // insert at end of list (see above notes)

        // View  (wxListView manages container for multiple items to display)
        basicListView->InsertItem(index, to_string(id));
        basicListView->SetItem(index, 1, name);
        basicListView->SetItem(index, 2, desc);


        // Model
        ItemData data{ id, name, desc }; // corresponding data item
        auto dataPtr = std::make_unique<ItemData>(data); // single-ownership pointer to (local) data
        //MUST SET PTR DATA SO CALL SetItemPtrData - NOT JUST AN INT 
        basicListView->SetItemPtrData(index, reinterpret_cast<wxUIntPtr>(dataPtr.get())); // set pointer as listview item client/meta-data

        // okay, can only store pointer in client data, but need to 
        // avoid dataPtr going out of scope at end of function - so we're essentially
        // maintaining our own list outside wxWidgets.  Add here...
        // NOT A FULL MODEL - NO SORTING OCCURS ON THIS MODEL ONLY IN VIEW
        itemCollection.insert(std::move(dataPtr)); // ownership passes to itemCollection (move utility func to force dataPtr to rValue)
    }


    // Function to sort by a given column (list event handler)
    void sortByColumn(wxListEvent& event) {

        int foo = 10;

        auto bar = reinterpret_cast<wxIntPtr*>(foo);

        int columnIndex = event.GetColumn();

        static auto idSort = [](wxIntPtr item1, wxIntPtr item2, wxIntPtr direction)->int {

            int i1 = reinterpret_cast<ItemData*>(item1)->id;
            int i2 = reinterpret_cast<ItemData*>(item2)->id;
            int dir = static_cast<int>(direction);

            return (i1 == i2) ? 0 : (i1 < i2) ? -dir : dir;
            };

        static auto nameSort = [](wxIntPtr item1, wxIntPtr item2, wxIntPtr direction)->int {

            auto s1 = reinterpret_cast<ItemData*>(item1)->name;
            auto s2 = reinterpret_cast<ItemData*>(item2)->name;

            return s1.compare(s2) * static_cast<int>(direction);
            };

        static auto descriptionSort = [](wxIntPtr item1, wxIntPtr item2, wxIntPtr direction)->int {

            auto s1 = reinterpret_cast<ItemData*>(item1)->description;
            auto s2 = reinterpret_cast<ItemData*>(item2)->description;

            return s1.compare(s2) * static_cast<int>(direction);
            };

        switch (columnIndex) {
        case 0:
            basicListView->SortItems(idSort, sortDirection);
            break;
        case 1:
            basicListView->SortItems(nameSort, sortDirection);
            break;
        case 2:
            basicListView->SortItems(descriptionSort, sortDirection);
            break;
        default:
            return;
        }

        // swap direction
        sortDirection = -sortDirection;
    }


    // Depricated for this version
    void sortByID(wxCommandEvent& event) {

        // Sort list based on client (meta) data set with SetItemData method
        basicListView->SortItems(
            [](wxIntPtr item1, wxIntPtr item2, wxIntPtr direction)->int {

                // This version stores ItemData* in client/meta data - so need to pull out if we want functioning button
                int i1 = reinterpret_cast<ItemData*>(item1)->id;
                int i2 = reinterpret_cast<ItemData*>(item2)->id;

                if (i1 == i2) {

                    return 0; // same
                }
                else if (i1 < i2) {

                    return -direction; // sign of value determines order 
                }
                else { // i1 > i2 

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