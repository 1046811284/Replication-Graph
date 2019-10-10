#pragma once

#include "CoreMinimal.h"

#include "ReplicationGraph.h"

#include "RTSReplicationGraph.generated.h"

class URTSReplicationGraphNode_Vision;

/** Replication graph based on real-time strategy team vision. */
UCLASS(transient, config = Engine)//Transient 说明符意味着，它不会保存或从磁盘加载；它就是一个派生的非持久值，所以没有必要存储它
class REPGRAPH_API URTSReplicationGraph : public UReplicationGraph
{
    GENERATED_BODY()

public:
	//迭代所有的类:  设置每个列的==>复制时间间隔
    virtual void InitGlobalActorClassSettings() override;
	//1)设置节点:  一直复制,的Actor
    virtual void InitGlobalGraphNodes() override;
	//2)设置节点: 一直复制到指定连接的Actor:
	//3)设置节点: 可见才复制到指定连接的Actor
    virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	//从图标中:  从指定的列表中==>添加要复制的actor
    virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
                                             FGlobalActorReplicationInfo& GlobalInfo) override;
	//从图标中:  从指定的列表中==>删除不复制的actor
    virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;


	//服务器切换关卡时:  清除无效Actor的复制 ==> 防止崩溃
    virtual int32 ServerReplicateActors(float DeltaSeconds) override;

private:
	//1列表: 一直复制到所有链接
    /** Actors that are always replicated, to everyone. */
    UPROPERTY()
    UReplicationGraphNode_ActorList* AlwaysRelevantNode;

	//2列表: actor一直复制到:  指定的几个链接
    /** Actors that are always replicated, for specific connections. */
    UPROPERTY()
    TMap<UNetConnection*, UReplicationGraphNode_AlwaysRelevant_ForConnection*> AlwaysRelevantForConnectionNodes;

	//3列表: actor复制到指定连接,  并且是可见的链接:
    /** Actors that are replicated, for specific connections, based on vision. */
    UPROPERTY()
    TMap<UNetConnection*, URTSReplicationGraphNode_Vision*> VisionForConnectionNodes;

    /** Cleans up the specified node to avoid crashes due to invalid NetGUIDs after seamless travel. */
    void CleanupNodeAfterSeamlessTravel(UReplicationGraphNode_ActorList* Node);

    /** Writes the current graph to the log. */
    void DebugLogGraph();
};
