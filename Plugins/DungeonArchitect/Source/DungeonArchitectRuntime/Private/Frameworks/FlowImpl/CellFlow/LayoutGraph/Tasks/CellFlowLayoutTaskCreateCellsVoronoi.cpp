//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/Tasks/CellFlowLayoutTaskCreateCellsVoronoi.h"


namespace DCEL {
	struct FHalfEdge;
    struct FVertex {
        FVector2D Location{};
    	FHalfEdge* IncidentEdge{};
    };

	struct FFace {
		FHalfEdge* OuterEdge{};
	};
    
	struct FHalfEdge {
		FVertex* Origin{};
		FHalfEdge* Twin{};
		FHalfEdge* Next{};
		FHalfEdge* Prev{};
		FFace* IncidentFace{};	// Face on the left side, when traversed from the origin to the dest (Twin->Origin)
	};

	class FGraph {
		
	};
}

void UCellFlowLayoutTaskCreateCellsVoronoi::GenerateCellsImpl(UDAFlowCellGraph* InCellGraph, const FRandomStream& InRandom) {

}
