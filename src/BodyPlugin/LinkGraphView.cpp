/**
   @author Shin'ichiro Nakaoka
*/

#include "LinkGraphView.h"
#include "BodySelectionManager.h"
#include <cnoid/RootItem>
#include <cnoid/Archive>
#include <cnoid/Link>
#include <cnoid/EigenUtil>
#include <cnoid/ViewManager>
#include "gettext.h"

using namespace std;
using namespace cnoid;


void LinkGraphView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<LinkGraphView>(
        N_("LinkGraphView"), N_("Link Trajectories"));
}


LinkGraphView::LinkGraphView()
    : graph(this)
{
    setDefaultLayoutArea(BottomCenterArea);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    static const char* xyzLabels[] = { "X", "Y", "Z" };
    static const char* rpyLabels[] = { "R", "P", "Y" };

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setSpacing(0);
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->setSpacing(0);
    vbox->addStretch();
    setupElementToggleSet(vbox, xyzToggles, xyzLabels, true);
    setupElementToggleSet(vbox, rpyToggles, rpyLabels, false);
    vbox->addStretch();
    
    hbox->addLayout(vbox);
    hbox->addWidget(&graph, 1);
    
    setLayout(hbox);

    rootItemConnection = 
        RootItem::instance()->sigSelectedItemsChanged().connect(
            [&](const ItemList<>& items){ onSelectedItemsChanged(items); });
}


void LinkGraphView::setupElementToggleSet
(QBoxLayout* box, ToggleToolButton toggles[], const char* labels[], bool isActive)
{
    for(int i=0; i < 3; ++i){
        toggles[i].setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        box->addWidget(&toggles[i]);
        toggles[i].setChecked(isActive);
        toggles[i].setText(labels[i]);

        toggleConnections.add(
            toggles[i].sigToggled().connect(
                [&](bool){ setupGraphWidget(); }));
    }
}


LinkGraphView::~LinkGraphView()
{
    rootItemConnection.disconnect();
    bodyItemConnections.disconnect();
}


QWidget* LinkGraphView::indicatorOnInfoBar()
{
    return &graph.statusLabel();
}


void LinkGraphView::onSelectedItemsChanged(ItemList<MultiSE3SeqItem> items)
{
    if(items.empty()){
        return;
    }

    if(itemInfos.size() == items.size()){
        bool unchanged = true;
        int i=0;
        for(list<ItemInfo>::iterator it = itemInfos.begin(); it != itemInfos.end(); ++it){
            if(it->item != items[i++]){
                unchanged = false;
                break;
            }
        }
        if(unchanged){
            return;
        }
    }
            
    itemInfos.clear();

    for(size_t i=0; i < items.size(); ++i){
        BodyItemPtr bodyItem = items[i]->findOwnerItem<BodyItem>();
        if(bodyItem){
            itemInfos.push_back(ItemInfo());
            list<ItemInfo>::iterator it = --itemInfos.end();
            it->item = items[i];
            it->seq = it->item->seq();
            it->bodyItem = bodyItem;

            it->connections.add(
                it->item->sigUpdated().connect(
                    [=](){ onDataItemUpdated(it); }));

            it->connections.add(
                it->item->sigDisconnectedFromRoot().connect(
                    [=](){ onDataItemDisconnectedFromRoot(it); }));
        }
    }

    updateBodyItems();
    setupGraphWidget();
}


void LinkGraphView::onDataItemDisconnectedFromRoot(std::list<ItemInfo>::iterator itemInfoIter)
{
    itemInfos.erase(itemInfoIter);
    updateBodyItems();
    setupGraphWidget();
}


void LinkGraphView::updateBodyItems()
{
    bodyItemConnections.disconnect();
    bodyItems.clear();
    
    for(list<ItemInfo>::iterator it = itemInfos.begin(); it != itemInfos.end(); ++it){

        set<BodyItemPtr>::iterator p = bodyItems.find(it->bodyItem);

        if(p == bodyItems.end()){

            bodyItems.insert(it->bodyItem);

            bodyItemConnections.add(
                BodySelectionManager::instance()->sigLinkSelectionChanged(it->bodyItem).connect(
                    [=](const std::vector<bool>&){ setupGraphWidget(); }));
            
            bodyItemConnections.add(
                it->bodyItem->sigDisconnectedFromRoot().connect(
                    [=](){ onBodyItemDisconnectedFromRoot(it->bodyItem); }));
        }
    }
}


void LinkGraphView::onBodyItemDisconnectedFromRoot(BodyItemPtr bodyItem)
{
    bool erased = false;
    list<ItemInfo>::iterator it = itemInfos.begin();
    while(it != itemInfos.end()){
        if(it->bodyItem == bodyItem){
            it = itemInfos.erase(it);
            erased = true;
        } else {
            ++it;
        }
    }
    if(erased){
        updateBodyItems();
        setupGraphWidget();
    }
}


