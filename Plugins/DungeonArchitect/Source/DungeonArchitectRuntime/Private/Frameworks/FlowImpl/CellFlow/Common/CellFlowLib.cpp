//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/FlowImpl/CellFlow/Common/CellFlowLib.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/FlowImpl/CellFlow/Common/CellFlowStats.h"

const FName UDAFlowCellGraph::StateTypeID = TEXT("CellGraphStateObject");
void UDAFlowCellGraph::CloneFromStateObject(const UObject* SourceObject) {
	const UDAFlowCellGraph* OtherGraph = Cast<UDAFlowCellGraph>(SourceObject);
	if (!OtherGraph) return;

	// Clone the group nodes. Since they are plain data objects, just copy them over
	GroupNodes = OtherGraph->GroupNodes;

	// Clone the leaf nodes
	LeafNodes.Reset();
	for (UDAFlowCellLeafNode* OtherLeafNode : OtherGraph->LeafNodes) {
		UDAFlowCellLeafNode* ClonedLeafNode = NewObject<UDAFlowCellLeafNode>(this, OtherLeafNode->GetClass(), NAME_None, RF_NoFlags, OtherLeafNode);
		LeafNodes.Add(ClonedLeafNode);
	}
}

namespace DA {
	void FCellGraphBuilder::CollapseEdges(UDAFlowCellGraph* InGraph, int MinGroupArea, const FRandomStream& Random, FCellAreaLookup& AreaLookup) {
		SCOPE_CYCLE_COUNTER(STAT_CFBuild_CollapseEdges);
		while (CollapsedBestGroupEdge(InGraph, MinGroupArea, Random, AreaLookup)) {}
	}
	
	void FCellGraphBuilder::AssignGroupColors(UDAFlowCellGraph* InGraph) {
		TArray<int> ActiveGroupIds;
		for (const FDAFlowCellGroupNode &GroupNode: InGraph->GroupNodes) {
			if (GroupNode.IsActive()) {
				ActiveGroupIds.Add(GroupNode.GroupId);
			}
		}

		const int NumActiveGroups = ActiveGroupIds.Num();
		if (NumActiveGroups > 0) {
			const float AngleDelta = 360.0f / static_cast<float>(NumActiveGroups);
			for (int i = 0; i < NumActiveGroups; i++) {
				const float Hue = AngleDelta * i;
				const int GroupId = ActiveGroupIds[i];
				FLinearColor HSV(Hue, 0.3f, 1.0f);
					
				InGraph->GroupNodes[GroupId].GroupColor = HSV.HSVToLinearRGB();
			}
		}
	}

	bool FCellGraphBuilder::CollapsedBestGroupEdge(UDAFlowCellGraph* InGraph, int MinGroupArea, const FRandomStream& Random, FCellAreaLookup& AreaLookup) {
		int GroupAId;
		{
			SCOPE_CYCLE_COUNTER(STAT_CFColl_PrepNodeGroups);
			if (!AreaLookup.GetGroupWithLeastArea(Random, GroupAId)) {
				return false;
			}

			if (AreaLookup.GetGroupArea(GroupAId) > MinGroupArea) {
				// No need to merge any further
				return false;
			}
		}
		
		const FDAFlowCellGroupNode& GroupA = InGraph->GroupNodes[GroupAId];

		int GroupBId = -1;
		TArray<int> ConnectedGroups;
		{
			SCOPE_CYCLE_COUNTER(STAT_CFColl_PrepConnGroups);
			TArray<int> ConnectionsA = GroupA.Connections.Array();
			if (ConnectionsA.Num() == 0) {
				return false;
			}

			float BestArea = MAX_flt;
			int BestConnectedGroup = -1;
			for (const int ConnectedGroupID: ConnectionsA) {
				const float ConnectedGroupArea = AreaLookup.GetGroupArea(ConnectedGroupID);
				if (ConnectedGroupArea < BestArea) {
					BestArea = ConnectedGroupArea;
					BestConnectedGroup = ConnectedGroupID;
				}
			}
			GroupBId = BestConnectedGroup;
		}

		check(GroupBId != -1);
		
		{
			SCOPE_CYCLE_COUNTER(STAT_CFColl_Merge);
			check(GroupAId != GroupBId);
			MergeGroups(InGraph, GroupAId, GroupBId, AreaLookup);
		}
		return true;
	}

	void FCellGraphBuilder::AssignGroupPreviewLocations(UDAFlowCellGraph* InGraph, const FCellAreaLookup& InAreaLookup) {
		SCOPE_CYCLE_COUNTER(STAT_CFBuild_AssignPreviewLoc);
		TArray<int> GroupCenterNode;
		GroupCenterNode.SetNum(InGraph->GroupNodes.Num());
		for (int i = 0; i < GroupCenterNode.Num(); i++) {
			GroupCenterNode[i] = -1;
		}
			
		for (int GroupId = 0; GroupId < InGraph->GroupNodes.Num(); GroupId++) {
			FDAFlowCellGroupNode& GroupNode = InGraph->GroupNodes[GroupId];
			if (GroupNode.LeafNodes.Num() > 0) {
				int BestLeafId = -1;
				
				FVector2D Center = FVector2D::ZeroVector;
				{
					float Sum = 0;
					for (const int LeafNodeId : GroupNode.LeafNodes) {
						const float Area = InAreaLookup.GetLeafArea(LeafNodeId); 
						Center += InGraph->LeafNodes[LeafNodeId]->GetCenter() * Area;
						Sum += Area;
					}
					Center /= Sum;
				}

				float BestDistance = MAX_flt;;
				for (const int LeafNodeId : GroupNode.LeafNodes) {
					const UDAFlowCellLeafNode* LeafNode = InGraph->LeafNodes[LeafNodeId];
					const float DistanceToCenter = (Center - LeafNode->GetCenter()).Size();
					if (DistanceToCenter < BestDistance) {
						BestDistance = DistanceToCenter;
						BestLeafId = LeafNodeId;
					}
				}
				GroupNode.PreviewLocation = InGraph->LeafNodes[BestLeafId]->GetCenter();
				GroupCenterNode[GroupId] = BestLeafId;
			}
		}
	}

