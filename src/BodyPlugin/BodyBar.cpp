/**
   @author Shin'ichiro Nakaoka
*/

#include "BodyBar.h"
#include "BodyItem.h"
#include "BodySelectionManager.h"
#include <cnoid/RootItem>
#include <cnoid/Archive>
#include "gettext.h"

using namespace std;
using namespace cnoid;

namespace cnoid {

class BodyBar::Impl
{
public:
    BodySelectionManager* bodySelectionManager;

    Impl(BodyBar* self);
    void applyBodyItemOperation(std::function<void(BodyItem* bodyItem)> func);
    void onCopyButtonClicked();
    void onPasteButtonClicked();
    void onOriginButtonClicked();
    void onPoseButtonClicked(BodyItem::PresetPoseID id);
    void onSymmetricCopyButtonClicked(int direction, bool doMirrorCopy);
    void doSymmetricCopy(BodyItem* bodyItem, int direction, bool doMirrorCopy);
};

}


BodyBar* BodyBar::instance()
{
    static BodyBar* instance = new BodyBar;
    return instance;
}


BodyBar::BodyBar()
    : ToolBar(N_("BodyBar"))
{
    impl = new Impl(this);
}


BodyBar::Impl::Impl(BodyBar* self)
{
    ToolButton* button;
    
    button = self->addButton(QIcon(":/Body/icon/storepose.svg"));
    button->setToolTip(_("Memory the current pose"));
    button->sigClicked().connect([&](){ onCopyButtonClicked(); });

    button = self->addButton(QIcon(":/Body/icon/restorepose.svg"));
    button->setToolTip(_("Recall the memorized pose"));
    button->sigClicked().connect([&](){ onPasteButtonClicked(); });
    
    button = self->addButton(QIcon(":/Body/icon/origin.svg"));
    button->setToolTip(_("Move the selected bodies to the origin"));
    button->sigClicked().connect([&](){ onOriginButtonClicked(); });

    button = self->addButton(QIcon(":/Body/icon/initialpose.svg"));
    button->setToolTip(_("Set the preset initial pose to the selected bodies"));
    button->sigClicked().connect([&](){ onPoseButtonClicked(BodyItem::INITIAL_POSE); });

    button = self->addButton(QIcon(":/Body/icon/stdpose.svg"));
    button->setToolTip(_("Set the preset standard pose to the selected bodies"));
    button->sigClicked().connect([&](){ onPoseButtonClicked(BodyItem::STANDARD_POSE); });

    button = self->addButton(QIcon(":/Body/icon/right-to-left.svg"));
    button->setToolTip(_("Copy the right side pose to the left side"));
    button->sigClicked().connect([&](){ onSymmetricCopyButtonClicked(1, false); });

    button = self->addButton(QIcon(":/Body/icon/flip.svg"));
    button->setToolTip(_("Mirror copy"));
    button->sigClicked().connect([&](){ onSymmetricCopyButtonClicked(0, true); });

    button = self->addButton(QIcon(":/Body/icon/left-to-right.svg"));
    button->setToolTip(_("Copy the left side pose to the right side"));
    button->sigClicked().connect([&](){ onSymmetricCopyButtonClicked(0, false); });

    bodySelectionManager = BodySelectionManager::instance();
}


BodyBar::~BodyBar()
{
    delete impl;
}


void BodyBar::Impl::applyBodyItemOperation(std::function<void(BodyItem* bodyItem)> func)
{
    auto& selected = bodySelectionManager->selectedBodyItems();
    if(!selected.empty()){
        for(auto& bodyItem: selected){
            func(bodyItem);
        }
    } else if(auto current = bodySelectionManager->currentBodyItem()){
        func(current);
    }
}


void BodyBar::Impl::onCopyButtonClicked()
{
    applyBodyItemOperation([](BodyItem* bodyItem){ bodyItem->copyKinematicState(); });
}


void BodyBar::Impl::onPasteButtonClicked()
{
    applyBodyItemOperation([](BodyItem* bodyItem){ bodyItem->pasteKinematicState(); });
}


void BodyBar::Impl::onOriginButtonClicked()
{
    applyBodyItemOperation([](BodyItem* bodyItem){ bodyItem->moveToOrigin(); });
}


void BodyBar::Impl::onPoseButtonClicked(BodyItem::PresetPoseID id)
{
    applyBodyItemOperation([id](BodyItem* bodyItem){ bodyItem->setPresetPose(id); });
}


void BodyBar::Impl::onSymmetricCopyButtonClicked(int direction, bool doMirrorCopy)
{
    applyBodyItemOperation(
        [this, direction, doMirrorCopy](BodyItem* bodyItem){
            doSymmetricCopy(bodyItem, direction, doMirrorCopy);
        });
}


void BodyBar::Impl::doSymmetricCopy(BodyItem* bodyItem, int direction, bool doMirrorCopy)
{
    const Listing& slinks = *bodyItem->body()->info()->findListing("symmetricJoints");
    if(slinks.isValid() && !slinks.empty()){
        int from = direction;
        int to = 1 - direction;
        BodyPtr body = bodyItem->body();
        for(int j=0; j < slinks.size(); ++j){
            const Listing& linkPair = *slinks[j].toListing();
            if(linkPair.size() == 1 && doMirrorCopy){
                Link* link = body->link(linkPair[0].toString());
                if(link){
                    link->q() = -link->q();
                }
            } else if(linkPair.size() >= 2){
                Link* link1 = body->link(linkPair[from].toString());
                Link* link2 = body->link(linkPair[to].toString());
                if(link1 && link2){
                    double sign = 1.0;
                    if(linkPair.size() >= 3){
                        sign = linkPair[2].toDouble();
                    }
                    if(doMirrorCopy){
                        double q1 = link1->q();
                        link1->q() = sign * link2->q();
                        link2->q() = sign * q1;
                    } else {
                        link2->q() = sign * link1->q();
                    }
                }
            }
        }
        bodyItem->body()->calcForwardKinematics();
        bodyItem->notifyKinematicStateUpdate();
    }
}
