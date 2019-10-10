#include "RTSReplicationGraph.h"

#include "RepGraph.h"

#include "ReplicationGraphTypes.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectIterator.h"

#include "RTSReplicationGraphNode_Vision.h"
#include "RTSVisibleComponent.h"


void URTSReplicationGraph::InitGlobalActorClassSettings()
{
    Super::InitGlobalActorClassSettings();

	//迭代所有的类:  设置每个列的==>复制时间间隔
    // First, we need to build a list of all classes that will ever be replicated.
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        UClass* Class = *ClassIt;

        // Check if it's an actor at all, and if so, if it's replicated.
        AActor* ActorCDO = Cast<AActor>(Class->GetDefaultObject());

		//1)这个类: 不可复制或无效 ==>不用设置
        if (!ActorCDO || !ActorCDO->GetIsReplicated())
        {
            continue;
        }

		//2)SKEL_ 和 REINST_开头的类: 不用设置
        // Skip SKEL and REINST classes.
        if (Class->GetName().StartsWith(TEXT("SKEL_")) || Class->GetName().StartsWith(TEXT("REINST_")))
        {
            continue;
        }

        FClassReplicationInfo ClassInfo;

		//根据类自己的update频率和服务器帧数==>设置复制复制时间间隔:
        // Replication Graph is frame based. Convert NetUpdateFrequency to ReplicationPeriodFrame based on Server
        // MaxTickRate.
        ClassInfo.ReplicationPeriodFrame = FMath::Max<uint32>(
            (uint32)FMath::RoundToFloat(NetDriver->NetServerMaxTickRate / ActorCDO->NetUpdateFrequency), 1);
		//设置这个类的,复制时间间隔:
        GlobalActorReplicationInfoMap.SetClassInfo(Class, ClassInfo);
    }
}

//1)设置节点:  一直复制,的Actor
void URTSReplicationGraph::InitGlobalGraphNodes()
{
    Super::InitGlobalGraphNodes();

    // Preallocate some replication lists.
    PreAllocateRepList(3, 12);
    PreAllocateRepList(6, 12);
    PreAllocateRepList(128, 64);

    // Replication lists will lazily be created as well, if necessary.
    // However, lazily allocating very large lists results in an error.
    PreAllocateRepList(8192, 2);

	//1)设置节点:  一直复制,的Actor
    // Setup node for actors that are always replicated, to everyone.
    AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
    AddGlobalGraphNode(AlwaysRelevantNode);
}


//2)设置节点: 一直复制到指定连接的Actor:
//3)设置节点: 可见才复制到指定连接的Actor
void URTSReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
{
    Super::InitConnectionGraphNodes(RepGraphConnection);

	//2)设置节点: 一直复制到指定连接的Actor:
    // Setup node for actors that are always replicated, for the specified connection.
    UReplicationGraphNode_AlwaysRelevant_ForConnection* AlwaysRelevantNodeForConnection = CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
		//加入Graph:
    AddConnectionGraphNode(AlwaysRelevantNodeForConnection, RepGraphConnection);
		//加入列表:
    AlwaysRelevantForConnectionNodes.Add(RepGraphConnection->NetConnection, AlwaysRelevantNodeForConnection);


	//3)设置节点: 可见才复制到指定连接的Actor
    // Setup node for actors that are replicated, for the specified connection, based on vision.
    URTSReplicationGraphNode_Vision* VisionNode = CreateNewNode<URTSReplicationGraphNode_Vision>();
		//加入Graph:
    AddConnectionGraphNode(VisionNode, RepGraphConnection);
		//加入列表:
    VisionForConnectionNodes.Add(RepGraphConnection->NetConnection, VisionNode);
}