void LinkGraphView::setupGraphWidget()
{
    auto bsm = BodySelectionManager::instance();
    graph.clearDataHandlers();

    for(list<ItemInfo>::iterator it = itemInfos.begin(); it != itemInfos.end(); ++it){
        if(it->bodyItem){
            auto seq = it->item->seq();
            int numParts = seq->numParts();
            auto body = it->bodyItem->body();
            auto linkSelection = bsm->linkSelection(it->bodyItem);
            for(size_t i=0; i < linkSelection.size(); ++i){
                if(linkSelection[i]){
                    if(auto link = body->link(i)){
                        if(link->index() < numParts){
                            addPositionTrajectory(it, link, seq);
                        }
                    }
                }
            }
        }
    }
}


void LinkGraphView::addPositionTrajectory
(std::list<ItemInfo>::iterator itemInfoIter, Link* link, std::shared_ptr<MultiSE3Seq> seq)
{
    
    for(int i=0; i < 2; ++i){
        ToggleToolButton* toggles = (i == 0) ? xyzToggles : rpyToggles;
        for(int j=0; j < 3; ++j){

            if(toggles[j].isChecked()){
    
                GraphDataHandlerPtr handler(new GraphDataHandler());

                handler->setLabel(link->name());
            
                handler->setFrameProperties(seq->numFrames(), seq->frameRate());
                handler->setDataRequestCallback(
                    [=](int frame, int size, double* out_values){
                        onDataRequest(itemInfoIter, link->index(), i, j, frame, size, out_values); });
                handler->setDataModifiedCallback(
                    [=](int frame, int size, double* values){
                        onDataModified(itemInfoIter, link->index(), i, j, frame, size, values); });
            
                graph.addDataHandler(handler);
                itemInfoIter->handlers.push_back(handler);
            }
        }
    }
}


void LinkGraphView::onDataItemUpdated(std::list<ItemInfo>::iterator itemInfoIter)
{
    auto seq = itemInfoIter->item->seq();
    int newNumFrames = seq->numFrames();
    double newFrameRate = seq->frameRate();
    
    for(size_t i=0; i < itemInfoIter->handlers.size(); ++i){
        itemInfoIter->handlers[i]->setFrameProperties(newNumFrames, newFrameRate);
        itemInfoIter->handlers[i]->update();
    }
}


void LinkGraphView::onDataRequest
(std::list<ItemInfo>::iterator itemInfoIter, int linkIndex, int type, int axis, int frame, int size, double* out_values)
{
    MultiSE3Seq::Part part = itemInfoIter->seq->part(linkIndex);
    if(type == 0){ // xyz
        for(int i=0; i < size; ++i){
            const SE3& p = part[frame + i];
            out_values[i] = p.translation()[axis];
        }
    } else { // rpy
        for(int i=0; i < size; ++i){
            const SE3& p = part[frame + i];
            out_values[i] = rpyFromRot(Matrix3(p.rotation()))[axis];
        }
    }
}


void LinkGraphView::onDataModified
(std::list<ItemInfo>::iterator itemInfoIter, int linkIndex, int type, int axis, int frame, int size, double* values)
{
    MultiSE3Seq::Part part = itemInfoIter->seq->part(linkIndex);
    if(type == 0){ // xyz
        for(int i=0; i < size; ++i){
            SE3& p = part[frame + i];
            p.translation()[axis] = values[i];
        }
    } else { // rpy
        for(int i=0; i < size; ++i){
            SE3& p = part[frame + i];
            Vector3 rpy(rpyFromRot(Matrix3(p.rotation())));
            rpy[axis] = values[i];
            p.rotation() = rotFromRpy(rpy);
        }
    }        
    
    itemInfoIter->connections.block();
    itemInfoIter->item->notifyUpdate();
    itemInfoIter->connections.unblock();
}


bool LinkGraphView::storeState(Archive& archive)
{
    if(graph.storeState(archive)){
        Listing& visibleElements = *archive.createFlowStyleListing("visibleElements");
        for(int i=0; i < 3; ++i){
            if(xyzToggles[i].isChecked()){
                visibleElements.append(i);
            }
        }
        for(int i=0; i < 3; ++i){
            if(rpyToggles[i].isChecked()){
                visibleElements.append(i+3);
            }
        }
        return true;
    }
    return false;
}


bool LinkGraphView::restoreState(const Archive& archive)
{
    if(graph.restoreState(archive)){
        const Listing& visibleElements = *archive.findListing("visibleElements");
        if(visibleElements.isValid()){
            auto block = toggleConnections.scopedBlock();
            for(int i=0; i < 3; ++i){
                xyzToggles[i].setChecked(false);
                rpyToggles[i].setChecked(false);
            }
            for(int i=0; i < visibleElements.size(); ++i){
                int index = visibleElements[i].toInt();
                if(index < 3){
                    xyzToggles[index].setChecked(true);
                } else {
                    rpyToggles[index-3].setChecked(true);
                }
            }
        }
        return true;
    }
    return false;
}
