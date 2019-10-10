#pragma once

#include "CoreMinimal.h"

#include "ReplicationGraph.h"

#include "RTSReplicationGraphNode_Vision.generated.h"

//复制actor:  当有URTSVisibleComponent组件, 并且可见时
/** Replicates actors to connections based on whether the actor is visible for that connection. */
UCLASS()
class REPGRAPH_API URTSReplicationGraphNode_Vision : public UReplicationGraphNode_ActorList  //更新列表: UReplicationGraphNode_ActorList
{
    GENERATED_BODY()

public:
    virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	//添加到列表:
    /** Adds the specified actor that could potentially become visible or hidden. */
    void AddVisibleActor(const FNewReplicatedActorInfo& ActorInfo);

	//从列表删除:
    /** Removes the specified actor that could potentially become visible or hidden. */
    void RemoveVisibleActor(const FNewReplicatedActorInfo& ActorInfo);

private:
    /** Actors that could potentially become visible or hidden. */
    FActorRepListRefView VisibleActorList;

    /** Rebuilt-every-frame list based on vision. */
    FActorRepListRefView ReplicationActorList;
};
