/**
   @author Shin'ichiro Nakaoka
*/

#include "JointGraphView.h"
#include "BodySelectionManager.h"
#include <cnoid/RootItem>
#include <cnoid/Archive>
#include <cnoid/Link>
#include <cnoid/ViewManager>
#include <QBoxLayout>
#include "gettext.h"

using namespace std;
using namespace cnoid;


void JointGraphView::initializeClass(ExtensionManager* ext)
{
    ext->viewManager().registerClass<JointGraphView>(
        N_("JointGraphView"), N_("Joint Trajectories"));
}


JointGraphView::JointGraphView()
    : graph(this)
{
    setDefaultLayoutArea(BottomCenterArea);
    
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(&graph);
    setLayout(vbox);

    rootItemConnection = 
        RootItem::instance()->sigSelectedItemsChanged().connect(
            [&](const ItemList<>& selectedItems){
                onSelectedItemsChanged(selectedItems); });
}


JointGraphView::~JointGraphView()
{
    rootItemConnection.disconnect();
    bodyItemConnections.disconnect();
}


QWidget* JointGraphView::indicatorOnInfoBar()
{
    return &graph.statusLabel();
}


void JointGraphView::onSelectedItemsChanged(ItemList<MultiValueSeqItem> items)
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
                    [=]{ onDataItemUpdated(it); }));

            it->connections.add(
                it->item->sigDisconnectedFromRoot().connect(
                    [=](){ onDataItemDisconnectedFromRoot(it); }));
        }
    }

    updateBodyItems();
    setupGraphWidget();
}


void JointGraphView::onDataItemDisconnectedFromRoot(std::list<ItemInfo>::iterator itemInfoIter)
{
    itemInfos.erase(itemInfoIter);
    updateBodyItems();
    setupGraphWidget();
}


void JointGraphView::updateBodyItems()
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


void JointGraphView::onBodyItemDisconnectedFromRoot(BodyItemPtr bodyItem)
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


void JointGraphView::setupGraphWidget()
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
                        int id = link->jointId();
                        if(id >= 0 && id < numParts){
                            addJointTrajectory(it, link, seq);
                        }
                    }
                }
            }
        }
    }
}


void JointGraphView::addJointTrajectory
(std::list<ItemInfo>::iterator itemInfoIter, Link* joint, std::shared_ptr<MultiValueSeq> seq)
{
    GraphDataHandlerPtr handler(new GraphDataHandler());

    handler->setLabel(joint->name());
    handler->setValueLimits(joint->q_lower(), joint->q_upper());
    handler->setVelocityLimits(joint->dq_lower(), joint->dq_upper());
                
    handler->setFrameProperties(seq->numFrames(), seq->frameRate());

    handler->setDataRequestCallback(
        [=](int frame, int size, double* out_values){
            onDataRequest(itemInfoIter, joint->jointId(), frame, size, out_values); });
    
    handler->setDataModifiedCallback(
        [=](int frame, int size, double* values){
            onDataModified(itemInfoIter, joint->jointId(), frame, size, values); });
                
    graph.addDataHandler(handler);
    itemInfoIter->handlers.push_back(handler);
}


void JointGraphView::onDataItemUpdated(std::list<ItemInfo>::iterator itemInfoIter)
{
    auto seq = itemInfoIter->item->seq();
    int newNumFrames = seq->numFrames();
    double newFrameRate = seq->frameRate();
    
    for(size_t i=0; i < itemInfoIter->handlers.size(); ++i){
        itemInfoIter->handlers[i]->setFrameProperties(newNumFrames, newFrameRate);
        itemInfoIter->handlers[i]->update();
    }
}


void JointGraphView::onDataRequest
(std::list<ItemInfo>::iterator itemInfoIter, int jointId, int frame, int size, double* out_values)
{
    MultiValueSeq::Part part = itemInfoIter->seq->part(jointId);
    for(int i=0; i < size; ++i){
        out_values[i] = part[frame + i];
    }
}


void JointGraphView::onDataModified
(std::list<ItemInfo>::iterator itemInfoIter, int jointId, int frame, int size, double* values)
{
    MultiValueSeq::Part part = itemInfoIter->seq->part(jointId);
    for(int i=0; i < size; ++i){
        part[frame + i] = values[i];
    }
    
    itemInfoIter->connections.block();
    itemInfoIter->item->notifyUpdate();
    itemInfoIter->connections.unblock();
}


bool JointGraphView::storeState(Archive& archive)
{
    return graph.storeState(archive);
}


bool JointGraphView::restoreState(const Archive& archive)
{
    return graph.restoreState(archive);
}
