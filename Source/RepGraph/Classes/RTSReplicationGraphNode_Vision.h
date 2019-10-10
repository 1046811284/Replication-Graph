#pragma once

#include "CoreMinimal.h"

#include "ReplicationGraph.h"

#include "RTSReplicationGraphNode_Vision.generated.h"

//����actor:  ����URTSVisibleComponent���, ���ҿɼ�ʱ
/** Replicates actors to connections based on whether the actor is visible for that connection. */
UCLASS()
class REPGRAPH_API URTSReplicationGraphNode_Vision : public UReplicationGraphNode_ActorList  //�����б�: UReplicationGraphNode_ActorList
{
    GENERATED_BODY()

public:
    virtual void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params) override;

	//��ӵ��б�:
    /** Adds the specified actor that could potentially become visible or hidden. */
    void AddVisibleActor(const FNewReplicatedActorInfo& ActorInfo);

	//���б�ɾ��:
    /** Removes the specified actor that could potentially become visible or hidden. */
    void RemoveVisibleActor(const FNewReplicatedActorInfo& ActorInfo);

private:
    /** Actors that could potentially become visible or hidden. */
    FActorRepListRefView VisibleActorList;

    /** Rebuilt-every-frame list based on vision. */
    FActorRepListRefView ReplicationActorList;
};
