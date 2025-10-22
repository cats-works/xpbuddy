#include <string>
#include <iostream>
#include <filesystem>

#include "acsfile.h"

using namespace std;

void PrintACF(string filename)
{
    libacsfile::Character *acs = new libacsfile::Character;
    try {
        // Initialize the ACFFile class and load the file
        acs->Load(filename);
        if(acs->Loaded())
        {
            cout << "Character GUID: " << acs->GUID() << endl;
            cout << "Character Name: " << acs->Name() << endl;
            cout << "Description: " << acs->Description() << endl;
            cout << "Width: " << acs->Width() << endl;
            cout << "Height: " << acs->Height() << endl;
            cout << "TTS Enabled: " << acs->TTSEnabled() << endl;
            if(acs->TTSEnabled())
            {
                cout << "TTS Style: " << acs->Style() << endl;
                cout << "TTS Age: " << acs->Age() << endl;
            }
            cout << "Balloon Enabled: " << acs->BalloonEnabled() << endl;
            if(acs->BalloonEnabled())
            {
                cout << "Balloon Font: " << acs->BalloonFont() << endl;

            }
            cout << acs->States().size() << "Available States: " << endl;
            for(auto &i : acs->States())
            {
                cout << "  " << i.first << " " << endl;
            }
            cout << endl;
            cout << acs->Animations().size() << " Available Animations: " << endl;
            for(auto &[k,a] : acs->Animations())
            {
                cout << "  " << k << endl;
                std::string retTransType;
                switch(a->Transition())
                {
                case libacsfile::Animation::TransitionReturnAnimation:
                    retTransType = "Return Animation";
                    break;
                case libacsfile::Animation::TransitionExitBranches:
                    retTransType = "Exit Branches";
                    break;
                case libacsfile::Animation::TransitionNone:
                    retTransType = "None";
                    break;

                }

                cout << "    Return Transition Type: " << retTransType << endl;
                if(a->ReturnAnimation().size() > 0)
                    cout << "    Return Animation: " << a->ReturnAnimation() << endl;

                cout << "    Frame Count: " << a->Frames().size() << endl;
                for(auto &[id,fr] : a->Frames())
                {
                    cout << "      Frame" << id << " " << fr->Images().size()
                         << (fr->Images().size() == 1 ? " image " : " images ")
                         << fr->Duration() << "ms duration" << endl;
                    for(auto &fimg : fr->Images())
                    {
                        cout << "        Image" << fimg->GetImageID() << " offset ("
                             << fimg->OffsetX() << ","
                             << fimg->OffsetY() << ")" << endl;
                    }
                    for(auto &branch : fr->Branches())
                    {
                        cout << "        Branch to Frame" << branch->FrameID() << " with " << branch->Probability() << " probability.\n";
                    }
                }
            }
            namespace fs = std::filesystem;

            fs::path outDir = acs->Name() + " ImageOutput";
            if (!fs::exists(outDir)) {
                fs::create_directory(outDir);
            }

            cout << acs->Images().size() << " Available Images: " << endl;
            for(auto &[k,a] : acs->Images())
            {
                cout << "  Image" << k << (a->Compressed() ? " C " : " U ") << a->Size() << "B" << endl;
                fs::path imagefn = outDir / ("Image" + std::to_string(k) + ".bmp");
                a->WriteToFile(imagefn);
            }
            cout << endl;
        }
    } catch (const exception& e) {
        cerr << "Error loading ACS file: " << e.what() << endl;
    }
    delete acs;
}

int main() {
    //PrintACF("/Users/cat/CLIPPIT.ACS");
    PrintACF("/Users/cat/Bonzi.ACS");
    //PrintACF("/Users/cat/Merlin.ACS");

    return 0;
}