	void FCellGraphBuilder::MergeGroups(UDAFlowCellGraph* InGraph, int GroupIdA, int GroupIdB, FCellAreaLookup& AreaLookup) {
		FDAFlowCellGroupNode& GroupA = InGraph->GroupNodes[GroupIdA];
		FDAFlowCellGroupNode& GroupB = InGraph->GroupNodes[GroupIdB];

		// Copy B to A and discard B
		for (const int GroupBLeafId : GroupB.LeafNodes) {
			GroupA.LeafNodes.Add(GroupBLeafId);
		}
		GroupB.LeafNodes.Reset();

		// Break all GroupB connections
		for (int ConnectedToB : GroupB.Connections) {
			InGraph->GroupNodes[ConnectedToB].Connections.Remove(GroupIdB);
			InGraph->GroupNodes[ConnectedToB].Connections.Add(GroupIdA);
			GroupA.Connections.Add(ConnectedToB);
		}
		GroupB.Connections.Reset();

		// Remove internal links (merged nodes do not connect to each other)
		TArray<int> ValidConnectionsA;
		for (int ConnectedCellID : GroupA.Connections) {
			if (!GroupA.LeafNodes.Contains(ConnectedCellID)) {
				ValidConnectionsA.Add(ConnectedCellID);
			}
		}
		GroupA.Connections = TSet(ValidConnectionsA);

		// Copy GroupB's area over ot A and then clear it on B
		const float MergedArea = AreaLookup.GetGroupArea(GroupIdA) + AreaLookup.GetGroupArea(GroupIdB); 
		AreaLookup.SetGroupArea(GroupIdA, MergedArea);
		AreaLookup.SetGroupArea(GroupIdB, 0);
	}

	////////////////////////////// FCellAreaLookup //////////////////////////////
	void FCellAreaLookup::Init(UDAFlowCellGraph* InGraph) {
		LeafAreas.SetNum(InGraph->LeafNodes.Num());
		for (int LeafId = 0; LeafId < InGraph->LeafNodes.Num(); LeafId++) {
			LeafAreas[LeafId] = InGraph->LeafNodes[LeafId]->GetArea();
		}

		GroupAreas.SetNum(InGraph->GroupNodes.Num());
		for (int GroupId = 0; GroupId < InGraph->GroupNodes.Num(); GroupId++) {
			float& GroupArea = GroupAreas[GroupId];
			GroupArea = 0;
			const TSet<int>& LeafNodeIds = InGraph->GroupNodes[GroupId].LeafNodes;
			for (const int LeafId : LeafNodeIds) {
				GroupArea += LeafAreas[LeafId];
			}
			if (GroupArea > 0) {
				TArray<int>& Groups = ActiveGroupIdsByArea.FindOrAdd(GroupArea);
				Groups.Add(GroupId);
			}
		}
	}

	void FCellAreaLookup::SetGroupArea(int GroupId, float NewArea) {
		const float OldArea = GroupAreas[GroupId];
		GroupAreas[GroupId] = NewArea;

		TArray<int>& OldAreaGroups = ActiveGroupIdsByArea.FindOrAdd(OldArea);
		OldAreaGroups.Remove(GroupId);
		if (OldAreaGroups.Num() == 0) {
			ActiveGroupIdsByArea.Remove(OldArea);
		}

		if (NewArea > 0) {
			TArray<int>& NewAreaGroups = ActiveGroupIdsByArea.FindOrAdd(NewArea);
			check(!NewAreaGroups.Contains(GroupId));
			NewAreaGroups.Add(GroupId);
		}
	}

	bool FCellAreaLookup::GetGroupWithLeastArea(const FRandomStream& InRandom, int& OutGroupId) const {
		float BestArea = MAX_flt;
		bool bFound = false;
		for (const auto& Entry : ActiveGroupIdsByArea) {
			const float Area = Entry.Key;
			const TArray<int>& GroupIds = Entry.Value;
			if (GroupIds.Num() == 0) {
				continue;
			}

			if (Area < BestArea) {
				BestArea = Area;
				bFound = true;
			}
		}

		if (!bFound) {
			OutGroupId = -1;
			return false;
		}

		const TArray<int>& Candidates = ActiveGroupIdsByArea[BestArea];
		OutGroupId = Candidates[InRandom.RandRange(0, Candidates.Num() - 1)];
		return true;
	}

}

