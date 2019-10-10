#pragma once

#include "CoreMinimal.h"

#include "ReplicationGraph.h"

#include "RTSReplicationGraph.generated.h"

class URTSReplicationGraphNode_Vision;

/** Replication graph based on real-time strategy team vision. */
UCLASS(transient, config = Engine)//Transient ˵������ζ�ţ������ᱣ���Ӵ��̼��أ�������һ�������ķǳ־�ֵ������û�б�Ҫ�洢��
class REPGRAPH_API URTSReplicationGraph : public UReplicationGraph
{
    GENERATED_BODY()

public:
	//�������е���:  ����ÿ���е�==>����ʱ����
    virtual void InitGlobalActorClassSettings() override;
	//1)���ýڵ�:  һֱ����,��Actor
    virtual void InitGlobalGraphNodes() override;
	//2)���ýڵ�: һֱ���Ƶ�ָ�����ӵ�Actor:
	//3)���ýڵ�: �ɼ��Ÿ��Ƶ�ָ�����ӵ�Actor
    virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	//��ͼ����:  ��ָ�����б���==>���Ҫ���Ƶ�actor
    virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
                                             FGlobalActorReplicationInfo& GlobalInfo) override;
	//��ͼ����:  ��ָ�����б���==>ɾ�������Ƶ�actor
    virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;


	//�������л��ؿ�ʱ:  �����ЧActor�ĸ��� ==> ��ֹ����
    virtual int32 ServerReplicateActors(float DeltaSeconds) override;

private:
	//1�б�: һֱ���Ƶ���������
    /** Actors that are always replicated, to everyone. */
    UPROPERTY()
    UReplicationGraphNode_ActorList* AlwaysRelevantNode;

	//2�б�: actorһֱ���Ƶ�:  ָ���ļ�������
    /** Actors that are always replicated, for specific connections. */
    UPROPERTY()
    TMap<UNetConnection*, UReplicationGraphNode_AlwaysRelevant_ForConnection*> AlwaysRelevantForConnectionNodes;

	//3�б�: actor���Ƶ�ָ������,  �����ǿɼ�������:
    /** Actors that are replicated, for specific connections, based on vision. */
    UPROPERTY()
    TMap<UNetConnection*, URTSReplicationGraphNode_Vision*> VisionForConnectionNodes;

    /** Cleans up the specified node to avoid crashes due to invalid NetGUIDs after seamless travel. */
    void CleanupNodeAfterSeamlessTravel(UReplicationGraphNode_ActorList* Node);

    /** Writes the current graph to the log. */
    void DebugLogGraph();
};