//从图标中:  从指定的列表中==>添加要复制的actor
void URTSReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo,
                                                      FGlobalActorReplicationInfo& GlobalInfo)
{
	//1)包含URTSVisibleComponent组件 ==>添加到VisionForConnectionNodes列表
    if (ActorInfo.Actor->FindComponentByClass<URTSVisibleComponent>())
    {
		//只要包含就加入列表, 然后是否可见==> 会进一步判断
        // Add for all connections. Every node will decide for themselves whether to replicate, or not.
        for (auto& ConnectionWithNode : VisionForConnectionNodes)
        {
            ConnectionWithNode.Value->AddVisibleActor(FNewReplicatedActorInfo(ActorInfo.Actor));
        }
    }
	//2)勾选了一直相关bAlwaysRelevant:  加入AlwaysRelevantNode列表
    else if (ActorInfo.Actor->bAlwaysRelevant)
    {
        // Add for everyone.
        AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
    }
	//3)勾选了bOnlyRelevantToOwner:  加入AlwaysRelevantForConnectionNodes列表
    else if (ActorInfo.Actor->bOnlyRelevantToOwner)
    {
        // Add for owning connection.
        if (UReplicationGraphNode_AlwaysRelevant_ForConnection* Node =
                AlwaysRelevantForConnectionNodes.FindRef(ActorInfo.Actor->GetNetConnection()))
        {
            Node->NotifyAddNetworkActor(FNewReplicatedActorInfo(ActorInfo.Actor));
        }
        else
        {
            UE_LOG(LogRTS, Error,
                   TEXT("URTSReplicationGraph::RouteAddNetworkActorToNodes - No AlwaysRelevant_ForConnection node "
                        "matches for %s."),
                   *ActorInfo.Actor->GetName());
        }
    }
    else
    {
        UE_LOG(LogRTS, Error,
               TEXT("URTSReplicationGraph::RouteAddNetworkActorToNodes - No replication graph node matches for %s."),
               *ActorInfo.Actor->GetName());

        // Fall back to always relevant.
        AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);
    }
}

//从图标中:  从指定的列表中==>删除不复制的actor
void URTSReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	//1)这个actor是否包含了URTSVisibleComponent(可自定义可见规则)组件:  ==> 从VisionForConnectionNodes列表中删除
    if (ActorInfo.Actor->FindComponentByClass<URTSVisibleComponent>())
    {
        for (auto& ConnectionWithNode : VisionForConnectionNodes)
        {
            ConnectionWithNode.Value->RemoveVisibleActor(FNewReplicatedActorInfo(ActorInfo.Actor));
        }
    }
	//2)是bAlwaysRelevant ==>  从AlwaysRelevantNode列表中删除
    else if (ActorInfo.Actor->bAlwaysRelevant)
    {
        AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
    }
	//3)是bOnlyRelevantToOwner ==>从AlwaysRelevantForConnectionNodes列表中删除
    else if (ActorInfo.Actor->bOnlyRelevantToOwner)
    {
		//判断Node(AlwaysRelevant的Node)中: 是否有这个actor
        if (UReplicationGraphNode* Node = AlwaysRelevantForConnectionNodes.FindRef(ActorInfo.Actor->GetNetConnection()))
        {
            Node->NotifyRemoveNetworkActor(ActorInfo);
        }
        else
        {
            UE_LOG(LogRTS, Error,
                   TEXT("URTSReplicationGraph::RouteRemoveNetworkActorToNodes - No AlwaysRelevant_ForConnection node "
                        "matches for %s."),
                   *ActorInfo.Actor->GetName());
        }
    }
    else
    {
        UE_LOG(LogRTS, Error,
               TEXT("URTSReplicationGraph::RouteRemoveNetworkActorToNodes - No replication graph node matches for %s."),
               *ActorInfo.Actor->GetName());
    }
}

int32 URTSReplicationGraph::ServerReplicateActors(float DeltaSeconds)
{
    // Manually remove invalid actors for all nodes. Fixes seamless travel crash.
    CleanupNodeAfterSeamlessTravel(AlwaysRelevantNode);

    for (auto& ConnectionAndNode : AlwaysRelevantForConnectionNodes)
    {
        CleanupNodeAfterSeamlessTravel(ConnectionAndNode.Value);
    }

    return Super::ServerReplicateActors(DeltaSeconds);
}

void URTSReplicationGraph::CleanupNodeAfterSeamlessTravel(UReplicationGraphNode_ActorList* Node)
{
    TArray<FActorRepListType> ActorList;
    Node->GetAllActorsInNode_Debugging(ActorList);

    for (AActor* Actor : ActorList)
    {
        if (!Actor->GetNetConnection() && !Actor->bAlwaysRelevant)
        {
            Node->NotifyRemoveNetworkActor(FNewReplicatedActorInfo(Actor));
            UE_LOG(LogRTS, Error, TEXT("URTSReplicationGraph::CleanupNodeAfterSeamlessTravel - %s removed from %s."),
                   *Actor->GetName(), *Node->GetName());
        }
    }
}

void URTSReplicationGraph::DebugLogGraph()
{
    FReplicationGraphDebugInfo DebugInfo(*GLog);
    DebugInfo.Flags = FReplicationGraphDebugInfo::ShowNativeClasses;
    LogGraph(DebugInfo);
}
